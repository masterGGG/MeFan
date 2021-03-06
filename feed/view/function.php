<?php
/* vim: set expandtab tabstop=4 softtabstop=4 shiftwidth=4: */
/**
 * @file function.php
 * @author richard <richard@taomee.com>
 * @date 2011-06-10
 */

require_once('config.php');
require_once('netclient_class.php');

function do_log($level, $log)
{
    static $arr_handle = array();

    if (!array_key_exists($level, $arr_handle) || $arr_handle[$level] == null) {
        $log_file = LOG_DIR . LOG_PREFIX . '_' . $level . '_' . date('Y_m_d') . '.log';
        $arr_handle[$level] = fopen($log_file, 'ab');
        if ($arr_handle[$level] === FALSE) {
            return FALSE;
        }
    }

    if (fwrite($arr_handle[$level], 
               '[' . date('Y-m-d H:i:s') . '] ' . print_r($log, true) . "\n") === FALSE) {
        $arr_handle[$level] = null;
        return FALSE;
    }

    return TRUE;
}

function feedid_to_binary($feedid)
{
    return pack('LSLL', $feedid['user_id'], $feedid['cmd_id'], $feedid['app_id'], $feedid['timestamp']) . base64_decode($feedid['magic']);
}

function binary_to_feedid($binary)
{
    $feedid = unpack("Luser_id/Scmd_id/Lapp_id/Ltimestamp", $binary);
    $feedid['magic'] = base64_encode(substr($binary, 14, 8));
    return $feedid;
}

function passive_feedid_to_binary($feedid)
{
    return pack('LSL2', $feedid['user_id'], $feedid['cmd_id'], $feedid['app_id'], $feedid['timestamp']) . base64_decode($feedid['magic']) . pack('L3',$feedid['sender_uid'] , $feedid['target_uid'] , $feedid['passive_magic']);
}

function binary_to_passive_feed_index($binary)
{
    $feedid = unpack("Luser_id/Scmd_id/Lapp_id/Ltimestamp/Lm1/Lm2/Lsender_uid/Ltarget_uid/Lpassive_magic/Lupdate_timestamp", $binary);
    unset($feedid['m1']);
    unset($feedid['m2']);
    $feedid['magic'] = base64_encode(substr($binary, 14, 8));
    return $feedid;
}

function get_passive_newsfeed_from_storageDB($uid, $timestamp, $cmd_id, $app_id, $count, &$arr_feeds) {
    $arr_storage_addr = explode(':', constant('STORAGE_ADDR'));
    $storage_client = new netclient($arr_storage_addr[0], $arr_storage_addr[1]); 
    if ($storage_client->open_conn(1) === FALSE) {
        do_log('error', 'ERROR: storage_client->open_conn');
        return FALSE;
    }

    $storage_rqst_len = 4 + 2 + 2 + 2 + 4 + 2 + 4 + 4 + 4;
    $storage_rqst = pack('LS3LSL3', $storage_rqst_len, 25, 1, 0x7, $uid, $cmd_id, $app_id, $timestamp, $count);
    
    if (($storage_resp = $storage_client->send_rqst($storage_rqst, TIMEOUT)) === FALSE) {
        do_log('error', __FUNCTION__ . 'ERROR: storage_client->send_rqst');
        return FALSE;
    }

    if ($storage_client->close_conn() === FALSE) {
        do_log('error', 'ERROR: storage_client->close_conn');
        return FALSE;
    }

    $rv_0 = unpack('Llen/Sret/Sunits', $storage_resp);
    if ($rv_0['ret'] != 0) {
        do_log('error', __FUNCTION__ ."----".__LINE__ ."ERROR: len: {$rv_0['len']} result: {$rv_0['ret']}");
        return FALSE;
    }
    DEBUG_V0716 && do_log('debug', __FUNCTION__ ."----".__LINE__ ."ERROR: len: {$rv_0['len']} result: {$rv_0['ret']}");

    $storage_resp = substr($storage_resp, 4 + 2 + 2);
    
    for ($i = 0; $i != $rv_0['units']; ++$i) {
        $row = unpack("Llen/Luser_id/Scmd_id/Lapp_id/Ltimestamp/L2magic/Lsender_uid/Ltarget_uid/Lpassive_magic/Lupdate_timestamp", $storage_resp);

        $row['data'] = json_decode(substr($storage_resp, 42, $row['len'] - 42), true);
        $storage_resp = substr($storage_resp, $row['len']);
        unset($row['len']);
        $arr_feeds[] = $row;
    }

    return count($arr_feeds);
}

function get_pass_feedid_from_stor($uid, $count, $timestamp) {
    $arr_feedid = array();
    $arr_storage_addr = explode(':', constant('STORAGE_ADDR'));
    $storage_client = new netclient($arr_storage_addr[0], $arr_storage_addr[1]); 
    if ($storage_client->open_conn(1) === FALSE) {
        do_log('error', 'ERROR: storage_client->open_conn');
        return FALSE;
    }

    $storage_rqst_len = 4 + 2 + 2 + 2 + 4 + 2 + 4 + 4 + 4;
    $storage_rqst = pack('LS3LSL3', $storage_rqst_len, 25, 1, 0x0, $uid, 0, 1, $timestamp, $count);
    
    if (($storage_resp = $storage_client->send_rqst($storage_rqst, TIMEOUT)) === FALSE) {
        do_log('error', __FUNCTION__ . 'ERROR: storage_client->send_rqst');
        return FALSE;
    }

    $rv_0 = unpack('Llen/Sret/Sunits', $storage_resp);
    if ($rv_0['ret'] != 0) {
        do_log('error', __FUNCTION__ ."----".__LINE__ ."ERROR: len: {$rv_0['len']} result: {$rv_0['ret']}");
        return FALSE;
    }
    DEBUG_V0709 && do_log('debug', __FUNCTION__ ."----".__LINE__ ."ERROR: len: {$rv_0['len']} result: {$rv_0['ret']}");

    $storage_resp = substr($storage_resp, 4 + 2 + 2);
    
    for ($i = 0; $i != $rv_0['units']; ++$i) {
        $row = unpack("Llen/Luser_id/Scmd_id/Lapp_id/Ltimestamp/L2magic/Lsender_uid/Ltarget_uid/Lpassive_magic/Lupdate_timestamp", $storage_resp);

        $row['magic'] = base64_encode(substr($storage_resp, 18, 8));
        $arr_feedid[] = $row;
        $storage_resp = substr($storage_resp, 42);
    }

    if ($storage_client->close_conn() === FALSE) {
        do_log('error', 'ERROR: storage_client->close_conn');
        return FALSE;
    }

    return $arr_feedid;
}
function get_passive_feedid($uid)
{
    $arr_feedid = array();

    $arr_outbox_addr = explode(':', constant('OUTBOX_ADDR'));
    $outbox_client = new netclient($arr_outbox_addr[0], $arr_outbox_addr[1]); 
    if ($outbox_client->open_conn(1) === FALSE) {
        do_log('error', 'get_feedid: outbox_client->open_conn');
        return FALSE;
    }

    $outbox_rqst_len = 4 + 2;
    $outbox_rqst_body = '';
    $outbox_rqst_body .= pack('L', $uid);
    $outbox_rqst_len += 4;

        $outbox_rqst = pack('LS', $outbox_rqst_len, 0xFFF8) . $outbox_rqst_body;
        $outbox_resp = FALSE;
        if (($outbox_resp = $outbox_client->send_rqst($outbox_rqst, TIMEOUT)) === FALSE) {
            do_log('error', 'get_feedid: outbox_client->send_rqst');
            return FALSE;
        }

    if ($outbox_client->close_conn() === FALSE) {
        do_log('error', 'get_feedid: outbox_client->close_conn');
        return FALSE;
    }

        $rv = unpack('Llen/Sresult', $outbox_resp);
        if ($rv['result'] != 0) {
            do_log('error', "get_feedid: len: {$rv['len']} result: {$rv['result']}");
            return FALSE;
        }
        
        $feedid_count = ($rv['len'] - 6) / 38;
        for ($j = 0; $j != $feedid_count; ++$j) {
            $binary = substr($outbox_resp, 6 + $j * 38, 38);
            $feedid = binary_to_passive_feed_index($binary);
            $arr_feedid[] = $feedid;
        }


    return $arr_feedid;
}

