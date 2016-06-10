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

local MongoClient = require(importPrepend .. "MongoClient")


BaseTest = {}
BaseTest.__index = BaseTest

	function BaseTest:setUp()
		self.db_uri = "mongodb://mr_user:mr_password@localhost:27017/?authSource=admin"
		self.database_name = "mr_test_suite"
		self.collection_name = "foo"

		self.client = MongoClient.new(self.db_uri)
		pcall(function() self.client:getDatabase(self.database_name):drop_database() end)
		self.database = self.client:getDatabase(self.database_name)
		self.collection = self.database:getCollection(self.collection_name)
	end

	function BaseTest:tearDown()
		pcall(function() self.database:drop_database() end)
	end

return BaseTest
