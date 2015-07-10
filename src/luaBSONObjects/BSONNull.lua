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

--- Lua Object representing BSONNull.
-- @type BSONNull

local BSONNull = {}
BSONNull.__index = BSONNull

	function BSONNull:__tostring()
		return "BSONNull"
	end
	
	function BSONNull:__eq(a, b)
		return BSONNull.isBSonNull(a) and BSONNull.isBSonNull(b)
	end

	---
	-- Creates a MongoClient instance.
	-- uses MongoDB connection URI (http://docs.mongodb.org/manual/reference/connection-string/).
	-- @param db_uri The MongoDB connection URI.
	function BSONNull.new()
		local self = setmetatable({}, BSONNull)
		return self
	end
	
	---
	-- Checks whether the object is a BSONNull object or not.
	-- @param object The object to be checked whether it is an BSONNull.
	-- @treturn bool Whether the object is an BSONNull or not.
	function BSONNull.isBSONNull(object)
		return getmetatable(object) == BSONNull
	end
		
return BSONNull