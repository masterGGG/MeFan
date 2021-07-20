CREATE TABLE t_user_score_statistic (
mimi int(11) unsigned NOT NULL COMMENT '米米号',
mole decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-摩尔庄园',
photography decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-摄影',
draw decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-绘画',
acg decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-二次元',
fashion decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-时尚',
life decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-生活',
game decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-游戏',
design decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-设计',
film decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-影视',
vedio decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-视频',
journey decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-旅行',
literature decimal(10,2) unsigned NOT NULL COMMENT '兴趣标签-文学',
common decimal(10,2) unsigned NOT NULL COMMENT '用户标签-普通用户',
attention decimal(10,2) unsigned NOT NULL COMMENT '用户标签-关注',
personal decimal(10,2) unsigned NOT NULL COMMENT '用户标签-达人',
hobby decimal(10,2) unsigned NOT NULL COMMENT '用户标签-大v',
official decimal(10,2) unsigned NOT NULL COMMENT '用户标签-官方',
PRIMARY KEY (mimi)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE t_article_score_statistic (
article_id int(11) unsigned NOT NULL COMMENT '帖子号',
timestamp int(11)  unsigned not null COMMENT '时间戳',
score int(11)  unsigned not null COMMENT '帖子热度',
feed_id varchar(50) not null COMMENT '唯一标识码',
PRIMARY KEY (article_id),
INDEX `timer` (timestamp)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
