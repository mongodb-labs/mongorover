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
local MongoCursor = require("mongorover.MongoCursor")
local CursorType = require("mongorover.CursorType")
local luaBSONObjects = require("mongorover.luaBSONObjects")
local InsertOneResult = require("mongorover.resultObjects.InsertOneResult")
local InsertManyResult = require("mongorover.resultObjects.InsertManyResult")
local UpdateResult = require("mongorover.resultObjects.UpdateResult")
local DeleteResult = require("mongorover.resultObjects.DeleteResult")

--- Collection level utilities for Mongo.
-- @module mongorover.MongoCollection

--- Collection level utilities for Mongo.
-- @type mongorover.MongoCollection
local MongoCollection = {__mode="k"}
	
	---
	-- Creates a new MongoCollection instance. Usually called by MongoDatabase's getCollection(...) method. 
	-- @see MongoDatabase.getCollection
	-- @tparam MongoDatabase database A MongoDatabase instance.
	-- @tparam string collection_name The name of the collection.
	-- @return A @{MongoCollection} instance.
	function MongoCollection.new(database, collection_name)
		local self = {
			database = database,
			collection_t = MongoModule.collection_new(database.database_t, collection_name)
		}
		
		setmetatable(self, {
			__index = MongoCollection
		})
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
		return self.collection_t:collection_count(luaBSONObjects, query, skip, limit)
	end

	---
	-- Selects documents in a collection and returns an iterator to the selected
	-- documents.
	-- Example usage at @{find.lua}.
	-- @tparam[opt] table query Specifies criteria using query operators.
	-- @tparam[opt] table fields Specifies the fields to return using projection
	-- operators. Default value returns all fields.
	-- @tparam[opt] table options additional parameters to alter behaviour of
	-- find.
	-- @tparam[opt] int options.skip The number of matching documents to skip
	-- before returning results.
	-- @tparam[opt] int options.limit The maximum number of results to return.
	-- @tparam[opt] boolean options.no_cursor_timeout if false (the default), any
	--  returned cursor is closed by the server after 10 minutes of
	--  inactivity. If set to True, the returned cursor will never
	--  time out on the server. Care should be taken to ensure that
	--  cursors with no_cursor_timeout turned on are properly closed.
	--  cursor_type the type of cursor to return. The valid
	--  options are defined by @{mongorover.CursorType}
	-- @tparam[opt] boolean options.oplog_replay If true, set the oplogReplay
	-- query tag.
	-- @tparam[opt] int options.batch_size Limits the number of documents
	-- returned in a single batch.
	-- @return A @{MongoCursor} with results.
	function MongoCollection:find(query, fields, options)
		query = query or {}
		fields = fields or {}
		options = options or {}
		local default_options = {
			skip = 0,
			limit = 0,
			no_cursor_timeout = false,
			cursor_type = CursorType.NON_TAILABLE,
			allow_partial_results = false,
			oplog_replay = false,
			batch_size = 0
		}

		setmetatable(options, {__index = default_options})

		local rev_index_cursor_type = {}
		for _cursor_type, val in pairs(CursorType) do
			rev_index_cursor_type[val] = true
		end
		assert(type(options.cursor_type) == "number" and rev_index_cursor_type[options.cursor_type],
					"not a valid value for cursor_type")

		local query_flags = options.cursor_type

		-- more_query_flags is needed because bitwise support for Lua 5.1 to 5.3
		-- is not fun. It can be done in the C layer by just iterating through
		-- this list and or'ing all the values to query_flags.
		local more_query_flags = {}
		if no_cursor_timeout then
			more_query_flags[#more_query_flags+1] = CursorType._QUERY_OPTIONS.no_timeout
		end
		if allow_partial_results then
			more_query_flags[#more_query_flags+1] = CursorType._QUERY_OPTIONS.partial
		end
		if oplog_replay then
			more_query_flags[#more_query_flags+1] = CursorType._QUERY_OPTIONS.oplog_replay
		end

		local cursor_t = self.collection_t:collection_find(luaBSONObjects,
																												query_flags,
																												more_query_flags,
																												options.skip,
																												options.limit,
																												options.batch_size,
																												query,
																												fields)

		return MongoCursor(self, cursor_t)
	end

	---
	-- Get a single document from the database. Input arguments are the same as
	-- @{MongoCollection:find}
	-- Example usage at @{find.lua}.
	-- @tparam[opt] table query Specifies criteria using query operators.
	-- @tparam[opt] table fields Specifies the fields to return using projection
	-- operators. Default value returns all fields.
	-- @tparam[opt] table options additional parameters to alter behaviour of
	-- find.
	-- @tparam[opt] int options.skip The number of matching documents to skip
	-- before returning results.
	-- @tparam[opt] int options.limit The maximum number of results to return.
	-- @tparam[opt] boolean options.no_cursor_timeout if false (the default), any
	--  returned cursor is closed by the server after 10 minutes of
	--  inactivity. If set to True, the returned cursor will never
	--  time out on the server. Care should be taken to ensure that
	--  cursors with no_cursor_timeout turned on are properly closed.
	--  cursor_type the type of cursor to return. The valid
	--  options are defined by @{mongorover.CursorType}
	-- @tparam[opt] boolean options.oplog_replay If true, set the oplogReplay
	-- query tag.
	-- @tparam[opt] int options.batch_size Limits the number of documents
	-- returned in a single batch.
	-- @treturn table First document found with the query provided.
	function MongoCollection:find_one(query, fields, options)
		options = options or {}
		options.limit = 1

		local results = self:find(query, fields, options)
		local ret_val
		for result in results do
			ret_val = result
			break
		end

		return ret_val
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
		local raw_result = self.collection_t:collection_update_one(luaBSONObjects, filter, update, upsert, false)
		return UpdateResult(raw_result)
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
		local raw_result = self.collection_t:collection_update_many(luaBSONObjects, filter, update, upsert, true)
		return UpdateResult(raw_result)
	end
	
	---
	-- Insert a single document.
	-- Example usage at @{insert_one.lua}.
	-- @tparam table document The document to insert. Must be mutable. If the document does not have an _id field,
	-- one will be added automatically.
	-- @return @{mongorover.resultObjects.InsertOneResult}
	function MongoCollection:insert_one(document)
		local acknowledged, inserted_id = self.collection_t:collection_insert_one(luaBSONObjects, document)
		return InsertOneResult(acknowledged, inserted_id)
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
		local raw_result, inserted_ids = self.collection_t:collection_insert_many(luaBSONObjects, documents, ordered)
		return InsertManyResult(raw_result, inserted_ids)
	end
	
	---
	-- Delete a single document.
	-- Example usage at @{delete_one.lua}.
	-- @tparam table selector  Specifies criteria using query operators. 
	-- @return @{mongorover.resultObjects.DeleteResult}
	function MongoCollection:delete_one(selector)
		local acknowledged, raw_result = self.collection_t:collection_delete_one(luaBSONObjects, selector)
		return DeleteResult(acknowledged, raw_result)
	end
	
	---
	-- Deletes all documents matching query selector.
	-- Example usage at @{delete_many.lua}
	-- @tparam table selector  Specifies criteria using query operators. 
	-- @return @{mongorover.resultObjects.DeleteResult}
	function MongoCollection:delete_many(selector)
		local acknowledged, raw_result = self.collection_t:collection_delete_many(luaBSONObjects, selector)
		return DeleteResult(acknowledged, raw_result)
	end
	
	---
	-- Perform an aggregation using the aggregation framework on this collection.
	-- Example usage at @{aggregation.lua}.
	-- @tparam {table,...} aggregationPipeline A list of aggregation pipeline stages.
	-- @treturn iterator An iterator with results.
	function MongoCollection:aggregate(aggregationPipeline)
		local cursor_t = self.collection_t:collection_aggregate(luaBSONObjects, aggregationPipeline)
		return MongoCursor(self, cursor_t)
	end

	---
	-- Create an index on any field or combination of fields in the collection.
	-- Example usage at @{indexing.lua}.
	-- @tparam table of fields and their corresponding index types
	-- @tparam table of options to be used when creating index
	-- @return string index name
	function MongoCollection:createIndex(keys,opt)
	    return self.collection_t:collection_create_index(luaBSONObjects, keys, opt)
	end

	---
	-- Delete an index for a specified collection.
	-- Example usage at @{indexing.lua}.
	-- @tparam string an index name or original table with indexed fields and their corresponding types
	-- @return a boolean true if index deleted successfully
	function MongoCollection:dropIndex(index)
	    return self.collection_t:collection_drop_index(luaBSONObjects, index)
	end

	---
	-- Retrieve an index for this collection
	-- Example usage at @{indexing.lua}.
	-- @return A @{MongoCursor} with results.
	function MongoCollection:findIndexes()
		local cursor_t = self.collection_t:collection_find_indexes(luaBSONObjects)
		return MongoCursor(self, cursor_t)
	end


local metatable = {
	__index = MongoCollection,
	__call = function(table, ...) 
					-- table is the same as MongoCollection
					return MongoCollection.new(...)
				end
}

setmetatable(MongoCollection, metatable)
return MongoCollection
