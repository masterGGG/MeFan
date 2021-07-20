<?php
//基础文件路径
if (!defined('DS'))  		define('DS', 			DIRECTORY_SEPARATOR) ;
if (!defined('ROOT')) 		define('ROOT', 			dirname(dirname(__FILE__)) . DS);
if (!defined('LIB_DIR')) 	define('LIB_DIR',			ROOT. 'lib'. DS);
if (!defined('HANDLER_DIR'))	define('HANDLER_DIR',	ROOT. 'handler' . DS);
if (!defined('CONFIG_DIR')) 	define('CONFIG_DIR',	ROOT. 'config' . DS);

// 日志相关的常量设置
if (!defined('LOG_PATH')) define('LOG_PATH', ROOT . 'log'. DS );
if (!defined('LOG_FILE_SIZE')) define('LOG_FILE_SIZE', 1048576 * 5); // 5M

// 新文章推荐计算进程数量
if (!defined('PROCESS_NEWS_NUM')) define('PROCESS_NEWS_NUM', '0');
if (!defined('NEWS_TOPIC_NAME')) define('NEWS_TOPIC_NAME', 'mifan-note-new-article');
if (!defined('NEWS_DEBUG')) define('NEWS_DEBUG', './kafka_news.debug');
if (!defined('NEWS_ERROR')) define('NEWS_ERROR', './kafka_news.error');
if (!defined('NEWS_GROUP')) define('NEWS_GROUP', 'mKConsumerCroupHistory');

if (!defined('DEL_TOPIC_NAME')) define('DEL_TOPIC_NAME', 'mifan-note-new-article');
// 全量推荐进程数量
if (!defined('PROCESS_HISTORY_NUM')) define('PROCESS_HISTORY_NUM', '1');
if (!defined('HISTORY_GROUP')) define('HISTORY_GROUP', 'mKHistoryCroup-');
if (!defined('HISTORY_DEBUG')) define('HISTORY_DEBUG', './kafka_history.debug');
if (!defined('HISTORY_ERROR')) define('HISTORY_ERROR', './kafka_history.error');
if (!defined('HISTORY_TOPIC_NAME')) define('HISTORY_TOPIC_NAME', 'mifan-note-recommend-alarm');

define('TIMEOUT', 5);

define('DEBUG', true);

/**
 * 以下为消息队列相关配置
 */
define('MKAFKA_BROKER_IP', '10.1.1.187');

/**
 * 以下为backend server相关配置
 */
// feedid相关配置
define('FEEDID_LEN', 22);
define('OUTBOX_OPCODE', 0xFFF4);
define('MAX_OUTBOX_REQUEST_COUNT', 100);
define('OUTBOX_RESPONSE_HEAD_LEN', 6);
$g_recommend_conf['backend'] = array(
    'tag_server'    => '0.0.0.0:22080',
    'outbox_server' => '0.0.0.0:43321',
    'friend_server' => '0.0.0.0:21145',
    'storage_server'=> '0.0.0.0:2222',
    'time_begin_server'=> '0.0.0.0:12006',
    'time_end_server'=> '0.0.0.0:12007',
    'recommend_server'=> '0.0.0.0:8088',
    'redis_server'=> array(
        'ip'    => '0.0.0.0',
        'port'  => 7021,
        'pwd'   => 'ta0mee@123',
        ),
    'redis_cache_server'=> array(
        'ip'    => '0.0.0.0',
        'port'  => 7021,
        'pwd'   => 'ta0mee@123',
        'cache0' => 'mifan:hotcache:0:24',
        'cache0cnt' => 50,
        'cache1' => 'mifan:hotcache:24:72',
        'cache1cnt' => 200,
        'cache2' => 'mifan:hotcache:72:0',
        'cache2cnt' => 500,
        ),
    'mysql_mifan_server' => array(
        'host'      => '10.1.1.65',
        'user'      => 'elvis',
        'pwd'       => 'elvispwd',
        'database'  => 'db_newsfeed_recommend',
        'table'     => 't_user_score_statistic',
        ),
    'mysql_basic_server' => array(
        'host'      => '10.1.1.102',
        'user'      => 'asadmin',
        'pwd'       => 'asw@123',
        'database'  => 'mifan',
        'tag_table' => 'user_interest_tag',
        'role_table'=> 'user_verify',
        'like_table'=> 'article_liker',
        'comment_table'=> 'article_comment',
        'article_table'=> 'article',
        ),
);
/**
 * 以下为推荐算法相关配置
 */
$g_recommend_conf['point']['article'] = array(
    'time'  => 3,       //文章时间初始分值
);

$g_recommend_conf['point']['attention'] = array(
    'default'     => 3,       //关注度的初始分值
    'like'        => 1,       //我对作者的点赞数分值
    'commenting'  => 2,       //我评论作者的分值
    'commented'   => 3,       //作者评论我的分值
);

$g_recommend_conf['point']['role'] = array(
    'common'    =>  3600,  //普通账号分值
    'hobby'     =>  7200,  //达人账号分值
    'person_identity'  =>  9600,  //大V 账号分值
    'official'  =>  12000, //官方账号分值
);

//用户关联对应的分值
$g_recommend_conf['point']['relation'] = array(
    0   =>  1,  //无关联分值
    1   =>  2,  //粉丝分值
    2   =>  4,  //关注分值
    3   =>  10, //互关分值
);

/*
 * Config copy from recommend-score
 */
define('DEFAULT_TAG_UNINTEREST',      15);
define('DEFAULT_TAG_INTEREST',        50);

define('DEFAULT_MOLE',       1);
define('DEFAULT_PHOTO',      2);
define('DEFAULT_DRAW',       3);
define('DEFAULT_ACG',        4);
define('DEFAULT_FASHION',    5);
define('DEFAULT_LIFE',       6);
define('DEFAULT_GAME',       7);
define('DEFAULT_DESIGN',     8);
define('DEFAULT_FILM',       9);
define('DEFAULT_VEDIO',      10);
define('DEFAULT_JOURNEY',    11);
define('DEFAULT_LITERATURE', 12);
define('DEFAULT_COMMON',     13);
define('DEFAULT_ATTENTION',  14);
define('DEFAULT_PERSONAL',   15);
define('DEFAULT_HOBBY',      16);
define('DEFAULT_OFFICIAL',   17);
$g_recommend_conf['tag_name'] = array(
    DEFAULT_MOLE        =>      'mole',
    DEFAULT_PHOTO       =>      'photography',
    DEFAULT_DRAW        =>      'draw',
    DEFAULT_ACG         =>      'acg',
    DEFAULT_FASHION     =>      'fashion',
    DEFAULT_LIFE        =>      'life',
    DEFAULT_GAME        =>      'game',
    DEFAULT_DESIGN      =>      'design',
    DEFAULT_FILM        =>      'film',
    DEFAULT_VEDIO       =>      'vedio',
    DEFAULT_JOURNEY     =>      'journey',
    DEFAULT_LITERATURE  =>      'literature',
    DEFAULT_COMMON      =>      'common', 
    DEFAULT_ATTENTION   =>      'attention',
    DEFAULT_PERSONAL    =>      'personal',
    DEFAULT_HOBBY       =>      'hobby',
    DEFAULT_OFFICIAL    =>      'official',
);
?>
