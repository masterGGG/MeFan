<?php
require_once('./lib/netclient_class.php');
global $g_redis_client;
function feedid_cmp_score($feedid_1, $feedid_2){
    return ($feedid_1['score'] < $feedid_2['score']) ? 1 : -1;
}

function feedid_cmp($feedid_1, $feedid_2){
    if ($feedid_1['timestamp'] == $feedid_2['timestamp']) {
        return 0;
    }

    return ($feedid_1['timestamp'] < $feedid_2['timestamp']) ? 1 : -1;
}

function _init_tag_redis_connect(&$cli) {
    global $g_recommend_conf;
    global $g_redis_client;
    $cli = new Redis();
    $cli->connect($g_recommend_conf['backend']['redis_cache_server']['ip'], $g_recommend_conf['backend']['redis_cache_server']['port']);
    $cli->auth($g_recommend_conf['backend']['redis_cache_server']['pwd']);
    if (empty($g_redis_client))
        $g_redis_client = $cli;
    return $cli;
}

function get_timestamp($uid, $type) {
    global $g_recommend_conf;
    global $g_redis_client;
    if (empty($g_redis_client))
        $g_redis_client = _init_tag_redis_connect($g_redis_client);

    if ($type == 'start')    {
        $time = $g_redis_client->hGet('feeds:recommend:'.$uid.':stat', 'startTime');
    } else if ($type = 'end')    {
        $time = $g_redis_client->hGet('feeds:recommend:'.$uid.':stat', 'endTime');
    } else  {
        DEBUG && log::write('['.__LINE__.'] Can not get time server type:'.$type, 'debug');
        return NULL;
    }

    if ((empty($time)) || ($time === FALSE))
        return NULL;

    return (int)$time;
}

function set_timestamp($uid, $time, $type) {
    global $g_recommend_conf;
    global $g_redis_client;
    if (empty($g_redis_client))
        $g_redis_client = _init_tag_redis_connect($g_redis_client);
    if ($type == 'start')    {
        if ($time == 0) {
            $rv = $g_redis_client->hdel('feeds:recommend:'.$uid.':stat', 'startTime');
            $rv = $g_redis_client->hdel('feeds:recommend:'.$uid.':stat', 'endTime');
        } else
            $rv = $g_redis_client->hSet('feeds:recommend:'.$uid.':stat', 'startTime', (int)$time);
    } else if ($type = 'end')    {
        $rv = $g_redis_client->hSet('feeds:recommend:'.$uid.':stat', 'endTime', (int)$time);
    } else  {
        DEBUG && log::write('['.__LINE__.'] Can not get time server type:'.$type, 'debug');
        return NULL;
    }
}

function _history_computering($uid, &$arr_feed) {
    global $g_recommend_conf;
    /*
     * 2019-08-07 计算每个用户的权重值
     */
    $arr_my_score = array();
    $arr_score = array();
    get_user_score($uid, $arr_my_score);
    foreach ($arr_feed as &$feed) {
        if (array_key_exists($feed['user_id'], $arr_score))
            $feed['score'] = $arr_score[$feed['user_id']];
        else {
            $feed['score'] = query_user_score_by_tag($uid, $feed['user_id'], $arr_my_score);
            $arr_score[$feed['user_id']] = $feed['score'];
        }
    }
}

