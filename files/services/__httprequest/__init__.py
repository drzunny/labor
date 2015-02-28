# -*-coding:utf8-*-
import urllib

def subscript(req):
    args = req['args']
    if 'url' not in args:
        raise ValueError('Url not found in args')
    resp = urllib.urlopen(args['url'])
    print(resp.read())
