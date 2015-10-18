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
    time.sleep(2)
    lb.use('__echo', what='single test!!!!')
    lb.use('__pyinclude')
    lb.use('__luainclude')
    time.sleep(10)
    
