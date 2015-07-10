local MongoClient = require("luaMongo.MongoClient")
local client = MongoClient.new("mongodb://localhost:27017/")
local BSONNull = require("luaMongo.luaBSONObjects.BSONNull")
local exampleDatabase = client:getDatabase("exampleDB")
local exampleCollection = exampleDatabase:getCollection("exampleCollection")

local exampleCollection = client.exampleDB.exampleCollection

-- Drop database to get a clean slate for this example.
exampleCollection:drop()

-- There are zero documents in the dropped collection.
print("number of documents in collection", exampleCollection:count({}))

-- After the insert, there should be three documents.
local arrayOfDocuments = {{color = "blue"}, {color = "blue"}, {color = "red"}}
exampleCollection:insert_many(arrayOfDocuments)
print("number of documents in collection", exampleCollection:count({}))

-- Update one document that has the color of blue and make it red.
local result = exampleCollection:update_one({color = "blue"}, {["$set"] = {color = "red"}})

-- Now there is one blue document and two red documents.
print("number of blue documents in collection", exampleCollection:count({color = "blue"}))
print("number of red documents in collection",  exampleCollection:count({color = "red"}))

-- result is an UpdateResult.
print(result)

-- It has information about the update operation.
for k,v in pairs(result) do
	print(k,v)
end


---- This is the output of this script.

-- number of documents in collection	0
-- number of documents in collection	3
-- number of blue documents in collection	1
-- number of red documents in collection	2
-- <UpdateResult object at table: 0x7ff003703750>
-- modified_count	1
-- matched_count	1
-- raw_result	table: 0x7ff0037035d0