function _get_history_feed($uid, $count, &$arr_feed) {
    global $g_recommend_conf;
    $arr_feed = array();

    $start_time = get_timestamp($uid, 'start');
    if ($start_time == NULL) {
//        DEBUG && log::write('['.__LINE__.'] Can not get start time', 'debug');
        $start_time = 0;
    }
    if (($end_time = get_timestamp($uid, 'end')) == NULL)  {
//        DEBUG && log::write('['.__LINE__.'] Can not get end time', 'debug');
        $end_time = 0;
    }
//    DEBUG && log::write('['.__LINE__.'] uid<'.$uid.'> start<'.$start_time.'> end<'.$end_time.'>', 'debug');
    if ($start_time != $end_time)
        $new_end_time = gettimeofday(true);//如果拉取时需要啦最新的和历史的feedid，保存当前时间戳，可以避免最新数组的排序

    if (get_update_mid($uid, $arr_uid) === FALSE)
        return FALSE;

    $arr_outbox_addr = explode(':', $g_recommend_conf['backend']['outbox_server']);
    $outbox_client = new netclient($arr_outbox_addr[0], $arr_outbox_addr[1]); 
    if ($outbox_client->open_conn(1) === FALSE) {
        log::write('['.__FILE__.']['.__LINE__.'] Connect server<'.$g_recommend['backend']['outbox_server'].'> failed', 'error');
        return FALSE;
    }

    $uid_count = count($arr_uid);
//    DEBUG && log::write('['.__FILE__.']['.__LINE__.'] update uid count<'.$uid_count.'>', 'debug');
    $arr_new = array();
    $arr_old = array();
    for ($i = 0; $i < $uid_count; $i += MAX_OUTBOX_REQUEST_COUNT) {
        $arr_slice_uid = array_slice($arr_uid, $i, MAX_OUTBOX_REQUEST_COUNT);

        $outbox_rqst_len = 4 + 2;
        $outbox_rqst_body = '';
        foreach ($arr_slice_uid as $_uid) {
            $outbox_rqst_body .= pack('L', $_uid);
            $outbox_rqst_len += 4;
        }

        $outbox_rqst = pack('LS', $outbox_rqst_len, OUTBOX_OPCODE) . $outbox_rqst_body;
        $outbox_resp = FALSE;
        if (($outbox_resp = $outbox_client->send_rqst($outbox_rqst, TIMEOUT)) === FALSE) {
            log::write('['.__FILE__.']['.__LINE__.'] Send failed', 'error');
            return FALSE;
        }

        $rv = unpack('Llen/Sresult', $outbox_resp);
        if ($rv['result'] != 0) {
            log::write('['.__FILE__.']['.__LINE__."] get_feedid: len: {$rv['len']} result: {$rv['result']}", 'error');
            return FALSE;
        }
//        DEBUG && log::write('['.__FILE__.']['.__LINE__.'] outbox result:'.print_r($rv, true), 'debug');

        $feedid_count = ($rv['len'] - OUTBOX_RESPONSE_HEAD_LEN) / FEEDID_LEN;
        for ($j = 0; $j != $feedid_count; ++$j) {
            $binary = substr($outbox_resp, OUTBOX_RESPONSE_HEAD_LEN + $j * FEEDID_LEN, FEEDID_LEN);
            $feedid = unpack("Luser_id/Scmd_id/Lapp_id/Ltimestamp", $binary);
//            DEBUG && log::write('['.__FILE__.']['.__LINE__.'] source feedid:'.print_r($feedid, true), 'debug');
            $tmp['user_id'] = $feedid['user_id'];
            $tmp['timestamp'] = $feedid['timestamp'];
            if ($start_time == $end_time) {//第一次做全量更新，全部放入最新列表中，然后取最新的count条
                $tmp['feedid'] = base64_encode($binary);
                $arr_new[] = $tmp;
//                    DEBUG && log::write('['.__FILE__.']['.__LINE__.'] first time new:'.print_r($tmp, true), 'debug');
            } else {
                if ($tmp['timestamp'] > $end_time) {
                    $tmp['feedid'] = base64_encode($binary);
//                    DEBUG && log::write('['.__FILE__.']['.__LINE__.'] new feedid:'.print_r($tmp, true), 'debug');
                    $arr_new[] = $tmp;
                } else if ($tmp['timestamp'] < $start_time) {
                    $tmp['feedid'] = base64_encode($binary);
                    $arr_old[] = $tmp;
//                    DEBUG && log::write('['.__FILE__.']['.__LINE__.'] old feedid:'.print_r($tmp, true), 'debug');
                }
            }
        }
    }

    if ($outbox_client->close_conn() === FALSE) {
        log::write('['.__FILE__.']['.__LINE__.'] Close failed', 'error');
        return FALSE;
    }

                    
    if (count($arr_new) > $count) {
//        DEBUG && log::write('['.__LINE__.'] new feedid count<'.count($arr_new).'>', 'debug');
        usort($arr_new, 'feedid_cmp');
        if ($start_time == $end_time) {
            $arr_feed = array_slice($arr_new, 0, $count); 
            set_timestamp($uid, $arr_feed[0]['timestamp'], 'end');
            $end_time = $arr_feed['0']['timestamp'];
//            DEBUG && log::write('['.__LINE__.'] new end time<'.$arr_feed[0]['timestamp'].'>', 'debug');
            set_timestamp($uid, $arr_feed[$count - 1]['timestamp'], 'start');
//            DEBUG && log::write('['.__LINE__.'] new start time<'.$arr_feed[$count - 1]['timestamp'].'>', 'debug');
        } else {
            $arr_feed = array_slice($arr_new, count($arr_new)-$count-1, $count);
            set_timestamp($uid, $arr_feed[0]['timestamp'], 'end');
            $end_time = $arr_feed['0']['timestamp'];
//            DEBUG && log::write('['.__FILE__.']['.__LINE__.'] new end time<'.$arr_feed[0]['timestamp'].'>', 'debug');
        }
    } else {
        usort($arr_old, 'feedid_cmp');
        $arr_old = array_slice($arr_old, 0, $count - count($arr_new));
//        DEBUG && log::write('['.__FILE__.']['.__LINE__.'] new feedid <'.count($arr_new).'> old nct <'.count($arr_old).'>', 'debug');
        set_timestamp($uid, $new_end_time, 'end');
        $end_time = $new_end_time;
//        DEBUG && log::write('['.__FILE__.']['.__LINE__.'] new end time<'.$new_end_time.'>', 'debug');
        set_timestamp($uid, $arr_old[count($arr_old) - 1]['timestamp'], 'start');
//        DEBUG && log::write('['.__FILE__.']['.__LINE__.'] new end time<'.$arr_old[count($arr_old) - 1]['timestamp'].'>', 'debug');
        $arr_feed = array_merge($arr_new, $arr_old);
    }
    _history_computering($uid, $arr_feed);
    return $arr_feed;
}
//获取最新更新文章的米米号集合，用于文章的检索
function get_update_mid($uid, &$arr_uid) { 
    global $g_recommend_conf;
    $arr_server_addr = explode(':', $g_recommend_conf['backend']['tag_server']);
    $redis_cli = new netclient($arr_server_addr[0], $arr_server_addr[1]);
    
    if ($redis_cli->open_conn(1) === FALSE){
        log::write('['.__FILE__.']['.__LINE__.'] Connect server<'.$g_recommend_conf['backend']['tag_server'].'> failed', 'error');
        return FALSE;
    }
    
    $timestamp = gettimeofday(true);
    $relation_rqst = pack("LLsLLLLL", 18+4+4+4, 10086, 0xA102, 0, $uid, 0, $timestamp, 300);
    $relation_resp = FALSE;
    
    if (($relation_resp = $redis_cli->send_rqst($relation_rqst,TIMEOUT)) === FALSE) {
        log::write('['.__FILE__.']['.__LINE__.'] Send server<'.$g_recommend['backend']['tag_server'].'> failed', 'error');
        return FALSE;
    }
    $rv = unpack('Llen/Lseq/scmd_id/Lcode/Lmimi', $relation_resp);
    if ($rv['code'] != 0) {
        log::write('['.__FILE__.']['.__LINE__.'] Server<'.$g_recommend['backend']['tag_server'].'> handle failed', 'error');
        return FALSE;
    }
    $relation_resp = substr($relation_resp, 18);
    $rv = unpack("Lunits", $relation_resp);
    $id_list = substr($relation_resp, 4);

//    empty($arr_uid[0]);
    for ($i = 0; $i < $rv['units']; ++$i) {
        $fan = unpack('Lid', $id_list);
        //array_push($arr_uid, $fan['id']);
        $arr_uid[] = $fan['id'];
        $id_list = substr($id_list, 4);
    }
    return TRUE;
}

