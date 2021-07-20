<?php
require './Mifan/noteRecommendAlarm.php';
$conf = new RdKafka\Conf();
$conf->setDrMsgCb(function ($kafka, $message) {
            file_put_contents("./kafka_consumer.log", var_export($message, true), FILE_APPEND);
            });
$conf->setErrorCb(function ($kafka, $err, $reason) {
            file_put_contents("./err_consumer.log", sprintf("Kafka error: %s (reason: %s)", rd_kafka_err2str($err), $reason).PHP_EOL, FILE_APPEND);
            });

//设置消费组
$conf->set('group.id', 'myConsumerGroup');

$rk = new RdKafka\Consumer($conf);
$rk->addBrokers("10.1.1.187");

$topicConf = new RdKafka\TopicConf();
$topicConf->set('request.required.acks', 1);
//在interval.ms的时间内自动提交确认、建议不要启动
//$topicConf->set('auto.commit.enable', 1);
$topicConf->set('auto.commit.enable', 0);
$topicConf->set('auto.commit.interval.ms', 100);

// 设置offset的存储为broker
 $topicConf->set('offset.store.method', 'broker');

 $topic = $rk->newTopic("mifan-note-recommend-alarm", $topicConf);

 // 参数1消费分区0
// $topic->consumeStart(0, RD_KAFKA_OFFSET_BEGINNING);
 $topic->consumeStart(0, RD_KAFKA_OFFSET_END); //
 //$topic->consumeStart(0, RD_KAFKA_OFFSET_STORED);

 while (true) {
//参数1表示消费分区，这里是分区0
//参数2表示同步阻塞多久
     $message = $topic->consume(0, 12 * 1000);
     if (is_null($message)) {
         sleep(1);
         echo "No more messages\n";
         continue;
     }
     switch ($message->err) {
         case RD_KAFKA_RESP_ERR_NO_ERROR:
             var_dump($message);
             $info_ = new \Mifan\noteRecommendAlarm();
             var_dump(substr($message->payload, strlen($message->key)));
             //$info_->ParseFromString(substr($message->payload, strlen($message->key)));
             $info_->ParseFromString($message->payload);
             $info_->dump();
             var_dump($message);
             break;
         case RD_KAFKA_RESP_ERR__PARTITION_EOF:
             echo "No more messages; will wait for more\n";
             break;
         case RD_KAFKA_RESP_ERR__TIMED_OUT:
             echo "Timed out\n";
             break;
         default:
             throw new \Exception($message->errstr(), $message->err);
             break;
     }
 }

