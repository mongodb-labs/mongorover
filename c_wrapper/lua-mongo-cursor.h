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

#ifndef MONGO_MODULE_LUA_MONGO_CURSOR_H
#define MONGO_MODULE_LUA_MONGO_CURSOR_H

#include "lua-mongoc-wrapper.h"
#include "lua-bson.h"

void lua_mongo_cursor_new(lua_State *L, mongoc_cursor_t *cursor);
int lua_mongo_cursor_iterate(lua_State *L);
int lua_mongo_cursor_destroy(lua_State *L);

static const struct luaL_Reg lua_mongo_cursor_methods[] = {
        { "next", lua_mongo_cursor_iterate },
        { "__gc", lua_mongo_cursor_destroy },
        { NULL, NULL },
};

static const struct luaL_Reg lua_mongo_cursor_functions[] = {
        { NULL, NULL }
};

typedef struct {
    mongoc_cursor_t *c_cursor;
} lua_mongo_cursor_t;

#endif //MONGO_MODULE_LUA_MONGO_CURSOR_H


