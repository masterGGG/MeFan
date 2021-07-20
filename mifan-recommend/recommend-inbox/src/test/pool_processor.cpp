#include <string.h>
#include <string>
#include "pool_processor.h"
using std::string;
#include "proto.h"
#include "redis_command.h"
#include "log.h"
#include "protoRecommend.pb.h"
#include "rdkafka.h"
int PoolProcessor::Init(const char *buf, const int len, char* out_buf, redisContext *context)
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

void PoolProcessor::GetError(int status)
{
	out_header->pkg_len = sizeof(proto_header);
	out_header->status_code = status;
}

bool PoolProcessor::ReplyCode(const int code) {
    if (code == REDIS_ERR) {
        GetError(FEED_REDIS_ERR);
        return false; 
    }
    
    return true;
}

bool PoolProcessor::ReplyInteger(int code, const redisReply &reply)
{
    if (!ReplyCode(code)) {
        ERROR_LOG("[%d] Redis internal error", __LINE__);
        return false;
    }

    if (reply.type == REDIS_REPLY_INTEGER)
        return true;

    ERROR_LOG("[%d] RedisReply type<%d> is not integer<%d>", __LINE__, reply.type, REDIS_REPLY_INTEGER);
    GetError(FEED_REDIS_ERR);
    return false; 
}

bool PoolProcessor::ReplyString(int code, const redisReply &reply)
{
    if (!ReplyCode(code)) {
        ERROR_LOG("[%d] Redis internal error", __LINE__);
        return false;
    }

    if (reply.type == REDIS_REPLY_STATUS)
    {
        if (0 == strcmp(reply.str,"OK")) {
            return true;
        }
        ERROR_LOG("[%d] Redis command reply status %s", __LINE__, reply.str);
    } else if (reply.type == REDIS_REPLY_STRING)
        return true;

    ERROR_LOG("[%d] RedisReply type<%d> is not string<%d>", __LINE__, reply.type, REDIS_REPLY_STRING);
    GetError(FEED_REDIS_ERR);
    return false; 
}
bool PoolProcessor::ReplyArray(int code, const redisReply &reply)
{
    if (!ReplyCode(code)) {
        ERROR_LOG("[%d] Redis internal error", __LINE__);
        return false;
    }

    if (reply.type == REDIS_REPLY_ARRAY)
        return true;
    
    ERROR_LOG("[%d] RedisReply type<%d> is not array<%d>", __LINE__, reply.type, REDIS_REPLY_ARRAY);
    GetError(FEED_REDIS_ERR);
    return false; 
}

bool PoolProcessor::GetPoolStat(const uint32_t userId, pool_info_t &info) {
    int code = REDIS_OK;
    redisReply *reply;
    redisAppendCommand(ctx, POOL_INFO_EXIST, userId);
    code = redisGetReply(ctx, (void **)&reply);
    if (!ReplyInteger(code, *reply)) {
        ERROR_LOG(POOL_INFO_EXIST, userId);
        GetError(FEED_REDIS_ERR);
        if (reply != nullptr)
            freeReplyObject(reply);
        return false;
    }
    if (reply->integer == 0) {
        //统计信息不存在，则创建初始统计信息
#ifdef DEBUG
        DEBUG_LOG("[%d] No record statistics in DB and init", __LINE__);
#endif
        info.readable = POOL_UNREADABLE;
        info.updatable = POOL_NO_UPDATE;
        info.hotCount = 0;
        info.hotIndex = 0;
        info.likeCount = 0;
        info.likeIndex = 0;
        if (reply != nullptr)
            freeReplyObject(reply);
        return true;
    }

    redisAppendCommand(ctx, POOL_INFO_GETALL, userId);
    code = redisGetReply(ctx, (void **)&reply);
    if (ReplyArray(code, *reply)) {
        if ((int)reply->elements == 12) {
            /*info.count = atoi(reply->element[1]->str);*/
            info.readable = atoi(reply->element[1]->str);
            info.updatable = atoi(reply->element[3]->str);
            info.hotCount = atoi(reply->element[5]->str);
            info.hotIndex = atoi(reply->element[7]->str);
            info.likeCount = atoi(reply->element[9]->str);
            info.likeIndex = atoi(reply->element[11]->str);
            if (reply != nullptr)
                freeReplyObject(reply);
            return true;
        } else {
#ifdef DEBUG
            DEBUG_LOG("[%d] Can get pool statistics from DB <%d>", __LINE__, (int)reply->elements);
            for (int i = 0; i < (int)reply->elements / 2; ++i) 
                DEBUG_LOG("[%d] Pool stat is <%s:%s>", __LINE__, reply->element[2*i]->str, reply->element[2*i + 1]->str);
#endif
        }
    }

    ERROR_LOG("Can not get pool statistics from DB");
    if (reply != nullptr)
        freeReplyObject(reply);
    GetError(FEED_REDIS_ERR);
    return true;
}

