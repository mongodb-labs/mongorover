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
        if (!(lua_table_to_bson(L, &query, 2, false, &error))) {
            throw_error = true;
            goto DONE;
        }
    }

    if (!(lua_isnumber(L, 3))) {
        throw_error = true;
        strncpy (error.message, "skip parameter must be a number", sizeof (error.message));
        goto DONE;
    }

    if (!(lua_isnumber(L, 4))) {
        luaL_error(L, "limit parameter must be a number");
    }

    skip = lua_tonumber(L, 3);
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
    bool throw_error = false;
    bson_error_t error;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_isnil(L, 3))) {
        if (!(lua_table_to_bson(L, &query, 3, false, &error))) {
            throw_error = true;
            goto DONE;
        }
    }

    if (!(lua_isnil(L, 4))) {
        if (!(lua_table_to_bson(L, &fields, 4, false, &error))) {
            throw_error = true;
            goto DONE;
        }
    }

    cursor = mongoc_collection_find (collection->c_collection,
                                     MONGOC_QUERY_NONE, 0, 0, 0,
                                     &query, &fields, NULL);

    lua_mongo_cursor_new(L, cursor);

DONE:
    bson_destroy (&query);
    bson_destroy (&fields);

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 1;
}

int
lua_mongo_collection_find_one (lua_State *L)
{
    collection_t *collection;
    bson_t query = BSON_INITIALIZER;
    bson_t fields = BSON_INITIALIZER;
    mongoc_cursor_t *cursor = NULL;
    const bson_t *doc;
    int num_ret_vals;
    bool throw_error = false;
    bson_error_t error;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_isnil(L, 2))) {
        if (!(lua_table_to_bson(L, &query, 2, false, &error))) {
            throw_error = true;
            goto DONE;
        }
    }

    if (!(lua_isnil(L, 3))) {
        if (!(lua_table_to_bson(L, &fields, 3, false, &error))) {
            throw_error = true;
            goto DONE;
        }
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
        if (!(bson_document_or_array_to_table (L, doc, true, &error))) {
            throw_error = true;
            goto DONE;
        }
    }

DONE:
    bson_destroy (&query);
    bson_destroy (&fields);

    if (cursor) {
        mongoc_cursor_destroy (cursor);
    }

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
    mongoc_bulk_operation_t *bulk_update = NULL;
    bson_t reply = BSON_INITIALIZER;
    bool throw_error = false;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_isnil(L, 2))) {
        if (!(lua_table_to_bson(L, &filter, 2, false, &error))) {
            throw_error = true;
            goto DONE;
        }
    }

    if (lua_isnil(L, 3)) {
        luaL_error(L, "update parameters must be included");
    } else {
        if (!(lua_table_to_bson(L, &update, 3, false, &error))) {
            throw_error = true;
            goto DONE;
        }
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
        throw_error = true;
        goto DONE;
    }

    if (!(bson_document_or_array_to_table(L, &reply, true, &error))) {
        throw_error = true;
        goto DONE;
    }

DONE:
    bson_destroy(&filter);
    bson_destroy(&update);
    bson_destroy(&reply);
    if (bulk_update) {
        mongoc_bulk_operation_destroy (bulk_update);
    }

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 1;
}

int
lua_mongo_collection_insert_one (lua_State *L)
{
    collection_t *collection;
    mongoc_bulk_operation_t *bulk_insert = NULL;
    bson_t bson_doc = BSON_INITIALIZER;
    bson_error_t error;
    bson_t reply = BSON_INITIALIZER;
    bool ret;
    bool throw_error = false;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_istable(L, 2))) {
        luaL_error(L, "second input must be a table");
    } else {
        if (!(lua_table_to_bson(L, &bson_doc, 2, true, &error))) {
            throw_error = true;
            goto DONE;
        }
    }

    bulk_insert = mongoc_collection_create_bulk_operation(collection->c_collection,
                                                          true, NULL);

    mongoc_bulk_operation_insert(bulk_insert, &bson_doc);
    ret = mongoc_bulk_operation_execute (bulk_insert, &reply, &error);

    if (!ret) {
        throw_error = true;
        goto DONE;
    }

    if (!(generate_InsertOneResult(L, ret, 2, &error))) {
        throw_error = true;
        goto DONE;
    }

DONE:

    bson_destroy(&bson_doc);
    bson_destroy(&reply);

    if (bulk_insert) {
        mongoc_bulk_operation_destroy(bulk_insert);
    }

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 1;
}

