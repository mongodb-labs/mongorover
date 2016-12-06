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
#include "lua-version-compat.h"

int
lua_mongo_collection_new(lua_State *L)
{
    const char *collection_name;
    database_t *database;
    collection_t *collection;

    int collection_name_index = 2;

    database = (database_t *) luaL_checkudata(L, 1, "lua_mongoc_database");

    collection_name = luaL_checkstring(L, collection_name_index);
    if (collection_name == NULL) {
        luaL_error(L, "collection name cannot be empty");
    }

    collection = (collection_t *) lua_newuserdata(L, sizeof(*collection));
    collection->c_collection = mongoc_database_get_collection(database->c_database, collection_name);
    collection->c_database = database->c_database;

    luaL_getmetatable (L, "lua_mongoc_collection");
    lua_setmetatable(L, -2);

    return 1;
}

int
lua_mongo_collection_drop(lua_State *L)
{
    collection_t *collection;
    bson_error_t error;
    bool ret;

    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");

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
lua_mongo_collection_count(lua_State *L)
{
    collection_t *collection;
    bson_t query = BSON_INITIALIZER;
    int64_t skip;
    int64_t limit;
    int64_t count;
    bson_error_t error;
    bool throw_error = false;

    int absolute_luaBSONObjects_index = 2;
    int query_index = 3;
    int skip_index = 4;
    int limit_index = 5;

    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_isnil(L, query_index))) {
        throw_error = !(lua_table_to_bson(L, &query, query_index, false, absolute_luaBSONObjects_index, &error));
        if (throw_error) {
            goto DONE;
        }
    }

    throw_error = !(lua_isnumber(L, skip_index));
    if (throw_error) {
        strncpy (error.message, "skip parameter must be a number", sizeof(error.message));
        goto DONE;
    }

    if (!(lua_isnumber(L, limit_index))) {
        luaL_error(L, "limit parameter must be a number");
    }

    skip = lua_tonumber(L, skip_index);
    limit = lua_tonumber(L, limit_index);

    count = mongoc_collection_count(collection->c_collection,
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

    lua_pushinteger_compat(L, count);

    return 1;
}


int
lua_mongo_collection_find(lua_State *L)
{
    collection_t *collection;
    uint32_t skip = 0;
    uint32_t limit = 0;
    uint32_t batch_size = 0;
    uint32_t query_flags = 0;
    bson_t query = BSON_INITIALIZER;
    bson_t fields = BSON_INITIALIZER;

    mongoc_cursor_t *cursor;
    bool throw_error = false;
    bson_error_t error;

    int absolute_luaBSONObjects_index = 2;
    int query_flags_index = 3;
    int more_query_flags_index = 4;
    int skip_index = 5;
    int limit_index = 6;
    int batch_size_index = 7;
    int query_index = 8;
    int fields_index = 9;

    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");
    query_flags = (uint32_t) luaL_checknumber(L, query_flags_index);

    lua_pushnil(L);
    while (lua_next(L, more_query_flags_index) != 0) {
        query_flags |= (int) lua_tonumber(L, -1);
        lua_pop(L, 1);
    }

    skip = (uint32_t) luaL_checknumber(L, skip_index);
    limit = (uint32_t) luaL_checknumber(L, limit_index);
    batch_size = (uint32_t) luaL_checknumber(L, batch_size_index);

    luaL_argcheck(L, lua_isnil(L, query_index) | lua_istable(L, query_index), query_index,
                  "query must be nil or a table");
    luaL_argcheck(L, lua_isnil(L, fields_index) | lua_istable(L, fields_index), fields_index,
                  "fields must be nil or a table");

    if (!(lua_isnil(L, query_index))) {
        throw_error = !(lua_table_to_bson(L, &query, query_index, false, absolute_luaBSONObjects_index, &error));
        if (throw_error) {
            goto DONE;
        }
    }

    if (!(lua_isnil(L, fields_index))) {
        throw_error = !(lua_table_to_bson(L, &fields, fields_index, false, absolute_luaBSONObjects_index, &error));
        if (throw_error) {
            goto DONE;
        }
    }

    cursor = mongoc_collection_find(collection->c_collection,
                                    query_flags,
                                    skip,
                                    limit,
                                    batch_size,
                                    &query,
                                    &fields,
                                    NULL);

    lua_mongo_cursor_new(L, cursor);

DONE:
    bson_destroy(&query);
    bson_destroy(&fields);

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 1;
}


