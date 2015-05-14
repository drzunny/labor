# -*- coding:utf8 -*-
try:
    import zmq
except ImportError, e:
    raise ImportError("libzmq not found, please install")

# Choose the fastest JSON library
try:
    import ujson as json
except ImportError:
    try:
        import simplejson as json
    except ImportError:
        import json

import re


# This client's env
env_py_labor_version = '0.0.5'
env_zmq_context = zmq.Context()
env_method_regex = re.compile(r'^[0-9]|[^\w]')

# Helpers
def _create_request(method, stype, headers, **kw):
    d = {
        'version': env_py_labor_version,
        'action': method,
        'args': json.dumps(kw),
        'headers': json.dumps(headers),
        'stype': stype
    }
    return json.dumps(d)


def _normalize_action_name(method):
    method_name = method.strip().lower()
    if method == '':
        raise ValueError('method name cannot be empty')
    r = env_method_regex.findall(method_name)
    if not r:
        return method_name
    else:
        raise ValueError('invalid method name: %s' % method)


# Implementations
class Labor(object):

    TYPE_PUSHPULL = 0

    def __init__(self, addr=None, con_type=-1):
        self.addr = None
        self.con_type = None
        self.connection = None

        if not addr or con_type not in (Labor.TYPE_PUSHPULL,):
            return
        else:
            self.connect(addr, con_type)

    def connect(self, addr, con_type):
        assert(addr is not None)
        if con_type not in (Labor.TYPE_PUSHPULL,):
            raise ValueError("invalid connection type")

        self.addr, self.con_type = addr, con_type
        # auto disconnect if using a same instance
        if self.connection:
            self.__disconnect()

        if con_type == Labor.TYPE_PUSHPULL:            
            self.connection = env_zmq_context.socket(zmq.PUSH)
            self.connection.sndhwm = 1100000
            self.connection.bind("tcp://%s" % addr)

    def use(self, method, headers={}, **kw):
        method = _normalize_action_name(method)
        req = _create_request(method, self.con_type, headers, **kw)

        # if this is a PUSHPULL operation, ignore the return
        self.connection.send(b'%s' % req)
        if self.con_type != Labor.TYPE_PUSHPULL:
            ret = self.connection.recv()
            return ret        

    def __disconnect(self):
        if self.connection:
            self.connection.disconnect('tcp://%s' % self.addr)
            self.addr, self.con_type = None, None

    def __del__(self):
        self.__disconnect()
        del self.addr
        del self.con_type
