#!/usr/bin/env python
#encoding=utf8


import hashlib
import time
import random
import urllib
import urllib2
import json
import sys
import base64
import redis        
import requests
import os
import shutil
import socket
import struct
import imgCheck_pb2
from google.protobuf.json_format import MessageToJson,Parse

reload(sys);
sys.setdefaultencoding('utf8')
import config_pic
configs = config_pic.configs

class Networkerror(RuntimeError):
    def __init__(self, arg):
        self.args = arg

class ImageCheckAPIDemo(object):
    """图片在线检测接口示例代码"""
    API_URL = configs['server']['url']

    def __init__(self, secret_id, secret_key):
        """
        Args:
            secret_id (str) 产品密钥ID，产品标识
            secret_key (str) 产品私有密钥，服务端生成签名信息使用
        """
        self.secret_id = secret_id
        self.secret_key = secret_key

    def gen_signature(self, params=None):
        """生成签名信息
        Args:
            params (object) 请求参数
        Returns:
            参数签名md5值
        """
        buff = ""
        for k in sorted(params.keys()):
            buff += str(k)+ str(params[k])
        buff += self.secret_key
        return hashlib.md5(buff).hexdigest()

    def check(self, params):
        """请求易盾接口
        Args:
            params (object) 请求参数
        Returns:
            请求结果，json格式
        """
        params["secret_id"] = self.secret_id
        params["timestamp"] = int(time.time() * 1000)
        params["nonce"] = int(random.random() * 100000000)
        params["signature"] = self.gen_signature(params)

        response = requests.post(self.API_URL, data=json.dumps(params))
        prediction = response.json()

        return prediction

class PictureInfo(object):
    def __init__(self, args):
        SQUARE_SUFFIX = configs['picture']['square']
        PICTURE_DELIMITER = '.'
        tmp=args.split('-', 2)
        self.ip=tmp[0]
        self.mimi=tmp[1]
        self.nail=tmp[2]
        self.type=self.nail[self.nail.rindex('.')+1:]
        self.src=self.nail[0:self.nail.rindex('_')] + PICTURE_DELIMITER + self.type
        self.square=self.nail[0:self.nail.rindex('_')] + SQUARE_SUFFIX + PICTURE_DELIMITER + self.type

    def getResource(self):
        if self.type == 'gif':
            return self.src
        #其他类型直接审核缩略图
        return self.nail

def delete_cdn_cache(myurl): 
    response = requests.post(myurl)
    res = response.content
    while True:
        pos = res.find("'code': ")
        if pos == -1 :
            break
        res = res[pos + len("'code': "):]
        code = int(res[:res.find(",")])
        if code != 0 :
	    sys.stderr.write("Picture[%s] deleted from CDN-cache failed error:%d\n" % (myurl, code))
    return

def delete_local_file(mypath):
    imgPath = configs['picture']['root'] + mypath
    if os.path.exists(imgPath) :
        os.remove(imgPath)
        print("Picture[%s] deleted success" % imgPath)
        delete_cdn_cache(configs['cdn']['refresh'] + mypath)
    return
    
def check_by_url(message):
    start = time.time()
    """示例代码入口"""
    SECRET_ID = configs['params']['id'] # 产品密钥ID，产品标识
    SECRET_KEY = configs['params']['key'] # 产品私有密钥，服务端生成签名信息使用，请严格保管，避免泄露
    
    image_check_api = ImageCheckAPIDemo(SECRET_ID, SECRET_KEY)
    pic_info = PictureInfo(message)
        
    IMAGE_URL = configs['picture']['root'] + pic_info.getResource()
#    print("Picture[%s] deleted success" % IMAGE_URL)
   
    if os.path.exists(IMAGE_URL) : 
        with open(IMAGE_URL, 'rb') as imgfd:
            filebuf = imgfd.read()
    else:
        print("Picture[%s] not exist" % IMAGE_URL)
        return    
            
    imagebase64 = {
        "name":pic_info.src,
        "type":2,
        "data":base64.urlsafe_b64encode(filebuf)
    }
   
    images = []
    images.append(imagebase64)
    
    params = {
        "images": json.dumps(images),
        "account": pic_info.mimi,
        "ip": pic_info.ip,
        "game_id": configs['params']['game_id'],
        "callback_url":configs['cdn']['callback']['root'] + configs['cdn']['callback']['path'] + pic_info.src + configs['cdn']['callback']['op']
    }
