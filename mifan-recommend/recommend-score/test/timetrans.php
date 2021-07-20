<?php
/*
$now_ = (int)gettimeofday(true);
$today_ = $now_ - $now_ % (24 * 3600) - 8 * 3600;
$yesterday_ = $today_ - 72*3600;
var_dump($today_);
var_dump($yesterday_);
return ;
*/
$end = 84*3600;
$cli = mysqli_connect('10.1.1.65', 'elvis', 'elvispwd', 'db_newsfeed_recommend');
for ($i = 0; $i != 100; ++$i) {
    $article = $i + 2140;
    $time = 1565020800 + mt_rand(0, $end);
    $feedid = 'test12345678901234567890=='.$time;
    $score = mt_rand(0, 1000);
    $query_sql = "insert into t_article_score_statistic values ({$article}, {$time}, {$score}, \"{$feedid}\")";
    $rv = mysqli_query($cli, $query_sql);
    var_dump($query_sql);
    var_dump($rv);
}
return ;

$time = $argv[1];
var_dump(strtotime($time));
?>
