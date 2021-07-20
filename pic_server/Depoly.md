# lighttpd
## 安装配置
1. 所需要的安装包`pcre-8.01.tar.gz`, `lighttpd-1.4.26.tar.gz`
2. install pcre
```
cd /usr/local/src
tar -zxvf pcre-8.01.tar.gz
cd pcre-8.01
./configure --prefix=/usr/local/pcre
make
make install
```
3. install lighttpd

```
tar -zxvf lighttpd-1.4.26.tar.gz
cd lighttpd-1.4.26
./configure --prefix=/usr/local/lighttpd
make
make instal
```

## 服务
|字段名|说明|
|---|---|
|服务端口|8080（不直接对外开放，请求由varnish转发而来）|
|服务目录|`/home/ian/local_service/pic_storage/lighttpd/`|
|启动脚本|`./restart.sh`|
|lighttpd 配置|`./etc/lighttpd.conf`|
|fastcgi 配置|`./etc/conf-enabled/10-fastcgi.conf`|
- httpd服务器，接受http的上传下载请求，然后分发给fastcgi进行具体的业务处理

**1. download.fcgi**
- 服务目录： `cd /home/ian/workspace/pic_storage/trunk/3pic/download_fcgi/trunk/src/`

- 接收参数：要下载的图片url `http://10.1.1.197:8081/download.fcgi/g01/M00/00/00/CgEBxVxREbmAUETXAAEqVyZGQRY768.jpg
`

**2. upload.cgi**  
- 服务目录： `/home/ian/workspace/pic_storage/trunk/3pic/upload_cgi/trunk/src/`

- **请求JSON数据格式: **

|字段名|说明|
|---|---|
|session_key|客户端自定义的session值|
|type|上传图片的类型|
|file|上传图片的二进制数据流，文件上传大小限制为`5x1024x1024字节（5Mb）`|
- **session_ley构造：**将下列字段以二进制的形式打包，再根据指定的密钥用des的方式对二进制流进行加密，最后转为16进制数据 

|字段名|类型|字节数|说明|
|---|---|---|---|
|uid|uint16_t|2|米米号|
|channel|uint16_t|2|未检查|
|albumid|uint16_t|2|相册id|
|i_ip|uint32_t|4|图片上传的服务器ip|
|i_time|uint16_t|2|上传时刻的时间戳（服务器会做时间检查，超时时间为1小时）|
|weight_limit|uint16_t|2|图片宽度限制|
|height_limit|uint16_t|2|图片高度限制|
|is_head|uint16_t|2|未检查|

- **应答JSON数据格式：**

```
{ 
	"result": "success", 
	"albumid": 0, 
	"photoid": 0, 
	"file_name": "2m.jpg", 
	"hostid": 0, 
	"lloc": "g01\/M00\/00\/00\/CgEBxVxhGVKAVKsFAAvFE0AZv0c668.jpg", 
	"len": 771347 
}
```

# varnish
|字段名|说明|
|---|---|
|服务端口|8081（图片HTTP服务器对外开放的唯一端口）|
|服务目录|`/home/ian/local_service/pic_storage/varnish/`|
|启动脚本|`./restart.sh`|

- 对于客户端的POST上传文件的请求通过PIPE让客户端和lighttpd直接连接通信
- 对于客户端的GET下载文件请求，先通过lookup查询本地缓存，命中直接返回给客户端，未找到再通过配置的backend端口（8080）访问后台服务器（lighttpd）获取，缓存之后返回给客户端

# fastDFS
一种分布式文件管理系统，包含`tracker_server`（负载均衡和调度，分配给具体storage_server来存取图片）和`storage_server`（文件存储）两个部分
- **tracker服务**

|字段名|说明|
|---|---|
|服务目录|`/home/ian/local_service/pic_storage/fdfs/tracker/`|
|启动脚本|`./tracker.sh start`|

- **storage服务**

