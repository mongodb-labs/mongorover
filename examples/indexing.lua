
local client = require("mongorover.MongoClient").new("mongodb://localhost/")
local db = client:getDatabase('exampleDB')
local col = db:getCollection("exampleCO")

print('\nRemoving prior data.')
col:drop()

local sample = {
	{name = "mango berry", txt='Juicy stone fruit', cost = 25},
	{name = "orange", txt='Color between red and yellow', cost = 11},
	{name = "grape", txt='Fruiting berry', cost = 3},
	{name = "banana", txt='Edible berry of flowering plants', cost = 1.5},
	{name = "persimmon", txt='Edible fruit of trees', cost = 4.1},
}
print('\nInserting sample data.')
col:insert_many(sample)

local res=col:find({})
for key in res do
	for k,v in pairs(key) do
		print(k,v)
	end
end


--local res=col:createIndex({txt='text'},{name='My text index'}); --adding lua table to bson document caused malformed bson document
local res=col:createIndex({txt='text', name='text'});
print('\nIndexes created: '..tostring(res))

print('\nPrinting search results.')
local res= col:find({['$text']={['$search']='berry'}})
for key in res do
	for k,v in pairs(key) do
		print(k,v)
	end
end

