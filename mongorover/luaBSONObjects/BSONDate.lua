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

--- Lua Object representing isBSONDate.
-- @type BSONBSONDate

local BSONDate = {}
BSONDate.__index = BSONDate

	function BSONDate:__tostring()
		return "BSONDate: " .. tostring(self.datetime)
	end

	function BSONDate:__eq(a, b)
		return BSONDate.isBSONDate(a) and BSONDate.isBSONDate(b)
	end

	---
	-- Creates a MongoClient instance.
	-- uses MongoDB connection URI (http://docs.mongodb.org/manual/reference/connection-string/).
	-- @param datetime The number of seconds since the Epoch.
	function BSONDate.new(datetime)
		local self = setmetatable({}, BSONDate)
		self.datetime = datetime
		return self
	end

	---
	-- Checks whether the object is a BSONBSONDate object or not.
	-- @param object The object to be checked whether it is an BSONBSONDate.
	-- @treturn bool Whether the object is an BSONBSONDate or not.
	function BSONDate.isBSONDate(object)
		return getmetatable(object) == BSONDate
	end

return BSONDate