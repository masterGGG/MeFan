<?php
//require_once('./lib/netclient_class.php');
global $g_redis_cache_client;
global $g_mysql_mifan_client;
global $g_mysql_basic_client;
global $g_new_score;
function get_user_default_tag($mimi, &$arr_tag) {
    global $g_score_conf;
    global $g_mysql_basic_client;
    $rv = _init_mysql_connect($g_mysql_basic_client, $g_score_conf['backend']['mysql_basic_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_basic_server'], true), 'error');
        return FALSE;
    }

    $query_sql = 'select tag_id from '.$g_score_conf['backend']['mysql_basic_server']['tag_table'].' where account='.$mimi;
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');
    $rv = mysqli_query($g_mysql_basic_client, $query_sql);

    if ($rv->num_rows == 0) {
        log::write('['.__LINE__.'] USER<'.$mimi.'> has none interest', 'warn');
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
   
//    if (FALSE == get_user_default_tag($mimi, $arr_tag)) 
//        return FALSE;
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

    $arr_tmp = array();
    for ($i = DEFAULT_MOLE; $i <= DEFAULT_LITERATURE; ++$i) 
        $arr_tmp[] = array('tag'=>$i,'score'=>$arr_score[$i]);

    usort($arr_tmp, 'tag_score_cmp');
    for($i = 0; $i != count($arr_tag); ++$i)
        $arr_tag[$i] = $arr_tmp[$i]['tag'];
//    DEBUG && log::write('['.__LINE__.'] USER<'.$mimi.'> real interest list :'.print_r($arr_tag, true), 'debug');
    return TRUE;
}

function update_user_role_score($mimi, $author, $cmd, &$arr_score) {
    global $g_redis_cache_client;
    global $g_mysql_basic_client;
    global $g_score_conf;
    //查看是否是我的关注对象
    $rv = _init_redis_connect($g_redis_cache_client, $g_score_conf['backend']['redis_relate_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['redis_relate_server'], true), 'error');
        return FALSE;
    }
    $rv = $g_redis_cache_client->zScore("feeds:$mimi:list", $author);
    if ($rv != false) {
        $arr_score[DEFAULT_ATTENTION] += $g_score_conf['score']['role_rate'][DEFAULT_ATTENTION][$cmd];
    }

    //查看作者的角色信息
    $rv = _init_mysql_connect($g_mysql_basic_client, $g_score_conf['backend']['mysql_basic_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_basic_server'], true), 'error');
        return FALSE;
    }

    $query_sql = 'select * from '.$g_score_conf['backend']['mysql_basic_server']['role_table'].' where account='.$author;
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');

    $rv = mysqli_query($g_mysql_basic_client, $query_sql);

    if ($rv->num_rows == 0) {
        log::write('['.__LINE__.'] USER<'.$author.'> has no role', 'error');
        return FALSE;
    }
    
    while ($row = mysqli_fetch_assoc($rv)) {
        if ($row[$g_score_conf['tag_name'][DEFAULT_OFFICIAL]] == 1) { 
            $arr_score[DEFAULT_OFFICIAL] += $g_score_conf['score']['role_rate'][DEFAULT_OFFICIAL][$cmd];
        } else if ($row[$g_score_conf['tag_name'][DEFAULT_HOBBY]] == 1) { 
            $arr_score[DEFAULT_HOBBY] += $g_score_conf['score']['role_rate'][DEFAULT_HOBBY][$cmd];
        } else if ($row['person_identity'] == 1) { 
            $arr_score[DEFAULT_PERSONAL] += $g_score_conf['score']['role_rate'][DEFAULT_PERSONAL][$cmd];
        } else
            $arr_score[DEFAULT_COMMON] += $g_score_conf['score']['role_rate'][DEFAULT_COMMON][$cmd];
    
        DEBUG && log::write('['.__LINE__.'] USER<'.$author.'> role list :'.print_r($row, true), 'debug');
    }
    
    return TRUE;
}

