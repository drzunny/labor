Put all the service module here. Each module should have it's own directory, like:

    + services/
    |
    +--+--- moduleA
       |      |
       |      +--- start.py
       |      |
       |      +--- helpers.py
       |
       +--- moduleB
       |
       +--- moduleC


`labor` support Lua module and Python module as services.

module can do event thing if you want, but we recommend you should use a module like a sandbox.
