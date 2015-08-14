local MongoClient = require("mongorover.MongoClient")
local client = MongoClient.new("mongodb://localhost:27017/")
local BSONNull = require("mongorover.luaBSONObjects.BSONNull")
local exampleDatabase = client:getDatabase("exampleDB")
local exampleCollection = exampleDatabase:getCollection("exampleCollection")

-- Drop database to get a clean slate for this example.
exampleCollection:drop()

-- There are zero documents in the dropped collection.
print("number of documents in collection", exampleCollection:count({}))

-- After the insert, there should be three documents.
local arrayOfDocuments = {{color = "blue"}, {color = "blue"}, {color = "red"}}
exampleCollection:insert_many(arrayOfDocuments)
print("number of documents in collection", exampleCollection:count({}))

-- Delete all documents that have the color field as blue.
local result = exampleCollection:delete_many({awesome = false})

-- Now there is only one document left.
print("number of documents in collection", exampleCollection:count({}))

-- result is a DeleteResult
print(result)

-- It has information about the delete operation. Notice the delete count is 2.
for k,v in pairs(result) do
	print(k,v)
end


---- This is the output of this script.

-- number of documents in collection	0
-- number of documents in collection	3
-- number of documents in collection	3
-- <DeleteResult object at table: 0x7fbd88c0dc50>
-- raw_result	table: 0x7fbd88c0dbd0
-- deleted_count	0
-- acknowledged	true