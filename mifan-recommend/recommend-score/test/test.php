#! /usr/bin/php
<?php

require dirname(__FILE__) . DIRECTORY_SEPARATOR . 'config' . DIRECTORY_SEPARATOR .  'setup.inc.php';
$arr_role = array();
/*
if ($argc < 4 || $argc > 5)
    echo "example: php test.php articleid author [cmdid]"
 */
global $g_fresh_time;
$cmd = 101;
//var_dump($argv);
$article = (int)$argv[1];
$author = (int)$argv[2];
if ($argc > 3)
    $cmd = (int)$argv[3];
//$rv = get_article_comment($article);
// $rv = init_article_default_score(1234, 1563974260, 12, "hahahha");
// $rv = update_article_info(1234, 914496998, 14);

// $time = (int)gettimeofday(true);
// $g_fresh_time = $time - $time % (24 * 3600) - 8 * 3600;
//var_dump(get_cache_fresh_time());
//$rv = do_article_cache_process();

$rv = do_article_process($cmd, $article, $author);
var_dump($rv);
?>