function _get_hot_feed($uid, $count, &$arr_feed) {
    global $g_redis_client;
    if (empty($g_redis_client))
        $g_redis_client = _init_tag_redis_connect($g_redis_client);
    
    $index = 0;
    $rv = $g_redis_client->hGet('feeds:recommend:'.$uid.':stat', 'hotArticleIndex');
    if ($rv !== FALSE)
        $index = (int)$rv;

    global $g_recommend_conf;
    $arr_feed[] = 'feeds:recommend:'.$uid.':hot';
    while ($count) {
        if ($index < $g_recommend_conf['backend']['redis_cache_server']['cache0cnt']) {
            if ($count < $g_recommend_conf['backend']['redis_cache_server']['cache0cnt'] - $index)
                $rv = $g_redis_client->zRevRange($g_recommend_conf['backend']['redis_cache_server']['cache0'], $index, $index + $count - 1, true);
            else 
                $rv = $g_redis_client->zRevRange($g_recommend_conf['backend']['redis_cache_server']['cache0'], $index, $g_recommend_conf['backend']['redis_cache_server']['cache0cnt'] - 1, true);
            foreach ($rv as $key => $value) {
                $arr_feed[] = $value;
                $arr_feed[] = $key;
                $count--;
                $index++;
//                DEBUG && log::write('['.__LINE__.']: INDEX<'.$index.'> COUNT<'.$count.'> FEED<'.$feed.'>', 'debug');
            }
            //如果当天的热文没有50条，那么将index增至50，并进行下次迭代
            if ($count > 0) 
                $index = $g_recommend_conf['backend']['redis_cache_server']['cache0cnt'];
//            DEBUG && log::write('['.__LINE__.']: INDEX<'.$index.'> COUNT<'.$count.'>', 'debug');
        } else if ($index < $g_recommend_conf['backend']['redis_cache_server']['cache1cnt'] + $g_recommend_conf['backend']['redis_cache_server']['cache0cnt']) {
            $real_index = $index - $g_recommend_conf['backend']['redis_cache_server']['cache0cnt'];
            if ($count < $g_recommend_conf['backend']['redis_cache_server']['cache1cnt'] - $real_index)
                $rv = $g_redis_client->zRevRange($g_recommend_conf['backend']['redis_cache_server']['cache1'], $real_index, $real_index + $count - 1, true);
            else 
                $rv = $g_redis_client->zRevRange($g_recommend_conf['backend']['redis_cache_server']['cache1'], $real_index, $g_recommend_conf['backend']['redis_cache_server']['cache1cnt'] - 1, true);
            foreach ($rv as $key => $value) {
                $arr_feed[] = $value;
                $arr_feed[] = $key;
                $count--;
                $index++;
            }
            if ($count > 0) 
                $index = $g_recommend_conf['backend']['redis_cache_server']['cache1cnt'] + $g_recommend_conf['backend']['redis_cache_server']['cache0cnt'];
//            DEBUG && log::write('['.__LINE__.']: INDEX<'.$index.'> COUNT<'.$count.'>', 'debug');
        } else if ($index < $g_recommend_conf['backend']['redis_cache_server']['cache2cnt'] + $g_recommend_conf['backend']['redis_cache_server']['cache1cnt'] + $g_recommend_conf['backend']['redis_cache_server']['cache0cnt']) {
            $real_index = $index - $g_recommend_conf['backend']['redis_cache_server']['cache0cnt'] - $g_recommend_conf['backend']['redis_cache_server']['cache1cnt'];
            if ($count < $g_recommend_conf['backend']['redis_cache_server']['cache2cnt'] - $real_index)
                $rv = $g_redis_client->zRevRange($g_recommend_conf['backend']['redis_cache_server']['cache2'], $real_index, $real_index + $count - 1, true);
            else 
                $rv = $g_redis_client->zRevRange($g_recommend_conf['backend']['redis_cache_server']['cache2'], $real_index, $g_recommend_conf['backend']['redis_cache_server']['cache2cnt'] - 1, true);
            foreach ($rv as $key => $value) {
                $arr_feed[] = $value;
                $arr_feed[] = $key;
                $count--;
                $index++;
//                DEBUG && log::write('['.__LINE__.']: INDEX<'.$index.'> COUNT<'.$count.'> FEED<'.$feed.'>', 'debug');
            }
            if ($count > 0) 
                $index = 0;
//            DEBUG && log::write('['.__LINE__.']: INDEX<'.$index.'> COUNT<'.$count.'>', 'debug');
        } else {
            $index = 0;
        }
    }
    $g_redis_client->hSet('feeds:recommend:'.$uid.':stat', 'hotArticleIndex', $index);
//    DEBUG && log::write('['.__LINE__.']: hSet feeds:recommend:'.$uid.':stat hotArticleIndex '.$index.' RET<'.$rv.'>', 'debug');
    return $arr_feed;
}

