#!/usr/bin/python
#encoding=utf8
import socket, struct, hashlib, sys

def usage():
    print("Add attention need 3 parameters, attention.py mid aid gname")
    print("mid is my mimi id")
    print("aid is attented mimi id")
    print("gname is the group add this relation")

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

def pack(userid, time):
#    body_buf = struct.pack("=I", time)
    head_buf = struct.pack("=IIHIII", 18 + 4, 0, 0xA101, 0, userid, time)
    send_buf = head_buf
#    send_buf += body_buf
    return send_buf

def unpack(recv_buf, body_unpack_f = ''):
    if len(recv_buf) < 18: raise Exception()
    head = struct.unpack("=IIHII", recv_buf[:18])
    if head[0] > 18:
        body1 = struct.unpack('=64s', recv_buf[18:82])
        body2 = struct.unpack('=64s', recv_buf[82:146])
        return str(head) + "\n" + str(body1) + "\n" + str(body2)
    else:
        body=()
        return str(head)

if __name__ == '__main__':
    IP = '127.0.0.1'
    PORT = 21145
    
    if len(sys.argv) != 3:
        usage()
    else:
        mid = (int)(sys.argv[1])
        time = (int)(sys.argv[2])
        print unpack(send_data(pack(mid, time), IP, PORT, True))
