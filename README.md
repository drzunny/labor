![labor](https://cdn3.iconfinder.com/data/icons/social-messaging-productivity-3/128/tools-128.png)

*logo image from Google*

## Not Fast, But Mini!

# Labor
---

`Labor` is a Job Scheduler Service. It aims to be as little as possible to influence the server's performance,
less CPU usage, less Memory usage.

You can use Lua/Python as the service language. Put your service code into `$LABOR_SERIVCE` directory and restart
labor, your service will be loaded automatically.


# Build
- - -

> labor support Windows and Linux

You should install Python 2.7.x and Gyp at first.

if you are Windows user:

+ Visual Studio 2013  (C++11 Required)

And, you are linux user:

+ At least GCC 4.4. (C++11 required)

Then, `gyp build.gyp`, the project files will be generated (MSVC project on Windows, Makefile on *nix). Build it and Enjoy it~


# How to use
- - -

> As a Service

labor --conf='you conf file' <other options>

## Other Options:

+ **--mode <mode>**: debug, normal, optimized. choose a running mode for you service
+ **--conf <file>**: the directory of `labor.conf`
+ **-h/--help**: print helper text
+ **-v/--version**: print labor's version
+ **--disabled <lang>**: disabled the model, py or lua or none, defaults to none

When labor has been startup, it will load all the service (both Lua and Python if you doesn't use `--disabled`).

> As a client

For our production environment, only Python client library support now, other language will be supported in future version.

you can find `setup.py` in `client/py` directory, use `python setup.py install`.

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
