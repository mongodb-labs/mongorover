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