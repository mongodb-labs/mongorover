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

int lua_mongo_client_new(lua_State *L) {
    client_t *client;
    const char *db_uri;

    mongoc_init ();

    client = (client_t *)lua_newuserdata(L, sizeof(*client));

    db_uri = luaL_checkstring(L, 1);
    if (db_uri == NULL) {
        luaL_error(L, "database uri cannot be empty");
    }

    client->c_client = mongoc_client_new (db_uri);

    // Add the metatable to the stack.
    luaL_getmetatable(L, "lua_mongoc_client");
    // Set the metatable on the userdata.
    lua_setmetatable(L, -2);
    return 1;
}

int
lua_mongo_client_get_database_names(lua_State *L) {
    char** database_names;
    bson_error_t error;
    client_t *client;
    int num_databases;

    client = (client_t *)luaL_checkudata(L, 1, "lua_mongoc_client");

    database_names = mongoc_client_get_database_names(client->c_client, &error);
    if (database_names) {
        for (num_databases = 0; database_names[num_databases]; num_databases++);
        lua_createtable(L, 0, num_databases);
        for (int i = 0; i < num_databases; i++) {
            //lua indexes start at 1
            lua_pushinteger(L, i + 1);
            lua_pushstring(L, database_names[i]);
            lua_settable(L, -3);
        }
        bson_strfreev (database_names);
    } else {
        luaL_error(L, error.message);
    }

    return 1;
}

int
lua_mongo_client_destroy(lua_State *L) {
    client_t *client;

    client = (client_t *)luaL_checkudata(L, 1, "lua_mongoc_client");
    if (client->c_client != NULL) {
        mongoc_client_destroy(client->c_client);
        client->c_client = NULL;
    }

    return 1;
}