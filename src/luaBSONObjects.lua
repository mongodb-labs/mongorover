--- Lua representations of BSON objects.
-- @module mongorover.luaBSONObjects

--- Lua representations of BSON objects.
-- @type mongorover.luaBSONObjects

local current_folder = (...):gsub('%.init$', '')
local BSONNull = require(current_folder .. '.BSONNull')
local ObjectId = require(current_folder .. '.ObjectId')

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