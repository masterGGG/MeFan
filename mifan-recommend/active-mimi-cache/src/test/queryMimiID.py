#!/usr/bin/python
#encoding=utf8
import socket, struct, hashlib, sys
def usage():
    print("Query attention need 3 parameters, attention.py mid")
    print("mid is my mimi id")

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

def pack(b, e, c):
    head_buf = struct.pack("=IIHIIIII", 18 + 12, 0, 0xA102, 0, 122740110, b, e, c)
    send_buf = head_buf
    return send_buf

def unpack(recv_buf, count, body_unpack_f = ''):
    if len(recv_buf) < 18: raise Exception()
    head = struct.unpack("=IIHII", recv_buf[:18])
    if head[3] == 0:
        if head[0] == 18:
            return "No mimiId query"
        num = struct.unpack('=I', recv_buf[18:22])
        print("Total error counts %d" % count)
        if num[0] == 0:
            return count
        for i in range(num[0]):
            print struct.unpack('=I', recv_buf[22+4*i:26+4*i])
#            body1 = struct.unpack('=64s', recv_buf[18:82])
    else:
        print("Error %d" % head[3])
        count = count + 1
        print("Total error counts %d" % count)
    return count

import time
if __name__ == '__main__':
    IP = '127.0.0.1'
    PORT = 21145
    count = 0
    
    if len(sys.argv) != 4:
        usage()
    else:
        begin = (int)(sys.argv[1])
        end = (int)(sys.argv[2])
        count = (int)(sys.argv[3])
        count = unpack(send_data(pack(begin, end, count), IP, PORT, True), count)
#            time.sleep(1)
