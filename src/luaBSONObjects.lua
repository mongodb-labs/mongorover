--- Lua representations of BSON objects.
-- @module mongorover.luaBSONObjects

--- Lua representations of BSON objects.
-- @type mongorover.luaBSONObjects

local importPrepend = ""
local mongorover_environment = _G["__MONGOROVER_TEST_ENVIRONMENT"]
if mongorover_environment == nil or mongorover_environment then
	importPrepend = "mongorover."
end

local BSONNull = require(importPrepend .. 'luaBSONObjects.BSONNull')
local ObjectId = require(importPrepend .. 'luaBSONObjects.ObjectId')

local objects = {BSONNull = BSONNull,
	ObjectId = ObjectId}

local luaBSONObjects = setmetatable(objects, {
	__index = objects,
	__newindex = function(table, key, value)
								error("resultObjects cannot be modified, it is a read-only table")
							end,
	__metatable = false
})

return luaBSONObjects