# -*-coding:utf8-*-

def subscript(req):
    args = req['args']
    if 'what' not in args:
        print('Hello world, Labor user')
    else:
        print('Hello world, %s' % args['what'])
