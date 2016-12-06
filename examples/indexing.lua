local client = require("mongorover.MongoClient").new("mongodb://localhost/")
local db = client:getDatabase('exampleDB')
local col = db:getCollection("exampleCO")

function printResult(res)
	for key in res do
		for k, v in pairs(key) do
			print(k, v)
		end
	end
end

local sample = {
	{name = "orange", txt = 'Color between red and yellow', cost = 11},
	{name = "grape", txt = 'Fruiting berry', cost = 3, lang='french'},
	{name = "mango berry", txt = 'Juicy stone fruit', cost = 25},
	{name = "grape", txt = 'Duplicate Fruiting berry', cost = 3},
	{name = "banana", txt = 'Edible berry of flowering plants', cost = 1.5},
	{name = "persimmon", txt = 'Edible fruit of trees', cost = 4.1}
}
print('\nInserting sample data:')
col:insert_many(sample)
printResult(col:find({}))

local indx = col:createIndex({name = 'text', txt = 'text'}, { 
--	name = 'myindex', 
	default_language = 'english', 
	language_override = 'lang', 
	unique = false, 
	sparse = true, 
	background = true, 
	expireAfterSeconds = 100, 
	textIndexVersion = 1,
	weights = {name = 10, txt = 3} 
} )
print('\nCreating text search index: '..indx)

print('\nExecuting text search for "berry": ')
local res = col:find({['$text'] = {['$search'] = 'berry'}})
printResult(res)

print('\nRetrieving index: ')
printResult(col:findIndexes(indx))

print('\nRemoving index: '..tostring(col:dropIndex(indx)))

print('\nRemoving data.')
col:drop()
