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

#ifndef LUA_VERSION_COMPAT_H
#define LUA_VERSION_COMPAT_H

#include "lua-mongoc-wrapper.h"

void setfuncs_compat(lua_State *L, const struct luaL_Reg *R, char *name_for_lua51);
void newlib_compat(lua_State *L, const struct luaL_Reg *R, char *name_for_lua51);

#endif //LUA_VERSION_COMPAT_H
