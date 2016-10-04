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

/**
 * generate_ObjectID:
 * @L: A lua_State.
 * @str: A char*.
 * @absolute_luaBSONObjects_index: An Int.
 * @error: A bson_error_t.
 *
 * Generates ObjectID with string given in str and leaves it on top of the
 * stack.
 *
 * Returns false if error occurred. Error propagated through the bson_error_t.
 */

bool
generate_ObjectID(lua_State *L,
                  char *str,
                  int absolute_luaBSONObjects_index,
                  bson_error_t *error)
{

    lua_pushstring(L, "ObjectId");
    lua_gettable(L, absolute_luaBSONObjects_index);
    lua_getfield(L, -1, "new");
    lua_pushstring(L, str);

    // Make call using 1 argument and getting 1 result
    if (lua_pcall(L, 1, 1, 0) != 0) {
        strncpy (error->message,
                 lua_tostring(L, -1),
                 sizeof(error->message));
        lua_pop(L, 1);
        return false;
    }
    // Remove variable ObjectID off of the stack to maintain stack integrity
    lua_remove(L, -2);

    return true;
}


/**
 * is_ObjectId:
 * @L: A lua_State.
 * @index: Location of object on stack.
 * @error: A bson_error_t.
 *
 * Takes variable ObjectId and uses it to call ObjectId.isObjectId(...)
 * on the object at the given index on the stack.
 *
 * Returns false if error occurred. Error propagated through the bson_error_t.
 */

bool
is_ObjectId(lua_State *L,
            int index,
            int absolute_luaBSONObjects_index)
{
    //TODO: make this error based
    int absolute_stack_index = index > 0 ? index : lua_gettop(L) + index + 1;
    bool ret;

    lua_getfield(L, absolute_luaBSONObjects_index, "ObjectId");
    lua_getfield(L, -1, "isObjectId");

    // Push value that we want to check if it is an ObjectId to top of stack.
    lua_pushvalue(L, absolute_stack_index);

    if (lua_pcall(L, 1, 1, 0) != 0) {
        luaL_error(L, "error running function `f': %s", lua_tostring(L, -1));
    }
    if (!lua_isboolean(L, -1)) {
        luaL_error(L, "ObjectId.isObjectId( ... ) did not return a boolean value");
    }

    ret = lua_toboolean(L, -1);
    // Pop off boolean value from stack, returning stack to it's original call state.
    lua_pop(L, 2);

    return ret;
}

/**
 * generate_BSONNull:
 * @L: A lua_State.
 * @absolute_luaBSONObjects_index: An Int.
 * @error: A bson_error_t.
 *
 * Generates BSONNull and leaves it on top of the stack.
 *
 * Returns false if error occurred. Error propagated through the bson_error_t.
 */
bool
generate_BSONNull(lua_State *L,
                  int absolute_luaBSONObjects_index,
                  bson_error_t *error)
{

    lua_getfield(L, absolute_luaBSONObjects_index, "BSONNull");
    lua_getfield(L, -1, "new");
    // Make call using 0 arguments and getting 1 result
    if (lua_pcall(L, 0, 1, 0) != 0) {
        strncpy (error->message,
                 lua_tostring(L, -1),
                 sizeof(error->message));
        lua_pop(L, 1);
        return false;
    }
    // Remove variable BSONNull off of the stack to maintain stack integrity
    lua_remove(L, -2);
    return true;
}


/**
 * is_BSONNull:
 * @L: A lua_State.
 * @index: Location of object on stack.
 * @error: A bson_error_t.
 *
 * Takes variable isObjectId and uses it to call BSONNull.isBSONNull(...)
 * on the object at the given index on the stack.
 *
 * Returns false if error occurred. Error propagated through the bson_error_t.
 */
bool
is_BSONNull(lua_State *L,
            int index,
            int absolute_luaBSONObjects_index)
{
    //TODO: make this error based
    bool ret;
    int absolute_stack_index = index > 0 ? index : lua_gettop(L) + index + 1;

    lua_getfield(L, absolute_luaBSONObjects_index, "BSONNull");
    lua_getfield(L, -1, "isBSONNull");
    lua_pushvalue(L, absolute_stack_index);
    if (lua_pcall(L, 1, 1, 0) != 0) {
        luaL_error(L, "error running function `f': %s", lua_tostring(L, -1));
    }

    ret = lua_toboolean(L, -1);
    lua_pop(L, 2);
    return ret;
}


/**
 * generate_BSONDate:
 * @L: A lua_State.
 * @datetime: An Int.
 * @absolute_luaBSONObjects_index: An Int.
 * @error: A bson_error_t.
 *
 * Generates a BSONDate and leaves it at the top of the stack.
 *
 * Returns false if error occurred. Error propagated through the bson_error_t.
 */
bool
generate_BSONDate(lua_State *L,
                  int64_t datetime,
                  int absolute_luaBSONObjects_index,
                  bson_error_t *error)
{
    lua_getfield(L, absolute_luaBSONObjects_index, "BSONDate");

    lua_getfield(L, -1, "new");
    // pushnumber and not pushinteger to allow for dates greater than 1e14
    // to be not-truncated or misfigured in some way for Lua 5.1 and 5.2
    lua_pushnumber(L, datetime / 1000);

    // Make call using 1 argument and getting 1 result
    if (lua_pcall(L, 1, 1, 0) != 0) {
        strncpy (error->message,
                 lua_tostring(L, -1),
                 sizeof(error->message));
        lua_pop(L, 1);
        return false;
    }
    // Remove variable BSONDate off of the stack to maintain stack integrity
    lua_remove(L, -2);

    return true;
}


/**
 * is_BSONDate:
 * @L: A lua_State.
 * @index: an Int.
 * @absolute_luaBSONObjects_index: An Int.
 *
 * Takes variable BSONDate and uses it to call BSONDate.isBSONDate(...)
 * on the object at the given index on the stack.
 *
 * Returns false if error occurred. Error propagated through the bson_error_t.
 */
bool
is_BSONDate(lua_State *L,
            int index,
            int absolute_luaBSONObjects_index)
{
    //TODO: make this error based
    bool ret;
    int absolute_stack_index = index > 0 ? index : lua_gettop(L) + index + 1;

    lua_getfield(L, absolute_luaBSONObjects_index, "BSONDate");
    lua_getfield(L, -1, "isBSONDate");
    lua_pushvalue(L, absolute_stack_index);
    if (lua_pcall(L, 1, 1, 0) != 0) {
        luaL_error(L, "error running function `f': %s", lua_tostring(L, -1));
    }

    ret = lua_toboolean(L, -1);
    lua_pop(L, 2);
    return ret;
}
