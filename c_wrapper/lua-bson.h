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

#ifndef MONGO_LUA_DRIVER_LUA_BSON_H
#define MONGO_LUA_DRIVER_LUA_BSON_H

#include <stdlib.h>
#include "lua-mongoc-wrapper.h"
#include "lua-object-generators.h"

bool find_and_set_or_create_id(lua_State *L,
                               int index,
                               bson_t *bson_doc,
                               bson_error_t *error);

int lua_array_length(lua_State *L, int index);

bool lua_table_to_bson(lua_State *L,
                       bson_t *bson_doc,
                       int index,
                       bool generate_id,
                       bson_error_t *error);

bool bson_document_or_array_to_table(lua_State *L,
                                     bson_t *bson_doc,
                                     bool is_table,
                                     bson_error_t *error);



#endif //MONGO_LUA_DRIVER_LUA_BSON_H
