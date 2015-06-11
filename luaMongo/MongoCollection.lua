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

ObjectId = require("ObjectId")
BSONNull = require("BSONNull")
InsertOneResult = require("InsertOneResult")

local MongoCollection = {__mode="k"}
MongoCollection.__index = MongoCollection

	--- Creates a new MongoCollection instance.
	-- @param database A MongoDatabase instance.
	-- @param collection_name
	function MongoCollection.new(database, collection_name)
		local self = setmetatable({}, MongoCollection)
		self.database = database
		self.collection_t = MongoModule.collection_new(database.database_t, collection_name)
		return self
	end
	
	--- Drops collection.
	function MongoCollection:drop()
		ret = self.collection_t:collection_drop()
		return ret
	end
	
	function MongoCollection:find_one(query, fields)
		return self.collection_t:collection_find_one(query, fields)
	end
	
	function MongoCollection:insert_one(doc)
		return self.collection_t:collection_insert_one(doc)
	end
	
	-- Converts Lua table to BSON string and prints it in C layer in JSON.
	function MongoCollection:convertLuaTableToBSON(table)
		self.collection_t:convert_lua_table_to_bson(table)
	end
	
	function MongoCollection:makeRandomObjectId()
		return self.collection_t:make_random_object_id()
	end
	
return MongoCollection