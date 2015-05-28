# -*-coding:utf8 -*-
from labor import Labor
import time
import sys

if __name__ == '__main__':
    if len(sys.argv) > 1:
        addr = sys.argv[1]
    else:
        addr = '127.0.0.1:1808'
    lb = Labor(addr, Labor.TYPE_PUSHPULL)
    time.sleep(1)
    print 'Start...'
    t = time.time()
    for i in range(0, 100):
        lb.use('__luahello', name='lua test!!!!')

    print 'Done,' , time.time() - t
    time.sleep(10)
    
