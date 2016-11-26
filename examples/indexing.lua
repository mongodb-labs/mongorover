

local client = require("mongorover.MongoClient").new("mongodb://localhost/")
local db = client:getDatabase('exampleDB')
local col = db:getCollection("exampleCO")

local sample = {
	{name = "orange", txt='Color between red and yellow', cost = 11},
	{name = "grape", txt='Fruiting berry', cost = 3},
	{name = "mango berry", txt='Juicy stone fruit', cost = 25},
	{name = "grape", txt='Duplicate Fruiting berry', cost = 4},
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

print('\nCreating text search index.')
local res=col:createIndex( {name='text'}, {name='myindex', unique=false, background=true} ); --Test text field search using index
print(tostring(res))
print('\nText search results.')
local res= col:find({ ['$text']={['$search']='berry'} })
for key in res do
	for k,v in pairs(key) do
		print(k,v)
	end
end
print('\nRemoving index.')
print( col:dropIndex('myindex') )


print('\nCreating compound text search index.')
local res=col:createIndex( {name='text', txt='text'}, {name='myindex', unique=false, background=false} ); --Test compound index
print(tostring(res))
print('\nCompound text search results.')
local res= col:find({ ['$text']={['$search']='berry'} })
for key in res do
	for k,v in pairs(key) do
		print(k,v)
	end
end
print('\nRemoving index.')
print( col:dropIndex('myindex') )


print('\nCreating numeric search index.')
local res=col:createIndex( {cost=1}, {name='myindex', unique=false, background=true} ); --Test numeric search using index
print(tostring(res))
print('\nNumeric search results.')
local res= col:find({cost={['$gt']=5.5}})
for key in res do
	for k,v in pairs(key) do
		print(k,v)
	end
end
print('\nRemoving index.')
print( col:dropIndex('myindex') )


print('\nCreating compound mixed index.')
local res=col:createIndex( {name='text', cost=1}, {name='myindex', unique=false, background=false} ); --Test compound index
print(tostring(res))
print('\nCompound text search results.')
local res= col:find({ ['$and']={{cost=4, ['$text']={['$search']='grape'} } } })
for key in res do
	for k,v in pairs(key) do
		print(k,v)
	end
end


print('\nRetrieving existing indexes.')
local res=col:findIndexes()
if( type(res)=='userdata' ) then --Legacy driver
	print(res)
else --Table results
	for key in res do
		for k,v in pairs(key) do
			print(k,v)
		end
	end
end

print('\nRemoving index.')
print( col:dropIndex('myindex') )

print('\nRemoving data.')
col:drop()
