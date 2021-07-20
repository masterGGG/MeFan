#ifndef _BUF_PROCESSOR_H
#define _BUF_PROCESSOR_H
#include <stdio.h>
#include "proto.h"
#include "hiredis.h"
#include <string>
#include <unordered_map>
using std::string;
using std::unordered_map;
enum {
    //用户的推荐池已经读完了
    POOL_UNREADABLE = 0,
    //用户的推荐池还有未读feed
    POOL_READABLE 
};
enum {
    //推荐池还未到达阈值，可以直接读未读的feed
    POOL_NO_UPDATE = 0,
    //达到阈值后，触发一次全量更新的通知
    POOL_TRI_UPDATE,
    //全量更新操作，清空已读的feed
    POOL_UPDATING
};
typedef struct PoolStat {
//    uint16_t count;         //用户拉取池的feed总数
    uint8_t  hotReadable;      //池中数据是否全部读完
    uint8_t  hotUpdatable;     //
    uint16_t  hotCount;
    uint16_t  hotIndex;
    uint8_t  likeReadable;      //池中数据是否全部读完
    uint8_t  likeUpdatable;     //
    uint16_t  likeCount;
    uint16_t  likeIndex;
} pool_info_t;

typedef std::unordered_map<std::string, std::string> PoolConfig;
//typedef std::unordered_map<char *, char *> PoolConfig;
#define CONF_CAPACITY      "pool_capacity"
#define CONF_LIKERATE      "pool_like_rate"
#define CONF_ALARMRATE     "pool_alarm_rate"
#define CONF_KAFKA_BROKER  "alarm_broker"
#define CONF_KAFKA_TOPIC   "alarm_topic"
#define CONF_KAFKA_OPTION  "alarm_option"
#define CONF_KAFKA_SCRIPT  "alarm_script"

class PoolProcessor 
{
	proto_header *in_header;
	char *in_body;
	int in_len;

	proto_header *out_header;
	char *out_body;
    redisContext *ctx;

    PoolConfig config_;
    const int capacity_;
    const int rate_;
    const int alarm_;
public:
	PoolProcessor(PoolConfig &&config, int capacity = 500, int rate = 80, int alarm = 20): in_len(0), config_(config), capacity_(capacity), rate_(rate), alarm_(alarm) {
//        in_len = 0;
    }

	int Init(const char *buf, const int len, char* out_buf, redisContext *ctx);
    
    void PullData(char* out_buf, const unsigned int max_len);
    void PushData(char* out_buf, const unsigned int max_len);
	void GetError(int status);
private:
    void NotifyProcess(int uid, int count, int index);
	bool ReplyCode(int status);
	bool ReplyInteger(int status, const redisReply &r);
	bool ReplyString(int status, const redisReply &r);
	bool ReplyArray(int status, const redisReply &r);
    bool GetPoolStat(const uint32_t userId, pool_info_t &stat);
    bool SetPoolStat(const uint32_t userId, pool_info_t &stat);
    bool CheckLikePoolStat(pool_info_t &info, int likeNeed, int userId);
    bool CheckHotPoolStat(pool_info_t &info, int hotNeed, int userId);
};
#endif
