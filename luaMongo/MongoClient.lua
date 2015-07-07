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

package.cpath = package.cpath .. ";./c_wrapper/build/?.dylib;../c_wrapper/build/?.dylib;./c_wrapper/build/?.so;../c_wrapper/build/?.so"
MongoModule = require("mongo_module")
local MongoDatabase = require("MongoDatabase")

local MongoClient = {}
MongoClient.__index = MongoClient

	--- Creates a MongoClient instance.
	-- uses MongoDB connection URI (http://docs.mongodb.org/manual/reference/connection-string/).
	-- @param db_uri The MongoDB connection URI.
	function MongoClient.new(db_uri)
		local self = setmetatable({}, MongoClient)
		self.client_t = MongoModule.client_new(db_uri)
		return self
	end

	--- Returns array of database names.
	function MongoClient:getDatabaseNames()
		database_names = self.client_t:client_get_database_names()
		return database_names
	end

	--- Returns a MongoDatabase object
	-- @param database_name The name of the database.
	function MongoClient:getDatabase(database_name)
		return MongoDatabase.new(self, database_name)
	end

return MongoClient