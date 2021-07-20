#include <string.h>
#include <string>
#include "pool_processor.h"
using std::string;
#include "proto.h"
#include "redis_command.h"
#include "log.h"
#include "protoRecommend.pb.h"
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
        info.hotReadable = POOL_UNREADABLE;
        info.hotUpdatable = POOL_NO_UPDATE;
        info.hotCount = 0;
        info.hotIndex = 0;
        info.likeReadable = POOL_UNREADABLE;
        info.likeUpdatable = POOL_NO_UPDATE;
        info.likeCount = 0;
        info.likeIndex = 0;
        if (reply != nullptr)
            freeReplyObject(reply);
        return true;
    }

    redisAppendCommand(ctx, POOL_INFO_GETALL, userId);
    code = redisGetReply(ctx, (void **)&reply);
    if (ReplyArray(code, *reply)) {
//        if ((int)reply->elements == 16) {
            /*info.count = atoi(reply->element[1]->str);*/
            info.hotReadable = atoi(reply->element[1]->str);
            info.hotUpdatable = atoi(reply->element[3]->str);
            info.hotCount = atoi(reply->element[5]->str);
            info.hotIndex = atoi(reply->element[7]->str);
            info.likeReadable = atoi(reply->element[9]->str);
            info.likeUpdatable = atoi(reply->element[11]->str);
            info.likeCount = atoi(reply->element[13]->str);
            info.likeIndex = atoi(reply->element[15]->str);
#ifdef DEBUG
            DEBUG_LOG("[%d] Can get pool statistics from DB <%d>", __LINE__, (int)reply->elements);
            for (int i = 0; i < (int)reply->elements / 2; ++i) 
                DEBUG_LOG("[%d] Pool stat is <%s:%s>", __LINE__, reply->element[2*i]->str, reply->element[2*i + 1]->str);
#endif
            if (reply != nullptr)
                freeReplyObject(reply);
            return true;
//        } else {
  //      }
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
    DEBUG_LOG(POOL_INFO_SETALL, userId, /*info.count,*/
            info.hotReadable, info.hotUpdatable,
            info.hotCount, info.hotIndex,
            info.likeReadable, info.likeUpdatable,
            info.likeCount, info.likeIndex);
    redisAppendCommand(ctx, POOL_INFO_SETALL, userId, /*info.count,*/
            info.hotReadable, info.hotUpdatable,
            info.hotCount, info.hotIndex,
            info.likeReadable, info.likeUpdatable,
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

void PoolProcessor::NotifyProcess(int uid, int count, int index) {
#ifdef DEBUG
    DEBUG_LOG("[%d] user<%d> Notify process do update count<%d> likeCnt<%d>", __LINE__, uid, count, index);
//    DEBUG_LOG("[%d] Kafka ip<%s> port<%s> topic<%s> option<%s>", __LINE__, uid, config_.at(CONF_KAFKA_IP), config_.at(CONF_KAFKA_PORT), config_.at(CONF_KAFKA_TOPIC), config_.at(CONF_KAFKA_OPTION));
    for (const auto &it : config_) {
        DEBUG_LOG("[NotifyProcess][%d] kafka config %s:<%s>", __LINE__, it.first.c_str(), it.second.c_str());
    }
#endif
    auto _exec = config_.find(CONF_KAFKA_SCRIPT);
    char _cmd[512];
    if (_exec != config_.end()) {
        snprintf(_cmd, sizeof(_cmd), "php %s %d %d %d", _exec->second.c_str(), uid, count, index);
    } else 
        snprintf(_cmd, sizeof(_cmd), "php ./bin/producer.php %d %d %d", uid, count, index);

#ifdef DEBUG
    DEBUG_LOG("[%d] Execute command<%s>", __LINE__, _cmd);
#endif
    system(_cmd);
    return ;
}

bool PoolProcessor::CheckHotPoolStat(pool_info_t &info, int count, int userId) {
    bool _needNotify = false;
    int _alarm = info.hotCount - (capacity_ - capacity_ * rate_ /100) * alarm_ / 100;
    int _newIndex = info.hotIndex + count;

    if (_alarm < 0) {       //配置修改，则触发一次更新
#ifdef DEBUG
        DEBUG_LOG("[%d] HOT COUNT<%d> ALARM<%d> INDEX<%d>", __LINE__, count, _alarm, info.hotIndex);
#endif
        info.hotReadable = POOL_UNREADABLE;
        info.hotIndex = count;
        _needNotify = true;   
        info.hotUpdatable = POOL_TRI_UPDATE;

        SetPoolStat(userId, info);
        return _needNotify;
    }
    if (_newIndex < _alarm) {               //a.2.1 如果拉取后的已读标志小于资源池的阈值,更新资源池已读下标位
        info.hotIndex = _newIndex;
    } else if (_newIndex < info.hotCount) {    //a.2.2 如果拉取后的标志位大于资源池阈值，判断是否需要做一次增量更新
        if (info.hotIndex < _alarm) {//TODO 触发全量更新，通过kafka消息队列
            _needNotify = true;   
            info.hotUpdatable = POOL_TRI_UPDATE;
        }
            
        info.hotIndex = _newIndex;
    } else {                                        //a.2.3 如果资源池已经读完，复位已读下标，读历史数据
        if (info.hotIndex < _alarm) {
            _needNotify = true;   //TODO 触发全量更新，通过kafka消息队列
            info.hotUpdatable = POOL_TRI_UPDATE;
        }
        info.hotIndex = 0;
        info.hotReadable = POOL_UNREADABLE;
    }
#ifdef DEBUG
    if (_needNotify)
        DEBUG_LOG("[%d] hotNeed<%d> statistic{likeReadable<%d>, likeUpdate<%d>, likeCount<%d>, likeIndex<%d>, hotRead<%d>, hotUpdate<%d>, hotCount<%d>, hotIndex<%d>}", __LINE__, count, info.likeReadable, info.likeUpdatable, info.likeCount, info.likeIndex, info.hotReadable, info.hotUpdatable, info.hotCount, info.hotIndex);
#endif
    SetPoolStat(userId, info);
    return _needNotify;
}

bool PoolProcessor::CheckLikePoolStat(pool_info_t &info, int count, int userId) {
    bool _needNotify = false;
    int _alarm = info.likeCount - capacity_ * rate_ * alarm_ / 100 / 100;
    int _newIndex = info.likeIndex + count;

    if (_alarm < 0) {       //配置修改，则触发一次更新
#ifdef DEBUG
        DEBUG_LOG("[%d] HOT COUNT<%d> ALARM<%d> INDEX<%d>", __LINE__, count, _alarm, info.hotIndex);
#endif
        info.likeReadable = POOL_UNREADABLE;
        info.likeIndex = count;
        _needNotify = true;   
        info.likeUpdatable = POOL_TRI_UPDATE;

        SetPoolStat(userId, info);
        return _needNotify;
    }
    if (_newIndex < _alarm) {               //a.2.1 如果拉取后的已读标志小于资源池的阈值,更新资源池已读下标位
        info.likeIndex = _newIndex;
    } else if (_newIndex < info.likeCount) {    //a.2.2 如果拉取后的标志位大于资源池阈值，判断是否需要做一次增量更新
        if (info.likeIndex < _alarm) {//TODO 触发全量更新，通过kafka消息队列
            _needNotify = true;   
            info.likeUpdatable = POOL_TRI_UPDATE;
        }
            
        info.likeIndex = _newIndex;
    } else {                                        //a.2.3 如果资源池已经读完，复位已读下标，读历史数据
        if (info.likeIndex < _alarm) {
            _needNotify = true;   //TODO 触发全量更新，通过kafka消息队列
            info.likeUpdatable = POOL_TRI_UPDATE;
        }
        info.likeIndex = 0;
        info.likeReadable = POOL_UNREADABLE;
    }
        
#ifdef DEBUG
    if (_needNotify)
        DEBUG_LOG("[%d] likeNeed<%d> likeNotify<%d> tatistic{likeReadable<%d>, likeUpdate<%d>, likeCount<%d>, likeIndex<%d>, hotRead<%d>, hotUpdate<%d>, hotCount<%d>, hotIndex<%d>}", __LINE__, count, _needNotify, info.likeReadable, info.likeUpdatable, info.likeCount, info.likeIndex, info.hotReadable, info.hotUpdatable, info.hotCount, info.hotIndex);
#endif
    SetPoolStat(userId, info);
    return _needNotify;
}

void PoolProcessor::PullData(char *out_buf, const unsigned int max_len) {
    mifan::pullReq _req;
    std::string data;

    int code = REDIS_OK;
    redisReply *reply;
    out_header->pkg_len = sizeof(proto_header);
    pool_info_t info;
    memset(&info, 0, sizeof(pool_info_t));
    data.assign(in_body, in_len - sizeof(proto_header));
    _req.ParseFromString(data);

    if (!GetPoolStat(in_header->user_id, info))
        return ;
    {
        mifan::pullRes _res;
        std::string _seria{};
        if (info.likeCount == 0 || info.hotCount == 0) {
//第一次拉取的对象的资源池一定是空的，此时需要触发更新
//            ERROR_LOG("[PullData][%d] LIKE<%d> HOT<%d> Need recommend", __LINE__, info.likeCount, info.hotCount);
            int _likeNeed = 0;
            int _hotNeed = 0;
            if (info.likeUpdatable == POOL_NO_UPDATE) {
                info.likeUpdatable = POOL_TRI_UPDATE;
                _likeNeed = capacity_ * rate_ / 100 - info.likeCount + info.likeIndex;
            }
            if (info.hotUpdatable == POOL_NO_UPDATE) {
                info.hotUpdatable = POOL_TRI_UPDATE;
                _hotNeed = capacity_ - capacity_ *  rate_ / 100 - info.hotCount + info.hotIndex;
            }
            SetPoolStat(in_header->user_id, info);
            if ( (_likeNeed > 0) || (_hotNeed > 0) )
                NotifyProcess(in_header->user_id, _likeNeed, _hotNeed);

            _res.SerializeToString(&_seria);
            memcpy(out_body,_seria.c_str(), _seria.size());
            out_header->pkg_len += _seria.size();
            return ;
        }

//检查当前推荐池的状态
//a.1 如果处于feedid导入阶段，那么返回给前端0条
        if ((info.hotUpdatable == POOL_UPDATING) || (info.likeUpdatable == POOL_UPDATING)) {
            _res.SerializeToString(&_seria);
            memcpy(out_body,_seria.c_str(), _seria.size());
            out_header->pkg_len += _seria.size();
#ifdef DEBUG
            DEBUG_LOG("[PullData][%d] My resource pool is updating", __LINE__);
#endif
            return ;
        }

        //先更新统计信息，然后再拉取资源
        bool _likeNotify = false;
        bool _hotNotify = false;
        int _likeIndex = info.likeIndex;
        int _hotIndex = info.hotIndex;
        int _likeNeed = _req.count() * rate_ / 100;
        int _hotNeed = _req.count() - _likeNeed;
        //是否需要全量更新
        _likeNotify = CheckLikePoolStat(info, _likeNeed, in_header->user_id); 
        _hotNotify = CheckHotPoolStat(info, _hotNeed, in_header->user_id); 
        
        redisAppendCommand(ctx, L_POOL_GET, in_header->user_id, _likeIndex, _likeIndex + _likeNeed - 1);
#ifdef DEBUG
        if (_likeNotify)
            DEBUG_LOG(L_POOL_GET, in_header->user_id, _likeIndex, _likeIndex + _likeNeed - 1);
#endif
        redisAppendCommand(ctx, H_POOL_GET, in_header->user_id, _hotIndex, _hotIndex + _hotNeed - 1);
#ifdef DEBUG
        if (_hotNotify)
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
            int _perRate = (int)reply->elements;
            /* 2019-08-07
             * 增加去重变量_uniqueSet和重复后需要新添加的条数_uniqueNeed;
             */
            std::unordered_set<string> _uniqueSet;
            int _uniqueNeed = 0;
            for (int i = 0; i < (int)reply->elements; ++i) {
#ifdef DEBUG
DEBUG_LOG("[PullData][%d] Get detail feedid<%s>", __LINE__, reply->element[i]->str);
#endif
                _res.add_feedid(reply->element[i]->str);
                _uniqueSet.emplace(reply->element[i]->str);
            }
        
            code = redisGetReply(ctx, (void **)&reply);
            if (ReplyArray(code, *reply)) {
                srand(_likeIndex + in_header->user_id);
                /* 2019-08-07
                 * 再拉取热文时，判断是否重复，若重复则去内容库重新拉取指定的条数
                 */
                //感兴趣和热文的比例
                for (int i = 0; i < (int)reply->elements; ++i) {
                    auto _it = _uniqueSet.find(reply->element[i]->str);
                    if (_it == _uniqueSet.end()) {
                        int _pos = _perRate == 0 ? 0 :rand() % _perRate - 1;
                        if (_pos < 0 || _pos > _perRate - 1)
                            _pos = 0;
                        if (_perRate == 0) {
                            _res.add_feedid(reply->element[i]->str);
                        } else {
                            _res.add_feedid(_res.feedid(_pos));
                            _res.set_feedid(_pos, reply->element[i]->str);
                        }
                        _uniqueSet.emplace(reply->element[i]->str);
                    } else {
                        _uniqueNeed++;
                    }
                    _perRate++;
                }
        
                bool _uniqueFlag = false;
                while (_uniqueNeed) {
#ifdef DEBUG
DEBUG_LOG("[PullData][%d] repeat COUNT<%d> ", __LINE__, _uniqueNeed);
#endif
                    if (info.likeReadable != POOL_UNREADABLE || info.hotReadable == POOL_UNREADABLE) {         //内容池数据全部消费，热文池还有余量，先消费热文池
                        //先更新用户资源池统计信息
                        _uniqueFlag = CheckLikePoolStat(info, _uniqueNeed, in_header->user_id);
                        _likeNotify = _likeNotify ? _likeNotify : _uniqueFlag;
                        //然后再从资源池拉取数据
                        redisAppendCommand(ctx, H_POOL_GET, in_header->user_id, info.hotIndex, info.hotIndex + _uniqueNeed);
#ifdef DEBUG
                        if (_likeNotify)
                            DEBUG_LOG(H_POOL_GET, in_header->user_id, info.hotIndex, info.hotIndex + _uniqueNeed);
#endif
                    } else {
                        //先更新用户资源池统计信息
                        _uniqueFlag = CheckHotPoolStat(info, _uniqueNeed, in_header->user_id);
                        _hotNotify = _hotNotify ? _hotNotify : _uniqueFlag;
                        //然后再从资源池拉取数据
                        redisAppendCommand(ctx, L_POOL_GET, in_header->user_id, info.likeIndex, info.likeIndex + _uniqueNeed);
#ifdef DEBUG
if (_hotNotify)
    DEBUG_LOG(L_POOL_GET, in_header->user_id, info.likeIndex, info.likeIndex + _uniqueNeed);
#endif
                    }
                    _uniqueNeed = 0;
                    code = redisGetReply(ctx, (void **)&reply);
                    if (ReplyArray(code, *reply)) {
                        srand(_likeIndex + in_header->user_id + _uniqueNeed);
                        for (int i = 0; i < (int)reply->elements; ++i) {
                            auto _it = _uniqueSet.find(reply->element[i]->str);
                            if (_it == _uniqueSet.end()) {
                                int _pos = rand() % _perRate;
                                if (_pos < 0 || _pos > _perRate - 1)
                                    _pos = 0;
#ifdef DEBUG
                                DEBUG_LOG("[PullData][%d] unique swap seed<%d> random<%d>", __LINE__, _perRate, _pos);
#endif
                                _res.add_feedid(_res.feedid(_pos));
                                _res.set_feedid(_pos, reply->element[i]->str);
                                _uniqueSet.emplace(reply->element[i]->str);
                            } else 
                                _uniqueNeed++;
                            _perRate++;
                        }
                    }
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

        if (_likeNotify || _hotNotify) {
            int _like = capacity_ * rate_ / 100;
            int _hot = capacity_ - _like;
            if (info.likeReadable != POOL_UNREADABLE)
                    _like = _like - (info.likeCount - info.likeIndex);
            if (info.hotReadable != POOL_UNREADABLE)
                    _hot = _hot - (info.hotCount - info.hotIndex);
            if (!_likeNotify)
                _like = 0;
            if (!_hotNotify)
                _hot = 0;
            NotifyProcess(in_header->user_id, _like, _hot);
        }
    }

    if (reply != nullptr)
        freeReplyObject(reply);
}

void PoolProcessor::PushData(char *out_buf, const unsigned int max_len) {
    /*
    redisReply *reply;
    pool_info_t info;
    memset(&info, 0, sizeof(pool_info_t));

    mifan::pushReq _req;
    std::string data;
    data.assign(in_body, in_len - sizeof(proto_header));
    _req.ParseFromString(data);
    out_header->pkg_len = sizeof(proto_header);

#ifdef DEBUG
            DEBUG_LOG("[PullData][%d] protobuf len %d", __LINE__, in_len);
#endif
    if (0 == _req.info_size()) {
        NotifyProcess(in_header->user_id, _req.index() * 100 / rate_, _req.index());
        return;
    }
    if (!GetPoolStat(in_header->user_id, info))
        return ;
    //
    if (info.updatable == POOL_UPDATING)
        return ;
    
#ifdef DEBUG
    DEBUG_LOG("[PushData][%d] request user<%u> count <%d> index<%u>", __LINE__, _req.userid(), _req.info_size(), _req.index());
#endif

    if (info.readable == POOL_READABLE) {
        redisAppendCommand(ctx, H_POOL_FLUSH, _req.userid(),  info.hotIndex - info.hotCount, -1);
#ifdef DEBUG
        DEBUG_LOG(H_POOL_FLUSH, _req.userid(), info.hotIndex - info.hotCount, -1);
#endif
        redisAppendCommand(ctx, L_POOL_FLUSH, _req.userid(), info.likeIndex - info.likeCount, -1);
#ifdef DEBUG
        DEBUG_LOG(L_POOL_FLUSH, _req.userid(), info.likeIndex - info.likeCount, -1);
#endif
    } else {
        redisAppendCommand(ctx, H_POOL_FLUSH, _req.userid(), 0, -1);
#ifdef DEBUG
        DEBUG_LOG(H_POOL_FLUSH, _req.userid(), 0, -1);
#endif
        redisAppendCommand(ctx, L_POOL_FLUSH, _req.userid(), 0, -1);
#ifdef DEBUG
        DEBUG_LOG(L_POOL_FLUSH, _req.userid(), 0, -1);
#endif
    }
    char _storeCmd[12800];
    int _pos = 0;
    _pos = sprintf(_storeCmd + _pos, L_POOL_INSERT, _req.userid());
    for (int i = 0; i < _req.index(); i++) {
#ifdef DEBUG
        DEBUG_LOG("[PushData][%d] Like <%d>feedid<%s> score<%d>", __LINE__, i, _req.info(i).feedid().c_str(), _req.info(i).score());
#endif
        _pos += sprintf(_storeCmd + _pos, " %u %s", _req.info(i).score(), _req.info(i).feedid().c_str());
    }
    _storeCmd[_pos] = '\0';
    redisAppendCommand(ctx, _storeCmd);
#ifdef DEBUG
//DEBUG_LOG("[PushData][%d] Like feed store<%s>", __LINE__, _storeCmd);
#endif

    _pos = sprintf(_storeCmd, H_POOL_INSERT, _req.userid());
    for (int i = _req.index(); i < _req.info_size(); i++) {
#ifdef DEBUG
//        DEBUG_LOG("[PushData][%d] Hot feedid<%s> score<%d>", __LINE__, _req.info(i).feedid().c_str(), _req.info(i).score());
#endif
        _pos += sprintf(_storeCmd + _pos, " %u %s", _req.info(i).score(), _req.info(i).feedid().c_str());
    }
    _storeCmd[_pos] = '\0';
    redisAppendCommand(ctx, _storeCmd);
#ifdef DEBUG
//DEBUG_LOG("[PushData][%d] Hot feed store<%s>", __LINE__, _storeCmd);
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
        DEBUG_LOG("[%d] Added <%d>resource to like set", __LINE__, (int)reply->integer);
#endif
        code = redisGetReply(ctx, (void **)&reply);
        if (!ReplyInteger(code, *reply)) {
            ERROR_LOG("[%d] Can not add resource to hot zset", __LINE__);
            GetError(FEED_REDIS_ERR);
        }
#ifdef DEBUG
        DEBUG_LOG("[%d] Added <%d>resource to hot set", __LINE__, (int)reply->integer);
#endif
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
    
    out_header->pkg_len = sizeof(proto_header);
    out_header->status_code = 0;
    out_header->cmd_id = in_header->cmd_id;
    out_header->seq_num = in_header->seq_num;
    out_header->user_id = in_header->user_id;
#ifdef DEBUG
    DEBUG_LOG("[%d] Push Done out_header {pkg_len:%d, code:%d, cmdid:%d}", __LINE__, out_header->pkg_len, out_header->status_code, out_header->cmd_id);
#endif
    */
}
