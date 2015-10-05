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

--- Collection level utilities for Mongo.
-- @module mongorover.MongoCollection

--- Collection level utilities for Mongo.
-- @type mongorover.MongoCollection
local MongoCollection = {__mode="k"}
MongoCollection.__index = MongoCollection

	---
	-- Creates a new MongoCollection instance. Usually called by MongoDatabase's getCollection(...) method. 
	-- @see MongoDatabase.getCollection
	-- @tparam MongoDatabase database A MongoDatabase instance.
	-- @tparam string collection_name The name of the collection.
	-- @return A @{MongoCollection} instance.
	function MongoCollection.new(database, collection_name)
		local self = setmetatable({}, MongoCollection)
		self.database = database
		self.collection_t = MongoModule.collection_new(database.database_t, collection_name)
		return self
	end
	
	---
	-- Drops collection.
	function MongoCollection:drop()
		self.collection_t:collection_drop()
	end
	
	---
	-- Returns the number of documents in a collection matching the query input parameter.
    -- Example usage at @{update_many.lua}.
	-- @tparam[opt] table query A table containing a query.
	-- @tparam[opt] int skip The number of documents to skip.
	-- @tparam[opt] int limit The maximum number of matching documents to return.
	-- @treturn int The number of documents matching the query provided.
	function MongoCollection:count(query, skip, limit)
		query = query or {}
		skip = skip or 0
		limit = limit or 0
		return self.collection_t:collection_count(query, skip, limit)
	end
	
	---
	-- Internal function used to create an anonymous function iterator that returns the next document.
	-- in the given cursor every time it is iterated over.
	-- @local
	-- @tparam MongoCollection collection Needs to instantiate with a reference to the collection to ensure the collection is
	-- not garbage collected before the cursor.
	-- @tparam MongoCursor mongo_cursor A cursor from the C wrapper.
	function createCursorIterator (collection, mongo_cursor)
		-- Table necessary to prevent MongoCollection from being garbage collected before cursor.
		-- Table has to have relevant information in it, to prevent garbage collection.
		local mongoCursorPointer = {collection=collection, cursor_t=mongo_cursor}
		setmetatable(mongoCursorPointer, {__mode = "k"})
		
		return function ()
                       return mongoCursorPointer["cursor_t"]:next()
                   end
	end
	
	---
	-- Selects documents in a collection and returns an iterator to the selected documents.
	-- Example usage at @{find.lua}.
	-- @tparam[opt] table query Specifies criteria using query operators. To return all documents, either
	-- do not use query parameter or pass in an empty document ({}).
	-- @tparam[opt] table fields projection  Specifies the fields to return using projection operators. Default value returns all fields.
	-- @treturn iterator An iterator with results.
	function MongoCollection:find(query, fields)
		query = query or {}
		fields = fields or {}
		local cursor_t = self.collection_t:collection_find(query, fields)
		return createCursorIterator(self, cursor_t)
	end
	
	---
	-- Returns one document that satisfies the specified query criteria.
	-- Example usage at @{find.lua}.
	-- @tparam[opt] table query Specifies criteria using query operators. 
	-- @tparam[opt] table fields Specifies the fields to return using projection operators. Default value returns all fields.
	-- @treturn table First document found with the query provided.
	function MongoCollection:find_one(query, fields)
		return self.collection_t:collection_find_one(query, fields)
	end
	
	---
	-- Update a single document matching the filter
	-- Example usage at @{update_one.lua}.
	-- @tparam table filter A query that matches the document to update.
	-- @tparam table update The modifications to apply.
	-- @tparam[opt] bool upsert If true, perform an insert if no documents match the filter.
	-- @return @{mongorover.resultObjects.UpdateResult}
	function MongoCollection:update_one(filter, update, upsert)
		upsert = upsert or false
		local raw_result = self.collection_t:collection_update_one(filter, update, upsert, false)
		return UpdateResult.new(raw_result)
	end
	
	---
	-- Update one or more documents that match the filter.
	-- Example usage at @{update_many.lua}.
	-- @tparam table filter A query that matches the documents to update.
	-- @tparam table update The modifications to apply.
	-- @tparam[opt] bool upsert If true, perform an insert if no documents match the filter.
	-- @return @{mongorover.resultObjects.UpdateResult}
	function MongoCollection:update_many(filter, update, upsert)
		upsert = upsert or false
		local raw_result = self.collection_t:collection_update_many(filter, update, upsert, true)
		return UpdateResult.new(raw_result)
	end
	
	---
	-- Insert a single document.
	-- Example usage at @{insert_one.lua}.
	-- @tparam table document The document to insert. Must be mutable. If the document does not have an _id field,
	-- one will be added automatically.
	-- @return @{mongorover.resultObjects.InsertOneResult}
	function MongoCollection:insert_one(document)
		return self.collection_t:collection_insert_one(document)
	end
	
	---
	-- Insert a list of documents.
	-- Example usage at @{insert_many.lua}.
	-- @tparam {table,...} documents A list of documents to insert.
	-- @tparam[opt] bool ordered If true (the default), documents will be inserted on the server serially, in the order provided.
	-- If false, documents will be inserted on the server in arbitrary order (possibly in parallel) and all documents inserts will
	-- be attempted
	-- @return @{mongorover.resultObjects.InsertManyResult}
	function MongoCollection:insert_many(documents, ordered)
		ordered = ordered or true
		return self.collection_t:collection_insert_many(documents, ordered)
	end
	
	---
	-- Delete a single document.
	-- Example usage at @{delete_one.lua}.
	-- @tparam table selector  Specifies criteria using query operators. 
	-- @return @{mongorover.resultObjects.DeleteResult}
	function MongoCollection:delete_one(selector)
		return self.collection_t:collection_delete_one(selector)
	end
	
	---
	-- Deletes all documents matching query selector.
	-- Example usage at @{delete_many.lua}
	-- @tparam table selector  Specifies criteria using query operators. 
	-- @return @{mongorover.resultObjects.DeleteResult}
	function MongoCollection:delete_many(selector)
		return self.collection_t:collection_delete_many(selector)
	end
	
	---
	-- Perform an aggregation using the aggregation framework on this collection.
	-- Example usage at @{aggregation.lua}.
	-- @tparam {table,...} aggregationPipeline A list of aggregation pipeline stages.
	-- @treturn iterator An iterator with results.
	function MongoCollection:aggregate(aggregationPipeline)
		local cursor_t = self.collection_t:collection_aggregate(aggregationPipeline)
		return createCursorIterator(self, cursor_t)
	end

return MongoCollection
