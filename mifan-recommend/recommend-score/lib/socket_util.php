<?php
function _init_mysql_connect(&$cli, $arr_conf) {
    if (empty($cli) || !$cli) {
        $cli = mysqli_connect($arr_conf['host'],
                $arr_conf['user'],
                $arr_conf['pwd'],
                $arr_conf['database']);
        if (!$cli) {
            return FALSE;
        }
    }
    return TRUE;
}

function _init_redis_connect(&$cli, $arr_conf) {
    if (empty($cli) || !$cli)
    $cli = new Redis();
    $cli->connect($arr_conf['ip'], $arr_conf['port']);
    $cli->auth($arr_conf['pwd']);
    return $cli;
}

