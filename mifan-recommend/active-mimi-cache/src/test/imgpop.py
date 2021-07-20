#!/usr/bin/python
#encoding=utf8
import socket, struct, hashlib, sys
import imgCheck_pb2
from google.protobuf.json_format import MessageToJson,Parse

def send_data(send_buf, ip = None, port = None, log = False):
    s = socket.socket(socket.AF_INET)
    s.connect((ip, port))
    if log: print("-------------NEW CONNECTION--------------")
    send_len = s.send(send_buf)
    if log: print("send_len:", send_len)
    recv_buf = s.recv(4)
    while len(recv_buf) < 4:
        recv_buf += s.recv(4 - len(recv_buf))
    recv_len = struct.unpack("=L", recv_buf[:4])[0]
    while len(recv_buf) < recv_len:
        recv_buf += s.recv(recv_len - len(recv_buf))
    if log: print("recv_len:", recv_len)
    s.close()
    return recv_buf

def pack(userid):
    imgbuf = imgCheck_pb2.imgPopRqst()
    imgbuf.retry = True
    imgbuf.retry = False
    # imgbuf.info = "10.1.1.197-914496998-niumao.jpg"
    body_buf = imgbuf.SerializeToString()
    print("body len ", len(body_buf))
    head_buf = struct.pack("=LLHLL", 18 + len(body_buf), 0, 0xB102, 0, userid)
    send_buf = head_buf
    send_buf += body_buf
    return send_buf

def unpack(recv_buf, body_unpack_f = ''):
    if len(recv_buf) < 18: raise Exception()
    head = struct.unpack("=LLHLL", recv_buf[:18])
    if head[0] > 18:
        body = imgCheck_pb2.imgPopResp()
        body.ParseFromString(recv_buf[18:])
        return str(head) + "\n" + body.info + "\n"
    else:
        body=()
        return str(head)

if __name__ == '__main__':
    IP = '127.0.0.1'
    PORT = 6698
    
    user_id = 123456
    #print unpack(send_data(pack(user_id), IP, PORT, True), '=64s64s')
    print unpack(send_data(pack(user_id), IP, PORT, True))
