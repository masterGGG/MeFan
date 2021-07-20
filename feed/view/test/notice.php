#!/usr/bin/php
<?php

 error_reporting(E_ALL);
 ini_set('display_errors', '0');
 ini_set('log_errors', '1');
 ini_set('error_log', './log/mysys.log');
 ini_set('memory_limit', '2048M');

set_time_limit(0);
//sleep(5);
function Post($url, $post = null)  
{  
    $context = array();  

    if (is_array($post))  
    {  
        ksort($post);  

        $context['http'] = array  
            (  
             'method' => 'POST',  
             'content' => http_build_query($post, '', '&'),  
            );  
    }  

    return file_get_contents($url, false, stream_context_create($context));  
}  

$data = array  
(  
'type' => 'passive',
'action' => 'notice',
//'cmd_id' => '22004,22005',
//'uid'=>'1227401111');
'uid'=>'1112');

var_dump(Post('http://10.1.1.197:10086/news_feed.php', $data));





