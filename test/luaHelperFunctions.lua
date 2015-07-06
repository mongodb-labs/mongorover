-- Happily copied from: http://lua-users.org/lists/lua-l/2014-09/msg00463.html

function table_eq(table1, table2)
   local avoid_loops = {}
   local function recurse(t1, t2)
      -- compare value types
      if type(t1) ~= type(t2) then return false end
      -- Base case: compare simple values
      if type(t1) ~= "table" then return t1 == t2 end
      -- Now, on to tables.
      -- First, let's avoid looping forever.
      if avoid_loops[t1] then return avoid_loops[t1] == t2 end
      avoid_loops[t1] = t2
      -- Copy keys from t2
      local t2keys = {}
      local t2tablekeys = {}
      for k, _ in pairs(t2) do
         if type(k) == "table" then table.insert(t2tablekeys, k) end
         t2keys[k] = true
      end
      -- Let's iterate keys from t1
      for k1, v1 in pairs(t1) do
         local v2 = t2[k1]
         if type(k1) == "table" then
            -- if key is a table, we need to find an equivalent one.
            local ok = false
            for i, tk in ipairs(t2tablekeys) do
               if table_eq(k1, tk) and recurse(v1, t2[tk]) then
                  table.remove(t2tablekeys, i)
                  t2keys[tk] = nil
                  ok = true
                  break
               end
            end
            if not ok then return false end
         else
            -- t1 has a key which t2 doesn't have, fail.
            if v2 == nil then return false end
            t2keys[k1] = nil
            if not recurse(v1, v2) then return false end
         end
      end
      -- if t2 has a key which t1 doesn't have, fail.
      if next(t2keys) then return false end
      return true
   end
   return recurse(table1, table2)
end


-- Happily copied from: http://stackoverflow.com/questions/7526223/how-do-i-know-if-a-table-is-an-array

---Checks if a table is used as an array. That is: the keys start with one and are sequential numbers
-- @param t table
-- @return nil,error string if t is not a table
-- @return true/false if t is an array/isn't an array
-- NOTE: it returns true for an empty table
function isArray(t)
    if type(t)~="table" then return nil,"Argument is not a table! It is: "..type(t) end
    --check if all the table keys are numerical and count their number
    local count=0
    for k,v in pairs(t) do
        if type(k)~="number" then return false else count=count+1 end
    end
    --all keys are numerical. now let's see if they are sequential and start with 1
    for i=1,count do
        --Hint: the VALUE might be "nil", in that case "not t[i]" isn't enough, that's why we check the type
        if not t[i] and type(t[i])~="nil" then return false end
    end
    return true
end


function inArray(element, array)
	local ret = false
	
	for _, v in pairs(array) do
		if element == v then
			ret = true
			break
		end
	end
	
	return ret
end