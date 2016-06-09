local MongoClient = require("mongorover.MongoClient")
local client = MongoClient.new("mongodb://localhost:27017/")
local exampleDatabase = client:getDatabase("exampleDB")
local exampleCollection = exampleDatabase:getCollection("exampleCollection")
local BSONNull = require("mongorover.luaBSONObjects.BSONNull")

-- Drop database to get a clean slate for this example.
exampleCollection:drop()

-- Insert documents for this example.
local animalDocuments = {
	{type = "cat", weight = 15},
	{type = "cat", weight = 10},
	{type = "cat", weight = 8.5},
	{type = "dog", weight = 15},
	{type = "dog", weight = 20},
}
exampleCollection:insert_many(animalDocuments)

-- Say I want to take all my cats on the new space expedition, but my space ship can only
-- accomodate 35 pounds. I would find out the result using aggregation.

local aggregationPipeline = {
	{
		["$match"] = {type = "cat"}
	},
	{
		["$group"]={
			_id = BSONNull.new(),
			totalCatWeight = {
				["$sum"] = "$weight"
			}
		}
	}
}

local results = exampleCollection:aggregate(aggregationPipeline)

-- We are only expecting one result from the aggregation framework.
result = nil
for singleResult in results do
	result = singleResult
	break
end

-- Looks like I might have barely enough to fit all my cats into my space ship!
-- The total weight of my cats is 33.5 pounds!
print("Total weight of cats", result.totalCatWeight)


---- This is the output of this script.

-- Total weight of cats	33.5