int
lua_mongo_collection_update(lua_State *L)
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

    int absolute_luaBSONObjects_index = 2;
    int filter_index = 3;
    int update_index = 4;
    int upsert_index = 5;
    int update_many_index = 6;

    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_isnil(L, filter_index))) {
        throw_error = !(lua_table_to_bson(L, &filter, filter_index, false, absolute_luaBSONObjects_index, &error));
        if (throw_error) {
            goto DONE;
        }
    }

    if (lua_isnil(L, update_index)) {
        luaL_error(L, "update parameters must be included");
    } else {
        throw_error = !(lua_table_to_bson(L, &update, update_index, false, absolute_luaBSONObjects_index, &error));
        if (throw_error) {
            goto DONE;
        }
    }

    if ((lua_isboolean(L, upsert_index))) {
        upsert = lua_toboolean(L, upsert_index);
    } else {
        luaL_error(L, "upsert parameter must be a boolean");
    }

    if ((lua_isboolean(L, update_many_index))) {
        update_many = lua_toboolean(L, update_many_index);
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

    ret = mongoc_bulk_operation_execute(bulk_update, &reply, &error);

    if (!ret) {
        throw_error = true;
        goto DONE;
    }

    throw_error = !(bson_document_or_array_to_table(L, &reply, true, absolute_luaBSONObjects_index, &error));
    if (throw_error) {
        goto DONE;
    }

DONE:
    bson_destroy(&filter);
    bson_destroy(&update);
    bson_destroy(&reply);
    if (bulk_update) {
        mongoc_bulk_operation_destroy(bulk_update);
    }

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 1;
}

int
lua_mongo_collection_insert_one(lua_State *L)
{
    collection_t *collection;
    mongoc_bulk_operation_t *bulk_insert = NULL;
    bson_t bson_doc = BSON_INITIALIZER;
    bson_error_t error;
    bson_t reply = BSON_INITIALIZER;
    bool ret;
    bool throw_error = false;

    int absolute_luaBSONObjects_index = 2;
    int document_index = 3;

    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_istable(L, document_index))) {
        luaL_error(L, "second input must be a table");
    } else {
        throw_error = !(lua_table_to_bson(L, &bson_doc, document_index, true, absolute_luaBSONObjects_index, &error));
        if (throw_error) {
            goto DONE;
        }
    }

    bulk_insert = mongoc_collection_create_bulk_operation(collection->c_collection,
                                                          true, NULL);
    mongoc_bulk_operation_insert(bulk_insert, &bson_doc);
    ret = mongoc_bulk_operation_execute(bulk_insert, &reply, &error);

    if (!ret) {
        throw_error = true;
        goto DONE;
    }

    //put return variables on top of stack to be returned
    lua_pushboolean(L, ret);
    lua_pushstring(L, "_id");
    lua_gettable(L, document_index);

DONE:
    bson_destroy(&bson_doc);
    bson_destroy(&reply);

    if (bulk_insert) {
        mongoc_bulk_operation_destroy(bulk_insert);
    }

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 2;
}

int
lua_mongo_collection_insert_many(lua_State *L)
{
    collection_t *collection;
    mongoc_bulk_operation_t *bulk_insert = NULL;
    bool ordered;
    bson_error_t error;
    bson_t reply = BSON_INITIALIZER;
    bool throw_error = false;
    int i, num_elements, lua_index;

    int absolute_luaBSONObjects_index = 2;
    int documents_index = 3;
    int ordered_index = 4;

    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (!(lua_istable(L, documents_index))) {
        luaL_error(L, "documents parameter must be a table");
    }

    if (!(lua_isboolean(L, ordered_index))) {
        luaL_error(L, "ordered parameter must be a boolean");
    } else {
        ordered = lua_toboolean(L, ordered_index);
    }

    bulk_insert = mongoc_collection_create_bulk_operation(collection->c_collection,
                                                          ordered, NULL);

    lua_pushnil(L);

    for (num_elements = 0, lua_index = 1;
         lua_next(L, documents_index) != 0;
         num_elements++, lua_index++) {
        bson_t bson_doc = BSON_INITIALIZER;
        throw_error = !(lua_isnumber(L, -2)) || lua_tonumber(L, -2) != lua_index;
        if (throw_error) {
            strncpy(error.message,
                    "malformed array of documents",
                    sizeof(error.message));
            goto DONE;
        }

        throw_error = !(lua_table_to_bson(L, &bson_doc, -1, true, absolute_luaBSONObjects_index, &error));
        if (throw_error) {
            goto DONE;
        }
        mongoc_bulk_operation_insert(bulk_insert, &bson_doc);
        lua_pop(L, 1);
        bson_destroy(&bson_doc);
    }

    throw_error = !(mongoc_bulk_operation_execute(bulk_insert, &reply, &error));
    if (throw_error) {
        goto DONE;
    }

    // Place reply on top of the stack
    throw_error = !(bson_document_or_array_to_table(L, &reply, true, absolute_luaBSONObjects_index, &error));
    if (throw_error) {
        goto DONE;
    }

    // make array of inserted ids
    lua_newtable(L);
    for (i = 0, lua_index = 1; i < num_elements; i++, lua_index++) {
        lua_rawgeti(L, documents_index, lua_index);
        lua_pushstring(L, "_id");
        lua_gettable(L, -2);

        throw_error = lua_isnil(L, -1);
        if (throw_error) {
            strncpy(error.message,
                    "_id was not generated for an insert many document",
                    sizeof(error.message));
            goto DONE;
        }

        lua_rawseti(L, -3, lua_index);
        lua_pop(L, 1);
    }

DONE:
    bson_destroy(&reply);

    if (bulk_insert) {
        mongoc_bulk_operation_destroy(bulk_insert);
    }

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 2;
}


