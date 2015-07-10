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

#include "lua-mongoc-client.h"
#include "lua-mongoc-database.h"

int
lua_mongo_database_new(lua_State *L)
{
    const char* database_name;
    client_t *client;
    database_t *database;

    client = (client_t *)luaL_checkudata(L, 1, "lua_mongoc_client");

    database_name = luaL_checkstring(L, 2);
    if (database_name == NULL)
        luaL_error(L, "database name cannot be empty");

    database = (database_t *)lua_newuserdata(L, sizeof(*database));
    database->c_database = mongoc_client_get_database(client->c_client, database_name);
    database->c_client = client->c_client;

    luaL_getmetatable(L, "lua_mongoc_database");
    lua_setmetatable(L, -2);

    return 1;
}


int
lua_mongo_database_get_collection_names (lua_State *L)
{
    database_t *database;
    int num_collections;
    char **collection_names;
    bson_error_t error;

    database = (database_t *)luaL_checkudata(L, 1, "lua_mongoc_database");

    collection_names = mongoc_database_get_collection_names (database->c_database, &error);
    if (collection_names) {
        for (num_collections = 0; collection_names[num_collections]; num_collections++);
        lua_createtable(L, 0, num_collections);
        for (int i = 0; i < num_collections; i++) {
            //lua indexes start at 1
            lua_pushinteger(L, i + 1);
            lua_pushstring(L, collection_names[i]);
            lua_settable(L, -3);
        }
        bson_strfreev (collection_names);
    } else {
        luaL_error(L, error.message);
    }

    return 1;
}


int
lua_mongo_database_drop (lua_State *L)
{
    database_t *database;
    bson_error_t error;
    bool ret;

    database = (database_t *)luaL_checkudata(L, 1, "lua_mongoc_database");

    if ((ret = mongoc_database_drop(database->c_database, &error))) {
        lua_pushboolean(L, ret);
    } else {
        luaL_error(L, error.message);
    }

    return 1;
}


int
lua_mongo_database_has_collection (lua_State *L)
{
    database_t *database;
    const char *database_name;
    bool has_collection;

    database = (database_t *)luaL_checkudata(L, 1, "lua_mongoc_database");
    database_name = luaL_checkstring(L, 2);
    if (database_name == NULL) {
        luaL_error(L, "database name cannot be empty");
    }

    has_collection = mongoc_database_has_collection(database->c_database, database_name, NULL);
    lua_pushboolean(L, has_collection);

    return 1;
}


int
lua_mongo_database_command_simple (lua_State *L) {
    database_t *database;
    char *command_name;
    bson_t command = BSON_INITIALIZER;
    bson_t reply = BSON_INITIALIZER;
    bson_error_t error;
    bool throw_error = false;

    database = (database_t *)luaL_checkudata(L, 1, "lua_mongoc_database");

    command_name = luaL_checkstring(L, 2);
    if (command_name == NULL) {
        luaL_error(L, "command_name must be a string");
    }

    // If user does not put in value, it defaults to 1. Assume this will
    // always be a value to append to the bson document.
    if (!(append_stack_value_to_bson_doc(L, &command, command_name,
                                         3, &error))) {
        throw_error = true;
        goto DONE;
    }

    if (lua_istable(L, 4)) {
        if (!(add_lua_table_contents_to_bson_doc(L, &command, 4, false, &error))) {
            throw_error = true;
            goto DONE;
        }
    } else if (!(lua_isnil(L, 4))) {
        throw_error = true;
        strncpy (error.message,
                 "options parameter can only be a table or nil value",
                 sizeof (error.message));
        goto DONE;
    }

    if (!(mongoc_database_command_simple(database->c_database, &command,
                                         NULL,
                                         &reply, &error))) {
        throw_error = true;
        goto DONE;
    }

    if (!(bson_document_or_array_to_table(L, &reply, true, &error))) {
        throw_error = true;
        goto DONE;
    }

DONE:
    bson_destroy(&command);
    bson_destroy(&reply);

    if (throw_error) {
        luaL_error(L, error.message);
    }

    return 1;

}


int
lua_mongo_database_destroy (lua_State *L)
{
    database_t *database;
    database = (database_t *)luaL_checkudata(L, 1, "lua_mongoc_database");

    if (database->c_database != NULL) {
        mongoc_database_destroy(database->c_database);
        database->c_database = NULL;
    }

    return 1;
}