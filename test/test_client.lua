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

package.path = package.path .. ';./luaMongo/?.lua;./test/?.lua;../luaMongo/?.lua'
MongoClient = require("MongoClient")
LuaUnit = require("luaunit")

TestClient = {}
	function TestClient:test_client_authenticate()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		client:getDatabaseNames()
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