function get_user_score($mimi, &$arr_score, &$arr_tag, $flag) {
    global $g_mysql_mifan_client;
    global $g_mysql_basic_client;
    global $g_score_conf;
    global $g_new_score;

    $rv = _init_mysql_connect($g_mysql_mifan_client, $g_score_conf['backend']['mysql_mifan_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_mifan_server'], true), 'error');
        return FALSE;
    }

    $query_sql = 'select * from '.$g_score_conf['backend']['mysql_mifan_server']['table'].' where mimi='.$mimi;
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');
    $rv = mysqli_query($g_mysql_mifan_client, $query_sql);

    if ($rv->num_rows == 0) {
        if ($flag === true)
            $g_new_score = true;
        return get_user_default_score($mimi, $arr_score, $arr_tag);
    }
    if ($flag === true)
        $g_new_score = false;
    
    while ($row = mysqli_fetch_assoc($rv)) {
        $arr_score[DEFAULT_MOLE] = $row[$g_score_conf['tag_name'][DEFAULT_MOLE]];
        $arr_score[DEFAULT_PHOTO] = $row[$g_score_conf['tag_name'][DEFAULT_PHOTO]];
        $arr_score[DEFAULT_DRAW] = $row[$g_score_conf['tag_name'][DEFAULT_DRAW]];
        $arr_score[DEFAULT_ACG] = $row[$g_score_conf['tag_name'][DEFAULT_ACG]];
        $arr_score[DEFAULT_FASHION] = $row[$g_score_conf['tag_name'][DEFAULT_FASHION]];
        $arr_score[DEFAULT_LIFE] = $row[$g_score_conf['tag_name'][DEFAULT_LIFE]];
        $arr_score[DEFAULT_GAME] = $row[$g_score_conf['tag_name'][DEFAULT_GAME]];
        $arr_score[DEFAULT_DESIGN] = $row[$g_score_conf['tag_name'][DEFAULT_DESIGN]];
        $arr_score[DEFAULT_FILM] = $row[$g_score_conf['tag_name'][DEFAULT_FILM]];
        $arr_score[DEFAULT_VEDIO] = $row[$g_score_conf['tag_name'][DEFAULT_VEDIO]];
        $arr_score[DEFAULT_JOURNEY] = $row[$g_score_conf['tag_name'][DEFAULT_JOURNEY]];
        $arr_score[DEFAULT_LITERATURE] = $row[$g_score_conf['tag_name'][DEFAULT_LITERATURE]];
        $arr_score[DEFAULT_COMMON] = $row[$g_score_conf['tag_name'][DEFAULT_COMMON]];
        $arr_score[DEFAULT_ATTENTION] = $row[$g_score_conf['tag_name'][DEFAULT_ATTENTION]];
        $arr_score[DEFAULT_PERSONAL] = $row[$g_score_conf['tag_name'][DEFAULT_PERSONAL]];
        $arr_score[DEFAULT_HOBBY] = $row[$g_score_conf['tag_name'][DEFAULT_HOBBY]];
        $arr_score[DEFAULT_OFFICIAL] = $row[$g_score_conf['tag_name'][DEFAULT_OFFICIAL]];
    }

//    if (FALSE == get_user_real_tag($mimi, $arr_score, $arr_tag)) 
//        return FALSE;
    get_user_real_tag($mimi, $arr_score, $arr_tag);

    return TRUE;
}

function update_user_score($mimi, $arr_score) {
    global $g_mysql_mifan_client;
    global $g_score_conf;

    $rv = _init_mysql_connect($g_mysql_mifan_client, $g_score_conf['backend']['mysql_mifan_server']);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Conn '.print_r($g_score_conf['backend']['mysql_mifan_server'], true), 'error');
        return FALSE;
    }

    global $g_new_score;
    if ($g_new_score === true)
        $query_sql = "insert into {$g_score_conf['backend']['mysql_mifan_server']['table']} values ({$mimi}, ".
            "{$arr_score[DEFAULT_MOLE]}, ".
            "{$arr_score[DEFAULT_PHOTO]},".
            "{$arr_score[DEFAULT_DRAW]},".
            "{$arr_score[DEFAULT_ACG]},".
            "{$arr_score[DEFAULT_FASHION]},".
            "{$arr_score[DEFAULT_LIFE]},".
            "{$arr_score[DEFAULT_GAME]},".
            "{$arr_score[DEFAULT_DESIGN]},".
            "{$arr_score[DEFAULT_FILM]},".
            "{$arr_score[DEFAULT_VEDIO]},".
            "{$arr_score[DEFAULT_JOURNEY]},".
            "{$arr_score[DEFAULT_LITERATURE]},".
            "{$arr_score[DEFAULT_COMMON]},".
            "{$arr_score[DEFAULT_ATTENTION]},".
            "{$arr_score[DEFAULT_PERSONAL]},".
            "{$arr_score[DEFAULT_HOBBY]},".
            "{$arr_score[DEFAULT_OFFICIAL]})";
    else
        $query_sql = "update {$g_score_conf['backend']['mysql_mifan_server']['table']} set {$g_score_conf['tag_name'][DEFAULT_MOLE]}={$arr_score[DEFAULT_MOLE]}, ".
            "{$g_score_conf['tag_name'][DEFAULT_PHOTO]}={$arr_score[DEFAULT_PHOTO]},".
            "{$g_score_conf['tag_name'][DEFAULT_DRAW]}={$arr_score[DEFAULT_DRAW]},".
            "{$g_score_conf['tag_name'][DEFAULT_ACG]}={$arr_score[DEFAULT_ACG]},".
            "{$g_score_conf['tag_name'][DEFAULT_FASHION]}={$arr_score[DEFAULT_FASHION]},".
            "{$g_score_conf['tag_name'][DEFAULT_LIFE]}={$arr_score[DEFAULT_LIFE]},".
            "{$g_score_conf['tag_name'][DEFAULT_GAME]}={$arr_score[DEFAULT_GAME]},".
            "{$g_score_conf['tag_name'][DEFAULT_DESIGN]}={$arr_score[DEFAULT_DESIGN]},".
            "{$g_score_conf['tag_name'][DEFAULT_FILM]}={$arr_score[DEFAULT_FILM]},".
            "{$g_score_conf['tag_name'][DEFAULT_VEDIO]}={$arr_score[DEFAULT_VEDIO]},".
            "{$g_score_conf['tag_name'][DEFAULT_JOURNEY]}={$arr_score[DEFAULT_JOURNEY]},".
            "{$g_score_conf['tag_name'][DEFAULT_LITERATURE]}={$arr_score[DEFAULT_LITERATURE]},".
            "{$g_score_conf['tag_name'][DEFAULT_COMMON]}={$arr_score[DEFAULT_COMMON]},".
            "{$g_score_conf['tag_name'][DEFAULT_ATTENTION]}={$arr_score[DEFAULT_ATTENTION]},".
            "{$g_score_conf['tag_name'][DEFAULT_PERSONAL]}={$arr_score[DEFAULT_PERSONAL]},".
            "{$g_score_conf['tag_name'][DEFAULT_HOBBY]}={$arr_score[DEFAULT_HOBBY]},".
            "{$g_score_conf['tag_name'][DEFAULT_OFFICIAL]}={$arr_score[DEFAULT_OFFICIAL]} ".
            "where mimi=$mimi";
    DEBUG && log::write('['.__LINE__.'] SQL<'.$query_sql.'>', 'debug');
    $rv = mysqli_query($g_mysql_mifan_client, $query_sql);

    return $rv;
}

