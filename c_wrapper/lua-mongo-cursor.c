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
#include "lua-mongo-cursor.h"


/**
 * lua_mongo_cursor_new
 * @L: A lua_State.
 * @find_result_cursor: A mongoc_cursor_t.
 *
 * Takes cursor and makes a lua object containing that cursor to be returned.
 * The userdata object is left on the top of the stack for caller to return.
 */

void
lua_mongo_cursor_new(lua_State *L, mongoc_cursor_t *find_result_cursor)
{

    lua_mongo_cursor_t *cursor;

    cursor = (lua_mongo_cursor_t *) lua_newuserdata(L, sizeof(*cursor));
    cursor->c_cursor = find_result_cursor;

    luaL_getmetatable(L, "lua_mongo_cursor");
    lua_setmetatable(L, -2);

    return;
}


int
lua_mongo_cursor_iterate(lua_State *L)
{
    lua_mongo_cursor_t *cursor;
    const bson_t *doc;
    bson_error_t error;
    bool throw_error = false;

    cursor = (lua_mongo_cursor_t *) luaL_checkudata(L, 1, "lua_mongo_cursor");

    if (mongoc_cursor_next(cursor->c_cursor, &doc)) {
        throw_error = !(bson_document_or_array_to_table(L, doc, true, &error));
        bson_destroy(doc);

        if (throw_error) {
            luaL_error(L, error.message);
        }

        return 1;
    } else {
        throw_error  = mongoc_cursor_error(cursor->c_cursor, &error);

        mongoc_cursor_destroy(cursor->c_cursor);
        cursor->c_cursor = NULL;

        if (throw_error) {
            luaL_error(L, error.message);
        }

        return 0;
    }
}


int
lua_mongo_cursor_destroy(lua_State *L)
{
    lua_mongo_cursor_t *cursor;
    cursor = (lua_mongo_cursor_t *) luaL_checkudata(L, 1, "lua_mongo_cursor");

    if (cursor->c_cursor != NULL) {
        mongoc_cursor_destroy(cursor->c_cursor);
        cursor->c_cursor = NULL;
    }
    return 1;
}
