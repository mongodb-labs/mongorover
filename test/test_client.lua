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
	function TestClient:test_client_authenticate()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		client:getDatabaseNames()
	end
	
	function TestClient:test_client_call_metamethod()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		
		-- make sure collection "foo" exists
		local database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		collection:insert_one({})
		
		local other_client = MongoClient("mongodb://user:password@localhost:27017/?authSource=admin")
		local client_db_names = client:getDatabaseNames()
		local other_client_db_names = other_client:getDatabaseNames()
		
		assert(table_eq(client_db_names, other_client_db_names), "__call metamethod not instantiating the same as MongoClient.new(...)")
		
		database:drop_database()
		
		client_db_names = client:getDatabaseNames()
		other_client_db_names = other_client:getDatabaseNames()
		assert(table_eq(client_db_names, other_client_db_names), "__call metamethod not instantiating the same as MongoClient.new(...)")
	end
	
	function TestClient:test_client_index_into_database()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")

		local database = client:getDatabase("foo")
		database:drop_database()
		
		local other_database = client.foo
		local has_database = other_database:hasCollection("bar")
		-- just dropped the database, shouldn't exist
		assert(has_database == false, "indexing from client to database doesn't work")
		
		-- reinitialize database since it got dropped
		database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		collection:insert_one({})
		
		local has_collection = other_database:hasCollection("bar")
		assert(has_collection == true, "indexing from client to database doesn't work")
	end
	
	function TestClient:test_client_no_authentication()
		local client = MongoClient.new("mongodb://localhost:27017/")
		local func = function() client:getDatabaseNames() end
		status, err = pcall( func )
		
		if err then
			local startInd, endInd = string.find(err, "not authorized on admin to execute command { listDatabases: 1 }")
			assertNotNil(startInd)
			assertNotNil(endInd)
		end
	end
	
	function TestClient:test_no_gc_until_database_gc()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		local database = client:getDatabase("students")
		client = nil
		collectgarbage()
		database = nil
		local func = function () collectgarbage() end
		status, err = pcall( func )
		
		if err then
			assert(err == -1, "client garbage collected before database was garbage collected")
		end
	end

LuaUnit:run()