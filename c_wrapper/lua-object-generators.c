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
 * generate_ObjectID
 * @L: A lua_State.
 * @str: A char*.
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
                  bson_error_t *error)
{
    lua_getglobal(L, "ObjectId");
    if (!lua_istable(L, -1)) {
        strncpy (error->message,
                 "ObjectId not a global variable",
                 sizeof (error->message));
        return false;
    }

    lua_getfield( L, -1, "new");
    lua_pushstring(L, str);

    // Make call using 1 argument and getting 1 result
    if (lua_pcall(L, 1, 1, 0) != 0) {
        strncpy (error->message,
                 lua_tostring(L, -1),
                 sizeof (error->message));
        lua_pop(L, 1);
        return false;
    }
    // Remove global variable ObjectID off of the stack to maintain stack integrity
    lua_remove (L, -2);

    return true;
}


/**
 * isObjectId
 * @L: A lua_State.
 * @index: Location of object on stack.
 * @error: A bson_error_t.
 *
 * Takes global variable isObjectId and uses it to call
 * isObjectId.isObjectId(...) on the object at the given index on the stack.
 *
 * Returns false if error occurred. Error propagated through the bson_error_t.
 */

bool
is_ObjectId(lua_State *L,
            int index)
{
    //TODO: make this error based
    int absolute_stack_index = index > 0 ? index : lua_gettop(L) + index + 1;
    bool ret;

    lua_getglobal(L, "ObjectId");
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


void
generate_BSONNull(lua_State *L)
{
    lua_getglobal(L, "BSONNull");
    lua_getfield( L, -1, "new");
    // Make call using 0 arguments and getting 1 result
    if (lua_pcall(L, 0, 1, 0) != 0)
        luaL_error(L, "error running function `f': %s", lua_tostring(L, -1));
    // Remove global variable BSONNull off of the stack to maintain stack integrity
    lua_remove (L, -2);
}


bool
is_BSONNull(lua_State *L,
            int index)
{
    //TODO: make this error based

    bool ret;
    int absolute_stack_index = index > 0 ? index : lua_gettop(L) + index + 1;

    lua_getglobal(L, "BSONNull");
    lua_getfield( L, -1, "isBSONNull");
    lua_pushvalue(L, absolute_stack_index);
    if (lua_pcall(L, 1, 1, 0) != 0) {
        luaL_error(L, "error running function `f': %s", lua_tostring(L, -1));
    }

    ret = lua_toboolean(L, -1);
    lua_pop(L, 2);
    return ret;
}


/**
 * generate_InsertOneResult
 * @L: A lua_State.
 * @acknowledged: A bool.
 * @index: An Int.
 * @error: A bson_error_t.
 *
 * Takes the inserted document on the stack and finds the _id. Creates an
 * InsertOneResult and leaves it on the stack to be returned by caller.
 *
 * Returns false if error occurred. Error propagated through the bson_error_t.
 */

bool
generate_InsertOneResult(lua_State *L,
                         bool acknowledged,
                         int index,
                         bson_error_t *error) {

    int absolute_stack_index = index > 0 ? index : lua_gettop(L) + index + 1;

    lua_getglobal(L, "InsertOneResult");
    if (!lua_istable(L, -1)) {
        strncpy (error->message,
                 "InsertOneResult not a global variable",
                 sizeof (error->message));
        return false;
    }

    lua_getfield( L, -1, "new");
    if (!lua_isfunction(L, -1)) {
        strncpy (error->message,
                 "InsertOneResult does not have method 'new'",
                 sizeof (error->message));
        return false;
    }

    lua_pushboolean(L, acknowledged);
    lua_pushstring(L, "_id");

    lua_gettable(L, absolute_stack_index);

    if (lua_pcall(L, 2, 1, 0) != 0) {
        strncpy (error->message,
                 lua_tostring(L, -1),
                 sizeof (error->message));
        lua_pop(L, 1);
        return false;
    }

    lua_remove(L, -2);

    return true;
}


/**
 * generate_InsertManyResult
 * @L: A lua_State.
 * @raw_result: A bson_t.
 * @index: An Int.
 * @num_elements: An Int.
 * @error: A bson_error_t.
 *
 * Takes in raw_result and converts it to a lua table. Takes array of
 * inserted documents at index on the stack and gets their _ids to be
 * returned in the inserted_ids field of an InsertManyResult. Leaves an
 * InsertManyResult on the top of the stack to be returned by caller.
 *
 * Returns false if error occurred. Error propagated through the bson_error_t.
 */

bool
generate_InsertManyResult(lua_State *L,
                          bson_t *raw_result,
                          int index,
                          int num_elements,
                          bson_error_t *error)
{
    int absolute_stack_index = index > 0 ? index : lua_gettop(L) + index + 1;
    int lua_index;

    lua_getglobal(L, "InsertManyResult");
    if (!lua_istable(L, -1)) {
        strncpy (error->message, "InsertManyResult not a gloabl variable",
                 sizeof (error->message));
        return false;
    }

    lua_getfield( L, -1, "new");
    if (!lua_isfunction(L, -1)) {
        strncpy (error->message, "InsertManyResult does not have method 'new'",
                 sizeof (error->message));
        return false;
    }

    // Place raw_result on top of the stack
    if (!(bson_document_or_array_to_table(L, raw_result, true, error))) {
        //Maintain stack integrity.
        lua_pop(L, 2);
        return false;
    }

    // Place array of inserted _ids onto the top of the stack.
    lua_newtable(L);

    for (int i=0, lua_index=1; i < num_elements; i++, lua_index++) {
        lua_rawgeti(L, absolute_stack_index, lua_index);
        lua_pushstring(L, "_id");
        lua_gettable(L, -2);
        if (lua_isnil(L, -1)) {
            luaL_error(L, "_id was not generated for an insert many document");
        }

        lua_rawseti(L, -3, lua_index);
        lua_pop(L, 1);
    }

    // Make call using 2 arguments and getting 1 result
    if (lua_pcall(L, 2, 1, 0) != 0) {
        strncpy (error->message,
                 lua_tostring(L, -1),
                 sizeof (error->message));
        return false;
    }

    // Remove global variable InsertManyResult off of the stack to maintain
    // stack integrity
    lua_remove (L, -2);

    return true;
}
