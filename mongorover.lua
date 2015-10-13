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

--- Lua driver for MongoDB.
-- @module mongorover

--- Lua driver for MongoDB.
-- @type mongorover

local importPrepend = ""
if not _G["__MONGOROVER_TEST_ENVIRONMENT"] then
	-- import from installed luarock
	importPrepend = "mongorover."
else
	-- import local
	importPrepend = "src."
end

local MongoClient = require(importPrepend .. 'MongoClient')
local MongoDatabase = require(importPrepend .. 'MongoDatabase')
local MongoCollection = require(importPrepend .. 'MongoCollection')
local luaBSONObjects = require(importPrepend .. 'luaBSONObjects')
local resultObjects = require(importPrepend .. 'resultObjects')

local objects = {_VERSION = "0.0.1",
	MongoClient = MongoClient,
	MongoDatabase = MongoDatabase,
	MongoCollection = MongoCollection,
	luaBSONObjects = luaBSONObjects,
	resultObjects = resultObjects}

local mongo_rover = setmetatable(objects, {
	__index = objects,
	__newindex = function(table, key, value)
								error("mongo_rover cannot be modified, it is a read-only table")
							end,
})

return mongo_rover