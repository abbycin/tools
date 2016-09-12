#!/usr/bin/env python
#coding=utf-8
# => Author: Abby Cin
# => Mail: abbytsing@gmail.com
# => Created Time: Mon 12 Sep 2016 08:13:43 PM CST

import socket
import time
import os
from multiprocessing import Process

class Client:
    def __init__(self):
        self.fd = socket.socket();
        self.fd.connect(("127.0.0.1", 8888))

    def __del__(self):
        self.fd.close()

    def io(self):
        self.fd.send("ping\n")
        self.fd.recv(5)

def dispatch():
    clients = []
    for i in range(0, 1000):
        clients.append(Client())

    pid = os.getpid()

    for i in range(0, 10):
        print 'child %d in round %d' %(pid, i + 1)
        time.sleep(30)
        for client in clients:
            #time.sleep(0.001)
            client.io()

if __name__ == '__main__':
    workers = []
    for i in range(0, 10):
        worker = Process(target = dispatch)
        worker.start()
        workers.append(worker)

    for worker in workers:
        worker.join()

