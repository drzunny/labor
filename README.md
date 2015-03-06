![labor](https://cdn3.iconfinder.com/data/icons/social-messaging-productivity-3/128/tools-128.png)

*logo image from Google*

**Not Fast, But Mini!**

# Labor
---

`Labor` is a Job Scheduler Service and it aims to be as little as possible to influence the server's performance. Saving your CPU and memory. 


# Build
- - -

Firstly, CMake(2.8+) is required. 

Secondly, `labor` is written by C++11, so your C++ compiler must support C++11's feature.

And then, Python 2.7 and Lua 5.1 are also necessary, `PYTHON_HOME` must be set into your environment variable.

### Windows

>  I only tested on MSVC 2012+

There is a VS2013 solution file in `/msvcbuild`.

> if python27_d.lib not found, copy the `python27.lib` and rename it to `python27_d.lib` in same directory

But using CMake to build is recommended.

All of dependence libraries have been put into `depends/win32_lib`.

### Ubuntu

Install `libzmq3`, `libzmq3-dev`, `lua5.1`, `lua5.1-dev`, `python2.7`, `python2.7-dev` and **CMake**

Ensure your GCC version is 4.4+

    CMake . && make

### CentOS

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

When labor has been startup, it will load all the service (both Lua and Python if you doesn't use `--disabled`).

You can write services yourself with Python/Lua, and put your serivce into $LABOR_SERVICES:

    $LABOR_SERVICE
    |
    +-- your_service_name
        |
        +------ __init__.py     # Python Service Entrypoint
        |
        +------ main.lua       -- Lua Serivce Entrypoint

> ### Client

Install labor's client library from `/client/py`

    python setup.py install

Here is the Demo:

```python

    from labor import Labor

    lb = Labor("127.0.0.1:8091", Labor.TYPE_PUBSUB)

    # call service registered in labor
    # these operation is non-blocking.
    lb.use('sendEmailMethod', 'helloworld@123.com', 'title', 'fuck')
    lb.use('httpRequest', 'http://127.0.0.1')
    lb.use('echo')

```
