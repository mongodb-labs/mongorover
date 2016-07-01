--- Lua representations of BSON objects.
-- @module mongorover.luaBSONObjects

--- Lua representations of BSON objects.
-- @type mongorover.luaBSONObjects

local BSONNull = require('mongorover.luaBSONObjects.BSONNull')
local ObjectId = require('mongorover.luaBSONObjects.ObjectId')

local objects = {
	BSONNull = BSONNull,
	ObjectId = ObjectId
}

local luaBSONObjects = setmetatable(objects, {
	__index = objects,
	__metatable = false,
	__newindex = function(table, key, value)
								error("resultObjects cannot be modified, it is a read-only table")
							end
})

return luaBSONObjects