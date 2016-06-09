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

#include "mongo-module.h"

int
luaopen_mongo_module(lua_State *L)
{
    //initialize mongo_client
    luaL_newmetatable(L, "lua_mongoc_client");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    setfuncs_compat(L, lua_mongoc_client_methods, "lua_mongoc_client");
    newlib_compat(L, lua_mongoc_client_functions, "lua_mongoc_client");

    //initialize mongo_database
    luaL_newmetatable(L, "lua_mongoc_database");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    setfuncs_compat(L, lua_mongoc_database_methods, "lua_mongoc_database");
    newlib_compat(L, lua_mongoc_database_functions, "lua_mongoc_database");

    //initialize mongo_collection
    luaL_newmetatable(L, "lua_mongoc_collection");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    setfuncs_compat(L, lua_mongoc_collection_methods, "lua_mongoc_collection");
    newlib_compat(L, lua_mongoc_collection_functions, "lua_mongoc_collection");

    //initialize mongo_cursor
    luaL_newmetatable(L, "lua_mongo_cursor");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    setfuncs_compat(L, lua_mongo_cursor_methods, "lua_mongo_cursor");
    newlib_compat(L, lua_mongo_cursor_functions, "lua_mongo_cursor");

    //initialize mongo/main module
    luaL_newmetatable(L, "mongo_module");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    setfuncs_compat(L, mongo_module_methods, "mongo_module");
    newlib_compat(L, mongo_module_functions, "mongo_module");

    return 1;
}