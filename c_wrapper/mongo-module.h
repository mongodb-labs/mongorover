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
#ifndef LUA_MONGO_MODULE_H
#define LUA_MONGO_MODULE_H

#include "lua-bson.h"
#include "lua-mongoc-client.h"
#include "lua-mongoc-collection.h"
#include "lua-mongo-cursor.h"
#include "lua-mongoc-database.h"
#include "lua-mongoc-wrapper.h"
#include "lua-version-compat.h"

static const struct luaL_Reg mongo_module_methods[] = {
    { NULL, NULL },
};

static const struct luaL_Reg mongo_module_functions[] = {
    { "client_new", lua_mongo_client_new },
    { "collection_new", lua_mongo_collection_new },
    { "database_new", lua_mongo_database_new },
    { NULL,  NULL }
};

#endif //LUA_MONGO_MODULE_H