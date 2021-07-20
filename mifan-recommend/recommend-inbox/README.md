# 数据设计
- 内部采用的async-server做网络通信
- 采用redis主备作为主要存储DB

**内部存储数据结构**
![recommend-inbox data](https://github.com/masterGGG/common/raw/master/recommend-inbox.PNG)

****
# 后台服务器说明


## 配置文件
```
pool_capacity           500     //推荐资源池的大小
pool_update_rate        20      //触发全量更新的百分比（此配置中即感兴趣文章只剩80条时触发全量更新）
pool_like_rate          80      //感兴趣文章的百分比（即资源池感兴趣总量为400）
```

## 协议包结构说明
字段名 | 字节数 | 说明
---|---|---
pkg_header | 18 | 包头
pkg_body | 不定 | 包体（采用protobuf格式序列化与发序列化）

## 包头格式说明
字段名 | 类型 | 字节数 | 说明
---|---|---|---
pkg_len | Uint32 | 4 | 协议长度 
seq_num |	uint32|	4 | 包序列号，客户端填写，服务端原样返回
cmd_id | uint16| 2 | 协议号
status_code | uint32| 4 | 错误码
user_id | uint32 | 4 | 米米号

## 错误码说明
宏定义|错误码|描述
---|---|---
FEED_RELATION_SUCC|0|请求成功
FEED_RELATION_ERR_SYS|1001|服务内部内存错误
FEED_RELATION_ERR_PARA|1002|参数错误
FEED_RELATION_ERR_REDIS|1003|redis服务出错

## 协议说明
### 拉取协议 1
```
//protobuf文件中的定义
message pullReq {
    int32 count = 1;
}

message pullRes {
    repeated string feedid = 1;
}
```

### 更新协议 2
```
//protobuf文件中的定义
message pushReq {
    message feedInfo {
        string feedid = 1;
        int32 score = 2;
    }

    int32 userid = 1;
    int32 index = 2;
    repeated feedInfo info = 3;
}
```