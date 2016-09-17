#!/usr/bin/env python
#coding=utf-8
# => Author: Abby Cin
# => Mail: abbytsing@gmail.com
# => Created Time: Mon 12 Sep 2016 08:13:43 PM CST

import socket
import time
import sys
import os
from multiprocessing import Process

class Client:
    def __init__(self, ipaddr, port):
        self.fd = socket.socket();
        self.fd.connect((ipaddr, port))

    def __del__(self):
        self.fd.close()

    def io(self):
        self.fd.send("ping\n")
        self.fd.recv(5)

def dispatch(ipaddr, port):
    clients = []
    for i in range(0, 1000):
        clients.append(Client(ipaddr, port))

    pid = os.getpid()

    for i in range(0, 30):
        print 'child %d in round %d' %(pid, i + 1)
        time.sleep(60)
        for client in clients:
            #time.sleep(0.001)
            client.io()

def loop(server_num, child_num, ipaddr, port):
    saved_port = port
    workers = []
    for i in range(0, child_num):
        worker = Process(target = dispatch, args = (ipaddr, port))
        port += 1
        if port == (saved_port + server_num):
            port = saved_port
        worker.start()
        workers.append(worker)

    for worker in workers:
        worker.join()

if __name__ == '__main__':
    if len(sys.argv) != 5:
        print '%s server_num child_num server_ip start_port' %(sys.argv[0])
        sys.exit(1)

    server_num = int(sys.argv[1])
    child_num = int(sys.argv[2])
    ipaddr = sys.argv[3]
    port = int(sys.argv[4])
    loop(server_num, child_num, ipaddr, port)