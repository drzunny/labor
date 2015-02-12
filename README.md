# Labor
---

Are you feeling very terrible when you want to execute some extra operation asynchronously
(like sending a email or a HTTP request to a callback URL) after your basic operation run successfully,
but you don't want to implement this feature by multi-threading or fork a new process?

**labor** is your good choice to solve it!

**labor** is a lightweight `serivce-framework` to finished to **EXTRA OPERATION**. It's asynchronously,
fast!

# Build
- - -

You should install Python 2.7.x and Gyp at first.

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

```