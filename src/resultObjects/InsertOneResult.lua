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
-- @submodule mongorover.resultObjects

--- The return type for insert_one().
--@type InsertOneResult

local InsertOneResult = {}
	
	InsertOneResult.__tostringx = function (table_location)
		InsertOneResult.__tostring = nil    
		local ret= "<InsertOneResult object at " .. tostring(table_location) .. ">"
		InsertOneResult.__tostring = InsertOneResult.__tostringx
		return ret
	end

	---
	-- Creates a InsertOneResult instance.
	-- @tparam bool acknowledged Whether the insert was acknowledged or not.
	-- @param inserted_id The _id of the inserted document.
	function InsertOneResult.new(acknowledged, inserted_id)
		local self = setmetatable({}, InsertOneResult)
		self.acknowledged = acknowledged
		self.inserted_id = inserted_id
		return self
	end
	
	---
	-- Checks whether the object is an InsertOneResult or not.
	-- @param object The object to be checked whether it is an InsertOneResult.
	-- @treturn bool Whether the object is an InsertOneResult or not.
	function InsertOneResult.isInsertOneResult(object)
		return getmetatable(object) == InsertOneResult
	end
		
	InsertOneResult.__tostring = InsertOneResult.__tostringx    

local metatable = {
	__index = InsertOneResult,
	__call = function(table, ...)
		-- table is the same as DeleteResult
		return InsertOneResult.new(...)
	end
}

setmetatable(InsertOneResult, metatable)
return InsertOneResult