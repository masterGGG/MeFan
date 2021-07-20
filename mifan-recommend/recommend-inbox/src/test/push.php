#! /usr/bin/php5
<?php
require 'Mifan/pullReq.php';
require 'Mifan/pullRes.php';
require 'Mifan/pushReq.php';
require 'Mifan/pushReq_feedInfo.php';
require 'lib/socket_util.php';
require 'lib/log.class.php';
require 'define.inc.php';
$ip = '10.1.1.197';
$port = 8088;

if (init_connect_and_nonblock($ip, $port, $sock)) {
    echo "can not connect server";
    return 0;
}

$uid = 38743982;
$cmd = 2;
$id_ = 1;
$feedid_prefix = 'summer:';

$protobuf = new \Mifan\pushReq();
$protobuf->setUserid($uid);
$protobuf->setIndex(100);

for ($i = 1; $i != 101; ++$i) {
    $info = new \Mifan\pushReq_feedInfo();
    $info->setFeedid($feedid_prefix.'like:'.$i);
    $info->setScore(mt_rand(1, 200));
    $protobuf->appendInfo($info);
}
for ($i = 1; $i != 26; ++$i) {
    $info = new \Mifan\pushReq_feedInfo();
    $info->setFeedid($feedid_prefix.'hot:'.$i);
    $info->setScore(mt_rand(1, 200));
    $protobuf->appendInfo($info);
}
$seri_buf = $protobuf->serializeToString();

$rqst = pack('LLSLL', 18 + strlen($seri_buf), 0, 2, 0, $uid).$seri_buf;

if (send_data_and_nonblock($sock, $rqst, 5000)) {
    echo "can not send data rto server";
    return -1;
}
    
$resp = "";
if (recv_data_and_nonblock($sock, 18, $resp, 5000)) {
    echo "can not recv data rto server";
    return -1;
}
$rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $resp);
var_dump($rv);
?>
