<?php
//require_once('./lib/netclient_class.php');
global $g_mysql_mifan_client;
global $g_mysql_basic_client;
global $g_redis_cache_client;

function get_article_like($article) {
    global $g_mysql_basic_client;
    global $g_score_conf;
    $rv = _init_mysql_connect($g_mysql_basic_client, $g_score_conf['backend']['mysql_basic_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_basic_server'], true), 'error');
        return null;
    }

    //查看帖子的点赞数
    $query_sql = 'select count(*) from '.$g_score_conf['backend']['mysql_basic_server']['like_table'].' where article_id='.$article;
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');
    $rv = mysqli_query($g_mysql_basic_client, $query_sql);
    if ($rv->num_rows == 0) {
        log::write('['.__LINE__.'] USER<'.$author.'> has no liker', 'debug');
        return 0;
    }
    $row = mysqli_fetch_assoc($rv);
    return (int)$row['count(*)'];
}

function get_article_comment($article) {
    global $g_mysql_basic_client;
    global $g_score_conf;
    $rv = _init_mysql_connect($g_mysql_basic_client, $g_score_conf['backend']['mysql_basic_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_basic_server'], true), 'error');
        return null;
    }

    //查看帖子的评论数
    $query_sql = 'select count(*) from '.$g_score_conf['backend']['mysql_basic_server']['comment_table'].' where article_id='.$article;
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');
    $rv = mysqli_query($g_mysql_basic_client, $query_sql);
    if ($rv->num_rows == 0) {
        log::write('['.__LINE__.'] USER<'.$author.'> has no comment', 'debug');
        return 0;
    }
    $row = mysqli_fetch_assoc($rv);
    return (int)$row['count(*)'];
}

function get_article_feedid($article, $author, &$time) {
    global $g_mysql_basic_client;
    global $g_score_conf;
    
    $rv = _init_mysql_connect($g_mysql_basic_client, $g_score_conf['backend']['mysql_basic_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_basic_server'], true), 'error');
        return null;
    }

    $query_sql = 'select create_time from '.$g_score_conf['backend']['mysql_basic_server']['article_table'].' where id='.$article;
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');
    $rv = mysqli_query($g_mysql_basic_client, $query_sql);
    if ($rv->num_rows == 0) {
        log::write('['.__LINE__.'] USER<'.$author.'> has no liker', 'debug');
        return null;
    }
    $row = mysqli_fetch_assoc($rv);
    $time =  strtotime($row['create_time']);
    
    global $g_mysql_mifan_client;
    $rv = _init_mysql_connect($g_mysql_mifan_client, $g_score_conf['backend']['mysql_mifan_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_mifan_server'], true), 'error');
        return null;
    }
    
    $d_index = (int)($author % 10000 / 100);
    $t_index = $author % 10000 % 100;
    $query_sql = "select cmd_id, app_id, magic from {$g_score_conf['backend']['mysql_mifan_server']['feed_database']}{$d_index}.{$g_score_conf['backend']['mysql_mifan_server']['feed_table']}{$t_index} where user_id={$author} and timestamp=$time";
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');
    $rv = mysqli_query($g_mysql_mifan_client, $query_sql);
    if ($rv->num_rows == 0) {
        log::write('['.__LINE__.'] USER<'.$author.'> has no feed', 'debug');
        return 0;
    }
    $row = mysqli_fetch_assoc($rv);

    $feed_id = base64_encode(pack('LSLLQ', $author, $row['cmd_id'], $row['app_id'], $time, $row['magic']));
    return $feed_id;
}

function init_article_default_score($article, $time, $score, $feedid) {
    global $g_score_conf;
    global $g_mysql_mifan_client;
    $rv = _init_mysql_connect($g_mysql_mifan_client, $g_score_conf['backend']['mysql_mifan_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_mifan_server'], true), 'error');
        return null;
    }
        
    $query_sql = "insert into {$g_score_conf['backend']['mysql_mifan_server']['hot_table']} values ({$article}, {$time}, {$score}, \"{$feedid}\")";
    $rv = mysqli_query($g_mysql_mifan_client, $query_sql);
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'> RET<'.$rv.'>', 'debug');
    return $rv;
}

function get_article_default_score($article, $author) {
    global $g_score_conf;
    $score = 0;
    $info = array();

    $rv = get_article_like($article);
    if (!isset($rv)) 
        return FALSE;
    $score += $rv * $g_score_conf['score']['article'][NEWS_LIKER];
    
    $rv = get_article_comment($article);
    if (!isset($rv)) 
        return FALSE;
    $score += $rv * $g_score_conf['score']['article'][NEWS_COMMENT];

    $info['feed_id'] = get_article_feedid($article, $author, $time);
    if (!$info['feed_id']) 
        return FALSE;
    $info['score'] = $score;
    $info['update'] = false;
    $info['time'] = $time;

    init_article_default_score($article, $time, $score, $info['feed_id']);
    return $info;
}

