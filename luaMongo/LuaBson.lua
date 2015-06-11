package.cpath = package.cpath .. ";./c_wrapper/build/?.dylib;../c_wrapper/build/?.dylib"
local MongoModule = require("mongo_module")

local LuaBson = {}
LuaBson.__index = LuaBson

	--- Creates a LuaBson instance. Translates table parameter into BSON.
	-- @param table Lua table that will interface with MongoDB
	function LuaBson.new(table)
		local self = setmetatable({}, LuaBson)
		self.lua_bson_t = MongoModule.lua_bson_new(table)
		return self
	end

return LuaBson