//TODO ?????????????????????????????????????????????
function reset_statistic_pfeed($uid, $cmd_id) {
    //??????
    $stat_serv = explode(':', constant('TAG_CACHE_ADDR'));
    $stat_client = new netclient($stat_serv[0], $stat_serv[1]);
    if ($stat_client->open_conn(1) === FALSE) {
        do_log('error', 'Connnect to statistic server failed'.print_r($stat_serv, true));
        return FALSE;
    }
        
    $protobuf = new \Mifan\pCommonStat();
    if ($cmd_id !== '')
        $protobuf->appendCmd($cmd_id);
    $body = $protobuf->serializeToString();
            
    $rqst = pack('L2SL2', 18 + strlen($body), 0, 0xA203, 0, $uid).$body;
    $resp = FALSE;
    if (($resp = $stat_client->send_rqst($rqst, TIMEOUT)) == FALSE) {
        do_log('error', 'Send request to statistic server failed'.print_r($stat_serv, true));
        return FALSE;
    }

    if ($stat_client->close_conn() === FALSE) {
        do_log('error', 'Statictis close_conn failed');
        return FALSE;
    }

    $result = unpack('Llen/Lseq/Scmd_id/Lresult/Lmid', $resp);
    if ($result['result'] != 0) {
        do_log('error', 'Statistic server query failed'.print_r($result, true));
        return FALSE;
    }

    return TRUE;
}

function get_feedid_by_tags($mid, $tag, $begin_time, $end_time, $cnt)
{
    $arr_feedid = array();
    $arr_tag_cache_addr = explode(':', constant('TAG_CACHE_ADDR'));
    $outbox_client = new netclient($arr_tag_cache_addr[0], $arr_tag_cache_addr[1]); 
    if ($outbox_client->open_conn(1) === FALSE) {
        do_log('error', 'get_feedid: outbox_client->open_conn');
        return FALSE;
    }
    $rqst = pack("LLSLL4L", 18+16, 0, 0xA104, 0, $mid, $tag, $begin_time, $end_time, $cnt);
    
    if (($outbox_resp = $outbox_client->send_rqst($rqst, TIMEOUT)) === FALSE) {
            do_log('error', 'get_feedid: tag_server_client->send_rqst');
            return FALSE;
    }
    $rv = unpack('Llen/Lseq/Scmd/Lresult/Luser_id', $outbox_resp);
    if ($rv["result"] != 0) {
            do_log('error', 'get_feedid: tag_server_client->send_rqst code :'.$rv["result"]);
            return FALSE;
    }
    $feedid_count = unpack('Lcnt', substr($outbox_resp, 18));
    $pos = 22;
    for ($j = 0; $j != $feedid_count['cnt']; ++$j) {
        $detail = unpack('Llen', substr($outbox_resp, $pos));
        $pos += 4;
        $binary = base64_decode(substr($outbox_resp, $pos, $detail['len']));
        $pos += $detail['len'];
        $feedid = binary_to_feedid($binary);
        $arr_feedid[] = $feedid;
    }
    if ($outbox_client->close_conn() === FALSE) {
        do_log('error', 'get_feedid: outbox_client->close_conn');
        return FALSE;
    }

    return $arr_feedid;
}

function get_feedid(&$arr_uid, $is_latest)
{
    $arr_feedid = array();

    $arr_outbox_addr = explode(':', constant('OUTBOX_ADDR'));
    $outbox_client = new netclient($arr_outbox_addr[0], $arr_outbox_addr[1]); 
    if ($outbox_client->open_conn(1) === FALSE) {
        do_log('error', 'get_feedid: outbox_client->open_conn');
        return FALSE;
    }

    if ($is_latest) {
        if (get_update_mid($arr_uid) === FALSE)
            return FALSE;
    } else if (get_friend_id($arr_uid) === FALSE) {
        return FALSE;
    }
    $uid_count = count($arr_uid);
    for ($i = 0; $i < $uid_count; $i += MAX_OUTBOX_REQUEST_COUNT) {
        $arr_slice_uid = array_slice($arr_uid, $i, MAX_OUTBOX_REQUEST_COUNT);

        $outbox_rqst_len = 4 + 2;
        $outbox_rqst_body = '';
        foreach ($arr_slice_uid as $uid) {
            $outbox_rqst_body .= pack('L', $uid);
            $outbox_rqst_len += 4;
        }

        $outbox_rqst = pack('LS', $outbox_rqst_len, OUTBOX_OPCODE) . $outbox_rqst_body;
        $outbox_resp = FALSE;
        if (($outbox_resp = $outbox_client->send_rqst($outbox_rqst, TIMEOUT)) === FALSE) {
            do_log('error', 'get_feedid: outbox_client->send_rqst');
            return FALSE;
        }

        $rv = unpack('Llen/Sresult', $outbox_resp);
        if ($rv['result'] != 0) {
            do_log('error', "get_feedid: len: {$rv['len']} result: {$rv['result']}");
            return FALSE;
        }

        $feedid_count = ($rv['len'] - OUTBOX_RESPONSE_HEAD_LEN) / FEEDID_LEN;
        for ($j = 0; $j != $feedid_count; ++$j) {
            $binary = substr($outbox_resp, OUTBOX_RESPONSE_HEAD_LEN + $j * FEEDID_LEN, FEEDID_LEN);
            $feedid = binary_to_feedid($binary);
            $arr_feedid[] = $feedid;
        }
    }

    if ($outbox_client->close_conn() === FALSE) {
        do_log('error', 'get_feedid: outbox_client->close_conn');
        return FALSE;
    }

    return $arr_feedid;
}

function feedid_cmp($feedid_1, $feedid_2)
{
    if ($feedid_1['timestamp'] == $feedid_2['timestamp']) {
        return 0;
    }

    return ($feedid_1['timestamp'] < $feedid_2['timestamp']) ? 1 : -1;
}

function passive_feedid_cmp($feedid_1, $feedid_2)
{
    if ($feedid_1['update_timestamp'] == $feedid_2['update_timestamp']) {
        return 0;
    }

    return ($feedid_1['update_timestamp'] < $feedid_2['update_timestamp']) ? 1 : -1;
}

$g_arr_app_id;
$g_arr_cmd_id;

