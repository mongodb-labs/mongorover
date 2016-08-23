==========
mongorover
==========
:Info: See `the mongo site <http://www.mongodb.org>`_ for more information. See `github <https://github.com/10gen-labs/mongorover>`_ for the latest source.
:Author: Christopher Wang
:Maintainer: Jesse Davis

About
=====

The mongorover module contains tools for interacting with MongoDB databases from Lua. It wraps both the 1.2.0-dev version of the `MongoDB C Driver <https://github.com/mongodb/mongo-c-driver/tree/1.2.0-dev>`_ and `libbson <https://github.com/mongodb/libbson>`_ library. Currently, this driver only provides simple functionality: the basic CRUD operations, aggregation, along with functions such as getDatabaseNames and getCollectionNames. While this driver doesn't provide all the specifications within the `MongoDB Driver CRUD API <https://github.com/mongodb/specifications/blob/master/source/crud/crud.rst>`_, the parts that are implemented do comply with their respective specs.


Bugs / Feature Requests
=======================

Think you’ve found a bug? Want to see a new feature in mongorover? Please open a github issue on this repository.


Please include all of the following information when opening an issue:

- Detailed steps to reproduce the problem, including full traceback, if possible.
- The exact lua version used ::

  $ lua -e "print(_VERSION)"

- The exact version of mongorover used ::

  $ lua -e "print(require("mongorover")._VERSION)"


Security Vulnerabilities
------------------------

If you’ve identified a security vulnerability in a driver or any other
MongoDB project, please report it according to the `instructions here
<http://docs.mongodb.org/manual/tutorial/create-a-vulnerability-report>`_.

Installation
============

Before installing mongorover, you have to install the MongoDB C Driver version 1.2, which is in beta at the time of this writing. Please download the latest 1.2 beta release from `the C Driver releases page <https://github.com/mongodb/mongo-c-driver/releases>`_ and follow the `Mongo C Driver installation guide <http://api.mongodb.org/c/current/installing.html#build-yourself>`_. If building on Windows, please refer to the Window specific section.

After installing the above C libraries successfully, you can install mongorover through the `LuaRocks
<https://luarocks.org/>`_ build system. ::

  $ luarocks install mongorover
  
You can also use LuaRocks_ to install from source ::
  
  $ luarocks make mongorover*.rockspec


Alternatively, you can build using `CMake <https://cmake.org/>`_, even though it is highly encouraged to use the Luarocks ecosystem. You can build by executing the following commands in the home directory. Take note that unless forced to use Lua 5.1 (see comment below), CMake_ will use the highest version of Lua found. ::

  $ mkdir build
  $ cd build
  $ cmake .. # can force Lua 5.1 by using -DLUA_FORCE_LUA51=ON
  $ make .


Dependencies
============

The mongorover distribution is supported and tested on Lua 5.1.5 and 5.2.3.

Additional dependencies are:

- `MongoDB C Driver <https://github.com/mongodb/mongo-c-driver/tree/1.2.0-dev>`_
- To generate documentation: `ldoc <https://github.com/stevedonovan/LDoc>`_
- To run the tests: `Luaunit <https://github.com/bluebird75/luaunit>`_. At the current moment, the luarock is outdated, so please install from github.

Examples
========
A basic use case of using the mongorover driver is below. For more see the *examples* section of the mongorover docs and see the `MongoDB CRUD Tutorial <http://docs.mongodb.org/manual/applications/crud/>`_ and the `MongoDB Aggregation Tutorial <http://docs.mongodb.org/manual/core/aggregation-introduction/>`_.

.. code-block:: lua

  > mongorover = require("mongorover")
  > client = mongorover.MongoClient.new("mongodb://localhost:27017/")
  > database = client:getDatabase("exampleDatabase")
  > collection = database:getCollection("exampleCollection")
  > result = collection:insert_one({x = 10})
  > print(result, result.inserted_id)
  <InsertOneResult object at table: 0x7fdc00f18340> ObjectID("559ff4bbd2b38b17296e56b1")

  > print(collection:insert_one({x = 8}).inserted_id)
  ObjectID("559ff4d0d2b38b17296e56b2")

  > print(collection:insert_one({x = 11}).inserted_id)
  ObjectID("559ff4d6d2b38b17296e56b3")

  > find_results = collection:find({})

  > for result in find_results do
  >> for k,v in pairs(result) do
  >>   print(k,v)
  >> end
  >> end

  _id ObjectID("559ff4bbd2b38b17296e56b1")
  x 10
  _id ObjectID("559ff4d0d2b38b17296e56b2")
  x 8
  _id ObjectID("559ff4d6d2b38b17296e56b3")
  x 11

Documentation
=============

The documentation is hosted online at `api.mongodb.org <http://api.mongodb.org/lua/current/>`_.

You will need ldoc_ installed to generate the
documentation. Documentation can be generated in the *doc/* directory by running the following command in the source directory. ::

  $ lua [path to ldoc.lua] .



Testing
=======

Before running tests, you need to make sure to be running a MongoDB in the background with authentication on. You can create the user to run the test suite with these commands in mongo shell. ::

  use admin
  db.createUser(
    {
      user: "mr_user",
      pwd: "mr_password",
      roles: [
         { role: "clusterMonitor", db: "admin" },
         { role: "readWrite", db: "mr_test_suite" },
         { role: "dbAdmin", db: "mr_test_suite" }
      ]
    }
  )

The easiest way to run the tests is to run the following command in the test directory. Note that you will need
Luaunit_ to run the tests. Make sure you installed luaunit from source instead of luarocks. ::

  $ lua RunAllTests.lua
