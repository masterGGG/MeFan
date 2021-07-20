#ifndef _BUF_PROCESSOR_H
#define _BUF_PROCESSOR_H
#include <stdio.h>
#include "proto.h"
#include "hiredis.h"

class BufProcessor 
{
	proto_header *in_header;
	char *in_body;
	int in_len;

	proto_header *out_header;
	char *out_body;
	//char out_buf[MAX_BUF_LEN];
	//int out_len;
    redisContext *ctx;
public:
	BufProcessor():in_len(0) {}

	int Init(const char *buf, const int len, char* out_buf, redisContext *ctx);
	void GetError(int status);
	bool CheckZSETReply(int status, const redisReply &r);
	bool CheckHashMapReply(int status, const redisReply &r);
	bool CheckHashMapDelReply(int status, const redisReply &r);
    
    void ImgPush(char* out_buf, const unsigned int max_len);
    void ImgPop(char* out_buf, const unsigned int max_len);
};

#endif
