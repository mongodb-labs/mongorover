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

local MongoModule = require("mongo_module")
local MongoDatabase = require("mongorover.MongoDatabase")

--- Tools for connecting to MongoDB.
-- @module mongorover.MongoClient

----
-- Tools for connecting to MongoDB.
-- @type mongorover.MongoClient

local MongoClient = {}

	---
	-- Creates a MongoClient instance.
	-- For documentation to create a mongoDB connection URI (http://docs.mongodb.org/manual/reference/connection-string/).
	-- @tparam string db_uri The MongoDB connection URI.
	-- @return A @{MongoClient} instance.
	function MongoClient.new(db_uri)
		db_uri = db_uri or "mongodb://localhost:27017"
		
		local self = {
			client_t = MongoModule.client_new(db_uri)
		}
		setmetatable(self, {
			__index = function(table, key)
				-- rawget(...) is the same as indexing into a table, however it does not invoke the metatable __index call if the key is not found
				-- which would cause an infinite loop. This will emulate the same behavior as doing MongoClient.__index = MongoClient
				-- but allows for us to get databases by indexing into the client
				if rawget(MongoClient, key) then
					return MongoClient[key]
				else
					return MongoClient.getDatabase(table, key)
				end
			end
		})
		return self
	end
	
	---
	-- Returns array of database names.
	-- @treturn {string,...} An array of database names.
	function MongoClient:getDatabaseNames()
		return self.client_t:client_get_database_names()
	end

	--- 
	-- Returns a MongoDatabase object.
	-- @tparam string database_name The name of the database.
	-- @treturn MongoDatabase A MongoDatabase object.
	-- @return A @{MongoDatabase} instance.
	function MongoClient:getDatabase(database_name)
		return MongoDatabase.new(self, database_name)
	end

local metatable = {
	__index = MongoClient,
	__call = function(table, ...)
					--table is the same as MongoClient, so just use MongoClient
					return MongoClient.new(...)
				end
}

setmetatable(MongoClient, metatable)
return MongoClient