|字段名|说明|
|---|---|
|服务目录|`/home/ian/local_service/pic_storage/fdfs/g02_storage/`|
|启动脚本|`./storaged.sh start`|
- [分布式文件系统的搭建](http://www.360doc.com/content/18/0121/11/412471_723826241.shtml)

## 部署常见问题
> * Tracker
```
问题：ERROR - file: sockopt.c, line: 770, bind port 6000 failed, errno: 99, error info: Cannot assign requested address.
原因：绑定的IP地址不对
方案：修改绑定IPbind_addr=10.30.100.2
问题：ERROR - file: shared_func.c, line: 1085, open file ./data/fdfs_trackerd.pid fail, errno: 2, error info: No such file or directory
原因：tracker配置文件中base_path的路径配置错误
方案：修改配置路径为base_path=../
```

> * Storeage
```
问题：ERROR - file: shared_func.c, line: 1085, open file ../data1/data/.recovery.mark fail, errno: 2, error info: No such file or directory
原因：文件保存路径配置错误
方案：配置为store_path0=../data1
问题：tracker_query_storage fail, error no: 2, error info: No such file or directory
原因：没有storage处于ACTIVE状态 方案：先执行fdfs_monitor storage.conf产看状态信息，删除tracker和storage指定的base_path下的data目录，重启服务？
```

# 辅助服务
## admin_proxy
- 代理服务器，统一外网对辅助服务的通信端口  
- 服务端口： 6998  
- 服务目录： `http://svn.taomee.com/framework/pic_storage/trunk/3pic/admin_proxy/trunk/`
- 启动脚本： `./daemon.sh start`

**协议包格式：**  

字段名|类型|字节数|说明|
---|---|---|---
len|uint32_t|4|包长
cmd|uint16_t|2|协议号 3005
id|uint32_t|4|米米号
seq|uint32_t|4|序列号
ret|uint32_t|4|状态值
body|uint8|不定|包体

## cut_svr
- 服务端口：**9020 - 9021**
- 通信协议： **tcp**
- 协议号： **3005**
- 服务目录： `http://svn.taomee.com/framework/pic_storage/trunk/3pic/cut_svr/trunk/src`
- 启动脚本： `./daemon.sh start`

**请求包体格式：**  

|字段名|类型|字节数|说明| 
|---|---|---|---|
|hostid|uint32_t|4||
|fdfs_url|char(64)|64|图片所在的url|
|clip_w|uint32_t|4|原图宽度|
|clip_h|uint32_t|4|原图高度|
|start_x|int32|4|原图裁剪位置|
|start_y|int32|4|原图裁剪位置|
|thumb_w|uint32_t|4|裁剪宽度|
|thumb_h|uint32_t|4|裁剪高度|
**应答包体格式：**  

|字段名|类型|字节数|说明| 
|---|---|---|---|
|hostid|uint32_t|4||
|fdfs_url|char(64)|64|裁剪图片url|

## del_svr
删除请求缓存服务，将需要删除的图片url缓存到log/delete.log,供del_file_ontime定时删除
- 服务端口：**9010 - 9011**
- 通信协议： **tcp**
- 协议号： **3002**
- 服务目录： `http://svn.taomee.com/framework/pic_storage/trunk/3pic/del_svr/trunk/src`
- 启动脚本： `./daemon.sh start`

**请求包体格式：**

|字段名|类型|字节数|说明|
|---|---|---|---|
|file_cnt|uint32_t|4|删除图片的数目|
|pic_info|pic_info[]|不定|图片信息|

**pic_info：**

|字段名|类型|字节数|说明|
|---|---|---|---|
|hostid|uint32_t|4||
|url|char(64)|64|图片url|

**应答包体格式：**

|字段名|类型|字节数|说明|
|---|---|---|---|
||||
|proto_len|uint32_t|4|包长，18|
|commandid|uint16_t|2|协议号，3002|
|userid|uint32_t|4||
|seq|uint32_t|4||
|result|uint32_t|4|处理结果|

##  del_file_ontime
定时删除del_svr缓存文件服务

- 服务目录： `http://svn.taomee.com/framework/pic_storage/trunk/3pic/del_file_ontime`
- 执行命令： `./del_file_ontime delete.log 10.1.1.197:6998`

## cs_del_svr
批量删除服务，同时会调用pruge.php脚本清除varnish缓存
- 服务端口：**9030 - 9031**
- 通信协议： **tcp**
- 协议号： **3030**

**请求包体格式：**

|字段名|类型|字节数|说明|
|---|---|---|---|
|file_cnt|uint32_t|4|删除图片的数目|
|pic_info|pic_info[]|不定|图片信息|

**pic_info：**

|字段名|类型|字节数|说明|
|---|---|---|---|
|hostid|uint32_t|4||
|url|char(64)|64|图片url|

## metadata_dbsvr（44上未用到此服务）

## sess_check_svr（44上未用到此服务）
session检查服务，同一个session只能check 30次，之后就过期了
- 服务端口：**7003**
- 通信协议： **tcp**
- 协议号： **0x10B5**

## monit_fdfs(未找到调用)
检查对应ip的主机上对应的storage group是否在运行服务。

## monit_varnish(未找到调用)
查看varnish缓存的命中率