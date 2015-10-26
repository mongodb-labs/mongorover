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

local importPrepend = ""
local mongorover_environment = _G["__MONGOROVER_TEST_ENVIRONMENT"]
if mongorover_environment == nil or mongorover_environment then
	importPrepend = "mongorover."
end

local InsertOneResult = require(importPrepend .. 'resultObjects.InsertOneResult')
local InsertManyResult = require(importPrepend .. 'resultObjects.InsertManyResult')
local UpdateResult = require(importPrepend .. 'resultObjects.UpdateResult')
local DeleteResult = require(importPrepend .. 'resultObjects.DeleteResult')

local objects = {InsertOneResult = InsertOneResult,
	InsertManyResult = InsertManyResult,
	UpdateResult = UpdateResult,
	DeleteResult = DeleteResult}

local resultObjects = setmetatable({}, {
	__index = objects,
	__metatable = false,
	__newindex = function(table, key, value)
								error("resultObjects cannot be modified, it is a read-only table")
							end
})

return resultObjects