--[[

Copyright 2015 MongoDB, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

--]]

package = "mongorover"
 version = "0.1-1"
 source = {
    url = "https://github.com/10gen-labs/mongorover",
    tag = "0.1",
 }
 description = {
    summary = "A Lua driver for MongoDB.",
    detailed = [[
       A Lua driver for MongoDB that currently supports the simple CRUD operations, aggregation, and simple commands. It wraps the new MongoDB C driver and conforms to the newly released CRUD specifications.
    ]],
    homepage = "http://api.mongodb.org/lua/current/",
    license = "Apache"
 }
 dependencies = {
    "lua >= 5.1, ~> 5.2"
    -- If you depend on other rocks, add them here
 }
 external_dependencies = {
    LIBLUA = {
      header = "lua.h"
    },
    LIBMONGOC = {
      header = "libmongoc-1.0/mongoc.h"
    },
    LIBBSON = {
      header = "libbson-1.0/bson.h"
    }
 }
 build = {
    type = "builtin",
     modules = {
        mongorover = "mongorover.lua",

        ["mongo_module"] = {
          sources = {"c_wrapper/mongo-module.c", "c_wrapper/lua-mongoc-client.c", "c_wrapper/lua-mongoc-database.c", "c_wrapper/lua-mongoc-collection.c", "c_wrapper/lua-bson.c", "c_wrapper/lua-version-compat.c", "c_wrapper/lua-object-generators.c", "c_wrapper/lua-mongo-cursor.c"},
          libraries = {"lua", "mongoc-1.0", "bson-1.0"},
          incdirs = {"$(LIBLUA_INCDIR)", "$(LIBMONGOC_INCDIR)/libmongoc-1.0", "$(LIBBSON_INCDIR)/libbson-1.0"},
          libdirs = {"$(LIBLUA_LIBDIR)", "$(LIBMONGOC_LIBDIR)", "$(LIBBSON_LIBDIR)"}
        },

        ["mongorover.MongoClient"] = "src/MongoClient.lua",
        ["mongorover.MongoDatabase"] = "src/MongoDatabase.lua",
        ["mongorover.MongoCollection"] = "src/MongoCollection.lua",
        ["mongorover.luaBSONObjects"] = "src/luaBSONObjects.lua",
          ["mongorover.luaBSONObjects.BSONNull"] = "src/luaBSONObjects/BSONNull.lua",
          ["mongorover.luaBSONObjects.ObjectId"] = "src/luaBSONObjects/ObjectId.lua",
        ["mongorover.resultObjects"] = "src/resultObjects.lua",
          ["mongorover.resultObjects.InsertOneResult"] = "src/resultObjects/InsertOneResult.lua",
          ["mongorover.resultObjects.InsertManyResult"] = "src/resultObjects/InsertManyResult.lua",
          ["mongorover.resultObjects.UpdateResult"] = "src/resultObjects/UpdateResult.lua",
          ["mongorover.resultObjects.DeleteResult"] = "src/resultObjects/DeleteResult.lua"
     }
 }