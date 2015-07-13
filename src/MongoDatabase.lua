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

local mongorover_environment = _G["_MONGOROVER_RELEASE"]

local MongoCollection = nil
if mongorover_environment == nil or mongorover_environment then
	MongoCollection = require("mongorover.MongoCollection")
else
	MongoCollection = require("MongoCollection")
end

--- Database level operations.
-- @module mongorover.MongoDatabase

----
--- Database level operations.
-- @type mongorover.MongoDatabase

local MongoDatabase = {__mode="k"}
MongoDatabase.__index = MongoDatabase

	---
	-- Creates a new MongoDatabase instance. Called by MongoClient's getDatabase(...) method.
	-- @see MongoClient.getDatabase
	-- @tparam MongoClient client A MongoClient instance.
	-- @tparam string database_name
	-- @return A @{MongoDatabase} instance.
	function MongoDatabase.new(client, database_name)
		local self = setmetatable({}, MongoDatabase)
		self.database_t = MongoModule.database_new(client.client_t, database_name)
		self.client = client
		return self
	end
	
	---
	-- Creates MongoCollection instance.
	-- @tparam string collection_name Name of collection.
	-- @return A @{MongoCollection} instance.
	function MongoDatabase:getCollection(collection_name)
		return MongoCollection.new(self, collection_name)
	end

	---
	-- Returns array of collection names.
	-- @treturn {string,...} An array containing the names of collections in the database.
	function MongoDatabase:getCollectionNames()
		collection_names = self.database_t:get_collection_names()
		return collection_names
	end

	---
	-- Drops the database.
	function MongoDatabase:drop_database()
		self.database_t:database_drop()
		self.database_t = nil
	end
	
	---
	-- Returns boolean whether the collection is present in the database.
	-- @tparam string collectionName The name of the database.
	-- @treturn boolean A boolean value whether the database has the collection.
	function MongoDatabase:hasCollection(collectionName)
		ret = self.database_t:has_collection(collectionName)
		return ret
	end
	
	---
	-- Issue a command to MongoDB by and get response back.
	-- @tparam string command 
	-- @param[opt] value Value for command. Defaults to 1.
	-- @tparam table options Additional options for database command.
	-- @treturn table Response from server.
	function MongoDatabase:command(command, value, options)
		value = value or 1
		code_options = code_options or nil
		return self.database_t:command_simple(command, value, options)
	end

return MongoDatabase