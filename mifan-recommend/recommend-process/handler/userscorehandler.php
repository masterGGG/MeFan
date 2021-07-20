<?php
//require_once('./lib/netclient_class.php');
global $g_redis_cache_client;
global $g_mysql_mifan_client;
global $g_mysql_basic_client;
function get_user_default_tag($mimi, &$arr_tag) {
    global $g_recommend_conf;
    global $g_mysql_basic_client;
    $rv = _init_mysql_connect($g_mysql_basic_client, $g_recommend_conf['backend']['mysql_basic_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_recommend_conf['backend']['mysql_basic_server'], true), 'error');
        return FALSE;
    }

    $query_sql = 'select tag_id from '.$g_recommend_conf['backend']['mysql_basic_server']['tag_table'].' where account='.$mimi;
    //DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');
    $rv = mysqli_query($g_mysql_basic_client, $query_sql);

    $arr_tag = array();
    if ($rv->num_rows == 0) {
        log::write('['.__LINE__.'] USER<'.$mimi.'>  QUERY<'.$query_sql.'>has none interest', 'warn');
        return FALSE;
    }

    while ($row = mysqli_fetch_assoc($rv)) 
        $arr_tag[] = $row['tag_id'];
    
//    DEBUG && log::write('['.__LINE__.'] USER<'.$mimi.'> default interest list :'.print_r($arr_tag, true), 'debug');
    return TRUE;
}

function get_user_default_score($mimi, &$arr_score, &$arr_tag) {
    $arr_score = array(
        DEFAULT_MOLE        => DEFAULT_TAG_UNINTEREST,
        DEFAULT_PHOTO       => DEFAULT_TAG_UNINTEREST,
        DEFAULT_DRAW        => DEFAULT_TAG_UNINTEREST,
        DEFAULT_ACG         => DEFAULT_TAG_UNINTEREST,
        DEFAULT_FASHION     => DEFAULT_TAG_UNINTEREST,
        DEFAULT_LIFE        => DEFAULT_TAG_UNINTEREST,
        DEFAULT_GAME        => DEFAULT_TAG_UNINTEREST,
        DEFAULT_DESIGN      => DEFAULT_TAG_UNINTEREST,
        DEFAULT_FILM        => DEFAULT_TAG_UNINTEREST,
        DEFAULT_VEDIO       => DEFAULT_TAG_UNINTEREST,
        DEFAULT_JOURNEY     => DEFAULT_TAG_UNINTEREST,
        DEFAULT_LITERATURE  => DEFAULT_TAG_UNINTEREST,
        DEFAULT_COMMON      => 10,
        DEFAULT_ATTENTION   => 30,
        DEFAULT_PERSONAL    => 20,
        DEFAULT_HOBBY       => 25,
        DEFAULT_OFFICIAL    => 35,
    );
   
    get_user_default_tag($mimi, $arr_tag); 

    foreach ($arr_tag as $tag) 
        $arr_score[$tag] = DEFAULT_TAG_INTEREST;
    
//    DEBUG && log::write('['.__LINE__.'] USER<'.$mimi.'> default score list :'.print_r($arr_score, true), 'debug');
    return true;
}

function tag_score_cmp($tag1, $tag2) {
    if ($tag1['score'] == $tag2['score'])
        return 0;
    return $tag1['score'] < $tag2['score'] ? 1 : -1; 
}
function get_user_real_tag($mimi, $arr_score, &$arr_tag) {
    if (FALSE == get_user_default_tag($mimi, $arr_tag)) 
        return TRUE;

    for ($i = DEFAULT_MOLE; $i <= DEFAULT_LITERATURE; ++$i) 
        $arr_tmp[] = array('tag'=>$i,'score'=>$arr_score[$i]);

    usort($arr_tmp, 'tag_score_cmp');
    for($i = 0; $i != count($arr_tag); ++$i)
        $arr_tag[$i] = $arr_tmp[$i]['tag'];
//    DEBUG && log::write('['.__LINE__.'] USER<'.$mimi.'> real interest list :'.print_r($arr_tag, true), 'debug');
    return TRUE;
}

function get_user_score($mimi, &$arr_score) {
    global $g_mysql_mifan_client;
    global $g_mysql_basic_client;
    global $g_recommend_conf;

    $rv = _init_mysql_connect($g_mysql_mifan_client, $g_recommend_conf['backend']['mysql_mifan_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_recommend_conf['backend']['mysql_mifan_server'], true), 'error');
        return FALSE;
    }

    $query_sql = 'select * from '.$g_recommend_conf['backend']['mysql_mifan_server']['table'].' where mimi='.$mimi;
//    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');
    $rv = mysqli_query($g_mysql_mifan_client, $query_sql);

    if ($rv->num_rows == 0) {
        return get_user_default_score($mimi, $arr_score, $arr_tag);
    }
    
    while ($row = mysqli_fetch_assoc($rv)) {
        $arr_score[DEFAULT_MOLE] = $row[$g_recommend_conf['tag_name'][DEFAULT_MOLE]];
        $arr_score[DEFAULT_PHOTO] = $row[$g_recommend_conf['tag_name'][DEFAULT_PHOTO]];
        $arr_score[DEFAULT_DRAW] = $row[$g_recommend_conf['tag_name'][DEFAULT_DRAW]];
        $arr_score[DEFAULT_ACG] = $row[$g_recommend_conf['tag_name'][DEFAULT_ACG]];
        $arr_score[DEFAULT_FASHION] = $row[$g_recommend_conf['tag_name'][DEFAULT_FASHION]];
        $arr_score[DEFAULT_LIFE] = $row[$g_recommend_conf['tag_name'][DEFAULT_LIFE]];
        $arr_score[DEFAULT_GAME] = $row[$g_recommend_conf['tag_name'][DEFAULT_GAME]];
        $arr_score[DEFAULT_DESIGN] = $row[$g_recommend_conf['tag_name'][DEFAULT_DESIGN]];
        $arr_score[DEFAULT_FILM] = $row[$g_recommend_conf['tag_name'][DEFAULT_FILM]];
        $arr_score[DEFAULT_VEDIO] = $row[$g_recommend_conf['tag_name'][DEFAULT_VEDIO]];
        $arr_score[DEFAULT_JOURNEY] = $row[$g_recommend_conf['tag_name'][DEFAULT_JOURNEY]];
        $arr_score[DEFAULT_LITERATURE] = $row[$g_recommend_conf['tag_name'][DEFAULT_LITERATURE]];
        $arr_score[DEFAULT_COMMON] = $row[$g_recommend_conf['tag_name'][DEFAULT_COMMON]];
        $arr_score[DEFAULT_ATTENTION] = $row[$g_recommend_conf['tag_name'][DEFAULT_ATTENTION]];
        $arr_score[DEFAULT_PERSONAL] = $row[$g_recommend_conf['tag_name'][DEFAULT_PERSONAL]];
        $arr_score[DEFAULT_HOBBY] = $row[$g_recommend_conf['tag_name'][DEFAULT_HOBBY]];
        $arr_score[DEFAULT_OFFICIAL] = $row[$g_recommend_conf['tag_name'][DEFAULT_OFFICIAL]];
    }

    return TRUE;
}

function get_user_all_tag($author, $uid, &$arr_tag) {
    $arr_score = array();
    get_user_score($uid, $arr_score);
    get_user_real_tag($uid, $arr_score, $arr_tag);

    global $g_redis_cache_client;
    global $g_mysql_basic_client;
    global $g_recommend_conf;
    //查看是否是我的关注对象
    $rv = _init_redis_connect($g_redis_cache_client, $g_recommend_conf['backend']['redis_cache_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_recommend_conf['backend']['redis_cache_server'], true), 'error');
        return FALSE;
    }
    $rv = $g_redis_cache_client->zScore("feeds:$uid:list", $author);
    if ($rv != false) {
        $arr_tag[] = DEFAULT_ATTENTION;
    }

    //查看作者的角色信息
    $rv = _init_mysql_connect($g_mysql_basic_client, $g_recommend_conf['backend']['mysql_basic_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_recommend_conf['backend']['mysql_basic_server'], true), 'error');
        return FALSE;
    }

    $query_sql = 'select * from '.$g_recommend_conf['backend']['mysql_basic_server']['role_table'].' where account='.$uid;
//DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');

    $rv = mysqli_query($g_mysql_basic_client, $query_sql);

    if ($rv->num_rows == 0) {
        log::write('['.__LINE__.'] USER<'.$uid.'> QUERY<'.$query_sql.'> has no role', 'error');
        return FALSE;
    }
    
    while ($row = mysqli_fetch_assoc($rv)) {
        if ($row[$g_recommend_conf['tag_name'][DEFAULT_OFFICIAL]] == 1) { 
            $arr_tag[] = DEFAULT_OFFICIAL;
        } else if ($row[$g_recommend_conf['tag_name'][DEFAULT_HOBBY]] == 1) { 
            $arr_tag[] = DEFAULT_HOBBY;
        } else if ($row['person_identity'] == 1) { 
            $arr_tag[] = DEFAULT_PERSONAL;
        } else
            $arr_tag[] = DEFAULT_COMMON;
    }
    
    //DEBUG && log::write('['.__LINE__.'] USER<'.$uid.'> TAG :'.print_r($arr_tag, true), 'debug');
    return TRUE;
}

function query_user_score_by_tag($author, $mimi, $arr_score) {
    $score = 0;
    $arr_tag = array();
    get_user_all_tag($author, $mimi, $arr_tag);
    foreach ($arr_tag as $tag) {
        $score += $arr_score[$tag];
    }

    return $score;
}
?>
