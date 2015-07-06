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
        // Ignore error if dropping collection that does not exist.
        if (strcmp(error.message, "ns not found") != 0) {
            luaL_error(L, error.message);
        }
    }

    return 1;
}


/**
 * lua_mongo_collection_count
 * wrapper for mongoc_collection_count
 * Currently does not support mongoc_query_flags_t and mongoc_read_prefs_t.
 */

int
lua_mongo_collection_count (lua_State *L)
{
    collection_t *collection;
    bson_t query = BSON_INITIALIZER;
    int64_t skip;
    int64_t limit;
    int64_t count;
    bson_error_t error;
    bool throw_error = false;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_isnil(L, 2))) {
        lua_table_to_bson(L, &query, 2, false);
    }

    if (!(lua_isnumber(L, 3))) {
        throw_error = true;
        strncpy (error.message, "skip parameter must be a number", sizeof (error.message));
        goto DONE;
    }

    skip = lua_tonumber(L, 3);

    if (!(lua_isnumber(L, 4))) {
        luaL_error(L, "limit parameter must be a number");
    }
    limit = lua_tonumber(L, 4);

    count = mongoc_collection_count (collection->c_collection,
                                     MONGOC_QUERY_NONE, &query,
                                     skip, limit, NULL, &error);

    if (count < 0) {
        throw_error = true;
        goto DONE;
    }

DONE:
    bson_destroy(&query);

    if (throw_error) {
        luaL_error(L, error.message);
    }

    lua_pushnumber(L, count);

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
lua_mongo_collection_update (lua_State *L)
{
    collection_t *collection;
    bson_t filter = BSON_INITIALIZER;
    bson_t update = BSON_INITIALIZER;
    bool upsert;
    bool update_many;
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

    if ((lua_isboolean(L, 4))) {
        upsert = lua_toboolean(L, 4);
    } else {
        luaL_error(L, "upsert parameter must be a boolean");
    }

    if ((lua_isboolean(L, 5))) {
        update_many = lua_toboolean(L, 5);
    } else {
        luaL_error(L, "update_many parameter must be a boolean");
    }

    bulk_update = mongoc_collection_create_bulk_operation
            (collection->c_collection,
            false, NULL);

    if (update_many) {
        mongoc_bulk_operation_update(bulk_update, &filter, &update, upsert);
    } else {
        mongoc_bulk_operation_update_one(bulk_update, &filter, &update, upsert);
    }

    ret = mongoc_bulk_operation_execute (bulk_update, &reply, &error);

    if (!ret) {
        luaL_error(L, error.message);
    }

    bson_document_or_array_to_table(L, &reply, true);

    return 1;
}

int
lua_mongo_collection_insert_one (lua_State *L)
{
    collection_t *collection;
    mongoc_bulk_operation_t *bulk_insert;
    bson_t bson_doc = BSON_INITIALIZER;
    bson_error_t error;
    bson_t reply;
    bool ret;
    bool throw_error = false;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_istable(L, 2))) {
        luaL_error(L, "second input must be a table");
    }

    bulk_insert = mongoc_collection_create_bulk_operation(collection->c_collection,
                                                          true, NULL);

    lua_table_to_bson(L, &bson_doc, 2, true);

    mongoc_bulk_operation_insert(bulk_insert, &bson_doc);
    ret = mongoc_bulk_operation_execute (bulk_insert, &reply, &error);

    if (!ret) {
        throw_error = true;
        goto DONE;
    }

    generate_InsertOneResult(L, ret, 2);

DONE:

    bson_destroy(&bson_doc);
    bson_destroy(&reply);
    mongoc_bulk_operation_destroy (bulk_insert);

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 1;
}

int
lua_mongo_collection_insert_many (lua_State *L)
{
    collection_t *collection;
    mongoc_bulk_operation_t *bulk_insert;
    bool ordered;
    bson_error_t error;
    bson_t reply;
    bool ret;
    bool throw_error = false;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_istable(L, 2))) {
        luaL_error(L, "second input must be a table");
    }

    if (!(lua_isboolean(L, 3))) {
        luaL_error(L, "ordered parameter must be a boolean");
    } else {
        ordered = lua_toboolean(L, 3);
    }

    bulk_insert = mongoc_collection_create_bulk_operation(collection->c_collection,
                                                          ordered, NULL);

    lua_pushnil(L);

    int num_elements, lua_index;
    for (num_elements = 0, lua_index = 1;
         lua_next(L, 2) != 0;
         num_elements++, lua_index++)
    {
        bson_t bson_doc = BSON_INITIALIZER;

        if (!(lua_isnumber(L, -2))) {
            luaL_error(L, "malformed array of documents");
        }

        if (lua_tonumber(L, -2) != lua_index) {
            luaL_error(L, "malformed array of documents");
        }

        lua_table_to_bson(L, &bson_doc, -1, true);
        mongoc_bulk_operation_insert(bulk_insert, &bson_doc);
        bson_destroy(&bson_doc);

        lua_pop(L, 1);
    }

    ret = mongoc_bulk_operation_execute (bulk_insert, &reply, &error);

    if (!(ret)) {
        throw_error = true;
        goto DONE;
    }

    generate_InsertManyResult(L, &reply, 2, num_elements);

DONE:
    bson_destroy(&reply);
    mongoc_bulk_operation_destroy (bulk_insert);

    if (throw_error) {
        luaL_error(L, error.message);
    }

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
