#! /usr/bin/php
<?php

require dirname(__FILE__) . DIRECTORY_SEPARATOR . 'config' . DIRECTORY_SEPARATOR .  'setup.inc.php';
require dirname(__FILE__) . DIRECTORY_SEPARATOR . 'Mifan' . DIRECTORY_SEPARATOR .  'noteScore.php';
declare(ticks=1);

$g_pid_arr = array();
$g_pid_file = "./pid_recommend_score";
$g_stop = 0;
global $g_process_id;

function daemon()
{
    $pid = pcntl_fork();
    if ($pid == -1)
    {
        die("could not fork\n");
    }
    else if ($pid)
    {
        exit();
    }
    else
    {
        if (posix_setsid() == -1)
        {
            die("could not detach from terminal\n");
        }
    }
}

function parent_signal_handler($signo)
{
    global $g_pid_arr;
    switch($signo)
    {
        case SIGCHLD:
            break;
        case SIGINT:
        case SIGTERM:
        case SIGQUIT:
            foreach($g_pid_arr as $key => $pid)
            {
                posix_kill($g_pid_arr[$key], SIGUSR1);
            }
            break;
        default:
            break;
    }
}

function child_signal_handler($signo)
{
        global $g_stop;
        switch($signo)
        {
                case SIGPIPE:
                        break;
                case SIGUSR1:
                        $g_stop++;
                        break;
                default:
                        break;
        }
}

function process_control()
{
    global $g_pid_arr;
    global $g_pid_file;
    global $g_process_id;
    daemon();
    $fdlock = fopen("./lock","w+");
    if (!flock($fdlock, LOCK_EX | LOCK_NB))
    {
        die("program have run!!!\n");
    }
    
    $fdpid = fopen($g_pid_file,"wb");                 
    if($fdpid)                                        
    {                                                  
        fwrite($fdpid, posix_getpid());           
        fclose($fdpid);                           
    }                                                  
    else                                               
    {                                                  
        die("file ".$g_pid_file." open failed.\n");
    }                                                   
   
//    DEBUG && log::write("[".__FUNCTION__."]:[".__LINE__."]"."start run"); 

    pcntl_signal(SIGINT, "parent_signal_handler", false); 
    pcntl_signal(SIGTERM, "parent_signal_handler", false); 
    pcntl_signal(SIGQUIT, "parent_signal_handler", false); 
    pcntl_signal(SIGCHLD, "parent_signal_handler", false); 
    
    for($i = 0; $i != PROCESS_NUM; ++$i)    {
        $pid = pcntl_fork();
        if ($pid == -1)        {
            die("fork error\n");
        }        else if ($pid)        {
            $g_pid_arr[$i] = $pid;
        }        else        {
            pcntl_signal(SIGINT, "child_signal_handler", false);
            pcntl_signal(SIGTERM, "child_signal_handler", false);
            pcntl_signal(SIGQUIT, "child_signal_handler", false);
            pcntl_signal(SIGPIPE, "child_signal_handler", false);
            pcntl_signal(SIGUSR1, "child_signal_handler", false);
            $g_process_id = $i;
            exit(children_history_process());
        }
    }
    while(count($g_pid_arr) > 0)    {
        $myId = pcntl_waitpid(-1, $status, 0);
        foreach($g_pid_arr as $key => $pid)         {
            if($myId == $pid)            {
                unset($g_pid_arr[$key]);
                $children_ret = pcntl_wexitstatus($status);
                log::write("[".__FUNCTION__."]:[".__LINE__."]"."return code ".$children_ret);
                if ($children_ret == -1)                {
                    sleep(2);
                    log::write("[".__FUNCTION__."]:[".__LINE__."]"."reboot children");
                    reboot_children_process($key);
                }                else if ($children_ret == 255)                 {
                    sleep(2);
                    log::write("reboot children");
                    reboot_children_process($key);
                }
            }
        }
        //usleep(100);
    }
    flock($fdlock, LOCK_UN);
    fclose($fdlock);

    log::write("[".__FUNCTION__."]:[".__LINE__."]"."program end");
    exit(0);
}

function reboot_children_process($key)
{
        global $g_pid_arr;
        global $g_process_id;
        global $arr_userid;
        $pid = pcntl_fork();
        if ($pid == -1)        {
            log::write("[".__FUNCTION__."]:[".__LINE__."]"."fork error\n","error");
        }        else if ($pid)        {
            $g_pid_arr[$key] = $pid;
        }        else        {
            pcntl_signal(SIGINT, "child_signal_handler", false);
            pcntl_signal(SIGTERM, "child_signal_handler", false);
            pcntl_signal(SIGQUIT, "child_signal_handler", false);
            pcntl_signal(SIGPIPE, "child_signal_handler", false);
            pcntl_signal(SIGUSR1, "child_signal_handler", false);
          
            $g_process_id = $key;
            exit(children_history_process());
        }
}

