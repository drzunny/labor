# -*-coding:utf8 -*-
from labor import Labor
import time
import zmq
import sys

if __name__ == '__main__':
    if len(sys.argv) > 1:
        addr = sys.argv[1]
    else:
        addr = '*:1808'
    lb = Labor(addr, Labor.TYPE_PUBSUB)
    for i in xrange(0, 50000):        
        lb.use('echo')
        time.sleep(0.001)
