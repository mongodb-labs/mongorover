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

package.path = package.path .. ';./luaMongo/?.lua;./test/?.lua;../luaMongo/?.lua'
ObjectId = require("ObjectId")
BSONNull = require("BSONNull")
InsertOneResult = require("resultObjects.InsertOneResult")
UpdateOneResult = require("resultObjects.UpdateOneResult")

require('tableEquality')

MongoClient = require("MongoClient")
local MongoCollection = require("MongoCollection")
LuaUnit = require("luaunit")

TestClient = {}

	function TestClient:test_database_drop()
		
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		
		local result = collection:insert_one({a=1})
		assert(result.acknowledged == true, "insert_one failed")
		
		has_database = database:hasCollection("bar")
		assert(has_database == true, "database does not exist after insert_one(...)")
		
		collection:drop()
		has_database = database:hasCollection("bar")
		assert(has_database == false, "database did not get dropped")
		
	
	end
	
	function TestClient:test_find_one_and_insert_one_with_id()
		
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		
		local allDifferentTypes = {a=1, b=2, c="C", d="D", e=true, z=BSONNull.new(), _id=ObjectId.new("55830e73d2b38bf021417851")}
		
		local result = collection:insert_one(allDifferentTypes)
		assert(result.acknowledged == true, "insert_one failed")
		local objectId = ObjectId.new(result.inserted_id)
		local check_result = collection:find_one({_id=result.inserted_id}, {_id=0})
		
		allDifferentTypes['_id'] = nil

		assert(table_eq(check_result, allDifferentTypes), "insert_one and find_one documents do not match")
	
	end

	function TestClient:test_find_one_and_insert_one_without_id()
		
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("bar")
		
		local allDifferentTypes = {a=1, b=2, c="C", d="D", e=true, z=BSONNull.new()}
		
		local result = collection:insert_one(allDifferentTypes)
		assert(result.acknowledged == true, "insert_one failed")
		local objectId = ObjectId.new(result.inserted_id)
		local check_result = collection:find_one({_id=result.inserted_id}, {_id=0})
		
		assert(table_eq(check_result, allDifferentTypes), "insert_one and find_one documents do not match")
	
	end
	
	function TestClient:test_find()
		
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("test")
		
		collection:drop()
		collection = database:getCollection("test")
		
		local docs = {}
		for i=1,5 do
			local result = collection:insert_one({ a= i<3 })
			assert(result.acknowledged == true, "insert_one failed")
		end
		
		local results = collection:find({a=true})
		
		local numDocuments = 0
		for result in results do
			assert(ObjectId.isObjectId(result["_id"], "find did not return object ids"))
			numDocuments = numDocuments + 1
		end
		
		assert(numDocuments == 2, "inserted two documents and found a different number")
		
	end
	
	function TestClient:test_update_one()
		
		local client = MongoClient.new("mongodb://user:password@localhost:27017/?authSource=admin")
		local database = client:getDatabase("foo")
		local collection = database:getCollection("test")
		
		collection:drop()
		collection = database:getCollection("test")
		
		local id1 = collection:insert_one({x=5}).inserted_id
		
		local result = collection:update_one({}, {["$inc"]= {x=1}})
		
		assert(UpdateOneResult.isUpdateOneResult(result))
		assert(result.modified_count == 1)
		assert(type(result.upserted_id) =="nil")
		assert(collection:find_one({_id=id1}).x == 6)
		
		local id2 = collection:insert_one({x=1}).inserted_id
		result = collection:update_one({x=6},  {["$inc"]= {x=1}})
		assert(UpdateOneResult.isUpdateOneResult(result))
		assert(result.matched_count == 1)
		assert(result.modified_count == 1)
		assert(type(result.upserted_id) =="nil")
		assert(collection:find_one({_id=id1}).x == 7)
		assert(collection:find_one({_id=id2}).x == 1)
		
		result = collection:update_one({x=2}, {["$set"] = {y=1}}, true)
		assert(UpdateOneResult.isUpdateOneResult(result))
		assert(result.matched_count == 0)
		assert(result.modified_count == 0)
		assert(ObjectId.isObjectId(result.upserted_id))
	
	end
	
LuaUnit:run()