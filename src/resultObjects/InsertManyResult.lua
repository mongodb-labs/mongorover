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

--- The return type for insert_many().
--@type InsertManyResult

local InsertManyResult = {}
	
	InsertManyResult.__tostringx = function (table_location)
		InsertManyResult.__tostring = nil    
		local ret = "<InsertManyResult object at " .. tostring(table_location) .. ">"
		InsertManyResult.__tostring = InsertManyResult.__tostringx
		return ret
	end

	---
	-- Creates a InsertManyResult instance.
	-- @tparam table raw_result Response from MongoDB converted into a Lua table.
	-- @tparam {table,...} inserted_ids Array of _ids that were inserted.
	function InsertManyResult.new(raw_result, inserted_ids)
		local self = setmetatable({}, InsertManyResult)
		self.raw_result = raw_result
		self.inserted_ids = inserted_ids
		return self
	end
	
	---
	-- Checks whether the object is an InsertManyResult or not.
	-- @param object The object to be checked whether it is an InsertManyResult.
	-- @treturn bool Whether the object is an InsertManyResult or not.
	function InsertManyResult.isInsertManyResult(object)
		return getmetatable(object) == InsertManyResult
	end
		
	InsertManyResult.__tostring = InsertManyResult.__tostringx    

local metatable = {
	__index = InsertManyResult,
	__call = function(table, ...)
		-- table is the same as DeleteResult
		return InsertManyResult.new(...)
	end
}

setmetatable(InsertManyResult, metatable)
return InsertManyResult