function get_article_info($article, $author) {
    global $g_mysql_mifan_client;
    global $g_score_conf;
    $rv = _init_mysql_connect($g_mysql_mifan_client, $g_score_conf['backend']['mysql_mifan_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_mifan_server'], true), 'error');
        return FALSE;
    }
        
    $query_sql = "select feed_id, score, timestamp from {$g_score_conf['backend']['mysql_mifan_server']['hot_table']} where  article_id={$article}";
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');
    $rv = mysqli_query($g_mysql_mifan_client, $query_sql);

    if ($rv->num_rows == 0) {
        //根据点赞评论计算旧帖子的热度
        return get_article_default_score($article, $author);
    }

    $info = array();
    while ($row = mysqli_fetch_assoc($rv)) {
        $info['feed_id'] = $row['feed_id'];
        $info['score'] = $row['score'];
        $info['time'] = $row['timestamp'];
        $info['update'] = true;
    }

    return $info;
}

function update_article_info($article, $arr_info) {
    global $g_mysql_mifan_client;
    global $g_score_conf;
    /*
     * a. 先更新mysql中的文章热度，
     * b. 然后更新缓存中的文章热度
     */

    //a.
    $rv = _init_mysql_connect($g_mysql_mifan_client, $g_score_conf['backend']['mysql_mifan_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_mifan_server'], true), 'error');
        return FALSE;
    }
        
    $query_sql = "update {$g_score_conf['backend']['mysql_mifan_server']['hot_table']} set score={$arr_info['score']} where article_id={$article}";
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');
    $rv = mysqli_query($g_mysql_mifan_client, $query_sql);
    if (!$rv) {
        log::write('['.__LINE__.'] SQL<'.$query_sql.'> RET<'.$rv.'>', 'error');
        return null;
    }

    //b.
    global $g_fresh_time;
    if ($arr_info['time'] - $g_fresh_time > SCORE_DAY ) { //对于非当天的文章，不更新文章在缓存池中的记录
        log::write('['.__LINE__.'] ARTICLE<'.$article.'> CREATE<'.$arr_info['time'].'> is not today FRESH<'.$g_fresh_time.'>', 'debug');
        return ;
    }

    global $g_redis_cache_client;
    $rv = _init_redis_connect($g_redis_cache_client, $g_score_conf['backend']['redis_cache_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['redis_cache_server'], true), 'error');
        return null;
    }

    $rv = $g_redis_cache_client->zAdd($g_score_conf['backend']['redis_cache_server']['cache0'], $arr_info['score'], $arr_info['feed_id']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] zAdd '.$g_score_conf['backend']['redis_cache_server']['cache0'].' '.$arr_info['score'].' '.$arr_info['feed_id'], 'error');
        return ;
    }
    return;
}
/**
 * @brief do_article_process  更新帖子热度的接口，主要逻辑如下
 *      1. 首先从热度表中获取帖子当前热度，存在如下两种情况： 
 *          a. 帖子不在热度表中，则根据（基础信息库中）帖子点赞数以及评论数计算帖子初始热度，并获得发帖时间以及feed_id
 *          b. 帖子已经存在，则直接根据帖子的id取出帖子当前热度
 *      2. 将相应行为的热度值进行累加
 *      3. 计算结果落库，并更新时间段缓存库（ZSET feedid hotscore）
 * @param $cmd      用户行为类型
 * @param $article  用户行为对应的帖子
 * @param $author   帖子作者
 *
 * @return 
 */
function do_article_process($cmd, $article, $author) {
    global $g_score_conf;
    global $g_fresh_time;

    $g_fresh_time = get_cache_fresh_time();
    if (!array_key_exists($cmd, $g_score_conf['support'])) {
        log::write('['.__LINE__.'] USER<'.$mimi.'> Invalid cmd<'.$cmd.'>', 'error');
        return;
    }

    //1  (info 中包含feedid以及热度值score, 时间time)
    $info = get_article_info($article, $author);
    if ($info === FALSE) 
        return ;
    else if ($info['update'] === false && $cmd != CLICK_ARTICLE)
        return;

    //2
    $info['score'] += $g_score_conf['score']['article'][$cmd];

    $rv = update_article_info($article, $info);
    return $info;
}


