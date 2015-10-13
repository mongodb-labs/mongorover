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

require('luaHelperFunctions')
local LuaUnit = require("luaunit")

dofile("setReleaseType.lua")
local importPrepend = ""
if _G["__MONGOROVER_TEST_ENVIRONMENT"] then
	package.path = package.path .. ';./src/?.lua;./test/?.lua;../src/?.lua'
	package.cpath = "../?.dylib;../?.so"
else
	importPrepend = "mongorover."
end
local MongoClient = require(importPrepend .. "MongoClient")


TestClient = {}
	function TestClient:test_database_drop()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		
		-- ensure database exists
		collection:insert_one({})
		
		local database_names = client:getDatabaseNames()
		assert(inArray("foo", database_names))
		
		database:drop_database()
		database_names = client:getDatabaseNames()
		assert(not inArray("foo", database_names))
	end
	
	function TestClient:test_get_collection_names()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		
		-- ensure database exists
		collection:insert_one({})
		
		local collection_names = database:getCollectionNames()
		assert(inArray("bar", collection_names))
		
		collection:drop()
		
		collection_names = database:getCollectionNames()
		assert(not inArray("bar", collection_names))
	end
	
	function TestClient:test_command()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		
		-- ensure database exists
		collection:drop()
		collection:insert_many({{x = 1}, {x = 1}, {x = 2}})
		
		-- Test default value defaulting to 1.
		local response = database:command("buildinfo")
		assert(response.ok)
		
		-- Test value given.
		response = database:command("collstats", "bar")
		assert(response.ok)
		
		-- Test command options.
		response = database:command("distinct", "bar", {key = "x"})
		local values = response.values
		
		assert(inArray(1, values))
		assert(inArray(2, values))		
	end

LuaUnit:run()