local MongoClient = require("luaMongo.MongoClient")
local client = MongoClient.new("mongodb://localhost:27017/")
local BSONNull = require("luaMongo.luaBSONObjects.BSONNull")
local exampleDatabase = client:getDatabase("exampleDB")
local exampleCollection = exampleDatabase:getCollection("exampleCollection")

-- Drop database to get a clean slate for this example.
exampleCollection:drop()

-- After the insert, there should be three documents.
local arrayOfDocuments = {{color = "blue"}, {color = "blue"}, {color = "red"}}
exampleCollection:insert_many(arrayOfDocuments)
print("number of documents in collection", exampleCollection:count({}))

-- Update all document that have the color of blue and make them red.
local result = exampleCollection:update_many({color = "blue"}, {["$set"] = {color = "red"}})

-- Now there are no blue documents and three red documents.
print("number of blue documents in collection", exampleCollection:count({color = "blue"}))
print("number of red documents in collection",  exampleCollection:count({color = "red"}))

-- result is an UpdateResult.
print(result)

-- It has information about the update operation.
for k,v in pairs(result) do
	print(k,v)
end


---- This is the output of this script.

-- number of documents in collection	3
-- number of blue documents in collection	0
-- number of red documents in collection	3
-- <UpdateResult object at table: 0x7f80f85082f0>
-- modified_count	2
-- matched_count	2
-- raw_result	table: 0x7f80f85085a0