int lua_mongo_collection_aggregate(lua_State *L)
{
    collection_t *collection;
    bson_t aggregation_pipeline = BSON_INITIALIZER;
    bson_t inner_aggregation_pipeline = BSON_INITIALIZER;
    mongoc_cursor_t *cursor = NULL;
    bson_error_t error;
    bool throw_error = false;

    int absolute_luaBSONObjects_index = 2;
    int aggregation_pipeline_index = 3;

    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");

    throw_error = !(lua_istable(L, aggregation_pipeline_index)) ||
                  !(lua_table_is_array(L, aggregation_pipeline_index));
    if (throw_error) {
        luaL_error(L, "aggregation pipeline must be an array");
    } else {
        bson_append_array_begin(&aggregation_pipeline, "pipeline", -1,
                                &inner_aggregation_pipeline);

        throw_error = !(lua_table_to_bson(L, &inner_aggregation_pipeline, aggregation_pipeline_index,
                                          false, absolute_luaBSONObjects_index, &error));
        if (throw_error) {
            goto DONE;
        }

        bson_append_array_end(&aggregation_pipeline, &inner_aggregation_pipeline);
    }

    cursor = mongoc_collection_aggregate(collection->c_collection, MONGOC_QUERY_NONE,
                                         &aggregation_pipeline,
                                         NULL, NULL);

DONE:
    bson_destroy(&aggregation_pipeline);
    bson_destroy(&inner_aggregation_pipeline);

    if (throw_error) {
        if (cursor) {
            mongoc_cursor_destroy(cursor);
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

    int absolute_luaBSONObjects_index = 2;
    int selector_index = 3;

    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");

    if ((lua_istable(L, selector_index))) {
        throw_error = !(lua_table_to_bson(L, &selector, selector_index, false, absolute_luaBSONObjects_index, &error));
        if (throw_error) {
            goto DONE;
        }
    } else {
        luaL_error(L, "second input must be a table");
    }

    bulk_remove = mongoc_collection_create_bulk_operation(collection->c_collection,
                                                          true, NULL);

    mongoc_bulk_operation_remove_one(bulk_remove, &selector);

    ret = mongoc_bulk_operation_execute(bulk_remove, &reply, &error);
    if (!ret) {
        throw_error = true;
        goto DONE;
    }

    // place return variables on top of stack
    lua_pushboolean(L, ret);
    throw_error = !(bson_document_or_array_to_table(L, &reply, true, absolute_luaBSONObjects_index, &error));
    if (throw_error) {
        goto DONE;
    }

DONE:
    bson_destroy(&selector);
    if (bulk_remove) {
        mongoc_bulk_operation_destroy(bulk_remove);
    }

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 2;
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

    int absolute_luaBSONObjects_index = 2;
    int selector_index = 3;

    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");

    if ((lua_istable(L, selector_index))) {
        throw_error = !(lua_table_to_bson(L, &selector, selector_index, false, absolute_luaBSONObjects_index, &error));
        if (throw_error) {
            goto DONE;
        }
    } else {
        luaL_error(L, "second input must be a table");
    }

    bulk_remove = mongoc_collection_create_bulk_operation(collection->c_collection,
                                                          true, NULL);

    mongoc_bulk_operation_remove(bulk_remove, &selector);

    ret = mongoc_bulk_operation_execute(bulk_remove, &reply, &error);
    if (!ret) {
        throw_error = true;
        goto DONE;
    }

    // place return variables on top of stack
    lua_pushboolean(L, ret);
    throw_error = !(bson_document_or_array_to_table(L, &reply, true, absolute_luaBSONObjects_index, &error));
    if (throw_error) {
        goto DONE;
    }

DONE:
    bson_destroy(&selector);
    if (bulk_remove) {
        mongoc_bulk_operation_destroy(bulk_remove);
    }

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 2;
}

int
lua_mongo_collection_destroy(lua_State *L)
{
    collection_t *collection;
    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");

    if (collection->c_collection != NULL) {
        mongoc_collection_destroy(collection->c_collection);
        collection->c_collection = NULL;
    }
    return 1;
}


int
lua_mongo_collection_create_index(lua_State *L)
{
    bool throw_error = false;
    bool created = false;

    char *index_name;
    collection_t *collection;
    bson_t keys = BSON_INITIALIZER;
    bson_t weights = BSON_INITIALIZER;
    bson_error_t error;

    mongoc_index_opt_t opt;
    mongoc_index_opt_init(&opt);
    
    int absolute_luaBSONObjects_index = 2;
    int keys_index = 3;
    int options_index = 4;

    if(!(lua_istable(L, keys_index))){
        luaL_error(L, "keys parameter must be a table");
    }else{
        throw_error = !(lua_table_to_bson(L, &keys, keys_index, false, absolute_luaBSONObjects_index, &error ));
        if(throw_error){
            goto DONE;
        }
        index_name = mongoc_collection_keys_to_index_string(&keys);
        opt.name = index_name;
    }

    if(lua_istable(L, options_index)){
        lua_getfield(L, options_index, "name");
        lua_getfield(L, options_index, "default_language");
        lua_getfield(L, options_index, "language_override");
        lua_getfield(L, options_index, "unique");
        lua_getfield(L, options_index, "sparse");
        lua_getfield(L, options_index, "background");
        lua_getfield(L, options_index, "expireAfterSeconds");
        lua_getfield(L, options_index, "textIndexVersion");
        lua_getfield(L, options_index, "weights");

        if(lua_isstring(L, -9)){
            opt.name = lua_tostring(L, -9);
            index_name = opt.name;
        }
        if(lua_isstring(L, -8)){
            opt.default_language = lua_tostring(L, -8);
        }
        if(lua_isstring(L, -7)){
            opt.language_override = lua_tostring(L, -7);
        }
        if(lua_isboolean(L, -6)){
            opt.unique = lua_toboolean(L, -6);
        }
        if(lua_isboolean(L, -5)){
            opt.sparse = lua_toboolean(L, -5);
        }
        if(lua_isboolean(L, -4)){
            opt.background = lua_toboolean(L, -4);
        }
        if(lua_isinteger_compat(L, -3)){
            opt.expire_after_seconds = lua_tointeger_compat(L, -3);
        }
        if(lua_isinteger_compat(L, -2)){
            opt.v = lua_tointeger_compat(L, -2);
        }
        if(lua_istable(L, -1)){
            throw_error = !(lua_table_to_bson(L, &weights, -1, false, absolute_luaBSONObjects_index, &error ));
            if(throw_error){
                goto DONE;
            }
            opt.weights = &weights;
        }
        lua_pop(L, 9);
    }

    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");
    created = mongoc_collection_create_index(collection->c_collection, &keys, &opt, &error);

    lua_pushstring(L, index_name);
    if(!created){
        throw_error = true;
        goto DONE;
    }

DONE:
    bson_destroy(&keys);
    bson_destroy(&weights);
    if(throw_error){
        luaL_error(L, error.message);
    }
    return 1;
}


int 
lua_mongo_collection_drop_index(lua_State *L)
{
    bool ret = false;
    bool throw_error = false;

    bson_t keys = BSON_INITIALIZER;
    collection_t *collection;
    bson_error_t error;

    char *index_name;
    int absolute_luaBSONObjects_index = 2;
    int index_name_index = 3;

    if(lua_isstring(L, index_name_index)){
        index_name = luaL_checkstring(L, index_name_index);
    }else if(lua_istable(L, index_name_index)){
        throw_error = !(lua_table_to_bson(L, &keys, index_name_index, false, absolute_luaBSONObjects_index, &error ));
        if(throw_error){
            goto DONE;
        }
        index_name = mongoc_collection_keys_to_index_string(&keys);
    }
    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");

    ret = mongoc_collection_drop_index(collection->c_collection, index_name, &error);
    lua_pushnil(L);

    if(!ret){
        throw_error = true;
        goto DONE;
    }

DONE:
    bson_destroy(&keys);
    if(throw_error){
        luaL_error(L, error.message);
    }
    return 1;
}


int
lua_mongo_collection_find_indexes(lua_State *L)
{
    collection_t *collection;
    mongoc_cursor_t *cursor = NULL;
    bson_error_t error;

    collection = (collection_t *) luaL_checkudata(L, 1, "lua_mongoc_collection");

    cursor = mongoc_collection_find_indexes(collection->c_collection, &error);
    lua_mongo_cursor_new(L, cursor);

    if(error.code){
        luaL_error(L, error.message);
    }
    return 1;
}
