local UpdateOneResult = {}
UpdateOneResult.__index = UpdateOneResult
	
	UpdateOneResult.__tostringx = function (table_location)
		UpdateOneResult.__tostring = nil    
		local ret = "<UpdateOneResult object at " .. tostring(table_location) .. ">"
		UpdateOneResult.__tostring = UpdateOneResult.__tostringx
		return ret
	end

	--- Creates a MongoClient instance.
	-- uses MongoDB connection URI (http://docs.mongodb.org/manual/reference/connection-string/).
	-- @param db_uri The MongoDB connection URI.
	function UpdateOneResult.new(raw_result)
		local self = setmetatable({}, UpdateOneResult)
		--'matched_count', 'modified_count', 'raw_result', 'upserted_id'
		self.matched_count = raw_result.nMatched
		self.modified_count = raw_result.nModified
		self.raw_result = raw_result
		if raw_result.upserted then
			self.upserted_id = raw_result.upserted[1]["_id"]
		end
		return self
	end
	
	function UpdateOneResult.isUpdateOneResult(o)
		return getmetatable(o) == UpdateOneResult
	end
	
	UpdateOneResult.__tostring = UpdateOneResult.__tostringx    

return UpdateOneResult