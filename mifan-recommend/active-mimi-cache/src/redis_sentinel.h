#ifndef __REDIS_SENRINEL_INFO__H
#define __REDIS_SENRINEL_INFO__H
/*************************************************************************
    > File Name: redis_sentinel.h
    > Author: ian
    > Mail: ian@taomee.com 
    > Created Time: Wed 20 Mar 2019 04:10:38 PM CST
 ************************************************************************/
#include "hiredis.h"
#include <string>
#include <vector>
using std::string;
using std::vector;

#define REDIS_AUTH "auth %s"

class RedisMasterInfo {
    //Master server IP
    string ip;
    //Master server Port
    int port;
    //Master server Password
    string pass;
    int timeout;
    //Context of connection to master
    redisContext *ctx;

public:
    RedisMasterInfo(string m_ip = 0, int m_port = 0, string pwd = 0, int time = 0):ip(m_ip),port(m_port),pass(pwd),timeout(time) {};
    ~RedisMasterInfo();

    //Init connection to redis master server
    int Init();
    int ReInit(string m_ip, int m_port);
    
    redisContext *GetContext() { return ctx; };
};

class RedisSentinelInfo {
    //Sentinel server IP list
    vector<string> ip_list;
    //Sentinel server Port list
    vector<int> port_list;
    //Master name configured in Sentinel server
    int count;
    int cur;
    int timeout;
    string name;

    redisContext *ctx;
public:
    //RedisSentinelInfo(string ip, int port, int time, string name):s_ip(ip),s_port(port),timeout(time),s_name(name) {}
    RedisSentinelInfo(int s_count, int time, string s_name):count(s_count),timeout(time),name(s_name) {
        cur = 0;

        ctx = nullptr;
    }

    ~RedisSentinelInfo();
    void AddServer(string s_ip, int s_port);
    //Init connection to redis sentinel server
    int _Init(int retry);
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
    int CheckOrUpdate(string &s_ip, int &s_port);
};
#endif
