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
InsertOneResult = require("InsertOneResult")

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
	
LuaUnit:run()