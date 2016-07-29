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

local luaBSONObjects = require("mongorover.luaBSONObjects")

--- Cursor class to iterate over Mongo query results.
-- @module mongorover.MongoCursor

--- Cursor class to iterate over Mongo query results.
-- @type mongorover.MongoCursor
local MongoCursor = {}

	---
	-- Creates a new MongoCursor instance. Should not be called directly by
	-- application developers - see @{mongorover.MongoCollection:find}.
	-- @tparam MongoCollection collection A MongoCollection instance.
	-- @param cursor_t An internal C userdata instance representing a mongoc_cursor_t
	-- @return A @{MongoCursor} instance.
	function MongoCursor.new(collection, cursor_t)
		local self = {
			collection = collection,
			cursor_t = cursor_t
		}
		setmetatable(self, {__mode = "k"})

		local function iterator()
			return self.cursor_t:next(luaBSONObjects)
		end

		setmetatable(self, {
			__index = MongoCursor,
			__call = function() return iterator() end
		})

		return self
	end


local metatable = {
	__index = MongoCursor,
	__call = function(table, ...)
					-- table is the same as MongoCursor
					return MongoCursor.new(...)
				end
}

setmetatable(MongoCursor, metatable)
return MongoCursor