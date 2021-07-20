#include <string.h>
#include <string>
#include "buf_processor.h"
using std::string;
#include "proto.h"
#include "redis_command.h"
#include "log.h"
#ifndef USE_PROTOBUF
#include "imgCheck.pb.h"
#endif

int BufProcessor::Init(const char *buf, const int len, char* out_buf, redisContext *context)
{
	in_header = (proto_header *)buf;
	in_body = (char*)(in_header + 1);
	in_len = len;
	memset(out_buf, 0, MAX_BUF_LEN);

	out_header = (proto_header*)out_buf;
	out_header->pkg_len = 0;
	out_header->seq_num = in_header->seq_num;
	out_header->cmd_id = in_header->cmd_id;
	out_header->status_code = 0;
	out_header->user_id = in_header->user_id;
	out_body = (char*)(out_header + 1);

    ctx = context;
	return in_header->cmd_id;
}

void BufProcessor::GetError(int status)
{
	out_header->pkg_len = sizeof(proto_header);
	out_header->status_code = status;
}

void BufProcessor::ImgPush(char *out_buf, const unsigned int max_len) {
    redisReply *reply;
    mifan::imgPush _req;
    std::string data;
    data.assign(in_body, in_len - sizeof(proto_header));
    _req.ParseFromString(data);

#ifdef DEBUG
    DEBUG_LOG("[%d] retry %d info: %s", __LINE__, _req.retry(), _req.info().c_str());
#endif
    if (false == _req.retry()) 
        redisAppendCommand(ctx, IMG_CHECK_PUSH, _req.info().c_str());
    else 
        redisAppendCommand(ctx, IMG_RETRY_PUSH, _req.info().c_str());
    
    int code;
    if ((code = redisGetReply(ctx, (void **)&reply)) == REDIS_ERR) {
        ERROR_LOG("[REDIS]:Server shutdown %s", reply->str);
        GetError(FEED_REDIS_ERR);
//        return ;
    } else if (reply->type != REDIS_REPLY_INTEGER) {
        ERROR_LOG("[%d] <%s> push to queue list failed %s", __LINE__, _req.info().c_str(), reply->str);
        GetError(FEED_REDIS_ERR);
    }
    
    out_header->pkg_len = sizeof(proto_header);
    if (reply != nullptr)
        freeReplyObject(reply);
}
void BufProcessor::ImgPop(char *out_buf, const unsigned int max_len) {
    redisReply *reply;
    out_header->pkg_len = sizeof(proto_header);
    mifan::imgPopRqst _req;
    std::string data;
    data.assign(in_body, in_len - sizeof(proto_header));
    _req.ParseFromString(data);
    
    if (true == _req.retry())
        redisAppendCommand(ctx, IMG_RETRY_POP);
    else
        redisAppendCommand(ctx, IMG_CHECK_POP);

    if (redisGetReply(ctx, (void **)&reply) == REDIS_ERR) {
        ERROR_LOG("[REDIS]:Server shutdown");
        GetError(FEED_REDIS_ERR);
        return ;
    }
        
    if (reply->type == REDIS_REPLY_STRING) {
        mifan::imgPopResp _res;
            _res.set_info(reply->str);
#ifdef DEBUG
            DEBUG_LOG("[%d] info: %s", __LINE__, _res.info().c_str());
#endif
        std::string _seria{};
        _res.SerializeToString(&_seria);
        memcpy(out_body,_seria.c_str(), _seria.size());
        out_header->pkg_len += _seria.size();
    }
    
    if (reply != nullptr)
        freeReplyObject(reply);
}
