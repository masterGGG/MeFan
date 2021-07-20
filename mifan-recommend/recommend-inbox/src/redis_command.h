/*************************************************************************
    > File Name: redis_command.h
    > Author: ian
    > Mail: ian@taomee.com 
    > Created Time: Wed Dec 26 01:23:06 2018
 ************************************************************************/
#ifndef __FEEDS_COMMAND__
#define __FEEDS_COMMAND__

#define PRFIX                  "feeds:recommend:"
#define UID_PREFIX             PRFIX "%u"

#define H_POOL                 UID_PREFIX ":hot"
#define L_POOL                 UID_PREFIX ":like"

//ZREVRANGE key 10 20
#define H_POOL_GET             "ZREVRANGE " H_POOL " %d %d"
#define L_POOL_GET             "ZREVRANGE " L_POOL " %d %d"
#define H_POOL_INSERT          "ZADD " H_POOL
#define L_POOL_INSERT          "ZADD " L_POOL
#define H_POOL_FLUSH           "ZREMRANGEBYRANK " H_POOL " %d %d"
#define L_POOL_FLUSH           "ZREMRANGEBYRANK " L_POOL " %d %d"

#define POOL_INFO              UID_PREFIX ":stat"
#define POOL_INFO_GETALL       "HGETALL " POOL_INFO
#define POOL_INFO_SETALL       "HMSET " POOL_INFO " hotReadable %u hotUpdatable %u hotCount %u hotIndex %u likeReadable %u likeUpdatable %u likeCount %u likeIndex %u"
#define POOL_INFO_EXIST        "EXISTS " POOL_INFO

#endif

