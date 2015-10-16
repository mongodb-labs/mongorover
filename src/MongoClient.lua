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

local importPrepend = ""
if _G["__MONGOROVER_TEST_ENVIRONMENT"] then
	-- Overwrite cpath to ensure only the locally created debug build is imported.
	package.cpath = "../?.dylib;../?.so"
else
	importPrepend = "mongorover."
end

local MongoModule = require("mongo_module")
local MongoDatabase = require(importPrepend .. "MongoDatabase")
local ObjectId = require(importPrepend .. "luaBSONObjects.ObjectId")
local BSONNull = require(importPrepend .. "luaBSONObjects.BSONNull")

--- Tools for connecting to MongoDB.
-- @module mongorover.MongoClient

----
-- Tools for connecting to MongoDB.
-- @type mongorover.MongoClient

local MongoClient = {}
MongoClient.__index = MongoClient

	---
	-- Creates a MongoClient instance.
	-- For documentation to create a mongoDB connection URI (http://docs.mongodb.org/manual/reference/connection-string/).
	-- @tparam string db_uri The MongoDB connection URI.
	-- @return A @{MongoClient} instance.
	function MongoClient.new(db_uri)
		local self = setmetatable({}, MongoClient)
		self.client_t = MongoModule.client_new(db_uri)
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

return MongoClient
