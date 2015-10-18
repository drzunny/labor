# -*-coding:utf8 -*-
from labor import Labor
import time
import zmq
import sys

if __name__ == '__main__':
    if len(sys.argv) > 1:
        addr = sys.argv[1]
    else:
        addr = '127.0.0.1:1808'
    lb = Labor(addr, Labor.TYPE_PUSHPULL)
    for i in xrange(0, 1000):        
        lb.use('__error')
        time.sleep(0.001)
