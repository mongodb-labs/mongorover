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

#include "lua-version-compat.h"
#include "lua-mongoc-wrapper.h"


/**
 * lua versions <= 5.2 do not have integers (a subtype of number)
 * if supported, push the integer onto the stack
 * if not, push it as a number
 */
void
lua_pushinteger_compat(lua_State *L,
                       int value)
{
#if LUA_VERSION_NUM >= 503
    lua_pushinteger(L, value);
#else
    lua_pushnumber(L, value);
#endif
}

void
setfuncs_compat(lua_State *L,
                const struct luaL_Reg *R,
                char *name_for_lua51)
{

#if LUA_VERSION_NUM >= 502
    luaL_setfuncs(L, R, 0);
#else
    lua_setglobal(L, name_for_lua51);
    luaL_register(L, name_for_lua51, R);
#endif
}

void
newlib_compat(lua_State *L,
              const struct luaL_Reg *R,
              char *name_for_lua51)
{

#if LUA_VERSION_NUM >= 502
    luaL_newlib(L, R);
#else
    luaopen_base(L);
    luaL_register(L, name_for_lua51, R);
#endif
}

