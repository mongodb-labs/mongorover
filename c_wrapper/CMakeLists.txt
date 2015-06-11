cmake_minimum_required(VERSION 3.2)
project(mongo_module C)
set(CMAKE_BUILD_TYPE Release)

INCLUDE(FindPkgConfig)
pkg_check_modules (LIBMONGOC REQUIRED libmongoc-1.0)

option(FORCE_LUA_5_1 "FORCE_LUA_5_1" OFF)

set(LIBLUA_LDFLAGS lua)
find_package(Lua)
if (NOT LUA_FOUND OR ${FORCE_LUA_5_1})
    find_package(Lua51 REQUIRED)
    set(LIBLUA_LDFLAGS lua.5.1)
    if (NOT LUA_FOUND)
        message( FATAL_ERROR "Lua 5.2 or Lua 5.1 is not installed." )
    endif()
endif()

include_directories(
    ${LIBMONGOC_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIR}
)
MESSAGE(${LUA_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -g")

set(SOURCE_FILES mongo-module.c lua-mongoc-client.c lua-mongoc-database.c lua-mongoc-collection.c)

add_library(mongo_module SHARED ${SOURCE_FILES} lua-version-compat.h lua-version-compat.c)# lua-mongoc-client.c lua-mongoc-database.c)

target_link_libraries(${CMAKE_PROJECT_NAME} ${LIBMONGOC_LDFLAGS} ${LIBLUA_LDFLAGS})
set_target_properties (${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")

