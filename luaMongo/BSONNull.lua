local BSONNull = {}
BSONNull.__index = BSONNull

	function BSONNull:__tostring()
		return "BSONNull"
	end
	
	function BSONNull:__eq(a, b)
		return BSONNull.isBSonNull(a) and BSONNull.isBSonNull(b)
	end

	--- Creates a MongoClient instance.
	-- uses MongoDB connection URI (http://docs.mongodb.org/manual/reference/connection-string/).
	-- @param db_uri The MongoDB connection URI.
	function BSONNull.new()
		local self = setmetatable({}, BSONNull)
		return self
	end
	
	function BSONNull.isBSONNull(o)
		return getmetatable(o) == BSONNull
	end
		
return BSONNull