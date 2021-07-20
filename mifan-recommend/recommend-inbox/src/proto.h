#ifndef _CENTER_PROTO_H
#define _CENTER_PROTO_H
#include <stdint.h>
#define MAX_BUF_LEN (1024*1024)

#define SYS_ERR 1001                /// 系统出错 一般是内存出错
#define PARAM_ERR 1002              /// 参数错误
#define FEED_REDIS_ERR 1003         /// redis错误

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

#define RECOMMEND_PULL_CMD           1
#define RECOMMEND_PUSH_CMD           2
#endif