function do_score_process($mimi, $cmd, $article, $author) {
    global $g_score_conf;
    $arr_my_score = array();
    $arr_my_tag = array();
    $arr_author_score = array();
    $arr_author_tag = array();
    $arr_role = array();

    if (!array_key_exists($cmd, $g_score_conf['support'])) {
        log::write('['.__LINE__.'] USER<'.$mimi.'> Invalid cmd<'.$cmd.'>', 'error');
        return;
    }
    //获取我的权重值和兴趣列表
    $rv = get_user_score($mimi, $arr_my_score, $arr_my_tag, true);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Can not get '.$mimi.' score from DB', 'error');
        return ;
    }
    
    //获取作者的权重值和兴趣列表
    $rv = get_user_score($author, $arr_author_score, $arr_author_tag, false);
    if ($rv === FALSE) {
        log::write('['.__LINE__.'] Can not get '.$author.' score from DB', 'error');
        return ;
    }
 
    $rv = update_user_role_score($mimi, $author, $cmd, $arr_my_score);
    if ($rv === FALSE) {
//        log::write('['.__LINE__.'] Can not get '.$author.' score from DB', 'warn');
        return ;
    }

    $arr_tag = array();
    $is_interest = false;
    if (count($arr_author_tag) == 0) {
        log::write('['.__LINE__.'] Author tag cnt<'.count($arr_author_tag).'>', 'error');
        $rv = update_user_score($mimi, $arr_my_score);
        if ($rv === FALSE) 
            log::write('['.__LINE__.'] USER<'.$author.'> score<'.print_r($arr_my_score, true).'> to DB failed', 'error');
        return ;
    } else if (count($arr_my_tag) != 0) {
    //判断文章是否是我感兴趣的以及兴趣度
        foreach ($arr_author_tag as $tag) {
            if (in_array($tag, $arr_my_tag)) {
                $is_interest = true;
                $arr_tag[] = $tag;
            }
        }
    }

    if (false === $is_interest) {
        $arr_tag = $arr_author_tag;
        foreach ($arr_tag as $tag) {
            $arr_my_score[$tag] += $g_score_conf['score']['uninterest_rate'][$cmd];
        }
    } else {
        foreach ($arr_tag as $tag) {
            $arr_my_score[$tag] += $g_score_conf['score']['interest_rate'][$cmd];
        }
    }
//    DEBUG && log::write('['.__LINE__.'] tag '.print_r($arr_tag, true).'score result'.print_r($arr_my_score, true), 'debug');
    $rv = update_user_score($mimi, $arr_my_score);
    if ($rv === FALSE) 
        log::write('['.__LINE__.'] USER<'.$author.'> score<'.print_r($arr_my_score, true).'> to DB failed', 'error');
    return;
}

?>
