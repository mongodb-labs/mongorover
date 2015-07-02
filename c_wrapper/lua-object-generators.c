/*
 * Copyright 2015 MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lua-object-generators.h"

void
generate_ObjectID(lua_State *L, char *str)
{
    lua_getglobal(L, "ObjectId");
    lua_getfield( L, -1, "new");
    lua_pushstring(L, str);

    // Make call using 1 argument and getting 1 result
    if (lua_pcall(L, 1, 1, 0) != 0) {
        luaL_error(L, "error running function `f': %s", lua_tostring(L, -1));
    }

    // Remove global variable ObjectID off of the stack to maintain stack integrity
    lua_remove (L, -2);
}


bool
is_ObjectId(lua_State *L)
{
    lua_getfield( L, -1, "isObjectId");
    if (lua_isfunction(L, -1)) {

        // Push value that we want to check if it is an ObjectId to top of stack.
        lua_pushvalue(L, -2);

        if (lua_pcall(L, 1, 1, 0) != 0) {
            luaL_error(L, "error running function `f': %s", lua_tostring(L, -1));
        }
        if (!lua_isboolean(L, -1)) {
            luaL_error(L, "ObjectId.isObjectId( ... ) did not return a boolean value");
        }

        int is_object_id = lua_toboolean(L, -1);
        // Pop off boolean value from stack, returning stack to it's original call state.
        lua_pop(L, 1);
        return is_object_id == 1;
    } else {
        lua_pop(L, 1);
        return false;
    }
}


void
generate_BSONNull(lua_State *L)
{
    lua_getglobal(L, "BSONNull");
    lua_getfield( L, -1, "new");

    // Make call using 0 arguments and getting 1 result
    if (lua_pcall(L, 0, 1, 0) != 0) {
        luaL_error(L, "error running function `f': %s", lua_tostring(L, -1));
    }

    // Remove global variable BSONNull off of the stack to maintain stack integrity
    lua_remove (L, -2);
}


bool
is_BSONNull(lua_State *L)
{
    lua_getfield( L, -1, "isBSONNull");
    if (lua_isfunction(L, -1)) {
        lua_pushvalue(L, -2);
        if (lua_pcall(L, 1, 1, 0) != 0) {
            luaL_error(L, "error running function `f': %s", lua_tostring(L, -1));
        }
        if (!lua_isboolean(L, -1)) {
            luaL_error(L, "BSONNull.isBSONNull( ... ) did not return a boolean value");
        }
        int is_lua_mongo_null = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return is_lua_mongo_null == 1;
    } else {
        lua_pop(L, 1);
        return false;
    }
}