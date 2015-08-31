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


require('luaHelperFunctions')
local LuaUnit = require("luaunit")

dofile("setReleaseType.lua")
local importPrepend = ""
if _G["__MONGOROVER_TEST_ENVIRONMENT"] then
	package.path = package.path .. ';./src/?.lua;./test/?.lua;../src/?.lua'
	package.cpath = "../?.dylib;../?.so"
else
	importPrepend = "mongorover."
end
local MongoClient = require(importPrepend .. "MongoClient")

TestClient = {}

	function TestClient:test_database_drop()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource = admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		
		local result = collection:insert_one({a = 1})
		assert(result.acknowledged == true, "insert_one failed")
		assert(InsertOneResult.isInsertOneResult(result))
		
		has_database = database:hasCollection("bar")
		assert(has_database == true, "database does not exist after insert_one(...)")
		
		collection:drop()
		has_database = database:hasCollection("bar")
		assert(has_database == false, "database did not get dropped")	
	end
	
	function TestClient:test_find_one_and_insert_one_with_id()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource = admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		
		collection:drop()
		
		local allDifferentTypes = {a = 1, b = 2, c = "C", d = "D", e = true, z = BSONNull.new(), _id = ObjectId.new("55830e73d2b38bf021417851")}
		local result = collection:insert_one(allDifferentTypes)
		assert(result.acknowledged == true, "insert_one failed")
		assert(InsertOneResult.isInsertOneResult(result))
		
		local check_result = collection:find_one({_id = result.inserted_id})
		assert(table_eq(check_result, allDifferentTypes), "insert_one and find_one documents do not match")
	end

	function TestClient:test_find_one_and_insert_one_without_id()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource = admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		
		local allDifferentTypes = {a = 1, b = 2, c = "C", d = "D", e = true, z = BSONNull.new()}
		
		local result = collection:insert_one(allDifferentTypes)
		assert(result.acknowledged == true, "insert_one failed")
		assert(InsertOneResult.isInsertOneResult(result))

		
		local check_result = collection:find_one({_id = result.inserted_id}	)
		assert(table_eq(check_result, allDifferentTypes), "insert_one and find_one documents do not match")
	end
	
	function TestClient:test_bad_key_in_document()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource = admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		
		local arrayAsKey = { [{1}] = 1 }
		local status, err = pcall(function() collection:insert_one(arrayAsKey) end)
		local indexOfError = string.find(err, "invalid key type: table")
		assert(status == false)
		assert(indexOfError)
	end
	
	function TestClient:test_insert_many()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource = admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("test")
		
		collection:drop()
		
		collection = database:getCollection("test")
		
		docs = {}
		for i = 1,5 do
			docs[i] = {}
		end
		
		result = collection:insert_many(docs)
		assert(InsertManyResult.isInsertManyResult(result))
		assert(isArray(result.inserted_ids))
		assert(#result.inserted_ids == 5)
		
		for _,doc in pairs(docs) do
			local _id = doc["_id"]
			assert(inArray(_id, result.inserted_ids))
			assert(collection:count({_id = _id}) == 1)
		end	
		
		docs = {}
		for i = 1,5 do
			docs[i] = {_id = i}
		end
		result = collection:insert_many(docs)
		assert(InsertManyResult.isInsertManyResult(result))
		assert(isArray(result.inserted_ids))
		assert(#result.inserted_ids == 5)
		
		for _,doc in pairs(docs) do
			local _id = doc["_id"]
			assert(inArray(_id, result.inserted_ids))
			assert(collection:count({_id = _id}) == 1)
		end		
	end
	
	function TestClient:test_find()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource = admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("test")
		
		collection:drop()
		collection = database:getCollection("test")
		
		local docs = {}
		for i = 1,5 do
			local result = collection:insert_one({ a = i < 3 })
			assert(result.acknowledged == true, "insert_one failed")
		end
		
		local results = collection:find({a = true})
		
		local numDocuments = 0
		for result in results do
			assert(ObjectId.isObjectId(result["_id"], "find did not return object ids"))
			numDocuments = numDocuments + 1
		end
		
		assert(numDocuments == 2, "inserted two documents and found a different number")
		
		-- Test finding 0 documents.
		numDocuments = 0
		results = collection:find({should_find_none = 0})
		for result in results do
			numDocuments = numDocuments + 1
		end
		assert(numDocuments == 0)
		
		-- Test for error on a bad operation.
		results = collection:find({a = {["$bad_op"] = 5}})
		local status, err = pcall(function() for result in results do print(result) end end)
		local indexOfError = string.find(err, "BadValue unknown operator: $bad_op")
		assert(status == false)
		assert(indexOfError)
	end
	
	function TestClient:test_update_one()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource = admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("test")
		
		collection:drop()
		collection = database:getCollection("test")
		
		local id1 = collection:insert_one({x = 5}).inserted_id
		
		local result = collection:update_one({}, {["$inc"] = {x = 1}})
		
		assert(UpdateResult.isUpdateResult(result))
		assert(result.modified_count == 1)
		assert(type(result.upserted_id) == "nil")
		assert(collection:find_one({_id = id1}).x == 6)
		
		local id2 = collection:insert_one({x = 1}).inserted_id
		result = collection:update_one({x = 6},  {["$inc"] = {x = 1}})
		assert(UpdateResult.isUpdateResult(result))
		assert(result.matched_count == 1)
		assert(result.modified_count == 1)
		assert(type(result.upserted_id) == "nil")
		assert(collection:find_one({_id = id1}).x == 7)
		assert(collection:find_one({_id = id2}).x == 1)
		
		result = collection:update_one({x = 2}, {["$set"] = {y = 1}}, true)
		assert(UpdateResult.isUpdateResult(result))
		assert(result.matched_count == 0)
		assert(result.modified_count == 0)
		assert(ObjectId.isObjectId(result.upserted_id))
	end
	
	function TestClient:test_update_many()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource = admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("test")
		
		collection:drop()
		collection = database:getCollection("test")
		
		collection:insert_one({x = 4, y = 3})
		collection:insert_one({x = 5, y = 5})
		collection:insert_one({x = 4, y = 4})
		
		local result = collection:update_many({x = 4}, {["$set"] = {y = 5}})
		assert(UpdateResult.isUpdateResult(result))
		assert(result.matched_count == 2)
		assert(result.modified_count == 2)
		assert(type(result.upserted_id) == "nil")
		
		result = collection:update_many({x = 5}, {["$set"] = {y = 6}})
		assert(UpdateResult.isUpdateResult(result))
		assert(result.matched_count == 1)
		assert(result.modified_count == 1)
		assert(type(result.upserted_id) == "nil")
		assert(collection:count({y = 6}) == 1)
		
		result = collection:update_many({x = 2}, {["$set"] = {y = 1}}, true)
		assert(UpdateResult.isUpdateResult(result))
		assert(result.matched_count == 0)
		assert(result.modified_count == 0)
		assert(ObjectId.isObjectId(result.upserted_id))
	end

	function TestClient:test_count()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource = admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("test")
		
		collection:drop()
		collection = database:getCollection("test")
		
		assert(collection:count() == 0)

		collection:insert_many({ {foo = "bar"}, {foo = "baz"} })
		local num_results = 0
		for result in collection:find({foo = "bar"}) do
			num_results = num_results + 1
		end
		assert(num_results == 1)
		assert(collection:count({foo = "bar"}) == 1)
		assert(collection:count({foo = {["$regex"] = "ba.*"}}) == 2)
	end
	
	function TestClient:test_aggregate()
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource = admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("test")
		
		collection:drop()
		local inserted_document = { foo = {1, 2} }
		collection:insert_one(inserted_document)
		inserted_document["_id"] = nil
		 local aggregationPipeline = { {["$project"] = {_id = false, foo = true}} }

		local results = collection:aggregate(aggregationPipeline)
				
		local result_array = {}
		local index = 1
		for result in results do
			result_array[index] = result
			index = index + 1
		end
				
		assert(#result_array == 1)
		assert(table_eq(inserted_document, result_array[1]))
	end
	
LuaUnit:run()
