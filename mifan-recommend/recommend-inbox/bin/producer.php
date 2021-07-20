<?php
require 'Mifan/noteRecommendAlarm.php';
//var_dump($argc);
if ($argc != 4) {
    echo "need 4 argc";
}
$kCnf = new RdKafka\Conf();

$kCnf->setDrMsgCb(function ($kafka, $message) {
           file_put_contents('./debug_kafka.log', var_export($message, true).PHP_EOL, FILE_APPEND); 
        });
$kCnf->setErrorCb(function ($kafka, $err, $reason) {
           file_put_contents('./error_kafka.log', sprintf("Kafka error: %s (reason: %s)", rd_kafka_err2str($err), $reason).PHP_EOL, FILE_APPEND); 
        });

$kproducer = new RdKafka\Producer($kCnf);
$kproducer->setLogLevel(LOG_DEBUG);
$kproducer->addBrokers("10.1.1.187");

$kTopicCnf = new RdKafka\TopicConf();
// -1必须等所有brokers同步完成的确认 1当前服务器确认 0不确认，这里如果是0回调里的offset无返回，如果是1和-1会返回offset
// 我们可以利用该机制做消息生产的确认，不过还不是100%，因为有可能会中途kafka服务器挂掉
$kTopicCnf->set('request.required.acks', 0);
$kTopic = $kproducer->newTopic("mifan-note-recommend-alarm", $kTopicCnf);
$option = 'protobuf';

$protobuf_ = new \Mifan\noteRecommendAlarm();
$protobuf_->setUserid($argv[1]);
$protobuf_->setCount($argv[2]);
$protobuf_->setIndex($argv[3]);
$data = $protobuf_->serializeToString();
$kTopic->produce(RD_KAFKA_PARTITION_UA, 0, $data, $option);

$len = $kproducer->getOutQlen();

while ($len > 0) {
    $len = $kproducer->getOutQlen();
//    var_dump($len);
    $kproducer->poll(50);
}
?>
