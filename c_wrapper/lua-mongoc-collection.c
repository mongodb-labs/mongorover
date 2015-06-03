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

#include "lua-mongoc-collection.h"

int
lua_mongo_collection_new(lua_State *L) {
    const char* collection_name;
    database_t *database;
    collection_t *collection;

    database = (database_t *)luaL_checkudata(L, 1, "lua_mongoc_database");

    collection_name = luaL_checkstring(L, 2);
    if (collection_name == NULL) {
        luaL_error(L, "collection name cannot be empty");
    }

    collection = (collection_t *)lua_newuserdata(L, sizeof(*collection));
    collection->c_collection = mongoc_database_get_collection(database->c_database, collection_name);
    collection->c_database = database->c_database;

    luaL_getmetatable(L, "lua_mongoc_collection");
    lua_setmetatable(L, -2);

    return 1;
}

int
lua_mongo_collection_drop (lua_State *L) {
    collection_t *collection;
    bson_error_t error;
    bool ret;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    ret = mongoc_collection_drop(collection->c_collection, &error);
    if (ret) {
        lua_pushboolean(L, ret);
    } else {
        luaL_error(L, error.message);
    }

    return 1;
}

int
lua_mongo_collection_destroy(lua_State *L) {
    collection_t *collection;
    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (collection->c_collection != NULL) {
        mongoc_collection_destroy(collection->c_collection);
        collection->c_collection = NULL;
    }

    return 1;
}