function _push_history_like_feed($uid, $arr_feed) {
    $arr_like = array();
    $arr_like[] = 'feeds:recommend:'.$uid.':like';
    for ($i = 0; $i != count($arr_feed); ++$i) {
        $arr_like[] = $arr_feed[$i]['score'];
        $arr_like[] = $arr_feed[$i]['feedid'];
    }
//    DEBUG && log::write('['.__LINE__.']: like set '.print_r($arr_like, true), 'debug');

    global $g_redis_client;
    if (empty($g_redis_client))
        $g_redis_client = _init_tag_redis_connect($g_redis_client);
        
    $stat = $g_redis_client->hGetall('feeds:recommend:'.$uid.':stat');
//    DEBUG && log::write('['.__LINE__.']: recommend<'.$rv.'> LIKE end'.print_r($stat, true), 'debug');

    if (!array_key_exists('likeReadable', $stat)) {
        $stat['likeReadable'] = 0;
        $stat['likeUpdatable'] = 1;
        $stat['likeCount'] = 0;
        $stat['likeIndex'] = 0;
    }
    $rv = $g_redis_client->hSet('feeds:recommend:'.$uid.':stat', 'likeUpdatable', 2);

    if ($stat['likeReadable'] == 0) {
        $rv = $g_redis_client->zRemRangeByRank('feeds:recommend:'.$uid.':like', 0, -1);
    } else {
        $rv = $g_redis_client->zRemRangeByRank('feeds:recommend:'.$uid.':like', 0, $stat['likeIndex']);
    }
    $stat['likeReadable'] = 1;
    $stat['likeUpdatable'] = 0;
    $stat['likeIndex'] = 0;

    if (count($arr_like) > 1)
        call_user_func_array(array($g_redis_client, 'zadd'), $arr_like);
    $stat['likeCount'] = (int)$g_redis_client->zSize('feeds:recommend:'.$uid.':like');
    
    $rv = $g_redis_client->hMset('feeds:recommend:'.$uid.':stat', $stat);
//    DEBUG && log::write('['.__LINE__.']: recommend<'.$rv.'> LIKE end'.print_r($stat, true), 'debug');
    return;
}

