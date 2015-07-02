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

#include "lua-mongoc-wrapper.h"
#include "lua-bson.h"
#include "lua-mongoc-collection.h"

int
lua_mongo_collection_new(lua_State *L)
{
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
lua_mongo_collection_drop (lua_State *L)
{
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
lua_mongo_collection_find (lua_State *L)
{
    collection_t *collection;
    bson_t query = BSON_INITIALIZER;
    bson_t fields = BSON_INITIALIZER;
    mongoc_cursor_t *cursor;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_isnil(L, 3))) {
        lua_table_to_bson(L, &query, 3, false);
    }

    if (!(lua_isnil(L, 4))) {
        lua_table_to_bson(L, &fields, 4, false);
    }

    cursor = mongoc_collection_find (collection->c_collection,
                                     MONGOC_QUERY_NONE, 0, 0, 0,
                                     &query, &fields, NULL);

    lua_mongo_cursor_new(L, cursor);

    bson_destroy (&query);
    bson_destroy (&fields);
    return 1;
}

int
lua_mongo_collection_find_one (lua_State *L)
{
    collection_t *collection;
    bson_t query = BSON_INITIALIZER;
    bson_t fields = BSON_INITIALIZER;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    int num_ret_vals;
    bool throw_error = false;
    bson_error_t error;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_isnil(L, 2))) {
        lua_table_to_bson(L, &query, 2, false);
    }

    if (!(lua_isnil(L, 3))) {
        lua_table_to_bson(L, &fields, 3, false);
    }

    cursor = mongoc_collection_find (collection->c_collection,
                                     MONGOC_QUERY_NONE, 0, 0, -1,
                                     &query, &fields, NULL);

    if (!(mongoc_cursor_next (cursor, &doc))) {
        throw_error = mongoc_cursor_error(cursor, &error);
        if (throw_error) {
            goto DONE;
        } else {
            // Cursor did not error, but did not have anything in it.
            num_ret_vals = 0;
        }
    } else {
        num_ret_vals = 1;
        bson_document_or_array_to_table (L, doc, true);
    }

    DONE:
    bson_destroy (&query);
    bson_destroy (&fields);
    mongoc_cursor_destroy (cursor);

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return num_ret_vals;
}


int
lua_mongo_collection_update_one (lua_State *L)
{
    collection_t *collection;
    bson_t filter = BSON_INITIALIZER;
    bson_t update = BSON_INITIALIZER;
    bool upsert;
    bson_error_t error;
    bool ret;
    mongoc_bulk_operation_t *bulk_update;
    bson_t reply;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_isnil(L, 2))) {
        lua_table_to_bson(L, &filter, 2, false);
    }

    if (lua_isnil(L, 3)) {
        luaL_error(L, "update parameters must be included");
    } else {
        lua_table_to_bson(L, &update, 3, false);
    }

    if((lua_isboolean(L, 4))) {
        upsert = lua_toboolean(L, 4);
    } else {
        luaL_error(L, "upsert parameter must be a boolean");
    }

    bulk_update = mongoc_collection_create_bulk_operation
            (collection->c_collection,
            false, NULL);

    mongoc_bulk_operation_update_one(bulk_update, &filter, &update, upsert);
    ret = mongoc_bulk_operation_execute (bulk_update, &reply, &error);

    if (!ret) {
        luaL_error(L, "Error: %s\n", error.message);
    }

    bson_document_or_array_to_table(L, &reply, true);

    return 1;
}

int
lua_mongo_collection_insert_one (lua_State *L)
{

    int num_args = lua_gettop(L);
    if (num_args != 2) {
        luaL_error(L,
                   "insert_one only takes in two parameters, %d given", num_args);
    }

    collection_t *collection;
    mongoc_bulk_operation_t *bulk_insert;
    bson_t bson_doc = BSON_INITIALIZER;
    bson_error_t error;
    bson_t reply;
    bool ret;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    bulk_insert = mongoc_collection_create_bulk_operation(collection->c_collection,
                                                          true, NULL);

    if (!(lua_istable(L, 2))) {
        luaL_error(L, "second input must be a table");
    }

    lua_table_to_bson(L, &bson_doc, 2, true);

    mongoc_bulk_operation_insert(bulk_insert, &bson_doc);
    ret = mongoc_bulk_operation_execute (bulk_insert, &reply, &error);

    if (!ret) {
        luaL_error(L, "Error: %s\n", error.message);
    }

    lua_getglobal(L, "InsertOneResult");
    lua_getfield( L, -1, "new");
    lua_pushboolean(L, ret);
    lua_place_bson_field_value_on_top_of_stack(L, &bson_doc, "_id");

    if (lua_pcall(L, 2, 1, 0) != 0) {
        luaL_error(L, "error running function `f': %s", lua_tostring(L, -1));
    }

    lua_remove(L, -2);

    bson_destroy(&bson_doc);
    bson_destroy(&reply);
    mongoc_bulk_operation_destroy (bulk_insert);

    return 1;
}

int
lua_mongo_collection_destroy (lua_State *L)
{
    collection_t *collection;
    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (collection->c_collection != NULL) {
        mongoc_collection_destroy(collection->c_collection);
        collection->c_collection = NULL;
    }
    return 1;
}
