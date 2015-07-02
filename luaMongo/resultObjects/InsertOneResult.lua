local InsertOneResult = {}
InsertOneResult.__index = InsertOneResult
	
	InsertOneResult.__tostringx = function (p)
		InsertOneResult.__tostring = nil    
		local s = "<InsertOneResult object at " .. tostring(p) .. ">"
		InsertOneResult.__tostring = InsertOneResult.__tostringx
		return s
	end

	--- Creates a MongoClient instance.
	-- uses MongoDB connection URI (http://docs.mongodb.org/manual/reference/connection-string/).
	-- @param db_uri The MongoDB connection URI.
	function InsertOneResult.new(acknowledged, _id)
		local self = setmetatable({}, InsertOneResult)
		self.acknowledged = acknowledged
		self.inserted_id = _id
		return self
	end
		
	InsertOneResult.__tostring = InsertOneResult.__tostringx    

return InsertOneResult