#    print("request is :%s" % params)    
    ret = image_check_api.check(params)
    sys.stderr.write("response is %s\n" % (ret))
    print("response is :%s" % ret)
            
    if ret["status_code"] == configs['server']['code']['succ']:
        mresults = ret["result"][0]
        if mresults['suggestion'] != configs['server']['result']['illegal']:
            print pic_info.src + " suggestion ok"
            return
        else:   # picture is illegal,and need to delete
            delete_local_file(pic_info.nail)
            delete_local_file(pic_info.square)
            my_dst = configs['picture']['fail'] + pic_info.src
            my_src = configs['picture']['root'] + pic_info.src
            if os.path.exists(my_src) :
                shutil.move(my_src, my_dst)
                delete_cdn_cache(configs['cdn']['refresh'] + pic_info.src)
                print("Picture[%s] move to [%s] success" % (pic_info.src, my_dst))
        end = time.time()
        print "cost time: " + str(end - start)
    elif ret["status_code"] == configs['server']['code']['retry']:
        raise Exception,"Yidun internal error!"

class InfoCache(object):
    def __init__(self):
        self.block = 0
        self.ip   = configs['cached']['host']
        self.port = configs['cached']['port']
        try:
            self.sock = socket.socket(socket.AF_INET)
            self.sock.connect((self.ip, self.port))
        except Exception, err:
            sys.stderr.write("Connect cache server err %s\n" % err)
            exit(-1)

    def sendandrecv_(self, send_buf):
        send_len = self.sock.send(send_buf)
        recv_buf = self.sock.recv(4)
        while len(recv_buf) < 4:
            recv_buf += self.sock.recv(4 - len(recv_buf))
        recv_len = struct.unpack("=L", recv_buf[:4])[0]
        while len(recv_buf) < recv_len:
            recv_buf += self.sock.recv(recv_len - len(recv_buf))

        head = struct.unpack("=LLHLL", recv_buf[:18])
        if head[3] != 0:
            sys.stderr.write("Redis internel errno[%d]" % head[3])
            return
        if head[0] > 18:
            body = imgCheck_pb2.imgPopResp()
            body.ParseFromString(recv_buf[18:])
            return body.info
        return

    def pack_(self, body_buf, cmdid):
        head_buf = struct.pack("=LLHLL", 18 + len(body_buf), 0, cmdid, 0, 0)
        return head_buf + body_buf

    def get_(self, retry):
        protobuf_ = imgCheck_pb2.imgPopRqst()
        protobuf_.retry = retry
        resp = self.sendandrecv_(self.pack_(protobuf_.SerializeToString(), 0xB102))
        if resp:
            self.block = 0
            print("Handle img check request:", resp)
            return resp
        else:
            if self.block <= 20:
                self.block += 4
        print("No data and sleep %d\n", self.block)
        time.sleep(self.block)
        return

    def add_(self, info_buf):
        protobuf_ = imgCheck_pb2.imgPush()
        protobuf_.retry = True
        protobuf_.info = info_buf
        self.sendandrecv_(self.pack_(protobuf_.SerializeToString(), 0xB101))

if __name__ == "__main__":
    redis_proxy = InfoCache()   
 
    retry=False
    resp = False
    while (True) :
        try:
            resp = redis_proxy.get_(False)
            if resp:
                check_by_url(resp)
            else:
#                print "No data in check queue, get from retry queue\n"
                resp = redis_proxy.get_(True)
                if resp:
                    retry=True
                    check_by_url(resp)
#                else:
#                    print "No data in retry queue, continue get from check queue\n"
            sys.stdout.flush()
        except requests.exceptions.ConnectionError, log:
            print log
        except Exception, log:
            if retry:
                sys.stderr.write("Internal error %s, won`t try url[%s] again\n" % (log, resp))
                retry = False
            elif resp:
                redis_proxy.add_(resp)
                print("Internal error %s, push url[%s] to retry queue\n" % (log, resp))
            else:
                sys.stderr.write("Other error : %s" % log)
