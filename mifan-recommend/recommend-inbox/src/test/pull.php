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
$count = $argv[$argc - 1];
echo $count;

$protobuf = new \Mifan\pullReq();
$protobuf->setCount($count);

$seri_buf = $protobuf->serializeToString();

$rqst = pack('LLSLL', 18 + strlen($seri_buf), 0, 1, 0, $uid).$seri_buf;

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
if ($rv['code'] != 0) {
    echo "can not recv data rto server";
    return -1;
}

$resp_body = substr($resp, 18);
$feed_list = new \Mifan\pullRes();
$feed_list->ParseFromString($resp_body);
var_dump($feed_list);
for ($i = 0; $i != $feed_list->getFeedidCount(); ++$i) {
    echo $feed_list->getFeedidAt($i);
}

socket_close($sock);
?>
