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

#ifndef LUA_MONGOC_COLLECTION_H
#define LUA_MONGOC_COLLECTION_H

#include "lua-mongoc-database.h"
#include "lua-mongoc-wrapper.h"

int lua_mongo_collection_new(lua_State *L);
int lua_mongo_collection_drop (lua_State *L);
int lua_mongo_collection_destroy(lua_State *L);

static const struct luaL_Reg lua_mongoc_collection_methods[] = {
    { "collection_drop", lua_mongo_collection_drop },
    { "__gc", lua_mongo_collection_destroy },
    { NULL, NULL },
};

static const struct luaL_Reg lua_mongoc_collection_functions[] = {
    { "new", lua_mongo_collection_new },
    { NULL,  NULL }
};

typedef struct {
    mongoc_collection_t *c_collection;
    mongoc_database_t *c_database;
} collection_t;

#endif //LUA_MONGOC_COLLECTION_H
