local MongoClient = require("mongorover.MongoClient")
local client = MongoClient.new("mongodb://localhost:27017/")
local exampleDatabase = client:getDatabase("exampleDB")
local exampleCollection = exampleDatabase:getCollection("exampleCollection")

-- Drop database to get a clean slate for this example.
exampleCollection:drop()

-- There are zero documents in the dropped collection.
print("number of documents in collection", exampleCollection:count({}))

-- After the insert, there should be two.
local arrayOfDocuments = {{foo = "bar", _id = "manually set _id"}, {foo = "baz"}}
exampleCollection:insert_many(arrayOfDocuments)
print("number of documents in collection", exampleCollection:count({}))

-- Delete one document with _id equal to "manually set _id"
local result = exampleCollection:delete_one({_id = "manually set _id"})

-- Now there is only one document left.
print("number of documents in collection", exampleCollection:count({}))

-- result is a DeleteResult
print(result)

-- It has information about the delete operation.
for k,v in pairs(result) do
	print(k,v)
end


---- This is the output of this script.

-- number of documents in collection	0
-- number of documents in collection	2
-- number of documents in collection	1
-- <DeleteResult object at table: 0x7fe1b16001f0>
-- raw_result	table: 0x7fe1b16001b0
-- acknowledged	true
-- deleted_count	1