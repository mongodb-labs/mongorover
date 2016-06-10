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

EXPORT_ASSERT_TO_GLOBALS = true

require('luaHelperFunctions')
require("luaunit")

dofile("setReleaseType.lua")
local importPrepend = ""
if _G["__MONGOROVER_TEST_ENVIRONMENT"] then
	package.path = package.path .. ';./src/?.lua;./test/?.lua;../src/?.lua'
	package.cpath = "../?.dylib;../?.so"
else
	importPrepend = "mongorover."
end

local BaseTest = require("BaseTest")
local MongoClient = require(importPrepend .. "MongoClient")


TestClient = {}
setmetatable(TestClient, {__index = BaseTest})

	function TestClient:test_client_authenticate()
		self.client:getDatabaseNames()
	end
	
	function TestClient:test_client_call_metamethod()
		-- other_client uses MongoClient(...) syntax, self.client uses MongoClient.new(...)
		-- make sure they reference the same collection

		local client_db_names = self.client:getDatabaseNames()

		local other_client = MongoClient(self.db_uri)
		local other_client_db_names = other_client:getDatabaseNames()
		
		assertTrue(table_eq(client_db_names, other_client_db_names),
								"__call metamethod not instantiating the same as MongoClient.new(...)")
		
		self.database:drop_database()
		
		client_db_names = self.client:getDatabaseNames()
		other_client_db_names = other_client:getDatabaseNames()
		assertTrue(table_eq(client_db_names, other_client_db_names),
								"__call metamethod not instantiating the same as MongoClient.new(...)")
	end
	
	function TestClient:test_client_index_into_database()	
		-- other_database will use the index syntax, self.database uses :getDatabase(...)

		assertEquals(self.database_name, "mr_test_suite", "this test assumes collection name is mr_test_suite")

			-- ensure collection exists
		self.collection:insert_one({})
		local other_database = self.client.mr_test_suite
		assertTrue(other_database:hasCollection("foo"), "indexing from client to database doesn't work")
		self.collection:drop() 

		-- just dropped the database, shouldn't exist
		assertFalse(other_database:hasCollection("foo"), "indexing from client to database doesn't work")
		
		-- reinitialize database since it got dropped
		self.collection = self.database.foo
			self.collection:insert_one({})
		
		-- collection should exist
		assertTrue(other_database:hasCollection("foo"), "indexing from client to database doesn't work")
	end
	
	function TestClient:test_client_no_authentication()
		local client_no_auth = MongoClient.new("mongodb://localhost:27017/")
		assertErrorMsgContains("not authorized on admin to execute command { listDatabases: 1 }",
														function() client_no_auth:getDatabaseNames() end,
														"incorrect error returned, could be wrong auth settings")
	end
	
	function TestClient:test_no_gc_until_database_gc()
		self.client = nil
		collectgarbage()
		assertNotNil(self.database.client, "client garbage collected before database")
	end

lu = LuaUnit.new()
lu:runSuite()