function _push_history_hot_feed($uid, $arr_hot) {
    global $g_redis_client;
    if (empty($g_redis_client))
        $g_redis_client = _init_tag_redis_connect($g_redis_client);
        
    $stat = $g_redis_client->hGetall('feeds:recommend:'.$uid.':stat');
//    DEBUG && log::write('['.__LINE__.']: recommend<'.$rv.'> HOT end'.print_r($stat, true), 'debug');
    if (!array_key_exists('hotReadable', $stat)) {
        $stat['hotReadable'] = 0;
        $stat['hotUpdatable'] = 1;
        $stat['hotCount'] = 0;
        $stat['hotIndex'] = 0;
    }
    $rv = $g_redis_client->hSet('feeds:recommend:'.$uid.':stat', 'hotUpdatable', 2);

    if ($stat['hotReadable'] == 0) {
        $rv = $g_redis_client->zRemRangeByRank('feeds:recommend:'.$uid.':hot', 0, -1);
    } else {
        $rv = $g_redis_client->zRemRangeByRank('feeds:recommend:'.$uid.':hot', 0, $stat['hotIndex']);
    }
    $stat['hotReadable'] = 1;
    $stat['hotUpdatable'] = 0;
    $stat['hotIndex'] = 0;

    if (count($arr_hot) > 1)
        call_user_func_array(array($g_redis_client, 'zadd'), $arr_hot);
    $stat['hotCount'] = (int)$g_redis_client->zSize('feeds:recommend:'.$uid.':hot');

    $rv = $g_redis_client->hMset('feeds:recommend:'.$uid.':stat', $stat);
//    DEBUG && log::write('['.__LINE__.']: recommend<'.$rv.'> HOT end'.print_r($stat, true), 'debug');
    return;
}

function do_history_process($uid, $count, $index) {
    DEBUG && log::write('['.__LINE__.']: USER<'.$uid.'> LIKE<'.$count.'> HOT<'.$index.'>', 'debug');

    if ($count > 0) {   //内容池需要更新
        $arr_feed = _get_history_feed($uid, $count, $arr_feed);
        usort($arr_feed, 'feedid_cmp_score');
        _push_history_like_feed($uid, $arr_feed);
    }
 
    if ($index > 0) {   //热文池需要更新
        _get_hot_feed($uid, $index, $arr_hot);
        _push_history_hot_feed($uid, $arr_hot);
    }
    return;
}

?>
