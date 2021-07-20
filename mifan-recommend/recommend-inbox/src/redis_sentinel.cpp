/*************************************************************************
    > File Name: redis_sentinel.cpp
    > Author: ian
    > Mail: ian@taomee.com 
    > Created Time: Wed 20 Mar 2019 04:27:48 PM CST
 ************************************************************************/
#include "redis_sentinel.h"
#include "log.h"

//using std::string;
//using std::vector;

/************************* Redis Master implement *************************/
RedisMasterInfo::~RedisMasterInfo() {
    if (ctx != NULL) {
        DEBUG_LOG("[REDIS]:Master server destructed");
        redisFree(ctx);
        ctx = NULL;
    }
}

int RedisMasterInfo::Init() {
    if (ip.empty() || port == 0) {
        ERROR_LOG("Invalid redis sentinel server %s:%d", ip.c_str(), port);
        return -1;
    }
    
    struct timeval tm = { 1, timeout};
    ctx = redisConnectWithTimeout(ip.c_str(), port, tm);
    
    if (ctx == NULL) {
        ERROR_LOG("Can not allocate redis context");
        return -1;
    } else if (ctx->err) {
        ERROR_LOG("Can not connect to redis server %s:%d %s", ip.c_str(), port, ctx->errstr);
        redisFree(ctx);
        ctx = NULL;
        return -1;
    }

    redisReply *reply;
    redisAppendCommand(ctx, REDIS_AUTH, pass.c_str());
    int code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR) {
        ERROR_LOG("[REDIS]:Redis Server pass:%s err:%s", pass.c_str(), ctx->errstr);
        return -1;
    }
    return 0;
}

int RedisMasterInfo::ReInit(string m_ip, int m_port) {
    if (m_ip == ip && port == m_port) {
        DEBUG_LOG("[REDIS]:Master server(%s:%d) not changed", ip.c_str(), port);
    } else {
        ip = m_ip;
        port = m_port;
        DEBUG_LOG("[REDIS]:Master server(%s:%d) had changed", ip.c_str(), port);
    }
    //release prev connection
    if (ctx != NULL) {
        redisFree(ctx);
        ctx = NULL;
    }

    return Init();
}


/************************* Redis Sentinel implement *************************/
RedisSentinelInfo::~RedisSentinelInfo() {
    if (ctx != NULL) {
        DEBUG_LOG("[REDIS]:Sentinel server destructed");
        redisFree(ctx);
        ctx = NULL;
    }
}

void RedisSentinelInfo::AddServer(string s_ip, int s_port) {
    if (s_ip.empty() || s_port == 0) {
        ERROR_LOG("Invalid redis sentinel server %s:%d", s_ip.c_str(), s_port);
        return ;
    }
    
    DEBUG_LOG("[REDIS]:Sentinel server add %s:%d", s_ip.c_str(), s_port);
    ip_list.push_back(s_ip);
    port_list.push_back(s_port);
    return ;
}

int RedisSentinelInfo::_Init(int retry) {
    //if (ip.empty() || port == 0) {
    if (retry == count) {
        ERROR_LOG("[REDIS SENTINEL] All Redis Sentinel Server shut down.");
        return -1;
    }
    if (ip_list.empty() || port_list.empty()) {
        ERROR_LOG("None redis sentinel server.");
        return -1;
    }
    
    struct timeval tm = { 1, timeout};
    string s_ip = ip_list[cur];
    int s_port = port_list[cur++];
    cur = cur % count;
    if (cur == 0) {
        ;//Do nothing Just warning
        DEBUG_LOG("[REDIS SENTINEL]:All redis sentinel server shutdown at once.");
    }
    else {
        if (ctx != NULL) {
            DEBUG_LOG("[REDIS SENTINEL]:Sentinel server destructed and reconnecting");
            redisFree(ctx);
            ctx = NULL;
        }
    }

    ctx = redisConnectWithTimeout(s_ip.c_str(), s_port, tm);
    if (ctx == NULL)
    {
        ERROR_LOG("Can not allocate redis sentinel context %s:%d", s_ip.c_str(), s_port);
        return -1;
    }
    else if (ctx->err)
    {
        redisFree(ctx);
        ctx = NULL;
        return _Init(retry + 1);
    }
    return 0;
}

#define REDIS_MASTER_QUERY "SENTINEL get-master-addr-by-name %s"
    /**
     * @brief CheckOrUpdate 
     *
     * @param ip
     * @param port
     *
     * @return 0 : Current ip:port redis is master Redis or Just skip check
     *         1 : Redis failover and need reconnected to server
     *        -1 : Cur sentinel server shutdown
     */
int RedisSentinelInfo::CheckOrUpdate(string &ip, int &port) {
    //
    int result = -1;
    if (-1 == _Init(cur)) {
        redisFree(ctx);
        ctx = NULL;
        return -1;
    }

    redisReply *reply;

    redisAppendCommand(ctx, REDIS_MASTER_QUERY, name.c_str());
    //DEBUG_LOG("[REDIS]:check Redis ip:port %s:%d Mname:%s", ip.c_str(), port, s_name.c_str());
	
    int code = redisGetReply(ctx, (void **)&reply);
    if (code == REDIS_ERR) {
        ERROR_LOG("[REDIS]:Sentinel Server %s:%d shutdown %s", ip.c_str(), port, ctx->errstr);
        /* Do nothing while sentinel server shutdown */
        redisFree(ctx);
        ctx = NULL;
        return -1;
    }
    
    string master_ip = reply->element[0]->str;
    int master_port = atoi(reply->element[1]->str);
    if (master_ip != ip || port != master_port) {
        DEBUG_LOG("[REDIS]:Redis master failover ip:%s port:%d", master_ip.c_str(), master_port);
        ip = master_ip;
        port = master_port;
        result = 1;
    }
    else
        result = 0;

    redisFree(ctx);
    ctx = NULL;
    return result;
}
