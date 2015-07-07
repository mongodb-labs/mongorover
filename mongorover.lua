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

local MongoClient = require('mongorover.MongoClient')
local MongoDatabase = require('mongorover.MongoDatabase')
local MongoCollection = require('mongorover.MongoCollection')
local luaBSONObjects = require('mongorover.luaBSONObjects')
local resultObjects = require('mongorover.resultObjects')

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