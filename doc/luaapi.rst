
.. luaapi:

luaapi -- Lua library to read/write DiscoDBs
====================================================================

The lua API links against libdiscodb at runtime and provides a thin wrapper to the functionality.

This api is currently **read only**

The following examples demonstrate a prebuilt ddb obtained using

.. code-block:: bash

    wget http://pkgs.bauman.space/discodb/myths.ddb -O /tmp/myths.ddb`





Basic discodb wrapper for Lua.

libdiscodb.so should be in :code:`LD_LIBRARY_PATH`



Open and list keys
------------------

.. code-block:: lua

    local ffi = require("ffi")
    local ddblib = ffi.load("libdiscodb.so", true)  -- Load lib into global
    local discodb = require("discodb")
    local db = discodb.open("/tmp/myths.ddb")
    local iter = db:keys()
    for i = 1,#iter do
        print(i, iter:next())
    end


Open and get individual key
---------------------------

.. code-block:: lua

    local ffi = require("ffi")
    local ddblib = ffi.load("libdiscodb.so", true)  -- Load lib into global
    local discodb = require("discodb")
    local db = discodb.open("/tmp/myths.ddb")
    local iter = db:get("Major")
    for i = 1,#iter do
        print(i, iter:next())
    end



Check Key Exists Before Accessing
---------------------------------
Depends on the control flow of choice.

Either handle the error of attempting to access :code:`next()` on a nil iter
or check iter is not nil before calling :code:`next()` on it.

.. code-block:: lua

    local ffi = require("ffi")
    local ddblib = ffi.load("libdiscodb.so", true)  -- Load lib into global
    local discodb = require("discodb")
    local db = discodb.open("/tmp/myths.ddb")
    local iter = db:get("Major")
    iter = db:get("key-should-not-exist")
    if iter then
        for i = 1,#iter do
            print(i, iter:next())
        end
    else
        print("key does not exist")
    end