function feedid_filter($feedid) 
{
    global $g_arr_app_id;
    global $g_arr_cmd_id;
    
    if ($g_arr_app_id !== array()) {
        if (FALSE == in_array($feedid['app_id'], $g_arr_app_id))
        {
            return FALSE;
        }
    }
    if ($g_arr_cmd_id !== array()) {
        if (FALSE == in_array($feedid['cmd_id'], $g_arr_cmd_id))
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

function get_passive_feeds($arr_feedid)
{
    $arr_feeds = array();
    
    $arr_storage_addr = explode(':', constant('STORAGE_ADDR'));
    $storage_client = new netclient($arr_storage_addr[0], $arr_storage_addr[1]); 
    if ($storage_client->open_conn(1) === FALSE) {
        do_log('error', 'ERROR: storage_client->open_conn');
        return FALSE;
    }

    $storage_rqst_len = 4 + 2 + 2;
    $storage_rqst_body = '';
    foreach ($arr_feedid as $feedid) {
        $storage_rqst_body .= passive_feedid_to_binary($feedid);
        $storage_rqst_len += 34;
    }

    $storage_rqst = pack('LSS', $storage_rqst_len, 26, count($arr_feedid)) . $storage_rqst_body;
    $storage_resp = FALSE;
    if (($storage_resp = $storage_client->send_rqst($storage_rqst, TIMEOUT)) === FALSE) {
        do_log('error', __FUNCTION__ . 'ERROR: storage_client->send_rqst');
        return FALSE;
    }


    $rv_0 = unpack('Llen/Sret/Sunits', $storage_resp);
    if ($rv_0['ret'] != 0) {
        do_log('error', __FUNCTION__ ."----".__LINE__ ."ERROR: len: {$rv_0['len']} result: {$rv_0['ret']}");
        return FALSE;
    }
    $storage_resp = substr($storage_resp, 4 + 2 + 2);
    
    for ($i = 0; $i != $rv_0['units']; ++$i) {
        $rv_1 = unpack('Llen', $storage_resp);

        $feed = binary_to_passive_feed_index(substr($storage_resp, 4 + 8));

        $temp = unpack('Lid1/Lid2', substr($storage_resp, 4)); 
        $feed['id1'] = $temp['id1'];
        $feed['id2'] = $temp['id2'];

        $feed_data = substr($storage_resp, 4 + 38 + 8, $rv_1['len'] - (4 + 38 + 8));
        $feed['data'] = json_decode($feed_data, TRUE);
        $arr_feeds[] = $feed;
        $storage_resp = substr($storage_resp, $rv_1['len']);
    }

    if ($storage_client->close_conn() === FALSE) {
        do_log('error', 'ERROR: storage_client->close_conn');
        return FALSE;
    }

    return $arr_feeds;
}

function get_feeds($arr_feedid)
{
    $arr_feeds = array();
    
    $arr_storage_addr = explode(':', constant('STORAGE_ADDR'));
    $storage_client = new netclient($arr_storage_addr[0], $arr_storage_addr[1]); 
    if ($storage_client->open_conn(1) === FALSE) {
        do_log('error', 'ERROR: storage_client->open_conn');
        return FALSE;
    }

    $storage_rqst_len = 4 + 2 + 2;
    $storage_rqst_body = '';
    foreach ($arr_feedid as $feedid) {
        $storage_rqst_body .= feedid_to_binary($feedid);
        $storage_rqst_len += FEEDID_LEN;
    }

    $storage_rqst = pack('LSS', $storage_rqst_len, 14, count($arr_feedid)) . $storage_rqst_body;
    $storage_resp = FALSE;
    if (($storage_resp = $storage_client->send_rqst($storage_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'ERROR: storage_client->send_rqst');
        return FALSE;
    }

    $rv_0 = unpack('Llen/Sret/Sunits', $storage_resp);
    if ($rv_0['ret'] != 0) {
        do_log('error', __FUNCTION__ . "ERROR: len: {$rv_0['len']} result: {$rv_0['ret']}");
        return FALSE;
    }
    $storage_resp = substr($storage_resp, 4 + 2 + 2);

    for ($i = 0; $i != $rv_0['units']; ++$i) {
        $rv_1 = unpack('Llen', $storage_resp);
        $feed = binary_to_feedid(substr($storage_resp, 4 + 8));
        $temp = unpack('Lid1/Lid2', substr($storage_resp, 4)); 
        $feed['id1'] = $temp['id1'];
        $feed['id2'] = $temp['id2'];
        $feed_data = substr($storage_resp, 4 + FEEDID_LEN + 8, $rv_1['len'] - (4 + FEEDID_LEN + 8));
        $feed['data'] = json_decode($feed_data, TRUE);
        $arr_feeds[] = $feed;
        $storage_resp = substr($storage_resp, $rv_1['len']);
    }

    if ($storage_client->close_conn() === FALSE) {
        do_log('error', 'ERROR: storage_client->close_conn');
        return FALSE;
    }

    return $arr_feeds;
}

function del_passive_feed($uid, $app_id, $cmd_id, $timestamp, $magic, $sender_uid, $target_uid, $passive_magic)
{
    $feedid = array('user_id' => $uid,
                    'app_id' => $app_id,
                    'cmd_id' => $cmd_id,
                    'timestamp' => $timestamp,
                    'magic' => $magic,
                    'sender_uid' => $sender_uid,
                    'target_uid' => $target_uid,
                    'passive_magic' => $passive_magic);

    $arr_storage_addr = explode(':', constant('STORAGE_ADDR'));
    $storage_client = new netclient($arr_storage_addr[0], $arr_storage_addr[1]); 
    if ($storage_client->open_conn(1) === FALSE) {
        do_log('error', 'del_feed: storage_client->open_conn');
        return FALSE;
    }

    $storage_rqst_len = 4 + 2 + 2;
    $storage_rqst_body = passive_feedid_to_binary($feedid);
    $storage_rqst_len += strlen($storage_rqst_body);

    $storage_rqst = pack('LSS', $storage_rqst_len, 23, 1) . $storage_rqst_body;
    $storage_resp = FALSE;
    if (($storage_resp = $storage_client->send_rqst($storage_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'del_feed: storage_client->send_rqst');
        return FALSE;
    }

    $rv_0 = unpack('Llen/Sret/Sunits', $storage_resp);
    if ($rv_0['ret'] != 0) {
        do_log('error', "del_feed: len: {$rv_0['len']} result: {$rv_0['ret']}");
        return FALSE;
    }

    return TRUE;
}

function del_feed($uid, $app_id, $cmd_id, $timestamp, $magic)
{
    $feedid = array('user_id' => $uid,
                    'app_id' => $app_id,
                    'cmd_id' => $cmd_id,
                    'timestamp' => $timestamp,
                    'magic' => $magic);

    $arr_storage_addr = explode(':', constant('STORAGE_ADDR'));
    $storage_client = new netclient($arr_storage_addr[0], $arr_storage_addr[1]); 
    if ($storage_client->open_conn(1) === FALSE) {
        do_log('error', 'del_feed: storage_client->open_conn');
        return FALSE;
    }

    $storage_rqst_len = 4 + 2 + 2;
    $storage_rqst_body = feedid_to_binary($feedid);
    $storage_rqst_len += strlen($storage_rqst_body);

    $storage_rqst = pack('LSS', $storage_rqst_len, 4, 1) . $storage_rqst_body;
    $storage_resp = FALSE;
    if (($storage_resp = $storage_client->send_rqst($storage_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'del_feed: storage_client->send_rqst');
        return FALSE;
    }

    $rv_0 = unpack('Llen/Sret/Sunits', $storage_resp);
    if ($rv_0['ret'] != 0) {
        do_log('error', "del_feed: len: {$rv_0['len']} result: {$rv_0['ret']}");
        return FALSE;
    }

    return TRUE;
}

function arr_cmp($arr1, $arr2)
{
    @sort($arr1);
    @sort($arr2);
    if ($arr1 == $arr2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

function get_timestamp($uid, $type)
{
    if ($type == 'haoyou')
    {
        $arr_time_addr = explode(':', constant('ACTIVE_TIME_ADDR'));
    }
    else if ($type = 'yuwo')
    {
        $arr_time_addr = explode(':', constant('PASSIVE_TIME_ADDR'));
    }
    else
    {
        return NULL;
    }


    $time_client = new netclient($arr_time_addr[0], $arr_time_addr[1]); 
    if ($time_client->open_conn(1) === FALSE) {
        do_log('error', 'del_feed: time_client->open_conn');
        return NULL;
    }

    $time_rqst = pack('L2SL2', 18, 0, 0x2001, 0, $uid);
    $time_resp = FALSE;
    if (($time_resp = $time_client->send_rqst($time_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'del_feed: time_client->send_rqst');
        return NULL;
    }

    $rv_0 = unpack('Llen/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $time_resp);
    if ($rv_0['status_code'] != 0) {
        do_log('error', __FUNCTION__ . "len: {$rv_0['len']} result: {$rv_0['status_code']}");
        return NULL;
    }
    $rv_0 = unpack('Llen/Lseq_num/Scmd_id/Lstatus_code/Luser_id/Ltimestamp', $time_resp);

    return $rv_0['timestamp'];
}

function set_timestamp($uid, $time, $type)
{
    if ($type == 'haoyou')
    {
        $arr_time_addr = explode(':', constant('ACTIVE_TIME_ADDR'));
    }
    else if ($type = 'yuwo')
    {
        $arr_time_addr = explode(':', constant('PASSIVE_TIME_ADDR'));
    }
    else
    {
        return false;
    }

    $time_client = new netclient($arr_time_addr[0], $arr_time_addr[1]); 
    if ($time_client->open_conn(1) === FALSE) {
        do_log('error', 'del_feed: time_client->open_conn');
        return false;
    }

    $time_rqst = pack('L2SL2L', 22, 0, 0x2002, 0, $uid, $time);
    $time_resp = FALSE;
    if (($time_resp = $time_client->send_rqst($time_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'del_feed: time_client->send_rqst');
        return false;
    }

    $rv_0 = unpack('Llen/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $time_resp);
    if ($rv_0['status_code'] != 0) {
        do_log('error', "del_feed: len: {$rv_0['len']} result: {$rv_0['status_code']}");
        return false;
    }

    return true;
}

require_once('Mifan/pQueryCnt.php');
require_once('Mifan/pQueryCntRes.php');
function get_pfeed_stat($uid, $type, $cmd_id) {
    $rv = array();
    $rv['num'] = 0;
    $stat_serv = explode(':', constant('TAG_CACHE_ADDR'));
    $stat_client = new netclient($stat_serv[0], $stat_serv[1]);
    if ($stat_client->open_conn(1) === FALSE) {
        do_log('error', 'Connnect to statistic server failed'.print_r($stat_serv, true));
        return FALSE;
    }

    $protobuf = new \Mifan\pQueryCnt();
    $protobuf->setMimi($uid);
    $protobuf->appendCmd($cmd_id + 15000);
            
    $body = $protobuf->serializeToString();
    $rqst = pack('L2SL2', 18 + strlen($body), 0, 0xA204, 0, $uid).$body;
    $resp = FALSE;
    if (($resp = $stat_client->send_rqst($rqst, TIMEOUT)) == FALSE) {
        do_log('error', 'Send request to statistic server failed'.print_r($stat_serv, true));
        return FALSE;
    }

    if ($stat_client->close_conn() === FALSE) {
        do_log('error', 'Statictis close_conn failed');
        return FALSE;
    }
    
    $result = unpack('Llen/Lseq/Scmd_id/Lresult/Lmid', $resp);
    if ($result['result'] != 0) {
        do_log('error', 'Statistic server query failed'.print_r($result, true));
        return FALSE;
    }
    
    $resp_protobuf = new \Mifan\pQueryCntRes();
    $resp_protobuf->ParseFromString(substr($resp, 18));
    for ($i = 0; $i != $resp_protobuf->getCntCount(); ++$i) {
        $rv['num'] = $resp_protobuf->getCntAt($i);
        DEBUG_V0716 && do_log('debug', '['.__LINE__.'] Cnt is set:'.$rv['num']);
    }
    return json_encode($rv);
}
require_once('Mifan/pCommonStat.php');
require_once('Mifan/pQueryStat.php');
require_once('Mifan/pQueryStat_detail.php');
function get_notice($arr_uid, $item, $arr_cmd)
{
    $type = explode(',',$item);
    foreach($type as $val)
    {
        if ($val == 'active')
        {
            $arr_feedid = get_feedid($arr_uid, FALSE);
            if ($arr_feedid === FALSE) {
                do_log('error', 'get_newsfeed: get_feedid');
                return FALSE;
            }

            $current_time = get_timestamp($arr_uid[0], 'haoyou');      
            if ($current_time === NULL)
            {
                return false;
            }

             // ???feedid????????????
            usort($arr_feedid, 'feedid_cmp');

            $uid = array();
            $num = 0;
            foreach($arr_feedid as $val)
            {
                if ($val['timestamp'] > $current_time)
                {
                    if (!in_array($val['user_id'],$uid))
                    {
                        $uid[] = $val['user_id'];
                    }
                    $num++;
                    
                }   
            }
            $rv['haoyou_uid'] = $uid;
            $rv['haoyou'] = $num;
            $rv['haoyou_timestamp'] = $current_time;
        }
        else if ($val == 'passive')
        {
            $stat_serv = explode(':', constant('TAG_CACHE_ADDR'));
            $stat_client = new netclient($stat_serv[0], $stat_serv[1]);
            if ($stat_client->open_conn(1) === FALSE) {
                do_log('error', 'Connnect to statistic server failed'.print_r($stat_serv, true));
                return FALSE;
            }
            $rv['num'] = 0;
            $protobuf = new \Mifan\pCommonStat();
            foreach ($arr_cmd as $cmd_id) {
                $protobuf->appendCmd($cmd_id);
                $rv[$cmd_id] = 0;
                $rv[$cmd_id.'_uid'] = array();
            }

            $body = $protobuf->serializeToString();
            $rqst = pack('L2SL2', 18 + strlen($body), 0, 0xA202, 0, $arr_uid[0]).$body;

            $resp = FALSE;
            if (($resp = $stat_client->send_rqst($rqst, TIMEOUT)) == FALSE) {
                do_log('error', 'Send request to statistic server failed'.print_r($stat_serv, true));
                return FALSE;
            }

            if ($stat_client->close_conn() === FALSE) {
                do_log('error', 'Statictis close_conn failed');
                return FALSE;
            }

            $result = unpack('Llen/Lseq/Scmd_id/Lresult/Lmid', $resp);
            if ($result['result'] != 0) {
                do_log('error', 'Statistic server query failed'.print_r($result, true));
                return FALSE;
            }

            $resp_protobuf = new \Mifan\pQueryStat();
            $resp_protobuf->ParseFromString(substr($resp, 18));
            $rv['num'] = $resp_protobuf->getCnt();
            for ($i = 0; $i != $resp_protobuf->getListCount(); ++$i) {
                $detail = $resp_protobuf->getListAt($i);
                $rv[$detail->getCmd()] = $detail->getCnt();
                for ($j = 0; $j != $detail->getMimiCount(); ++$j) {
                    $rv[$detail->getCmd().'_uid'][] = $detail->getMimiAt($j);
                }
            }
        }
    }

    if (isset($rv))
    {
        return json_encode($rv);
    }
    else
    {
        return false;
    }
}
        
function get_passive_newsfeed($uid, $offset, $count, $timestamp, $cmd_id)
{
    $have_next = 0;
    $next_offset = 0;
    if ($cmd_id != '') {
        DEBUG_V0716 && do_log('debug', '['.__LINE__.'] CMD is set:'.$cmd_id.' timestamp:'.$timestamp.' count:'.$count);
        $app_id = 1;
        $cmd_id += 15000;
        $arr_feeds = array();
        $feed_cnt = get_passive_newsfeed_from_storageDB($uid, $timestamp, $cmd_id, $app_id, $count, $arr_feeds);
        if ($feed_cnt == $count) {
            $have_next = 1;
            $next_offset = $offset == -1 ? $count : $offset + $count;
        }
       
        if ($offset == -1)        
            reset_statistic_pfeed($uid, $cmd_id); 
        $arr_result = array();
        $arr_result['current_page'] = $arr_feeds;
        $arr_result['have_next'] = $have_next;
        $arr_result['next_offset'] = $next_offset;
        return json_encode($arr_result);
    }

    if ($offset < MAX_OUTBOX_PASS_CNT) {                        // ???outbox-server???????????????feedid
        $arr_feedid = get_passive_feedid($uid);
        if ($arr_feedid === FALSE) {
            do_log('error', 'get_newsfeed: get_feedid');
            return FALSE;
        }

        $cnt_feedid = count($arr_feedid);
        if ($cnt_feedid == 0) {
            $arr_result = array();
            $arr_result['current_page'] = array();
            $arr_result['have_next'] = 0;
            $arr_result['next_offset'] = $offset + $count;
            return json_encode($arr_result);
        } 
        //??????????????????????????????
        $up_time = gettimeofday(true);

        if ($offset == 0) {
            $ncount = 0;
            $last_time = get_timestamp($uid, 'yuwo');      
            if ($last_time === NULL) 
                $last_time = $up_time; 
            foreach($arr_feedid as $val) {
                if ($val['timestamp'] <= $last_time) 
                    break;
                $ncount++;
            }
            $arr_feedid = array_slice($arr_feedid, $offset, $ncount);
            DEBUG_V0716 && do_log('debug', "lastTime:".$last_time." eraase count:".$ncount." reset count:".count($arr_feedid));
        } else {
            if ($cnt_feedid ==MAX_OUTBOX_PASS_CNT) {
                $have_next = 1;
                if ($offset == -1)
                    $next_offset = $count;
                else
                    $next_offset = $offset + $count;
            } else {
                if ($offset < $cnt_feedid) {
                    $have_next = 1;
                    if ($offset == -1)
                        $next_offset = $cnt_feedid > $count ? $count : $cnt_feedid;
                    else
                        $next_offset = $cnt_feedid > $count + $offset ? $count + $offset : $cnt_feedid;
                }
            }
        }
        DEBUG_V0716 && do_log('debug', '['.__LINE__."] Outbox CNT:<".$cnt_feedid."> have next:<".$have_next." >reset count:<".$next_offset.'>');

        usort($arr_feedid, 'passive_feedid_cmp');
        // ???feedid????????????
        $total_count = count($arr_feedid);
        if ($offset != -1) { 
            if ($count !== 0) 
                $arr_feedid = array_slice($arr_feedid, $offset, $count);
        } else {
            reset_statistic_pfeed($uid, 0); 
            DEBUG_V0716 && do_log('debug', "Query pass feed:mimi<".$uid.'>');
            $arr_feedid = array_slice($arr_feedid, 0, $count);
        }
        if ($offset <= 0) 
            set_timestamp($uid, $up_time, 'yuwo');
    } else { 
        $arr_feedid = get_pass_feedid_from_stor($uid, $count, $timestamp);
//        DEBUG_V0716 && do_log('debug', '['.__LINE__.'] got feedid from stor:'.print_r($arr_feedid, true));
        if ($arr_feedid != FALSE) {
            $total_count = count($arr_feedid);
            $have_next = 1;
            $next_offset = $offset + $total_count;
        } else {
            $total_count = 0;
            $arr_feedid = array();
        }
        DEBUG_V0716 && do_log('debug', '['.__LINE__."] Query pass feed:mimi<".$uid.'> from storage count:'.$total_count);
    }

//    DEBUG_V0716 && do_log('debug', '['.__LINE__."] Query pass feedid cnt<".count($arr_feedid).'>');
    // ??????arr_app_id???arr_cmd_id??????????????????feedid????????????
    global $g_arr_app_id;
    global $g_arr_cmd_id;
    $g_arr_app_id = array();
    $g_arr_cmd_id[] = 22004;
    $g_arr_cmd_id[] = 22005;
    if (count($arr_feedid) > 0)
        $arr_feedid = array_filter($arr_feedid, 'feedid_filter');
    DEBUG_V0716 && do_log('debug', '['.__LINE__."] Filter pass feedid cnt<".count($arr_feedid).'>');

    // ???storage-server??????feed??????
    $arr_feeds = array();
    if (count($arr_feedid) > 0) {
        $arr_feeds = get_passive_feeds($arr_feedid);
        if ($arr_feeds === FALSE) {
            return FALSE;
        }
    }

    //do_log('debug', print_r($arr_feeds,true));

//    usort($arr_feeds, 'passive_feedid_cmp');

    $arr_result = array();
    $arr_result['current_page'] = $arr_feeds;
    $arr_result['have_next'] = $have_next;
    $arr_result['next_offset'] = $next_offset;
    return json_encode($arr_result);
}

function get_newsfeed_by_tags($my_id, $arr_app_id, $arr_cmd_id, $count, $begin_time, $end_time, $my_tag) {
    /* ????????????????????????tag???????????????????????????????????????????????????????????? ???????????????????????????*/
    $arr_feedid = get_feedid_by_tags($my_id, $my_tag, $begin_time, $end_time, $count);
    if ($arr_feedid === FALSE) {
        do_log('error', 'get_newsfeed: get_feedid');
        return FALSE;
    }
    //do_log('error', __LINE__.'tag:'.$my_tag.'count:'.$count.' get_newsfeed: last_time:');

    // ??????arr_app_id???arr_cmd_id??????????????????feedid????????????
    global $g_arr_app_id;
    global $g_arr_cmd_id;
    $g_arr_app_id = $arr_app_id;
    $g_arr_cmd_id = $arr_cmd_id;
    $arr_feedid = array_filter($arr_feedid, 'feedid_filter');
    
    // ???????????????feedid????????????
    usort($arr_feedid, 'feedid_cmp');
    $offset = 0; 
    $total_count = count($arr_feedid);
    $arr_feedid = array_slice($arr_feedid, $offset);
    $have_next = 0;
    if ($count !== 0 && $total_count == $count) {
        $have_next = 1;
    }    
    // ???storage-server??????feed??????
    $arr_feeds = array();
    if (count($arr_feedid) > 0) {
        $arr_feeds = get_feeds($arr_feedid);
        if ($arr_feeds === FALSE) {
            return FALSE;
        }
    }

    usort($arr_feeds, 'feedid_cmp');
    // ????????????????????????, ?????????????????????
    $i = 0;
    $last_feed = FALSE;
    $arr_join_friend = array();
    
    foreach ($arr_feeds as $key_0 => &$feed_0) {
        if (isset($feed_0['fold'])) {
            continue;
        }
    }

    $arr_result = array();
    $arr_result['current_page'] = $arr_feeds;
    $arr_result['have_next'] = $have_next;
    $arr_result['next_offset'] = $offset + $total_count;

    return json_encode($arr_result);
}

require 'Mifan/pullReq.php';
require 'Mifan/pullRes.php';
function get_recommend_feedid($uid, $count) {
    $addr_ = explode(':', constant('RECOMMEND_ADDR'));
    $client_ = new netClient($addr_[0], $addr_[1]);
    if ($client_->open_conn(1) === FALSE) {
        do_log('error', __LINE__.' Connnect to recommend server failed'.print_r($addr_, true));
        return FALSE;
    }

    $protobuf = new \Mifan\pullReq();
    $protobuf->setCount($count);

    $seri_buf = $protobuf->serializeToString();

    $rqst_ = pack('LLSLL', 18 + strlen($seri_buf), 0, 1, 0, $uid).$seri_buf;
    $resp_ = FALSE;
    if (($resp_ = $client_->send_rqst($rqst_, TIMEOUT)) == FALSE) {
        do_log('error', __LINE__.' Recv from recommend server failed'.print_r($addr_, true));
        return FALSE;
    }

    $client_->close_conn();
            
    $result = unpack('Llen/Lseq/Scmd_id/Lresult/Lmid', $resp_);
    if ($result['result'] != 0) {
        do_log('error', __LINE__.' Recommend server query failed'.print_r($result, true));
        return FALSE;
    }
    
    $resp_body = substr($resp_, 18);
    $feed_list = new \Mifan\pullRes();
    $feed_list->ParseFromString($resp_body);
    if (0 == $feed_list->getFeedidCount()) {
        do_log('error', __LINE__.' Recommend have no resource');
        return FALSE;
    }
    //var_dump($feed_list);
    $arr_feedid = array();
    for ($i = 0; $i != $feed_list->getFeedidCount(); ++$i) {
        $binary = base64_decode($feed_list->getFeedidAt($i));
        $arr_feedid[] =  binary_to_feedid($binary);
    }

    return $arr_feedid;
}
function get_newsfeed_of_recommend($arr_uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp) {
    $my_id = $arr_uid[0];
    $arr_feedid = get_recommend_feedid($my_id, $count);
    if ($arr_feedid === FALSE) {
        $arr_feedid = get_maylike_feedid($my_id);
        if ($arr_feedid === FALSE) {
            do_log('error', 'get_newsfeed_of_maylike: get_feedid');
            return FALSE;
        }
        return get_newsfeed_common($my_id, $arr_feedid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp, TRUE);
    } else {
        //do_log('error', __LINE__.'xxx get_newsfeed: last_time: '.print_r($arr_feedid, true));
        // ???storage-server??????feed??????
        $arr_feeds = array();
        if (count($arr_feedid) > 0) {
            $arr_feeds = get_feeds($arr_feedid);
            if ($arr_feeds === FALSE) {
                return FALSE;
            }
        }

        $article_list = array();
        foreach ($arr_feeds as $key_0 => &$feed_0) 
            $article_list[] = $feed_0['data']['article_id'];
        do_log('error', 'Recommend Request<mimi:'.$my_id.',offset:'.$offset.',conut:'.$count.'> - Response<'.json_encode($article_list).'> ');
        $arr_result = array();
        $arr_result['current_page'] = $arr_feeds;
        $arr_result['have_next'] = 1;
        $arr_result['next_offset'] = $offset == -1 ? $count : $offset + $count;
        return json_encode($arr_result);
    }
}

function get_newsfeed_of_latest($arr_uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp) {
    $my_id = $arr_uid[0];
    $arr_feedid = get_feedid($arr_uid, TRUE);
    if ($arr_feedid === FALSE) {
        do_log('error', 'get_newsfeed: get_feedid');
        return FALSE;
    }
    //do_log('error', __LINE__.'xxx get_newsfeed: last_time: '.$timestamp);
//    notify_kafka_active_user($my_id, $timestamp);
    return get_newsfeed_common($my_id, $arr_feedid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp, TRUE);
}

function get_newsfeed($arr_uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp) {
    //get mine & my friends' feed id
    $my_id = $arr_uid[0];
    // ???outbox-server???????????????feedid
    $arr_feedid = get_feedid($arr_uid, FALSE);
    if ($arr_feedid === FALSE) {
        do_log('error', 'get_newsfeed: get_feedid');
        return FALSE;
    }
    //get_newsfeed_filter_feedid();
    return get_newsfeed_common($my_id, $arr_feedid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp, FALSE);
}

function get_newsfeed_common($my_id, $arr_feedid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp, $is_latest)
{
   //??????????????????????????????
    $up_time = gettimeofday(true); 
    if ($offset == -1) 
        $last_time = $up_time;
    else {
        $last_time = get_timestamp($my_id, 'haoyou');      
        if ($last_time === NULL) 
            $last_time = gettimeofday(true); 
    }

    if ($offset == 0) {
        $news_count = count($arr_feedid);
        for($i = 0; $i < $news_count; ++$i) {
            if ($arr_feedid[$i]['timestamp'] <= $last_time) 
                unset($arr_feedid[$i]);
        }
    } else if ($offset > 0) { 
        $news_count = count($arr_feedid);
        for($i = 0; $i < $news_count; ++$i) {
            if ($arr_feedid[$i]['timestamp'] >= $timestamp )
                unset($arr_feedid[$i]);
        }    
    }   

    // ??????arr_app_id???arr_cmd_id??????????????????feedid????????????
    global $g_arr_app_id;
    global $g_arr_cmd_id;
    $g_arr_app_id = $arr_app_id;
    $g_arr_cmd_id = $arr_cmd_id;
    $arr_feedid = array_filter($arr_feedid, 'feedid_filter');
    
    // ???????????????feedid????????????
    usort($arr_feedid, 'feedid_cmp');
     
    // ????????????
    $total_count = count($arr_feedid);
    if ($count !== 0) {
        $arr_feedid = array_slice($arr_feedid, 0, $count);
    }

    $next_offset = count($arr_feedid);
    if ($offset <= 1) {
        set_timestamp($my_id, $up_time, 'haoyou');
    } else
        $next_offset += $offset;

    $have_next = 0;
    if ($count !== 0 && $total_count > $count) {
        $have_next = 1;
    }    

    // ???storage-server??????feed??????
    $arr_feeds = array();
    if (count($arr_feedid) > 0) {
        $arr_feeds = get_feeds($arr_feedid);
        if ($arr_feeds === FALSE) {
            return FALSE;
        }
    }

    usort($arr_feeds, 'feedid_cmp');
    //do_log('storage-server', $arr_feeds);

    // ????????????????????????, ?????????????????????
    $i = 0;
    $last_feed = FALSE;
    $arr_join_friend = array();
    // ????????????????????????
    $is_latest = TRUE;
    $article_list = array();
    foreach ($arr_feeds as $key_0 => &$feed_0) {
        if ($is_latest) {
            $article_list[] = $feed_0['data']['article_id'];
        }
        if (isset($feed_0['fold'])) {
            continue;
        }
    }
    if ($is_latest)
        do_log('error', 'Latest Request<mimi:'.$my_id.',offset:'.$offset.',conut:'.$count.'> - Response<'.json_encode($article_list).'> ');
    $arr_result = array();
    $arr_result['current_page'] = $arr_feeds;
    $arr_result['have_next'] = $have_next;
    $arr_result['next_offset'] = $next_offset;

    return json_encode($arr_result);
}

function get_class_newsfeed($arr_uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp)
{
    // ???outbox-server???????????????feedid

    
    $arr_feedid = get_feedid($arr_uid, FALSE);
    if ($arr_feedid === FALSE) {
        do_log('error', 'get_newsfeed: get_feedid');
        return FALSE;
    }

    // ??????arr_app_id???arr_cmd_id??????????????????feedid????????????
    global $g_arr_app_id;
    global $g_arr_cmd_id;
    $g_arr_app_id = $arr_app_id;
    $g_arr_cmd_id = $arr_cmd_id;
    $arr_feedid = array_filter($arr_feedid, 'feedid_filter');

    // ???????????????feedid????????????
    usort($arr_feedid, 'feedid_cmp');
     
    // ????????????
    $total_count = count($arr_feedid);
    if ($count !== 0) {
        $arr_feedid = array_slice($arr_feedid, $offset, $count);
    } else {
        $arr_feedid = array_slice($arr_feedid, $offset);
    }
    
    if ($offset != 0)
    {
        foreach($arr_feedid as $val)
        {
            if ($val['timestamp'] >= $timestamp)
            {
                array_shift($arr_feedid);
            }
        }
    }

    $have_next = 0;
    if ($count !== 0 && $total_count > $offset + $count) {
        $have_next = 1;
    }    

    // ???storage-server??????feed??????
    $arr_feeds = array();
    if (count($arr_feedid) > 0) {
        $arr_feeds = get_feeds($arr_feedid);
        if ($arr_feeds === FALSE) {
            return FALSE;
        }
    }

    usort($arr_feeds, 'feedid_cmp');

    //do_log('storage-server', $arr_feeds);

    // ????????????????????????, ?????????????????????
    $i = 0;
    $last_feed = FALSE;
    $arr_join_friend = array();
    foreach ($arr_feeds as $key => &$feed) {
        //?????????????????????
        
        if ($feed['cmd_id'] == 7002 && $feed['app_id'] == 7002)
        {
            //do_log('bbb',print_r($feed,true));
            if (count($feed['data']['target_uid'] == 1)) //???????????????????????????????????????
            {
                $flag_join_friend = 0;
                foreach ($arr_join_friend as $temp_key => $temp_feed)
                {
                    if ($feed['user_id'] == $temp_feed['data']['target_uid'][0] && $temp_feed['user_id'] == $feed['data']['target_uid'][0])
                    {
                        unset($arr_feeds[$key]);
                        $flag_join_friend = 1;
                        break;
                    }
                }
                if ($flag_join_friend == 0)
                {
                $arr_join_friend[] = $feed;
                }
            }

            unset($last_feed);
            $last_feed = FALSE;
            continue;
        }
        else if ($feed['cmd_id'] == 7017 && $feed['app_id'] == 141)
        {
            unset($last_feed);
            $last_feed = FALSE;
            continue;
        }

        if ($last_feed !== FALSE && 
            $feed['user_id'] == $last_feed['user_id'] &&
            $feed['app_id'] == $last_feed['app_id'] &&
            $feed['cmd_id'] == $last_feed['cmd_id']) {     // TODO
            $last_feed['fold'] = $i;
            $feed['fold'] = $i;
        } else {
            $i++;
        }

        unset($last_feed);
        $last_feed = &$feed;
    }

    // ????????????????????????
    foreach ($arr_feeds as $key_0 => &$feed_0) {
        if (isset($feed_0['fold'])) {
            continue;
        }

        if ($feed_0['app_id'] == 7002 && $feed_0['cmd_id'] == 7002) {          // ?????????
            // ???$key_0??????????????????
            foreach ($arr_feeds as $key_1 => $feed_1) {
                if ($key_1 == $key_0) {
                    break;
                }

                if ($feed_1['app_id'] == $feed_0['app_id'] && 
                    $feed_1['cmd_id'] == $feed_0['cmd_id'] && 
                    arr_cmp($feed_1['data']['target_uid'],$feed_0['data']['target_uid'])) {
                    // ??????
                    if (is_array($feed_1['user_id'])) {
                        array_push($feed_1['user_id'], $feed_0['user_id']);
                    } else {
                        $feed_1['user_id'] = array($feed_1['user_id'], $feed_0['user_id']);
                    }
                    unset($arr_feeds[$key_0]);
                    break;
                }
            }
        }
        else if ($feed_0['app_id'] == 141 && $feed_0['cmd_id'] == 7017) {       //????????????
            // ???$key_0??????????????????
            foreach ($arr_feeds as $key_1 => $feed_1) {
                if ($key_1 == $key_0) {
                    break;
                }

                if ($feed_1['app_id'] == $feed_0['app_id'] && 
                    $feed_1['cmd_id'] == $feed_0['cmd_id'] && 
                    arr_cmp($feed_1['data']['badge_id'],$feed_0['data']['badge_id'])) {
                    // ??????
                    if (is_array($feed_1['user_id'])) {
                        array_push($feed_1['user_id'], $feed_0['user_id']);
                    } else {
                        $feed_1['user_id'] = array($feed_1['user_id'], $feed_0['user_id']);
                    }
                    unset($arr_feeds[$key_0]);
                    break;
                }
            }
        }
    }

    $arr_result = array();
    $arr_result['current_page'] = $arr_feeds;
    $arr_result['have_next'] = $have_next;
    $arr_result['next_offset'] = $offset + $count;

    return json_encode($arr_result);
}

function get_following_feedid($uid)
{
    $arr_feedid = array();

    $arr_relation_addr = explode(':', constant('RELATION_ADDR'));
    
    $relation_client = new netclient($arr_relation_addr[0], $arr_relation_addr[1]); 
    if ($relation_client->open_conn(1) === FALSE) {
        do_log('error', 'get_following_feed: time_client->open_conn');
        return NULL;
    }

    $relation_rqst_len = 14;
    $relation_rqst_body = '';
    $relation_rqst_body .= pack('L', $uid);
    $relation_rqst_len += 4;

    $relation_rqst = pack('LLSL', $relation_rqst_len, rand(), 0xB102, 0).$relation_rqst_body;
    $relation_resp = FALSE;

    if (($relation_resp = $relation_client->send_rqst($relation_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'get attention failed: relation_client->send_rqst');
        return FALSE;
    }

    $res_header = unpack('Llen/Lseq/Scmd_id/Lresult/Lmid', $relation_resp);
    if ($res_header['result'] != 0) {
        do_log('error', "get attention feedid: len: {$res_header['len']} result: {$res_header['result']}");
        return FALSE;
    }

    $feedid_count = ($res_header['len'] - 22) / 8;
    for ($j = 0; $j != $feedid_count; ++$j) {
        $binary = substr($relation_resp, 22 + $j * 8, 8);
        $feedid = unpack('Luser_id/Ltimestamp', $binary);
        $arr_feedid[] = $feedid;
    }
    
    if ($relation_client->close_conn() === FALSE) {
        do_log('error', 'get_attenion_feedid: relation_client->close_conn');
        return FALSE;
    }

//    do_log('error', 'get_attenion_feedid: relation_client->close_conn'.print_r($arr_feedid, true));
    return $arr_feedid;
}

function get_friend_id(&$arr_uid)
{
    $arr_redis_addr = explode(':', constant('RELATION_ADDR'));
    $redis_cli = new netclient($arr_redis_addr[0], $arr_redis_addr[1]);
    
    if ($redis_cli->open_conn(1) === FALSE){
        do_log('error', 'get_feedid: relation_client->open_conn');
        return FALSE;
    }
    
    $relation_rqst = pack("LLsLLLLL", 18+4+4+4, 0, 0xB102, 0, $arr_uid[0], $arr_uid[0], 0, 0);
    $relation_resp = FALSE;
    
    if (($relation_resp = $redis_cli->send_rqst($relation_rqst,TIMEOUT)) === FALSE) {
        do_log('error', 'get_friend_id: relation_client->send_rqst');
        return FALSE;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $relation_resp);
    if ($rv['code'] != 0) {
        do_log('error', 'get_friend_id: relation_client internal err');
        return FALSE;
    }
    $relation_resp = substr($relation_resp, 18);
    $rv = unpack("Lunits", $relation_resp);
    $id_list = substr($relation_resp, 4);
    //do_log('debug', '['.__LINE__.'] len: ' . strlen($relation_resp).'  count :'.$rv['units']);
    
    for ($i = 0; $i < $rv['units']; ++$i) {
        $fan = unpack('Lid/Ltime', $id_list);
        array_push($arr_uid, $fan['id']);
        $id_list = substr($id_list, 8);
    }
    return TRUE;
}

function get_update_mid(&$arr_uid) { //,$count) {
    $arr_redis_addr = explode(':', constant('TAG_CACHE_ADDR'));
    $redis_cli = new netclient($arr_redis_addr[0], $arr_redis_addr[1]);
    
    if ($redis_cli->open_conn(1) === FALSE){
        do_log('error', 'get_feedid: relation_client->open_conn');
        return FALSE;
    }
    
    $timestamp = gettimeofday(true);
    $relation_rqst = pack("LLsLLLLL", 18+4+4+4, 10086, 0xA102, 0, $arr_uid[0], 0, $timestamp, 200);
    $relation_resp = FALSE;
    
    if (($relation_resp = $redis_cli->send_rqst($relation_rqst,TIMEOUT)) === FALSE) {
        do_log('error', 'get_fans_id: relation_client->send_rqst');
        return FALSE;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $relation_resp);
    if ($rv['code'] != 0) {
        do_log('error', 'get_fans_id: relation_client internal err');
        return FALSE;
    }
    $relation_resp = substr($relation_resp, 18);
    $rv = unpack("Lunits", $relation_resp);
    $id_list = substr($relation_resp, 4);

    unset($arr_uid[0]);
    for ($i = 0; $i < $rv['units']; ++$i) {
        $fan = unpack('Lid', $id_list);
        array_push($arr_uid, $fan['id']);
        $id_list = substr($id_list, 4);
    }
    return TRUE;
}


require_once('Mifan/feedidFromMine.php');
require_once('Mifan/queryFeedidFromMine.php');
/**
 * @brief get_pull_feedlist ??????????????????????????????
 *
 * @param $uid
 * @param $begintime
 * @param $endtime
 * @param $cnt
 *
 * @return 
 */
function get_pull_feedlist($mid,  $begintime, $endtime, $cnt) {
    $arr_feedid = array();
    $arr_tag_cache_addr = explode(':', constant('TAG_CACHE_ADDR'));
    $feed_client = new netclient($arr_tag_cache_addr[0], $arr_tag_cache_addr[1]); 
    if ($feed_client->open_conn(1) === FALSE) {
        do_log('error', '['.__LINE__.']:get_feedid: feed_client->open_conn');
        return FALSE;
    }

do_log('error', '['.__LINE__.']: mimi:'.$mid);
    $rqst_protobuf = new \Mifan\feedidFromMine();
    $rqst_protobuf->setBeginTime($begintime);
do_log('error', '['.__LINE__.']: begintime:'.$rqst_protobuf->getBeginTime());
    $rqst_protobuf->setEndTime($endtime);
do_log('error', '['.__LINE__.']: endtime:'.$rqst_protobuf->getEndTime());
    $rqst_protobuf->setCnt($cnt);
do_log('error', '['.__LINE__.']: count:'.$rqst_protobuf->getCnt());
    $rqst_body = $rqst_protobuf->serializeToString();
    $rqst = pack("LLSLL", 18 + strlen($rqst_body), 0, 0xA106, 0, $mid).$rqst_body;
do_log('error', '['.__LINE__.']: body len:'.strlen($rqst_body).' content:<'.$rqst_body.'>');
   
    if (($outbox_resp = $feed_client->send_rqst($rqst, TIMEOUT)) === FALSE) {
            do_log('error', 'get_feedid: tag_server_client->send_rqst');
            return FALSE;
    }
    $rv = unpack('Llen/Lseq/Scmd/Lresult/Luser_id', $outbox_resp);
    if ($rv["result"] != 0) {
            do_log('error', 'get_feedid: tag_server_client->send_rqst code :'.$rv["result"]);
            return FALSE;
    }
    
    if ($feed_client->close_conn() === FALSE) {
        do_log('error', 'get_feedid: outbox_client->close_conn');
        return FALSE;
    }
do_log('error', '['.__LINE__.']: resp body len:'.$rv['len']);
    $resp_protobuf = new \Mifan\queryFeedidFromMine();
    $resp_protobuf->ParseFromString(substr($outbox_resp, 18));
    $feedid_encode_arr = $resp_protobuf->getFeedid();
    $arr_feedid = array();
do_log('error', '['.__LINE__.']: feedid encode:'.print_r($feedid_encode_arr, true));
    foreach ($feedid_encode_arr as $fid) {
        $arr_feedid[] = binary_to_feedid(base64_decode($fid));
    }
do_log('error', '['.__LINE__.']: feedid decode:'.print_r($arr_feedid, true));

    // ???storage-server??????feed??????
    $arr_feeds = array();
    if (count($arr_feedid) > 0) {
        $arr_feeds = get_feeds($arr_feedid);
        if ($arr_feeds === FALSE) {
            return FALSE;
        }
    }

    $have_next = $cnt == count($arr_feeds) ? 1 : 0;
    $arr_result = array();
    $arr_result['current_page'] = $arr_feeds;
    $arr_result['have_next'] = $have_next;

    return json_encode($arr_result);
}
//???????????????????????????outbox???????????????????????????outbox??????count????????????count???????????????????????????
function get_homepage_feedid_outbox($uid, $offset, $count, &$arr_feedid) {
    $arr_outbox_addr = explode(':', constant('OUTBOX_ADDR'));
    $outbox_client = new netclient($arr_outbox_addr[0], $arr_outbox_addr[1]); 
    if ($outbox_client->open_conn(1) === FALSE) {
        do_log('error', '['.__FUNCTION__.']['.__LINE__.'] outbox_client->open_conn');
        return;
    }
        
    $outbox_rqst = pack('LSL', 4 + 2 + 4, OUTBOX_OPCODE, $uid);
    $outbox_resp = FALSE;
    if (($outbox_resp = $outbox_client->send_rqst($outbox_rqst, TIMEOUT)) === FALSE) {
        do_log('error', '['.__FUNCTION__.']['.__LINE__.'] outbox_client->send_rqst');
        return ;
    }


    if ($outbox_client->close_conn() === FALSE) {
        do_log('error', '['.__FUNCTION__.']['.__LINE__.'] outbox_client->close_conn');
        //return ;
    }
    $rv = unpack('Llen/Sresult', $outbox_resp);
    if ($rv['result'] != 0) {
        do_log('error', '['.__FUNCTION__.']['.__LINE__."]get_feedid: len: {$rv['len']} result: {$rv['result']}");
            return ;
    }

    $feedid_count = ($rv['len'] - OUTBOX_RESPONSE_HEAD_LEN) / FEEDID_LEN;
    //DEBUG && do_log('error', '['.__FUNCTION__.']['.__LINE__.'] outbox_client->response'.print_r($rv, true));
    //?????????????????????feedid
    if ($feedid_count > $offset + $count)
        $feedid_count = $offset + $count;

    //DEBUG && do_log('error', '['.__FUNCTION__.']['.__LINE__.'] outbox feedid count'.$feedid_count);
    $arr_feedid = array();
    for ($j = $offset; $j != $feedid_count; ++$j) {
        $binary = substr($outbox_resp, OUTBOX_RESPONSE_HEAD_LEN + $j * FEEDID_LEN, FEEDID_LEN);
        $feedid = binary_to_feedid($binary);
        $arr_feedid[] = $feedid;
    }
}

//???????????????outbox???????????????????????????storage????????????????????????
function get_homepage_feed_storage($uid, $offset, $count, $timestamp, &$arr_feeds) {
    if (!isset($uid, $count, $timestamp)) {
        do_log('error', '['.__FUNCTION__.']['.__LINE__.'] uid&count&timestamp must not null');
        return FALSE;
    }

//    do_log('error', '['.__FUNCTION__.']['.__LINE__.'] feedid from storage');
    $rqst_pack_format = array('LS2LS2L2S2');
    $rqst_pack_content = array(
            'len'       =>   28,
            'op'        =>   11,
            'units'     =>   1,
            'mimi'      =>   $uid,
            'flag'      =>   0x7,
            'cmd_id'    =>   7003,
            'app_id'    =>   1,
            'starttime' =>   $timestamp - 1,
            'prev_num'  =>   $count,
            'next_num'  =>   0,
        );
    $para = array_merge($rqst_pack_format, $rqst_pack_content);
    $storage_rqst = call_user_func_array('pack', array_values($para));
    do_log('error', '['.__FUNCTION__.']['.__LINE__.'] rqst to storage'.print_r($storage_rqst, true));

    $arr_storage_addr = explode(':', constant('STORAGE_ADDR'));
    $storage_client = new netclient($arr_storage_addr[0], $arr_storage_addr[1]); 
    if ($storage_client->open_conn(1) === FALSE) {
        do_log('error', 'ERROR: storage_client->open_conn');
        return FALSE;
    }

    $storage_resp = FALSE;
    if (($storage_resp = $storage_client->send_rqst($storage_rqst, TIMEOUT)) === FALSE) {
        do_log('error', 'ERROR: storage_client->send_rqst');
        return FALSE;
    }

    if ($storage_client->close_conn() === FALSE) {
        do_log('error', 'ERROR: storage_client->close_conn');
    //    return FALSE;
    }

    $rv_0 = unpack('Llen/Sret/Sunits', $storage_resp);
        do_log('error', __FUNCTION__ . "ERROR: len: {$rv_0['len']} result: {$rv_0['ret']}");
    if ($rv_0['ret'] != 0) {
        do_log('error', __FUNCTION__ . "ERROR: len: {$rv_0['len']} result: {$rv_0['ret']}");
        return FALSE;
    }
    $storage_resp = substr($storage_resp, 4 + 2 + 2);
    for ($i = 0; $i != $rv_0['units']; ++$i) {
        $rv_1 = unpack('Llen', $storage_resp);
        $feed = binary_to_feedid(substr($storage_resp, 4));
        $temp = unpack('Lid1/Lid2', substr($storage_resp, 4 + FEEDID_LEN - 8)); 
        $feed['id1'] = $temp['id1'];
        $feed['id2'] = $temp['id2'];
        $feed_data = substr($storage_resp, 4 + FEEDID_LEN, $rv_1['len'] - (4 + FEEDID_LEN));
        $feed['data'] = json_decode($feed_data, TRUE);
        $arr_feeds[] = $feed;
        $storage_resp = substr($storage_resp, $rv_1['len']);
    }

}

function get_homepage_feed($uid, $offset, $count, $timestamp, &$arr_feeds) {
    if ($offset < MAX_OUTBOX_CNT) { 
        $arr_feeds = array();
        get_homepage_feedid_outbox($uid, $offset, $count, $arr_feedid);
//        do_log('error', '['.__FUNCTION__.']['.__LINE__.'] feedid list'.print_r($arr_feedid, true));
        // ???storage-server??????feed??????
        if (count($arr_feedid) > 0) {
            $arr_feeds = get_feeds($arr_feedid);
            if ($arr_feeds === FALSE) 
                return FALSE;
            usort($arr_feeds, 'feedid_cmp');
        }
    }
    else 
        get_homepage_feed_storage($uid, $offset,$count, $timestamp, $arr_feeds);
}

//?????????????????????API
function get_homepage_newsfeed($uid, $arr_app_id, $arr_cmd_id, $offset, $count, $timestamp) {
    
    $arr_result = array();
    $arr_result['current_page'] = array();
    $arr_result['have_next'] = 0;
    $arr_result['next_offset'] = 0;
    $arr_feedid = array();

    if ($offset < 0)
        $offset = 0;
    $arr_feeds = array();
    get_homepage_feed($uid, $offset, $count, $timestamp, $arr_feeds);
    $cnt = count($arr_feeds);
    if ($cnt == 0) {
        do_log('error', '['.__LINE__.']:search no self feeds in cache');
        return json_encode($arr_result);
    }
    
    $arr_result['current_page'] = $arr_feeds;
    $arr_result['have_next'] = 1;
    $arr_result['next_offset'] = $offset + $cnt;
    return json_encode($arr_result);
}
function get_recommend_user($uid, $type, $offset, $count) {
    $result = array();
    $next_offset = 0;
    $arr_uid = array();
    $key_ = 'feeds:'.$uid.':recommends';
    DEBUG && do_log('debug', '['.__LINE__.']: mimi:['.$key_.'] offset:['.$offset.'] count:['.$count.']');

    $list_ = ['10.25.144.75:7000', '10.25.144.75:7001', '10.25.144.75:7002'];
    $client_ = new RedisCluster(NULL, $list_);
    $active = $client_->exists($key_);
    DEBUG_V0716 && do_log('debug', '['.__LINE__.']: user:['.$uid.'] active:['.$active.']');
    if ($active = 1)
        $arr_uid = $client_->lRange($key_, $offset, $offset + $count - 1);
    else
        $arr_uid = $client_->lRange('feeds:default:recommends', $offset, $offset + $count - 1);
    $cnt = count($arr_uid);
    if ($cnt == $count)
        $next_offset = $offset + $count;
    if ($next_offset >= 500)
        $next_offset = 0;
    $result['list'] = $arr_uid;
    $result['next_offset'] = $next_offset;
    return json_encode($result);
}
function get_maylike_feedid($uid) {
    $arr_uid = array();
    $key_ = 'feeds:'.$uid.':recommends';

    $list_ = ['10.25.144.75:7000', '10.25.144.75:7001', '10.25.144.75:7002'];
    $client_ = new RedisCluster(NULL, $list_);
    $active = $client_->exists($key_);
    DEBUG_V0716 && do_log('debug', '['.__LINE__.']: user:['.$uid.'] active:['.$active.']');
    if ($active = 1)
        $arr_uid = $client_->lRange($key_, 10, 310);
    else
        $arr_uid = $client_->lRange('feeds:default:recommends', 10, 310);

    if (count($arr_uid) == 0)
        return FALSE;
    
    $arr_feedid = array();

    $arr_outbox_addr = explode(':', constant('OUTBOX_ADDR'));
    $outbox_client = new netclient($arr_outbox_addr[0], $arr_outbox_addr[1]); 
    if ($outbox_client->open_conn(1) === FALSE) {
        do_log('error', 'get_feedid: outbox_client->open_conn');
        return FALSE;
    }

    $uid_count = count($arr_uid);
    for ($i = 0; $i < $uid_count; $i += MAX_OUTBOX_REQUEST_COUNT) {
        $arr_slice_uid = array_slice($arr_uid, $i, MAX_OUTBOX_REQUEST_COUNT);

        $outbox_rqst_len = 4 + 2;
        $outbox_rqst_body = '';
        foreach ($arr_slice_uid as $uid) {
            $outbox_rqst_body .= pack('L', $uid);
            $outbox_rqst_len += 4;
        }

        $outbox_rqst = pack('LS', $outbox_rqst_len, OUTBOX_OPCODE) . $outbox_rqst_body;
        $outbox_resp = FALSE;
        if (($outbox_resp = $outbox_client->send_rqst($outbox_rqst, TIMEOUT)) === FALSE) {
            do_log('error', 'get_feedid: outbox_client->send_rqst');
            return FALSE;
        }

        $rv = unpack('Llen/Sresult', $outbox_resp);
        if ($rv['result'] != 0) {
            do_log('error', "get_feedid: len: {$rv['len']} result: {$rv['result']}");
            return FALSE;
        }

        $feedid_count = ($rv['len'] - OUTBOX_RESPONSE_HEAD_LEN) / FEEDID_LEN;
        for ($j = 0; $j != $feedid_count; ++$j) {
            $binary = substr($outbox_resp, OUTBOX_RESPONSE_HEAD_LEN + $j * FEEDID_LEN, FEEDID_LEN);
            $feedid = binary_to_feedid($binary);
            $arr_feedid[] = $feedid;
        }
    }

    if ($outbox_client->close_conn() === FALSE) {
        do_log('error', 'get_feedid: outbox_client->close_conn');
        return FALSE;
    }

    return $arr_feedid;
}
