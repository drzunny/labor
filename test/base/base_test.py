# -*-coding:utf8 -*-
from labor import Labor
import labor
import time
import zmq

if __name__ == '__main__':
    lb = Labor('*:1808', Labor.TYPE_PUBSUB)   
    for i in xrange(0, 10000):        
        lb.use('echo')
        time.sleep(0.001)
