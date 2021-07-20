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

// 全量推荐进程数量
if (!defined('PROCESS_NUM')) define('PROCESS_NUM', '1');
if (!defined('HISTORY_GROUP')) define('HISTORY_GROUP', 'mKScoreCroup');
if (!defined('HISTORY_DEBUG')) define('HISTORY_DEBUG', './kafka_history.debug');
if (!defined('HISTORY_ERROR')) define('HISTORY_ERROR', './kafka_history.error');
if (!defined('HISTORY_TOPIC_NAME')) define('HISTORY_TOPIC_NAME', 'mifan-note-update-score');

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
$g_score_conf['backend'] = array(
    'redis_relate_server'=> array(
        'ip'    => '0.0.0.0',
        'port'  => 7021,
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
        'cache_timer' => 'mifan:hotcache:fresh',
        ),
    'mysql_mifan_server' => array(
        'host'      => '10.1.1.65',
        'user'      => 'elvis',
        'pwd'       => 'elvispwd',
        'database'  => 'db_newsfeed_recommend',
        'table'     => 't_user_score_statistic',
        'hot_table'     => 't_article_score_statistic',
        'feed_database'     => 'db_newsfeed_',
        'feed_table'     => 't_newsfeed_',
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
define('SCORE_DAY',                  3600*24);          // array($target_uid)
/**
 * 以下为更新权重相关配置
 */
define('NEWS_LIKER',                   7004);          // array($target_uid)
define('NEWS_COMMENT',                   7005);
define('NEWS_FANS',                      7006);          // 关注协议
define('CLICK_ARTICLE',                 101);          // 点击阅读文章协议
$g_score_conf['support'] = array(
    CLICK_ARTICLE   => true,
    NEWS_LIKER      => true,
    NEWS_COMMENT    => true,
);

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

define('DEFAULT_TAG_UNINTEREST',      15);
define('DEFAULT_TAG_INTEREST',        50);
$g_score_conf['tag_name'] = array(
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
$g_score_conf['score'] = array(
    'article' => array(
        CLICK_ARTICLE   => 2,
        NEWS_LIKER      => 4,
        NEWS_COMMENT    => 10,
    ),
    'interest_rate' => array(
        CLICK_ARTICLE   => 50*0.02,
        NEWS_LIKER      => 50*0.05,
        NEWS_COMMENT    => 50*0.08,
    ),
    'uninterest_rate' => array(
        CLICK_ARTICLE   => 15*0.1,
        NEWS_LIKER      => 15*0.25,
        NEWS_COMMENT    => 15*0.4,
    ),
    'role_rate' => array(
        DEFAULT_COMMON => array (
            CLICK_ARTICLE   => 10*0.1,
            NEWS_LIKER      => 10*0.25,
            NEWS_COMMENT    => 10*0.4,
        ),
        DEFAULT_ATTENTION => array (
            CLICK_ARTICLE   => 30*0.03,
            NEWS_LIKER      => 30*0.06,
            NEWS_COMMENT    => 30*0.12,
        ),
        DEFAULT_PERSONAL => array (
            CLICK_ARTICLE   => 20*0.06,
            NEWS_LIKER      => 20*0.15,
            NEWS_COMMENT    => 20*0.24,
        ),
        DEFAULT_HOBBY => array (
            CLICK_ARTICLE   => 25*0.04,
            NEWS_LIKER      => 25*0.1,
            NEWS_COMMENT    => 25*0.16,
        ),
        DEFAULT_OFFICIAL => array (
            CLICK_ARTICLE   => 35*0.02,
            NEWS_LIKER      => 35*0.05,
            NEWS_COMMENT    => 35*0.08,
        ),
    ),
);
/*
$g_score_conf['operator'] = array(
    CLICK_ARTICLE   => 'mifan_cllick_article',
    NEWS_LIKER      => 'mifan_news_liker',
    NEWS_COMMENT    => 'mifan_news_comment',
);
*/
?>
