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

--- Result objects that are returned from MongoDB CRUD operations.
-- @module mongorover.resultObjects

--- Result objects that are returned from MongoDB CRUD operations.
-- @type mongorover.resultObjects

local InsertOneResult = require('mongorover.resultObjects.InsertOneResult')
local InsertManyResult = require('mongorover.resultObjects.InsertManyResult')
local UpdateResult = require('mongorover.resultObjects.UpdateResult')
local DeleteResult = require('mongorover.resultObjects.DeleteResult')

local objects = {InsertOneResult = InsertOneResult,
	InsertManyResult = InsertManyResult,
	UpdateResult = UpdateResult,
	DeleteResult = DeleteResult}

local resultObjects = setmetatable(objects, {
	__index = objects,
	__newindex = function(table, key, value)
								error("resultObjects cannot be modified, it is a read-only table")
							end,
	__metatable = false
})

return resultObjects