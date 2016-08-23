cmake_minimum_required(VERSION 3.2)

project(mongo_module C)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_BUILD_TYPE Debug)

# http://www.fysx.org/2013/08/26/switching-between-lua-and-luajit-using-cmake/
# There is no FindLua52, only FindLua51 and FindLua50
# FindLua finds Lua 5.3 instead of Lua 5.2 when both are installed
OPTION (LUA_FORCE_LUA51 "Force Lua 5.1 instead of default Lua" OFF)
UNSET(Lua_FOUND CACHE)
UNSET(LUA_INCLUDE_DIR CACHE)
UNSET(LUA_LIBRARY CACHE)
SET (Lua_FOUND FALSE)

IF (LUA_FORCE_LUA51)
    INCLUDE(FindLua51)
ELSE (LUA_FORCE_LUA51)
    INCLUDE(FindLua)
ENDIF (LUA_FORCE_LUA51)

INCLUDE(FindPkgConfig)
pkg_check_modules (LIBMONGOC REQUIRED libmongoc-1.0)

include_directories(
        ${LIBMONGOC_INCLUDE_DIRS}
        ${LUA_INCLUDE_DIR}
)
MESSAGE(${LUA_LIBRARIES})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -ggdb3 -O0")

set(SOURCE_FILES c_wrapper/mongo-module.c
        c_wrapper/lua-mongoc-client.c
        c_wrapper/lua-mongoc-database.c
        c_wrapper/lua-mongoc-collection.c
        c_wrapper/lua-bson.c
        c_wrapper/lua-object-generators.c)

# creates library in the parent directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)

add_library(mongo_module SHARED ${SOURCE_FILES}
        c_wrapper/lua-version-compat.c
        c_wrapper/lua-object-generators.c
        c_wrapper/lua-mongo-cursor.c)

target_link_libraries(${CMAKE_PROJECT_NAME} ${LIBMONGOC_LDFLAGS} ${LIBLUA_LDFLAGS})
set_target_properties (${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "" COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")