bool PoolProcessor::SetPoolStat(const uint32_t userId, pool_info_t &info) {
    int code = REDIS_OK;
    redisReply *reply;
    redisAppendCommand(ctx, POOL_INFO_SETALL, userId, /*info.count,*/
            info.readable, info.updatable,
            info.hotCount, info.hotIndex,
            info.likeCount, info.likeIndex);
    code = redisGetReply(ctx, (void **)&reply);

    if (ReplyString(code, *reply)) {
#ifdef DEBUG
        DEBUG_LOG("[%d] Record pool statistics <%s>", __LINE__, reply->str);
#endif
        if (reply != nullptr)
            freeReplyObject(reply);
        return true;
    }

    ERROR_LOG("[%d] Can not record pool statistics to DB", __LINE__);
    GetError(FEED_REDIS_ERR);
    if (reply != nullptr)
        freeReplyObject(reply);
    return false;
}

static rd_kafka_t *rk = nullptr;
static rd_kafka_topic_t *rkt = nullptr;
/*
每条消息调用一次该回调函数，说明消息是传递成功(rkmessage->err == RD_KAFKA_RESP_ERR_NO_ERROR)
还是传递失败(rkmessage->err != RD_KAFKA_RESP_ERR_NO_ERROR)               
该回调函数由rd_kafka_poll()触发，在应用程序的线程上执行
 */
