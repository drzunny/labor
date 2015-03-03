# -*-coding:utf8-*-

env_counter_number = 0

def subscript(req):
    # the variable is not read only. so we have to use `global` to refer it
    global env_counter_number
    env_counter_number += 1
    print('You have been request:%d' % env_counter_number)
