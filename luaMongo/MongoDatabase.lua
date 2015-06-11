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

local MongoCollection = require("MongoCollection")

local MongoDatabase = {__mode="k"}
MongoDatabase.__index = MongoDatabase

	--- Creates a new MongoDatabase object.
	-- @param client A MongoClient instance.
	-- @param database_name
	function MongoDatabase.new(client, database_name)
		local self = setmetatable({}, MongoDatabase)
		self.database_t = MongoModule.database_new(client.client_t, database_name)
		self.client = client
		return self
	end
	
	--- Creates MongoCollection instance.
	-- @param collection_name
	function MongoDatabase:getCollection(collection_name)
		return MongoCollection.new(self, collection_name)
	end

	--- Returns array of collection names.
	function MongoDatabase:getCollectionNames()
		collection_names = self.database_t:get_collection_names()
		return collection_names
	end

	--- Drops the database.
	function MongoDatabase:drop_database()
		self.database_t:database_drop()
		self.database_t = nil
	end
	
	--- Returns boolean whether the collection is present in the database.
	-- @param database_name
	function MongoDatabase:hasCollection(database_name)
		ret = self.database_t:has_collection(database_name)
		return ret
	end

return MongoDatabase