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

local UpdateResult = {}
UpdateResult.__index = UpdateResult
	
	UpdateResult.__tostringx = function (table_location)
		UpdateResult.__tostring = nil    
		local ret = "<UpdateResult object at " .. tostring(table_location) .. ">"
		UpdateResult.__tostring = UpdateResult.__tostringx
		return ret
	end

	--- Creates a MongoClient instance.
	-- uses MongoDB connection URI (http://docs.mongodb.org/manual/reference/connection-string/).
	-- @param db_uri The MongoDB connection URI.
	function UpdateResult.new(raw_result)
		local self = setmetatable({}, UpdateResult)
		--'matched_count', 'modified_count', 'raw_result', 'upserted_id'
		self.matched_count = raw_result.nMatched
		self.modified_count = raw_result.nModified
		self.raw_result = raw_result
		if raw_result.upserted then
			self.upserted_id = raw_result.upserted[1]["_id"]
		end
		return self
	end
	
	function UpdateResult.isUpdateResult(o)
		return getmetatable(o) == UpdateResult
	end
	
	UpdateResult.__tostring = UpdateResult.__tostringx    

return UpdateResult