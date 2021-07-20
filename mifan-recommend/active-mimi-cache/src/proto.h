#ifndef _CENTER_PROTO_H
#define _CENTER_PROTO_H
#include <stdint.h>
#define MAX_BUF_LEN (1024*1024)

#define SYS_ERR 1001                /// 系统出错 一般是内存出错
#define PARAM_ERR 1002              /// 参数错误
#define FEED_REDIS_ERR 1003

typedef struct {
    uint32_t pkg_len;
    uint32_t seq_num;
    uint16_t cmd_id;
    uint32_t status_code;
    uint32_t user_id;
} __attribute__((packed)) proto_header;

int atoi_safe(const char *nptr);
//=====================Redis主备切换通知========================
#define MASTER_CHANGE_CMD	            0xA001

typedef struct {
	uint32_t port;
	uint32_t len;
    char ip[0];
}__attribute__((packed)) master_change_req;

//=====================最近更新文章的mimi号========================
#define FEED_MIMIID_UPDATE_CMD	        0xA101
typedef struct {
	uint32_t timer;
}__attribute__((packed)) feed_mimiid_update_req;

//2019-06-27 新增删除tag内的feed的接口,使用protobuf传输数据
#define ACTIVE_USER_QUERY_CMD       0x1
#define ACTIVE_USER_CHECK_CMD       0x2

#endif
