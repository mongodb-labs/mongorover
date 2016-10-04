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

#ifndef MONGO_MODULE_LUA_OBJECT_GENERATORS_H
#define MONGO_MODULE_LUA_OBJECT_GENERATORS_H

#include "lua-bson.h"
#include "lua-mongoc-wrapper.h"

bool generate_ObjectID(lua_State *L,
                       char *str,
                       int absolute_luaBSONObjects_index,
                       bson_error_t *error);

bool is_ObjectId(lua_State *L,
                 int index,
                 int absolute_luaBSONObjects_index);

bool generate_BSONNull(lua_State *L,
                       int absolute_luaBSONObjects_index,
                       bson_error_t *error);

bool is_BSONNull(lua_State *L,
                 int index,
                 int absolute_luaBSONObjects_index);

bool generate_BSONDate(lua_State *L,
                       int64_t datetime,
                       int absolute_luaBSONObjects_index,
                       bson_error_t *error);

bool is_BSONDate(lua_State *L,
                 int index,
                 int absolute_luaBSONObjects_index);

#endif //MONGO_MODULE_LUA_OBJECT_GENERATORS_H