static void dr_msg_cb(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque){
    if(rkmessage->err)
        ERROR_LOG("[%d] error<%s>", __LINE__, rd_kafka_err2str(rkmessage->err));
    else
        DEBUG_LOG("[%d] alarm succ", __LINE__);
    /* rkmessage被librdkafka自动销毁*/
}
void PoolProcessor::NotifyProcess(int uid, int count, int index) {
#ifdef DEBUG
    DEBUG_LOG("[%d] user<%d> NOtify process do update count<%d> likeCnt<%d>", __LINE__, uid, count, index);
//    DEBUG_LOG("[%d] Kafka ip<%s> port<%s> topic<%s> option<%s>", __LINE__, uid, config_.at(CONF_KAFKA_IP), config_.at(CONF_KAFKA_PORT), config_.at(CONF_KAFKA_TOPIC), config_.at(CONF_KAFKA_OPTION));
//    for (const auto &it : config_) {
//        DEBUG_LOG("[NotifyProcess][%d] kafka config %s:<%s>", __LINE__, it.first.c_str(), it.second.c_str());
//    }
#endif
    char errstr[512];
    if (rk == NULL) {
        auto _conf = rd_kafka_conf_new();
	    rd_kafka_conf_set(_conf, "message.send.max.retries", "3", NULL, 0);
	    rd_kafka_conf_set(_conf, "retry.backoff.ms", "500", NULL, 0);

        rd_kafka_conf_set_dr_msg_cb(_conf, dr_msg_cb);
        rk = rd_kafka_new(RD_KAFKA_PRODUCER, _conf, errstr, sizeof(errstr));
        if (nullptr == rk) {
            ERROR_LOG("[%d] user<%d> count<%d> likeCnt<%d> error<%s>", __LINE__, uid, count, index, errstr);
            return ;
        }
        auto iter = config_.find(CONF_KAFKA_BROKER);
        if (iter != config_.end()) {
#ifdef DEBUG
            DEBUG_LOG("[%d] broker<%s> in config file", __LINE__, iter->second.c_str());
#endif
//            if (rd_kafka_conf_set(_conf, "bootstrap.servers", iter->second.c_str(), 
//                        errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
            if (rd_kafka_brokers_add(rk, iter->second.c_str()) < 1) {
                ERROR_LOG("[%d] user<%d> count<%d> likeCnt<%d> error<%s>", __LINE__, uid, count, index, errstr);
                return ;
            }
        } else {
#ifdef DEBUG
            DEBUG_LOG("[%d] No broker in config file and use default", __LINE__);
#endif
            if (rd_kafka_conf_set(_conf, "bootstrap.servers", "10.1.1.187:9092", 
                        errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
                ERROR_LOG("[%d] user<%d> count<%d> likeCnt<%d> error<%s>", __LINE__, uid, count, index, errstr);
                return ;
            }
        }
    }

    if (nullptr == rkt) {
	    /* Kafka topic configuration */
    	auto topic_conf = rd_kafka_topic_conf_new();
	    rd_kafka_topic_conf_set(topic_conf, "auto.offset.reset", "earliest",
				NULL, 0);
		if (rd_kafka_topic_conf_set(topic_conf,
						    "request.required.acks",
						    0,
                            errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
            ERROR_LOG("[%d] user<%d> count<%d> likeCnt<%d> error<%s>", __LINE__, uid, count, index, errstr);
            return ;
        }

        auto _rkTopic = config_.find(CONF_KAFKA_TOPIC);
        if (_rkTopic != config_.end()) {
#ifdef DEBUG
            DEBUG_LOG("[%d] Topic<%s> in config file", __LINE__, _rkTopic->second.c_str());
#endif
            rkt = rd_kafka_topic_new(rk, _rkTopic->second.c_str(), topic_conf);
        } else 
            rkt = rd_kafka_topic_new(rk, "mifan-note-recommend-alarm", topic_conf);

        if (rkt == nullptr) {
            ERROR_LOG("[%d] user<%d> count<%d> likeCnt<%d> error<Can not use target topic>", __LINE__, uid, count, index);
            return ;
        }
    }

    mifan::noteRecommendAlarm _req;
    std::string _seria{};
    _req.set_userid(uid);
    _req.set_count(count);
    _req.set_index(index);
    if (rd_kafka_produce(rkt, RD_KAFKA_PARTITION_UA, RD_KAFKA_MSG_F_COPY, const_cast<char *>("protobuf"), strlen("protobuf"), _seria.c_str(), _seria.size(), NULL) == -1) {
        ERROR_LOG("[%d] user<%d> count<%d> likeCnt<%d> topic<%s> error<%s>", __LINE__, uid, count, index, 
                rd_kafka_topic_name(rkt), rd_kafka_err2str(rd_kafka_last_error()));
        return ;
    }

    DEBUG_LOG("[%d] user<%d> count<%d> likeCnt<%d> topic<%s> error<%s>", __LINE__, uid, count, index, 
                rd_kafka_topic_name(rkt), rd_kafka_err2str(rd_kafka_last_error()));
    rd_kafka_poll(rk, 50);
    rd_kafka_flush(rk, 1000);
}

void PoolProcessor::PullData(char *out_buf, const unsigned int max_len) {
    int code = REDIS_OK;
    redisReply *reply;
    out_header->pkg_len = sizeof(proto_header);
    pool_info_t info;
    memset(&info, 0, sizeof(pool_info_t));
   
    if (!GetPoolStat(in_header->user_id, info))
        return ;
    if (info.likeCount == 0) {
        ERROR_LOG("[PullData][%d] info count <%d>", __LINE__, info.likeCount);
        return ;
    }

    mifan::pullReq _req;
    std::string data;
    data.assign(in_body, in_len - sizeof(proto_header));
    _req.ParseFromString(data);

    {
        mifan::pullRes _res;
        std::string _seria{};
//检查当前推荐池的状态
//a.1 如果处于feedid导入阶段，那么返回给前端0条
        if (info.updatable == POOL_UPDATING) {
            _res.SerializeToString(&_seria);
            memcpy(out_body,_seria.c_str(), _seria.size());
            out_header->pkg_len += _seria.size();
#ifdef DEBUG
            DEBUG_LOG("[PullData][%d] My resource pool is updating", __LINE__);
#endif
            return ;
        }

        //先更新统计信息，然后再拉取资源
        int _likeIndex = info.likeIndex;
        int _hotIndex = info.hotIndex;
        //是否需要全量更新
        bool _needNotify = false;   
        int _likeNeed = _req.count() * rate_ / 100;
        int _hotNeed = _req.count() - _likeNeed;
        int _likeAlarm = info.likeCount - capacity_ * rate_ * alarm_ / 100 / 100;
        int _likeNewIndex = info.likeIndex + _likeNeed;
#ifdef DEBUG
        DEBUG_LOG("[PullData][%d] control data {likenewIndex<%d>, alarmIndex<%d>}", __LINE__, _likeNewIndex, _likeAlarm);
#endif
        if (_likeNewIndex < _likeAlarm) {
//a.2.1 如果拉取后的已读标志小于资源池的阈值,更新资源池已读下标位
            info.likeIndex = _likeNewIndex;
            info.hotIndex = info.hotIndex + _hotNeed;
        } else if (_likeNewIndex < info.likeCount) {
//a.2.2 如果拉取后的标志位大于资源池阈值，判断是否需要做一次增量更新
            if (info.likeIndex < _likeAlarm) {
                _needNotify = true;   //TODO 触发全量更新，通过kafka消息队列
                info.updatable = POOL_TRI_UPDATE;
            }
            
            info.likeIndex = _likeNewIndex;
            info.hotIndex = info.hotIndex + _hotNeed;
        } else {
//a.2.3 如果资源池已经读完，复位已读下标，读历史数据
            info.readable = POOL_UNREADABLE;
            info.likeIndex = 0;
            info.hotIndex = 0;
        }
        
        if (!SetPoolStat(in_header->user_id, info)) {
            ERROR_LOG("[PullData][%d] update pool statistic failed", __LINE__);
            return ;
        }
        
#ifdef DEBUG
DEBUG_LOG("[PullData][%d] request count <%d> statistic{read<%d>, update<%d>, likeCount<%d>, likeIndex<%d>, hotCount<%d>, hotIndex<%d>}", __LINE__, _req.count(), info.readable, info.updatable, info.likeCount, info.likeIndex, info.hotCount, info.hotIndex);
#endif
        redisAppendCommand(ctx, L_POOL_GET, in_header->user_id, _likeIndex, _likeIndex + _likeNeed - 1);
#ifdef DEBUG
DEBUG_LOG(L_POOL_GET, in_header->user_id, _likeIndex, _likeIndex + _likeNeed - 1);
#endif
        redisAppendCommand(ctx, H_POOL_GET, in_header->user_id, _hotIndex, _hotIndex + _hotNeed - 1);
#ifdef DEBUG
DEBUG_LOG(H_POOL_GET, in_header->user_id, _hotIndex, _hotIndex + _hotNeed - 1);
#endif
        code = redisGetReply(ctx, (void **)&reply);
        if (!ReplyArray(code, *reply)) {
            ERROR_LOG("[PullData][%d] Get data from like pool", __LINE__);
            GetError(FEED_REDIS_ERR);
            code = redisGetReply(ctx, (void **)&reply);
        } else {
#ifdef DEBUG
DEBUG_LOG("[PullData][%d] Get like pool succ<%d>", __LINE__, (int)reply->elements);
#endif
            for (int i = 0; i < (int)reply->elements; ++i) {
#ifdef DEBUG
DEBUG_LOG("[PullData][%d] Get detail feedid<%s>", __LINE__, reply->element[i]->str);
#endif
                _res.add_feedid(reply->element[i]->str);
            }
        
            code = redisGetReply(ctx, (void **)&reply);
            if (ReplyArray(code, *reply)) {
                srand(_likeIndex + in_header->user_id);
                //感兴趣和热文的比例
                int _perRate = _likeNeed / _hotNeed;
#ifdef DEBUG
                DEBUG_LOG("[PullData][%d] insert rate<%d>, feedidCnt<%d>", __LINE__, _perRate, (int)reply->elements);
#endif
                for (int i = 0; i < (int)reply->elements; ++i) {
                    int _pos = rand() % _perRate;
#ifdef DEBUG
                    DEBUG_LOG("[PullData][%d] swap seed<%d> random<%d>", __LINE__, _likeIndex, _pos);
#endif
                    _pos += i * (_perRate + 1);

#ifdef DEBUG
                    DEBUG_LOG("[PullData][%d] swap index<%d>", __LINE__, _pos);
#endif
                    _res.add_feedid(_res.feedid(_pos));
#ifdef DEBUG
                    DEBUG_LOG("[PullData][%d] swap<%d> old feedid<%s>", __LINE__, _pos, _res.feedid(_pos).c_str());
#endif
                    _res.set_feedid(_pos, reply->element[i]->str);
#ifdef DEBUG
                    DEBUG_LOG("[PullData][%d] swap hot feedid<%s>", __LINE__, reply->element[i]->str);
#endif
                }
            }
        }

        if (code == REDIS_OK) {
            _res.SerializeToString(&_seria);
            memcpy(out_body,_seria.c_str(), _seria.size());
            out_header->pkg_len += _seria.size();
        } else {
            ERROR_LOG("[PullData][%d] Get data from hot pool", __LINE__);
            GetError(FEED_REDIS_ERR);
        }

        if (_needNotify) {
            int _count = capacity_;
            if (info.readable != POOL_UNREADABLE) {
                    _count = capacity_ - (info.likeCount + info.hotCount - info.likeIndex - info.hotIndex);
            }
            int _index = _count * rate_ / 100;
            NotifyProcess(in_header->user_id, _count, _index);
        }
    }

    if (reply != nullptr)
        freeReplyObject(reply);
}

void PoolProcessor::PushData(char *out_buf, const unsigned int max_len) {
    redisReply *reply;
    out_header->pkg_len = sizeof(proto_header);
    pool_info_t info;
    memset(&info, 0, sizeof(pool_info_t));

    if (!GetPoolStat(in_header->user_id, info))
        return ;
    //
    if (info.updatable == POOL_UPDATING)
        return ;
    
    mifan::pushReq _req;
    std::string data;
    data.assign(in_body, in_len - sizeof(proto_header));
    _req.ParseFromString(data);
#ifdef DEBUG
    DEBUG_LOG("[PushData][%d] request user<%u> count <%d> index<%u>", __LINE__, _req.userid(), _req.info_size(), _req.index());
#endif

    if (info.readable == POOL_READABLE) {
        redisAppendCommand(ctx, H_POOL_FLUSH, info.hotIndex - info.hotCount, -1);
        redisAppendCommand(ctx, L_POOL_FLUSH, info.likeIndex - info.likeCount, -1);
    } else {
        redisAppendCommand(ctx, H_POOL_FLUSH, 0, -1);
        redisAppendCommand(ctx, L_POOL_FLUSH, 0, -1);
    }
    char _storeCmd[12800];
    int _pos = 0;
    _pos = sprintf(_storeCmd + _pos, L_POOL_INSERT, _req.userid());
    for (int i = 0; i < _req.index(); i++) {
#ifdef DEBUG
        DEBUG_LOG("[PushData][%d] Like feedid<%s> score<%d>", __LINE__, _req.info(i).feedid().c_str(), _req.info(i).score());
#endif
        _pos += sprintf(_storeCmd + _pos, " %u %s", _req.info(i).score(), _req.info(i).feedid().c_str());
    }
    _storeCmd[_pos] = '\0';
    redisAppendCommand(ctx, _storeCmd);
#ifdef DEBUG
DEBUG_LOG("[PushData][%d] Like feed store<%s>", __LINE__, _storeCmd);
#endif

    _pos = sprintf(_storeCmd, H_POOL_INSERT, _req.userid());
    for (int i = _req.index(); i < _req.info_size(); i++) {
#ifdef DEBUG
        DEBUG_LOG("[PushData][%d] Hot feedid<%s> score<%d>", __LINE__, _req.info(i).feedid().c_str(), _req.info(i).score());
#endif
        _pos += sprintf(_storeCmd + _pos, " %u %s", _req.info(i).score(), _req.info(i).feedid().c_str());
    }
    _storeCmd[_pos] = '\0';
    redisAppendCommand(ctx, _storeCmd);
#ifdef DEBUG
DEBUG_LOG("[PushData][%d] Hot feed store<%s>", __LINE__, _storeCmd);
#endif

    int code = redisGetReply(ctx, (void **)&reply);
    if (ReplyInteger(code, *reply)) {
#ifdef DEBUG
        DEBUG_LOG("[PushData][%d] Hot feed flush cnt<%d>", __LINE__, (int)reply->integer);
#endif
    }

    code = redisGetReply(ctx, (void **)&reply);
    if (ReplyInteger(code, *reply)) {
#ifdef DEBUG
        DEBUG_LOG("[PushData][%d] Like feed flush cnt<%d>", __LINE__, (int)reply->integer);
#endif
    }
    code = redisGetReply(ctx, (void **)&reply);
    if (ReplyInteger(code, *reply)) {
#ifdef DEBUG
        DEBUG_LOG("[%d] Added <%d>resource to like set", __LINE__, _req.index());
#endif
        code = redisGetReply(ctx, (void **)&reply);
        if (!ReplyInteger(code, *reply)) {
            ERROR_LOG("[%d] Can not add resource to hot zset", __LINE__);
            GetError(FEED_REDIS_ERR);
        }
    } else {
        ERROR_LOG("[%d] Can not add resource to like zset", __LINE__);
        GetError(FEED_REDIS_ERR);
        redisGetReply(ctx, (void **)&reply);
    }
   
    if (info.readable == POOL_READABLE) {
        info.likeCount += _req.index();
        info.hotCount += _req.info_size() - _req.index();
    } else {
        info.readable = POOL_READABLE;
        info.likeCount = _req.index();
        info.hotCount = _req.info_size() - _req.index();
        info.likeIndex = 0;
        info.hotIndex = 0;
    }
    info.updatable = POOL_NO_UPDATE;
    SetPoolStat(_req.userid(), info);
    if (reply != nullptr)
        freeReplyObject(reply);
}
