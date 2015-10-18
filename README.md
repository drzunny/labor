# About labor
---

**labor** is a lightweight job service, it try to be as little as possible to influence your server's performance. :)


# Build
- - -

You need CMake(2.8+) and a C++ compiler with C++11 support (GCC 4.4+ and MSVC 2012+).


### for Windows

> 1. `PYTHON_HOME` must be set.
> 2. if python27_d.lib not found, copy the `python27.lib` and rename it to `python27_d.lib` in same directory

There is a VS2013 solution file in `/msvcbuild`, and you can find all the third-party libraries in `depends/win32_lib`

but i recommend to build labor by CMake.

### for Ubuntu/Debian

Install `libzmq3`, `libzmq3-dev`, `lua5.1`, `lua5.1-dev`, `python2.7`, `python2.7-dev` and **CMake**

Ensure your GCC version is 4.4+

    CMake . && make

### for CentOS

Install `lua`, `lua-devel`, `python`, `python-devel`, `zeromq3`, `zeromq3-devel`.

```
    cmake . && make
```


# How to use
- - -

> ### Server/Service

labor --conf='you conf file' <other options>

## Other Options:

+ **--conf <file>**: the directory of `labor.conf`
+ **-h/--help**: print helper text
+ **-v/--version**: print labor's version
+ **--disabled <lang>**: disabled the model, py or lua or none, defaults to none

The services will be loaded after `labor` startup. (some services will be ignored if you use `--disabled`<lang-of-service>)

You can write services yourself by Python/Lua, and put your service into $LABOR_SERVICES:

    $LABOR_SERVICE
    |
    +-- your_service_name
        |
        +------ __init__.py     # Python Service Entry point
        |
        +------ init.lua       -- Lua Service Entry point

> ### Client

Install labor's client library from `/client/py`

    python setup.py install

Here is the Demo:

```python

    from labor import Labor

    lb = Labor("127.0.0.1:1808")

    # call service registered in labor
    # these operation is non-blocking.
    lb.use('sendEmailMethod', addr='helloworld@123.com', title='title', text='hi')
    lb.use('httpRequest', url='http://127.0.0.1')
    lb.use('echo')

```
