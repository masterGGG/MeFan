<?php
// 加载文件
if (!defined('DS')) define('DS', DIRECTORY_SEPARATOR) ;

//基础路径和日志常量
require_once (dirname(__FILE__) . DS . 'define.inc.php');

//用于记录日志
require_once (LIB_DIR . 'log.class.php'); 

//网络连接的基础库
require_once (LIB_DIR . 'socket_util.php');

//更新用户权重的操作
require_once (HANDLER_DIR . 'userscorehandler.php');
//更新帖子热度的操作
require_once (HANDLER_DIR . 'articlescorehandler.php');
?>