function children_history_process() {
    global $g_stop;
    global $g_process_id;
//    DEBUG && log::write('['.__FILE__.']:['.__LINE__.']'.' global work id <'.$g_process_id.'>', 'debug');
    
    $kGroupName = HISTORY_GROUP;
    $kTopic = __init_kafka_consumer($kGroupName, HISTORY_TOPIC_NAME, HISTORY_DEBUG, HISTORY_ERROR);
    if (!isset($kTopic)) {
        log::write('Can not connect Kafka Server<'.HISTORY_TOPIC_NAME.'>', 'error');
        return -1;
    }

    while(!$g_stop) {
        //每次循环迭代时检查热点库是否需要刷新（每天00:00一刷）
        do_article_cache_process();

        //参数1表示消费分区，这里是分区0
        //参数2表示同步阻塞多久
        $message = $kTopic->consume(0, TIMEOUT * 1000);
        if (is_null($message)) {
            sleep(1);
//            DEBUG && log::write('['.__FILE__.']['.__LINE__.']:No more messages just block for news notification', 'debug');
            continue;
        }
        switch ($message->err) {
            case RD_KAFKA_RESP_ERR_NO_ERROR:
                $info_ = new \Mifan\noteScore();
                $info_->ParseFromString($message->payload);
                DEBUG && log::write('['.__LINE__.'] Partition<'.$g_process_id.'>:Get message user<'.$info_->getUser().'> cmd<'.$info_->getCmd().'> article<'.$info_->getArticle().'>, author <'.$info_->getAuthor().'>', 'debug');
                do_score_process($info_->getUser(), $info_->getCmd(), $info_->getArticle(), $info_->getAuthor());
                do_article_process($info_->getCmd(), $info_->getArticle(), $info_->getAuthor());
                break;
            case RD_KAFKA_RESP_ERR__PARTITION_EOF:
                DEBUG && log::write('['.__FILE__.']['.__LINE__.']:No more messages; will wait for more', 'debug');
                break;
            case RD_KAFKA_RESP_ERR__TIMED_OUT:
                DEBUG && log::write('['.__FILE__.']['.__LINE__.']:Time out', 'debug');
                break;
            default:
                DEBUG && log::write('['.__FILE__.']['.__LINE__.']:'.$message->err, 'debug');
                throw new \Exception($message->errstr(), $message->err);
                break;
        }
    }

    return 0;
}

function __init_kafka_consumer($group_name, $topic_name, $debug_file, $error_file) {
    global $g_process_id;
    DEBUG && log::write('['.__FILE__.']['.__LINE__.']:kafka<'.MKAFKA_BROKER_IP.'> group<'.$group_name.'> topic<'.$topic_name.'> partition<'.$g_process_id.'> debug<'.$debug_file.'> error<'.$error_file.'>', 'debug');
    $conf = new RdKafka\Conf();
    $conf->setDrMsgCb(function ($kafka, $message) {
            file_put_contents($debug_file, var_export($message, true), FILE_APPEND);
            });
    $conf->setErrorCb(function ($kafka, $err, $reason) {
            file_put_contents($error_file, sprintf("Kafka error: %s (reason: %s)", rd_kafka_err2str($err), $reason).PHP_EOL, FILE_APPEND);
            });

    //设置消费组
    $conf->set('group.id', $group_name);

    $rk = new RdKafka\Consumer($conf);
    $rk->addBrokers(MKAFKA_BROKER_IP);

    $topicConf = new RdKafka\TopicConf();
    $topicConf->set('request.required.acks', 1);
    
    //在interval.ms的时间内自动提交确认、建议不要启动
    //$topicConf->set('auto.commit.enable', 1);
    $topicConf->set('auto.commit.enable', 0);
    $topicConf->set('auto.commit.interval.ms', 100);

    // 设置offset的存储为file
    //$topicConf->set('offset.store.method', 'file');
    // 设置offset的存储为broker
     $topicConf->set('offset.store.method', 'broker');
    //$topicConf->set('offset.store.path', __DIR__);

    $topic = $rk->newTopic($topic_name, $topicConf);

    // 参数1消费分区0
    // RD_KAFKA_OFFSET_BEGINNING 重头开始消费
    // RD_KAFKA_OFFSET_STORED 最后一条消费的offset记录开始消费
    // RD_KAFKA_OFFSET_END 最后一条消费
    // $topic->consumeStart(0, RD_KAFKA_OFFSET_BEGINNING);
    $topic->consumeStart(0, RD_KAFKA_OFFSET_END);

    return $topic;
}

ini_set("display_errors", "0");
ini_set("log_errors", "1");
ini_set("error_log", "log/mysys.log");
ini_set('memory_limit', '1024M');
error_reporting(E_ALL & ~E_NOTICE);
date_default_timezone_set("Asia/Shanghai");

process_control();

?>
