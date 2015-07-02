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
InsertOneResult = require("resultObjects.InsertOneResult")
UpdateOneResult = require("resultObjects.UpdateOneResult")

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
	
	function createCursorIterator (collection, mongoCursor)
		local fetch_one = nil
		local cursor_t = mongoCursor
		-- Table necessary to prevent MongoCollection from being garbage collected before cursor.
		-- Table has to have relevant information in it, to prevent gc.
		local mongoCursorPointer = {collection=collection, cursor_t=mongoCursor}
		setmetatable(mongoCursorPointer, {__mode = "k"})
		
		return function ()
			fetch_one = mongoCursorPointer["cursor_t"]:next()
			   if fetch_one then
				   return fetch_one
				end
			 end
	end
	
	function MongoCollection:find(query, fields)
		cursor_t = self.collection_t:collection_find(self, query, fields)
		return createCursorIterator(self, cursor_t)
	end
	
	function MongoCollection:find_one(query, fields)
		return self.collection_t:collection_find_one(query, fields)
	end
	
	function MongoCollection:update_one(filter, update, upsert)
		upsert = upsert or false
		local raw_result = self.collection_t:collection_update_one(filter, update, upsert)
		return UpdateOneResult.new(raw_result)
	end
	
	function MongoCollection:insert_one(doc)
		return self.collection_t:collection_insert_one(doc)
	end
	
	function MongoCollection:makeRandomObjectId()
		return self.collection_t:make_random_object_id()
	end
	
return MongoCollection