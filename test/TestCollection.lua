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

EXPORT_ASSERT_TO_GLOBALS = true

require('luaHelperFunctions')
require("luaunit")

dofile("setReleaseType.lua")
local importPrepend = ""
if _G["__MONGOROVER_TEST_ENVIRONMENT"] then
	package.path = package.path .. ';./src/?.lua;./test/?.lua;../src/?.lua'
	package.cpath = "../?.dylib;../?.so"
else
	importPrepend = "mongorover."
end

local BaseTest = require("BaseTest")
local MongoClient = require(importPrepend .. "MongoClient")
local ObjectId = require(importPrepend .. "luaBSONObjects.ObjectId")
local BSONNull = require(importPrepend .. "luaBSONObjects.BSONNull")
local InsertOneResult = require(importPrepend .. "resultObjects.InsertOneResult")
local InsertManyResult = require(importPrepend .. "resultObjects.InsertManyResult")
local UpdateResult = require(importPrepend .. "resultObjects.UpdateResult")
local DeleteResult = require(importPrepend .. "resultObjects.DeleteResult")


TestCollection = {}
setmetatable(TestCollection, {__index = BaseTest})

	function TestCollection:test_database_drop()
		local result = self.collection:insert_one({a = 1})
		assertTrue(result.acknowledged, "insert_one failed")
		assertTrue(InsertOneResult.isInsertOneResult(result))
		
		has_database = self.database:hasCollection("foo")
		assertTrue(has_database, "database does not exist after insert_one(...)")
		
		self.collection:drop()
		has_database = self.database:hasCollection("bar")
		assertFalse(has_database, "database did not get dropped")	
	end
	
	function TestCollection:test_find_one_and_insert_one_with_id()
		local allDifferentTypes = {
			a = 1, b = 2, c = "C", d = "D", e = true,
			f = {gh_issue_number = "34"}, g = {key1 = "foo", key2 = "bar"},
			h = {1}, i = {1, 2},
			z = BSONNull.new(), _id = ObjectId.new("55830e73d2b38bf021417851")
		}

		local result = self.collection:insert_one(allDifferentTypes)
		assertTrue(result.acknowledged, "insert_one failed")
		assertTrue(InsertOneResult.isInsertOneResult(result))
		
		local check_result = self.collection:find_one({_id = result.inserted_id})
		assertTrue(table_eq(check_result, allDifferentTypes), "insert_one and find_one documents do not match")
	end

	function TestCollection:test_find_one_and_insert_one_without_id()
		local allDifferentTypes = {a = 1, b = 2, c = "C", d = "D", e = true, z = BSONNull.new()}
		
		local result = self.collection:insert_one(allDifferentTypes)
		assertTrue(result.acknowledged, "insert_one failed")
		assertTrue(InsertOneResult.isInsertOneResult(result))
		
		local check_result = self.collection:find_one({_id = result.inserted_id}	)
		assertTrue(table_eq(check_result, allDifferentTypes),
								"insert_one and find_one documents do not match")
	end
	
	function TestCollection:test_bad_key_in_document()
		local arrayAsKey = { [{1}] = 1 }
		assertErrorMsgContains("invalid key type: table",
														function() self.collection:insert_one(arrayAsKey) end)
	end
	
	function TestCollection:test_insert_many()
		docs = {}
		for i = 1,5 do
			docs[i] = {}
		end
		
		result = self.collection:insert_many(docs)
		assertTrue(InsertManyResult.isInsertManyResult(result))
		assertTrue(isArray(result.inserted_ids))
		assertEquals(#result.inserted_ids, 5)
		
		for _, doc in pairs(docs) do
			local _id = doc["_id"]
			assertTrue(inArray(_id, result.inserted_ids))
			assertEquals(self.collection:count({_id = _id}), 1)
		end	
		
		docs = {}
		for i = 1,5 do
			docs[i] = {_id = i}
		end
		result = self.collection:insert_many(docs)
		assertTrue(InsertManyResult.isInsertManyResult(result))
		assertTrue(isArray(result.inserted_ids))
		assertEquals(#result.inserted_ids, 5)
		
		for _,doc in pairs(docs) do
			local _id = doc["_id"]
			assertTrue(inArray(_id, result.inserted_ids))
			assertEquals(self.collection:count({_id = _id}), 1)
		end		
	end
	
	function TestCollection:test_find()
		local docs = {}
		for i = 1,5 do
			local result = self.collection:insert_one({ a = i < 3 })
			assert(result.acknowledged == true, "insert_one failed")
		end
		
		local results = self.collection:find({a = true})
		
		local numDocuments = 0
		for result in results do
			assertTrue(ObjectId.isObjectId(result["_id"]), "find did not return object ids")
			numDocuments = numDocuments + 1
		end
		
		assertEquals(numDocuments, 2, "inserted two documents and found a different number")
		
		-- Test finding 0 documents.
		numDocuments = 0
		results = self.collection:find({should_find_none = 0})
		for result in results do
			numDocuments = numDocuments + 1
		end
		assertEquals(numDocuments, 0)
		
		-- Test for error on a bad operation.
		results = self.collection:find({a = {["$bad_op"] = 5}})
		assertErrorMsgContains("BadValue unknown operator: $bad_op",
														function() for result in results do print(result) end end)
	end
	
	function TestCollection:test_update_one()
		local id1 = self.collection:insert_one({x = 5}).inserted_id
		local result = self.collection:update_one({}, {["$inc"] = {x = 1}})
		
		assertTrue(UpdateResult.isUpdateResult(result))
		assertEquals(result.modified_count, 1)
		assertNil(result.upserted_id)
		assertEquals(self.collection:find_one({_id = id1}).x, 6)
		
		local id2 = self.collection:insert_one({x = 1}).inserted_id
		result = self.collection:update_one({x = 6},  {["$inc"] = {x = 1}})

		assertTrue(UpdateResult.isUpdateResult(result))
		assertEquals(result.matched_count, 1)
		assertEquals(result.modified_count, 1)
		assertNil(result.upserted_id)
		assertEquals(self.collection:find_one({_id = id1}).x, 7)
		assertEquals(self.collection:find_one({_id = id2}).x, 1)
		
		result = self.collection:update_one({x = 2}, {["$set"] = {y = 1}}, true)

		assertTrue(UpdateResult.isUpdateResult(result))
		assertEquals(result.matched_count, 0)
		assertEquals(result.modified_count, 0)
		assertTrue(ObjectId.isObjectId(result.upserted_id))
	end
	
	function TestCollection:test_update_many()
		self.collection:insert_many({
			{x = 4, y = 3},
			{x = 5, y = 5},
			{x = 4, y = 4}
		})

		local result = self.collection:update_many({x = 4}, {["$set"] = {y = 5}})

		assertTrue(UpdateResult.isUpdateResult(result))
		assertEquals(result.matched_count, 2)
		assertEquals(result.modified_count, 2)
		assertNil(result.upserted_id)
		
		result = self.collection:update_many({x = 5}, {["$set"] = {y = 6}})

		assertTrue(UpdateResult.isUpdateResult(result))
		assertTrue(result.matched_count, 1)
		assertEquals(result.modified_count, 1)
		assertNil(result.upserted_id)
		assertEquals(self.collection:count({y = 6}), 1)
		
		result = self.collection:update_many({x = 2}, {["$set"] = {y = 1}}, true)

		assertTrue(UpdateResult.isUpdateResult(result))
		assertEquals(result.matched_count, 0)
		assertEquals(result.modified_count, 0)
		assertTrue(ObjectId.isObjectId(result.upserted_id))
	end

	function TestCollection:test_count()
		assert(self.collection:count() == 0)

		self.collection:insert_many({ {foo = "bar"}, {foo = "baz"} })

		local num_results = 0
		for result in self.collection:find({foo = "bar"}) do
			num_results = num_results + 1
		end

		assertEquals(num_results, 1)
		assertEquals(self.collection:count({foo = "bar"}), 1)
		assertEquals(self.collection:count({foo = {["$regex"] = "ba.*"}}), 2)
	end
	
	function TestCollection:test_aggregate()
		local inserted_document = { foo = {1, 2} }
		self.collection:insert_one(inserted_document)

		inserted_document["_id"] = nil
		local aggregationPipeline = { {["$project"] = {_id = false, foo = true}} }

		local results = self.collection:aggregate(aggregationPipeline)
				
		local result_array = {}
		local index = 1
		for result in results do
			result_array[index] = result
			index = index + 1
		end
				
		assertEquals(#result_array, 1)
		assertTrue(table_eq(inserted_document, result_array[1]))
	end
	
lu = LuaUnit.new()
lu:runSuite()
