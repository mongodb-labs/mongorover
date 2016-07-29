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
local lu = require("luaunit")

local BaseTest = require("BaseTest")

local ObjectId = require("mongorover.luaBSONObjects.ObjectId")
local BSONNull = require("mongorover.luaBSONObjects.BSONNull")
local InsertOneResult = require("mongorover.resultObjects.InsertOneResult")
local InsertManyResult = require("mongorover.resultObjects.InsertManyResult")
local UpdateResult = require("mongorover.resultObjects.UpdateResult")


TestCollection = {}
setmetatable(TestCollection, {__index = BaseTest})

	function TestCollection:test_database_drop()
		local result = self.collection:insert_one({a = 1})
		lu.assertTrue(result.acknowledged, "insert_one failed")
		lu.assertTrue(InsertOneResult.isInsertOneResult(result))
		
		has_database = self.database:hasCollection("foo")
		lu.assertTrue(has_database, "database does not exist after insert_one(...)")
		
		self.collection:drop()
		has_database = self.database:hasCollection("bar")
		lu.assertFalse(has_database, "database did not get dropped")
	end
	
	function TestCollection:test_find_one_and_insert_one_with_id()
		local allDifferentTypes = {
			a = 1, b = 2, c = "C", d = "D", e = true,
			f = {gh_issue_number = "34"}, g = {key1 = "foo", key2 = "bar"},
			h = {1}, i = {1, 2},
			z = BSONNull.new(), _id = ObjectId.new("55830e73d2b38bf021417851")
		}

		local result = self.collection:insert_one(allDifferentTypes)
		lu.assertTrue(result.acknowledged, "insert_one failed")
		lu.assertTrue(InsertOneResult.isInsertOneResult(result))
		
		local check_result = self.collection:find_one({_id = result.inserted_id})
		lu.assertTrue(table_eq(check_result, allDifferentTypes), "insert_one and find_one documents do not match")
	end

	function TestCollection:test_find_one_and_insert_one_without_id()
		local allDifferentTypes = {a = 1, b = 2, c = "C", d = "D", e = true, z = BSONNull.new()}
		
		local result = self.collection:insert_one(allDifferentTypes)
		lu.assertTrue(result.acknowledged, "insert_one failed")
		lu.assertTrue(InsertOneResult.isInsertOneResult(result))
		
		local check_result = self.collection:find_one({_id = result.inserted_id}	)
		lu.assertTrue(table_eq(check_result, allDifferentTypes),
								"insert_one and find_one documents do not match")
	end
	
	function TestCollection:test_bad_key_in_document()
		local arrayAsKey = { [{1}] = 1 }
		lu.assertErrorMsgContains("invalid key type: table",
														function() self.collection:insert_one(arrayAsKey) end)
	end
	
	function TestCollection:test_insert_many()
		docs = {}
		for i = 1,5 do
			docs[i] = {}
		end
		
		result = self.collection:insert_many(docs)
		lu.assertTrue(InsertManyResult.isInsertManyResult(result))
		lu.assertTrue(isArray(result.inserted_ids))
		lu.assertEquals(#result.inserted_ids, 5)
		
		for _, doc in pairs(docs) do
			local _id = doc["_id"]
			lu.assertTrue(inArray(_id, result.inserted_ids))
			lu.assertEquals(self.collection:count({_id = _id}), 1)
		end	
		
		docs = {}
		for i = 1,5 do
			docs[i] = {_id = i}
		end
		result = self.collection:insert_many(docs)
		lu.assertTrue(InsertManyResult.isInsertManyResult(result))
		lu.assertTrue(isArray(result.inserted_ids))
		lu.assertEquals(#result.inserted_ids, 5)
		
		for _,doc in pairs(docs) do
			local _id = doc["_id"]
			lu.assertTrue(inArray(_id, result.inserted_ids))
			lu.assertEquals(self.collection:count({_id = _id}), 1)
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
			lu.assertTrue(ObjectId.isObjectId(result["_id"]), "find did not return object ids")
			numDocuments = numDocuments + 1
		end
		
		lu.assertEquals(numDocuments, 2, "inserted two documents and found a different number")
		
		-- Test finding 0 documents.
		numDocuments = 0
		results = self.collection:find({should_find_none = 0})
		for result in results do
			numDocuments = numDocuments + 1
		end
		lu.assertEquals(numDocuments, 0)
		
		-- Test for error on a bad operation.
		results = self.collection:find({a = {["$bad_op"] = 5}})
		lu.assertErrorMsgContains("unknown operator: $bad_op",
														function() for result in results do print(result) end end)
	end

	function TestCollection:test_find_options()
		for i = 0,9 do
			self.collection:insert_one({x = i})
		end

		lu.assertEquals(10, self.collection:count())

		local total = 0
		local results = self.collection:find(nil, nil, {skip = 4, limit = 2})
		for result in results do
			total = total + result.x
		end

		lu.assertEquals(total, 9)
	end
	
	function TestCollection:test_update_one()
		local id1 = self.collection:insert_one({x = 5}).inserted_id
		local result = self.collection:update_one({}, {["$inc"] = {x = 1}})
		
		lu.assertTrue(UpdateResult.isUpdateResult(result))
		lu.assertEquals(result.modified_count, 1)
		lu.assertNil(result.upserted_id)
		lu.assertEquals(self.collection:find_one({_id = id1}).x, 6)
		
		local id2 = self.collection:insert_one({x = 1}).inserted_id
		result = self.collection:update_one({x = 6},  {["$inc"] = {x = 1}})

		lu.assertTrue(UpdateResult.isUpdateResult(result))
		lu.assertEquals(result.matched_count, 1)
		lu.assertEquals(result.modified_count, 1)
		lu.assertNil(result.upserted_id)
		lu.assertEquals(self.collection:find_one({_id = id1}).x, 7)
		lu.assertEquals(self.collection:find_one({_id = id2}).x, 1)
		
		result = self.collection:update_one({x = 2}, {["$set"] = {y = 1}}, true)

		lu.assertTrue(UpdateResult.isUpdateResult(result))
		lu.assertEquals(result.matched_count, 0)
		lu.assertEquals(result.modified_count, 0)
		lu.assertTrue(ObjectId.isObjectId(result.upserted_id))
	end
	
	function TestCollection:test_update_many()
		self.collection:insert_many({
			{x = 4, y = 3},
			{x = 5, y = 5},
			{x = 4, y = 4}
		})

		local result = self.collection:update_many({x = 4}, {["$set"] = {y = 5}})

		lu.assertTrue(UpdateResult.isUpdateResult(result))
		lu.assertEquals(result.matched_count, 2)
		lu.assertEquals(result.modified_count, 2)
		lu.assertNil(result.upserted_id)
		
		result = self.collection:update_many({x = 5}, {["$set"] = {y = 6}})

		lu.assertTrue(UpdateResult.isUpdateResult(result))
		lu.assertTrue(result.matched_count, 1)
		lu.assertEquals(result.modified_count, 1)
		lu.assertNil(result.upserted_id)
		lu.assertEquals(self.collection:count({y = 6}), 1)
		
		result = self.collection:update_many({x = 2}, {["$set"] = {y = 1}}, true)

		lu.assertTrue(UpdateResult.isUpdateResult(result))
		lu.assertEquals(result.matched_count, 0)
		lu.assertEquals(result.modified_count, 0)
		lu.assertTrue(ObjectId.isObjectId(result.upserted_id))
	end

	function TestCollection:test_count()
		assert(self.collection:count() == 0)

		self.collection:insert_many({ {foo = "bar"}, {foo = "baz"} })

		local num_results = 0
		for result in self.collection:find({foo = "bar"}) do
			num_results = num_results + 1
		end

		lu.assertEquals(num_results, 1)
		lu.assertEquals(self.collection:count({foo = "bar"}), 1)
		lu.assertEquals(self.collection:count({foo = {["$regex"] = "ba.*"}}), 2)
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
				
		lu.assertEquals(#result_array, 1)
		lu.assertTrue(table_eq(inserted_document, result_array[1]))
	end
	
lu.run()
