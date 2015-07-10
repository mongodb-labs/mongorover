local MongoClient = require("luaMongo.MongoClient")
local client = MongoClient.new("mongodb://localhost:27017/")
local BSONNull = require("luaMongo.luaBSONObjects.BSONNull")
local exampleDatabase = client:getDatabase("exampleDB")
local exampleCollection = exampleDatabase:getCollection("exampleCollection")

-- Drop database to get a clean slate for this example.
exampleCollection:drop()

-- Create array of documents to insert.
local arrayOfDocuments = {{foo = "bar", _id = "manually set _id"}, {foo = "baz"}}

-- Insert document into database
local result = exampleCollection:insert_many(arrayOfDocuments)

-- Result is an InsertManyResult.
print(result)

-- Access the raw result from the server using the raw_result field.
for k,v in pairs(result.raw_result) do
	print(k,v)
end

-- Get list of inserted _ids from InsertManyResult using the inserted_ids field.
-- The first document manually set it's _id, the second document lets MongoDB create one automatically.
for k,v in ipairs(result.inserted_ids) do
	print(k,v)
end

-- Lua is pass by reference, and the _ids are placed in the documents inserted above.
for _,doc in pairs(arrayOfDocuments) do
	print("_id:", doc._id)
	print("foo value:",	doc.foo)
end


---- This is the output of this script.

-- <InsertManyResult object at table: 0x7f8e00c0a8c0>
-- nUpserted	0
-- nInserted	2
-- nRemoved	0
-- writeErrors	table: 0x7f8e00e00f50
-- nModified	0
-- nMatched	0
-- 1	manually set _id
-- 2	ObjectID("55a01f40d2b38b2b323661c1")
-- _id:	manually set _id
-- foo value:	bar
-- _id:	ObjectID("55a01f40d2b38b2b323661c1")
-- foo value:	baz