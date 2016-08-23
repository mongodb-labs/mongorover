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

---
-- @submodule mongorover.luaBSONObjects

--- Lua Object representing ObjectId.
-- @type ObjectId

local ObjectId = {}
ObjectId.__index = ObjectId

	function ObjectId:__tostring()
		return "ObjectID(\"" .. self.key .. "\")"
	end

	---
	-- Creates an ObjectId object with the corresponding key.
	-- @tparam string key A hexadecimal string representation of the ObjectId of length 24.
	function ObjectId.new(key)
		assert(string.len(key) == 24, "key parameter must be a hexidecimal string representing an ObjectId with length of 24.")
		local self = setmetatable({}, ObjectId)
		self.key = key
		return self
	end
	
	---
	-- Checks whether the object is a ObjectId object or not.
	-- @param object The object to be checked whether it is an ObjectId.
	-- @treturn bool Whether the object is an ObjectId or not.
	function ObjectId.isObjectId(object)
		return getmetatable(object) == ObjectId
	end
	
	---
	-- Returns hexidecimal string representation of the ObjectId.
	-- @treturn String hexidecimal string representation of the ObjectId.
	function ObjectId:getKey()
		return self.key
	end
		
return ObjectId