int
lua_mongo_collection_insert_many (lua_State *L)
{
    collection_t *collection;
    mongoc_bulk_operation_t *bulk_insert = NULL;
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
        if (!(lua_isnumber(L, -2)) || lua_tonumber(L, -2) != lua_index) {
            throw_error = true;
            strncpy(error.message,
                    "malformed array of documents",
                    sizeof(error.message));
            goto DONE;
        }

        if (!(lua_table_to_bson(L, &bson_doc, -1, true, &error))) {
            throw_error = true;
            goto DONE;
        }
        mongoc_bulk_operation_insert(bulk_insert, &bson_doc);
        lua_pop(L, 1);
        bson_destroy(&bson_doc);
    }

    if (!(mongoc_bulk_operation_execute (bulk_insert, &reply, &error))) {
        throw_error = true;
        goto DONE;
    }

    if (!(generate_InsertManyResult(L, &reply, 2, num_elements, &error))) {
        throw_error = true;
        goto DONE;
    }

DONE:
    bson_destroy(&reply);

    if (bulk_insert){
        mongoc_bulk_operation_destroy (bulk_insert);
    }

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 1;
}


int lua_mongo_collection_aggregate (lua_State *L)
{
    collection_t *collection;
    bson_t aggregation_pipeline = BSON_INITIALIZER;
    bson_t inner_aggregation_pipeline = BSON_INITIALIZER;
    mongoc_cursor_t *cursor = NULL;
    bson_error_t error;
    bool throw_error = false;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_istable(L, 2)) || !(lua_table_is_array(L, 2))) {
        luaL_error(L, "aggregation pipeline must be an array");
    } else {
        bson_append_array_begin(&aggregation_pipeline, "pipeline", -1,
                                &inner_aggregation_pipeline);

        if (!(lua_table_to_bson(L, &inner_aggregation_pipeline, 2, false, &error))) {
            throw_error = true;
            goto DONE;
        }

        bson_append_array_end(&aggregation_pipeline, &inner_aggregation_pipeline);
    }

    cursor = mongoc_collection_aggregate (collection->c_collection, MONGOC_QUERY_NONE,
                                          &aggregation_pipeline,
                                          NULL, NULL);

DONE:
    bson_destroy(&aggregation_pipeline);
    bson_destroy(&inner_aggregation_pipeline);

    if (throw_error) {
        if (cursor) {
            mongoc_cursor_destroy (cursor);
        }
        luaL_error(L, error.message);
    }

    lua_mongo_cursor_new(L, cursor);

    return 1;
}


int
lua_mongo_collection_delete_one(lua_State *L)
{
    collection_t *collection;
    bson_t selector = BSON_INITIALIZER;
    mongoc_bulk_operation_t *bulk_remove = NULL;
    bson_error_t error;
    bson_t reply;
    bool throw_error = false;
    bool ret;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if ((lua_istable(L, 2))) {
        if (!(lua_table_to_bson(L, &selector, 2, false, &error))) {
            throw_error = true;
            goto DONE;
        }
    } else {
        luaL_error(L, "second input must be a table");
    }

    bulk_remove = mongoc_collection_create_bulk_operation(collection->c_collection,
                                                          true, NULL);

    mongoc_bulk_operation_remove_one(bulk_remove, &selector);

    ret = mongoc_bulk_operation_execute(bulk_remove, &reply, &error);
    if (!(ret)) {
        throw_error = true;
        goto DONE;
    }

    if (!(generate_DeleteResult(L, &reply, ret, &error))) {
        throw_error = true;
        goto DONE;
    }

DONE:
    bson_destroy(&selector);
    if (bulk_remove) {
        mongoc_bulk_operation_destroy (bulk_remove);
    }

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 1;
}

int
lua_mongo_collection_delete_many(lua_State *L)
{
    collection_t *collection;
    bson_t selector = BSON_INITIALIZER;
    mongoc_bulk_operation_t *bulk_remove = NULL;
    bson_error_t error;
    bson_t reply;
    bool throw_error = false;
    bool ret;

    collection = (collection_t *)luaL_checkudata(L, 1, "lua_mongoc_collection");

    if ((lua_istable(L, 2))) {
        if (!(lua_table_to_bson(L, &selector, 2, false, &error))) {
            throw_error = true;
            goto DONE;
        }
    } else {
        luaL_error(L, "second input must be a table");
    }

    bulk_remove = mongoc_collection_create_bulk_operation(collection->c_collection,
                                                          true, NULL);

    mongoc_bulk_operation_remove(bulk_remove, &selector);

    ret = mongoc_bulk_operation_execute(bulk_remove, &reply, &error);
    if (!(ret)) {
        throw_error = true;
        goto DONE;
    }

    if (!(generate_DeleteResult(L, &reply, ret, &error))) {
        throw_error = true;
        goto DONE;
    }

    DONE:
    bson_destroy(&selector);
    if (bulk_remove) {
        mongoc_bulk_operation_destroy (bulk_remove);
    }

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
