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

-- True when a release version in luarocks. Set to false to manually use cmake and develop/debug locally.
_G["RELEASE"] = true

-- Keep MongoDatabase local.
local MongoDatabase = nil

if _G["RELEASE"] then
	MongoModule = require("mongo_module")
	MongoDatabase = require("mongorover.MongoDatabase")

	ObjectId = require("mongorover.luaBSONObjects.ObjectId")
	BSONNull = require("mongorover.luaBSONObjects.BSONNull")
	InsertOneResult = require("mongorover.resultObjects.InsertOneResult")
	InsertManyResult = require("mongorover.resultObjects.InsertManyResult")
	UpdateResult = require("mongorover.resultObjects.UpdateResult")
	DeleteResult = require("mongorover.resultObjects.DeleteResult")
else
	package.cpath = package.cpath .. ";./c_wrapper/build/?.dylib;../c_wrapper/build/?.dylib;./c_wrapper/build/?.so;../c_wrapper/build/?.so"
	MongoModule = require("mongo_module")
	MongoDatabase = require("MongoDatabase")

	ObjectId = require("luaBSONObjects/ObjectId")
	BSONNull = require("luaBSONObjects/BSONNull")
	InsertOneResult = require("resultObjects/InsertOneResult")
	InsertManyResult = require("resultObjects/InsertManyResult")
	UpdateResult = require("resultObjects/UpdateResult")
	DeleteResult = require("resultObjects/DeleteResult")
end
	



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
		database_names = self.client_t:client_get_database_names()
		return database_names
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