function get_cache_fresh_time() {
    global $g_redis_cache_client;
    global $g_score_conf;
    $rv = _init_redis_connect($g_redis_cache_client, $g_score_conf['backend']['redis_cache_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['redis_cache_server'], true), 'error');
        return null;
    }

    $rv = $g_redis_cache_client->exists($g_score_conf['backend']['redis_cache_server']['cache_timer']);
    if ($rv === FALSE || $rv == 0) {
        log::write('['.__LINE__.'] exists '.$g_score_conf['backend']['redis_cache_server']['cache_timer'].' RET<'.$rv.'>', 'error');
        return null;
    }

    $rv = $g_redis_cache_client->get($g_score_conf['backend']['redis_cache_server']['cache_timer']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] exists '.$g_score_conf['backend']['redis_cache_server']['cache_timer'], 'error');
        return null;
    }

    return (int)$rv;
}

function update_hot_pool($endtime, $starttime, $poolcount, $poolname) {
//    DEBUG && log::write('['.__LINE__.'] POOL<'.$poolname.'> COUNT<'.$poolcount.'> START<'.$starttime.'> END<'.$endtime.'>', 'debug');
    global $g_score_conf;
    global $g_mysql_mifan_client;
    $rv = _init_mysql_connect($g_mysql_mifan_client, $g_score_conf['backend']['mysql_mifan_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_mifan_server'], true), 'error');
        return null;
    }
        
    $query_sql = "select feed_id, score from ".$g_score_conf['backend']['mysql_mifan_server']['hot_table'].
            " where timestamp>=".$starttime.
            " and timestamp<".$endtime.
            " order by score desc limit ".$poolcount;
    //DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'> ', 'debug');
    $rv = mysqli_query($g_mysql_mifan_client, $query_sql);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] SQL<'.$query_sql.'> RET<'.$rv.'>', 'error');
        return null;
    }
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'> RET<'.$rv->num_rows.'>', 'debug');

    $arr_hot = array();
    $arr_hot[] = $poolname;
    while ($row = mysqli_fetch_assoc($rv)) {
        $arr_hot[] = $row['score'];
        $arr_hot[] = $row['feed_id'];
    }
    //DEBUG && log::write('['.__LINE__.'] SET '.print_r($arr_hot, true), 'debug');
    
    $rv = _init_redis_connect($g_redis_cache_client, $g_score_conf['backend']['redis_cache_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['redis_cache_server'], true), 'error');
        return null;
    }
    $rv = $g_redis_cache_client->del($poolname);
    $rv = call_user_func_array(array($g_redis_cache_client, 'zadd'), $arr_hot);
    DEBUG && log::write('['.__LINE__.'] ZADD '.$poolname.' RET<'.$rv.'>', 'debug');
    return $rv;
}

function do_article_cache_process() {
    global $g_fresh_time;

    $g_fresh_time = get_cache_fresh_time();
    $now_ = (int)gettimeofday(true);
    $today_ = $now_ - $now_ % (24 * 3600) - 8 * 3600;
    if ($g_fresh_time != null) {
        if ($today_ == $g_fresh_time) {
//            DEBUG && log::write('['.__LINE__.'] FRESH<'.date('Y-m-d H:i:s', $g_fresh_time).'> today had freshed<'.date('Y-m-d H:i:s', $now_).'>', 'debug');
            return ;
        }
    }
    DEBUG && log::write('['.__LINE__.'] FRESH<'.date('Y-m-d H:i:s', $g_fresh_time).'> today<'.date('Y-m-d H:i:s', $today_).'> need freshed', 'debug');
    $g_fresh_time = $today_;
    
    global $g_score_conf;
    update_hot_pool($g_fresh_time + 24 * 3600, 
            $g_fresh_time, 
            $g_score_conf['backend']['redis_cache_server']['cache0cnt'],
            $g_score_conf['backend']['redis_cache_server']['cache0']);
    update_hot_pool($g_fresh_time, 
            $g_fresh_time - 72 * 3600, 
            $g_score_conf['backend']['redis_cache_server']['cache1cnt'],
            $g_score_conf['backend']['redis_cache_server']['cache1']);
    update_hot_pool($g_fresh_time - 72 * 3600, 
            0,  
            $g_score_conf['backend']['redis_cache_server']['cache2cnt'],
            $g_score_conf['backend']['redis_cache_server']['cache2']);

    global $g_redis_cache_client;
    $rv = _init_redis_connect($g_redis_cache_client, $g_score_conf['backend']['redis_cache_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['redis_cache_server'], true), 'error');
        return null;
    }

    $rv = $g_redis_cache_client->set($g_score_conf['backend']['redis_cache_server']['cache_timer'], $g_fresh_time);
    DEBUG && log::write('['.__LINE__.'] redis <SET '.$g_score_conf['backend']['redis_cache_server']['cache_timer'].' '. $g_fresh_time. '> RET<'.$rv.'>', 'debug');
}
?>
