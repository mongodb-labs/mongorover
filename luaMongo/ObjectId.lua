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

local ObjectId = {}
ObjectId.__index = ObjectId

	function ObjectId:__tostring()
		return "ObjectID(\"" .. self.key .. "\")"
	end

	--- Creates a MongoClient instance.
	-- uses MongoDB connection URI (http://docs.mongodb.org/manual/reference/connection-string/).
	-- @param db_uri The MongoDB connection URI.
	function ObjectId.new(key)
		local self = setmetatable({}, ObjectId)
		self.key = key
		return self
	end
	
	function ObjectId.isObjectId(o)
		return getmetatable(o) == ObjectId
	end
	
	function ObjectId:getKey()
		return self.key
	end
		
return ObjectId