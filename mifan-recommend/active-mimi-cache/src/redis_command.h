/*************************************************************************
    > File Name: redis_command.h
    > Author: ian
    > Mail: ian@taomee.com 
    > Created Time: Wed Dec 26 01:23:06 2018
 ************************************************************************/
#ifndef __IMG_CHECK_COMMAND__
#define __IMG_CHECK_COMMAND__

#define IMG_CHECK_QUEUE  "image:check:queue"
#define IMG_RETRY_QUEUE  "image:retry:queue"

#define IMG_CHECK_PUSH   "LPUSH " IMG_CHECK_QUEUE " %s"

#define IMG_CHECK_POP    "RPOP " IMG_CHECK_QUEUE

#define IMG_RETRY_PUSH   "LPUSH " IMG_RETRY_QUEUE " %s"

#define IMG_RETRY_POP    "RPOP " IMG_RETRY_QUEUE

#endif

