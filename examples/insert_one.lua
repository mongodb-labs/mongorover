local MongoClient = require("mongorover.MongoClient")
local client = MongoClient.new("mongodb://localhost:27017/")
local BSONNull = require("mongorover.luaBSONObjects.BSONNull")
local exampleDatabase = client:getDatabase("exampleDB")
local exampleCollection = exampleDatabase:getCollection("exampleCollection")

-- Create document to insert.
local document_to_insert = {foo = "bar"}

-- Insert document into database
local result = exampleCollection:insert_one(document_to_insert)

-- result is an InsertOneResult
print(result)

-- Can access fields such as acknowledged and the ObjectId given to the document inserted above.
print(result.acknowledged, result.inserted_id)


---- This is the output of this script.

-- <InsertOneResult object at table: 0x7f9d09e05330>
-- true	ObjectID("55a01f67d2b38b2b4b07d4b1")