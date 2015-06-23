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

#include "lua-mongoc-wrapper.h"

void generate_ObjectID(lua_State *L, char *str);
bool is_ObjectId(lua_State *L);
void generate_BSONNull(lua_State *L);
bool is_BSONNull(lua_State *L);

#endif //MONGO_MODULE_LUA_OBJECT_GENERATORS_H
