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

#include "lua-bson.h"

void _append_to_bson_doc(lua_State *L,
                        bson_t *bson_doc,
                        const char *key);

void _iterate_and_add_values_document_or_array_to_table(lua_State *L,
                                                       bson_t *bson_doc,
                                                       bson_iter_t *iter,
                                                       bool is_table);


/**
 * lua_table_is_array
 * @L: A lua_State.
 * @index: An int.
 *
 * Takes a table at indice on stack and iterates through it. If it only has
 * ascending number indices starting at 1, then it is an array. If not, it is
 * a table.
 */

bool
lua_table_is_array(lua_State *L, int index)
{

    int num_keys;

    if (index != -1 && lua_gettop(L) != index) {
        lua_pushvalue(L, index);
    }

    // Iterate through keys and check if they are all numbers.
    lua_pushnil(L);
    for (num_keys = 0; lua_next(L, -2) != 0; num_keys++) {
        if ((lua_type(L, -2)) != LUA_TNUMBER) {
            lua_pop(L, 2);
            return false;
        }
        lua_pop(L, 1);
    }

    // Empty table defaults to an empty table instead of empty array.
    if (num_keys == 0) {
        return false;
    }

    // Iterate through like ipairs, and make sure the indices are in ascending
    // order and there are no gaps.
    for (int i=1 ; i < num_keys; i++) {
        lua_rawgeti(L,-1,i);

        // If the index does not exist, it will cause lua_isnil to return nil.
        if ( lua_isnil(L,-1) ) {
            lua_pop(L,1);
            return false;
        }

        lua_pop(L,1);
    }

    if (index != -1 && lua_gettop(L) != index) {
        lua_pop(L, 1);
    }

    return true;
}


/**
 * _append_to_bson_doc
 * @L: A lua_State.
 * @bson_doc: A bson_t.
 * @key: A const char *.
 *
 * While iterating through a table, key and value will be at the top of the
 * stack. This function takes the top value off of the stack, which should be
 * the value and converts it to its appropriate bson form and adds it to the
 * bson_doc.
 */

void
_append_to_bson_doc (lua_State *L,
                    bson_t *bson_doc,
                    const char *key)
{
    switch (lua_type(L, -1)) {
        case LUA_TBOOLEAN: {
            bool bool_value = lua_toboolean(L, -1);
            bson_append_bool(bson_doc, key, -1, bool_value);
            break;
        };
        case LUA_TNUMBER: {
            lua_Number double_value = lua_tonumber(L, -1);
            bson_append_double(bson_doc, key, -1, double_value);
            break;
        };
        case LUA_TSTRING: {
            const char *value;
            value = lua_tostring(L, -1);
            bson_append_utf8(bson_doc, key, -1, value, -1);
            break;
        };
        case LUA_TTABLE: {

            if (is_ObjectId(L)) {
                const char *object_id_key;
                bson_oid_t oid;

                lua_getfield( L, -1, "getKey");
                if (lua_isfunction(L, -1)) {

                    // Copy ObjectId on the stack as an input so we can use it as "self".
                    lua_pushvalue(L, -2);
                    if (lua_pcall(L, 1, 1, 0) != 0) {
                        luaL_error(L, "error running function `f': %s", lua_tostring(L, -1));
                    }

                    object_id_key = luaL_checkstring(L, -1);
                    if (object_id_key == NULL) {
                        luaL_error(L, "ObjectId:getKey() did not return a string");
                    }

                    // Pop off the returned string.
                    lua_pop(L, 1);

                    bson_oid_init_from_string (&oid, object_id_key);
                    bson_append_oid(bson_doc, key, -1, &oid);

                } else {
                    luaL_error(L, "ObjectId does not have method getKey");
                }
            } else if (is_BSONNull(L)) {
                bson_append_null(bson_doc, key, -1);
            } else {

                bson_t subdocument;
                if (lua_table_is_array(L, -1)) {
                    bson_append_array_begin(bson_doc, key, -1, &subdocument);
                    lua_table_to_bson(L, &subdocument, -1, false);
                    bson_append_array_end(bson_doc, &subdocument);
                } else {
                    bson_append_document_begin(bson_doc, key, -1, &subdocument);
                    lua_table_to_bson(L, &subdocument, -1, false);
                    bson_append_document_end(bson_doc, &subdocument);
                }
            }
            break;
        };
        default: {
            luaL_error(L, "invalid value type: %s", lua_typename(L, lua_type(L, -2)));
        }
    }
}


/**
 * lua_table_contains_id_field:
 * @L: A lua_State.
 * @index: An int.
 * @bson_doc: A bson_t.
 *
 * Takes a table at given index and returns whether table["_id"] exists or not.
 * If it exists, it will be appended to the bson document.
 */

bool
append_id_to_bson_doc_if_id_exists(lua_State *L,
                                   int index,
                                   bson_t *bson_doc)
{
    bool contains_id;

    int stack_size = lua_gettop(L);
    if (index != -1 && stack_size != index) {
        lua_pushvalue(L, index);
    }
    lua_pushstring(L, "_id");
    lua_gettable(L, -2);

    contains_id = !lua_isnil(L, -1);

    if (contains_id) {
        _append_to_bson_doc(L, bson_doc, "_id");
    }

    lua_pop(L, 1);

    if (index != -1 && stack_size != index) {
        lua_pop(L, 1);
    }

    return contains_id;
}


/**
 * lua_table_to_bson
 * @L: A lua_State.
 * @bson_doc: A bson_t.
 * @_id_required: A boolean.
 * @id_field_in_doc: A boolean
 *
 * Takes lua table at the very top of the stack and iterates through it,
 * converting its contents into the bson_doc.
 *
 * If _id_required is true, an _id with ObjectId will be created at the
 * beginning of the object if it is not present already in the document.
 */

void
lua_table_to_bson(lua_State *L,
                  bson_t *bson_doc,
                  int index,
                  bool _id_required)
{
    if(!lua_istable(L, index)) {
        luaL_error(L, "value at index %d is not a table", index);
    }

    if (!(lua_checkstack (L, 3))) {
        luaL_error(L, "too many levels of embedded arrays,"
                "would cause stack overflow in C API");
    }

    if (index != -1 && lua_gettop(L) != index) {
        lua_pushvalue(L, index);
    }



    if (_id_required && !(append_id_to_bson_doc_if_id_exists(L, -1, bson_doc))) {
        bson_iter_t iter;
        if (!bson_iter_init_find (&iter, bson_doc, "_id")) {
            bson_oid_t oid;
            bson_oid_init (&oid, NULL);
            BSON_APPEND_OID (bson_doc, "_id", &oid);
        }
    }

    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        bool number_string_as_index = false;
        switch (lua_type(L, -2)) {

            case LUA_TNUMBER:
                // Occurs when a string containing a number is used as a key
                // in document opposed to index in an array.
                number_string_as_index = true;
            case LUA_TSTRING: {
                const char *key;

                if (!number_string_as_index) {
                    key = lua_tostring(L, -2);
                } else {
                    // Function tostring converts the value on the stack.
                    // Need to make copy, convert it, then pop to allow lua_next
                    // to iterate correctly.
                    lua_pushvalue(L, -2);
                    key = lua_tostring(L, -1);
                    lua_pop(L, 1);
                }

                if (_id_required) {
                    if (strcmp(key, "_id") == 0){
                        lua_pop(L, 1);
                        continue;
                    }
                }

                _append_to_bson_doc(L, bson_doc, key);
                break;
            }
            default: {
                luaL_error(L,"invalid key type: %s", lua_typename(L, lua_type(L, -2)));
                break;
            }
        }
        lua_pop(L, 1);
    }

    if (index != -1 && lua_gettop(L) != index) {
        lua_pop(L, 1);
    }
}


/**
 * bson_is_array
 * @L: A lua_State.
 * @iter: A bson_iter_t.
 *
 * Takes in bson_iter_t pointing at the head of a document and traverses it's
 * keys to determine whether the bson object is an array or a document.
 * Returns true if it is an array, returns false if it is not.
 */

bool
bson_is_array(lua_State *L, bson_iter_t iter)
{
    for (int i=0; bson_iter_next(&iter); i++) {
        char *key = bson_iter_key(&iter);
        long ret = strtol(key, NULL, 10);
        if (ret != i) {
            return false;
        }
    }
    return true;
}


/**
 * bson_subdocument_or_subarray_to_table:
 * @L: A lua_State.
 * @bson_doc: BSON document that is being converted to a lua table.
 * @iter: A bson_iter_t.
 *
 * Takes in iter pointing at a document, subdocument, or subarray. Depending on
 * if it is a subarray or a document it will call
 * _iterate_and_add_values_document_or_array_to_table( ... ) to push the values
 * on the stack accordingly.
 */

void
bson_subdocument_or_subarray_to_table(lua_State *L,
                                      bson_t *bson_doc,
                                      bson_iter_t *iter)
{

    bson_iter_t child;
    bool is_array;
    lua_newtable (L);

    if (bson_iter_recurse (iter, &child)) {
        is_array = bson_is_array(L, child);
        _iterate_and_add_values_document_or_array_to_table(
                L, bson_doc, &child, !is_array);
    }
}


/**
 * _iterate_and_add_values_document_or_array_to_table:
 * @L: A lua_State.
 * @bson_doc: BSON document that is being parsed.
 * @iter: A bson_iter_t.
 * @is_table: Whether the bson_doc passed in is a table or an array
 *
 * This will iterate through a bson document and add its values to a table that
 * is pre-existing on the top of the stack. If it is an "array" it will be a
 * lua table with number indices beginning at index 1.
 */

void
_iterate_and_add_values_document_or_array_to_table(lua_State *L,
                                                  bson_t *bson_doc,
                                                  bson_iter_t *iter,
                                                  bool is_table)
{

    for (int i=0; bson_iter_next(iter); i++) {
        const char *key;
        const bson_value_t *value;

        key = bson_iter_key(iter);
        value = bson_iter_value (iter);
        switch (value->value_type) {
            case BSON_TYPE_DOUBLE: {
                lua_Number val = value->value.v_double;
                if (is_table) {
                    lua_pushnumber(L, val);
                    lua_setfield(L, -2, key);
                } else {
                    lua_pushnumber(L, val);
                    // Lua indices start at 1.
                    lua_rawseti(L, -2, i + 1);
                }
                break;
            };
            case BSON_TYPE_UTF8: {
                char *val = value->value.v_utf8.str;
                if (is_table) {
                    lua_pushstring(L, val);
                    lua_setfield(L, -2, key);
                } else {
                    lua_pushstring(L, val);
                    lua_rawseti(L, -2, i + 1);
                }
                break;
            };
            case BSON_TYPE_DOCUMENT: {
                if (is_table) {
                    bson_subdocument_or_subarray_to_table(L, bson_doc, iter);
                    lua_setfield(L, -2, key);
                } else {
                    bson_subdocument_or_subarray_to_table(L, bson_doc, iter);
                    lua_rawseti(L, -2, i + 1);
                }
            };
            case BSON_TYPE_ARRAY: {
                if (is_table) {
                    bson_subdocument_or_subarray_to_table(L, bson_doc, iter);
                    lua_setfield(L, -2, key);
                } else {
                    bson_subdocument_or_subarray_to_table(L, bson_doc, iter);
                    lua_rawseti(L, -2, i + 1);
                }
                break;
            };
            case BSON_TYPE_BINARY:
                luaL_error(L, "BSON_TYPE_BINARY not supported yet");
                break;
            case BSON_TYPE_UNDEFINED:
                luaL_error(L, "BSON_TYPE_UNDEFINED not supported yet");
                break;
            case BSON_TYPE_OID: {
                char str[25];
                bson_oid_to_string (value->value.v_oid.bytes, str);
                generate_ObjectID(L, str);
                lua_setfield(L, -2, key);
                break;
            };
            case BSON_TYPE_BOOL: {
                if (is_table) {
                    lua_pushboolean(L, value->value.v_bool);
                    lua_setfield(L, -2, key);
                } else {
                    lua_pushboolean(L, value->value.v_bool);
                    lua_rawseti(L,-2,i + 1);
                }
                break;
            };
            case BSON_TYPE_DATE_TIME:
                luaL_error(L, "BSON_TYPE_DATE_TIME not supported yet");
                break;
            case BSON_TYPE_NULL: {
                generate_BSONNull(L);
                lua_setfield(L, -2, key);
                break;
            };
            case BSON_TYPE_REGEX:
                luaL_error(L, "BSON_TYPE_DATE_TIME not supported yet");
                break;
            case BSON_TYPE_DBPOINTER:
                luaL_error(L, "BSON_TYPE_DBPOINTER not supported yet");
                break;
            case BSON_TYPE_CODE:
                luaL_error(L, "BSON_TYPE_CODE not supported yet");
                break;
            case BSON_TYPE_SYMBOL:
                luaL_error(L, "BSON_TYPE_SYMBOL not supported yet");
                break;
            case BSON_TYPE_CODEWSCOPE:
                luaL_error(L, "BSON_TYPE_CODEWSCOPE not supported yet");
                break;
            case BSON_TYPE_INT32: {
                lua_Number val = value->value.v_int32;
                if (is_table) {
                    lua_pushnumber(L, val);
                    lua_setfield(L, -2, key);
                } else {
                    lua_pushnumber(L, val);
                    lua_rawseti(L, -2, i + 1);
                }
                break;
            };
            case BSON_TYPE_TIMESTAMP:
                luaL_error(L, "not supported yet2\n");
                break;
            case BSON_TYPE_INT64: {
                lua_Number val = value->value.v_int64;
                if (is_table) {
                    lua_pushnumber(L, val);
                    lua_setfield(L, -2, key);
                } else {
                    lua_pushnumber(L, val);
                    lua_rawseti(L, -2, i + 1);
                }
                break;
            };
            case BSON_TYPE_MAXKEY: {
                luaL_error(L, "BSON_TYPE_MAXKEY not supported yet\n");
                break;
            };
            case BSON_TYPE_MINKEY: {
                luaL_error(L, "BSON_TYPE_MINKEY not supported yet\n");
                break;
            };
            case BSON_TYPE_EOD: {
                luaL_error(L, "BSON_TYPE_EOD not supported yet\n");
                break;
            };
            default:
                break;
        }
    }
}


/**
 * bson_document_or_array_to_table:
 * @L: A lua_State.
 * @cursor: a mongoc_cursor_t.
 * @bson_doc: A bson_t.
 * @field: A char *.
 *
 * Returns field in BSON document at specific field, and places it at the
 * top of the stack.
 */

void
bson_document_or_array_to_table (lua_State *L,
                                 mongoc_cursor_t *cursor,
                                 bson_t *bson_doc,
                                 bool is_table)
{
    bson_iter_t iter;
    lua_newtable (L);
    if (bson_iter_init (&iter, bson_doc)) {
        _iterate_and_add_values_document_or_array_to_table(L, bson_doc, &iter, is_table);
    }
}


/**
 * lua_place_bson_field_value_on_top_of_stack:
 * @L: A lua_State.
 * @bson_doc: A bson_t.
 * @field: A char *.
 *
 * Returns field in BSON document at specific field, and places it at the
 * top of the stack.
 */

void
lua_place_bson_field_value_on_top_of_stack(lua_State *L,
                                           bson_t *bson_doc,
                                           char *field) {
    bson_iter_t find_id_iter;
    if (bson_iter_init (&find_id_iter, bson_doc) &&
        bson_iter_find (&find_id_iter, field)) {
        const bson_value_t *value;

        value = bson_iter_value (&find_id_iter);
        switch (value->value_type) {
            case BSON_TYPE_DOUBLE: {
                lua_Number val = value->value.v_double;
                lua_pushnumber(L, val);
                break;
            };
            case BSON_TYPE_UTF8: {
                lua_pushstring(L, value->value.v_utf8.str);
                break;
            };
            case BSON_TYPE_DOCUMENT: {
                luaL_error(L, "_id cannot be a document");
                break;
            };
            case BSON_TYPE_ARRAY: {
                luaL_error(L, "_id cannot be a table");
                break;
            };
            case BSON_TYPE_BINARY:
                luaL_error(L, "BSON_TYPE_BINARY not supported yet");
                break;
            case BSON_TYPE_UNDEFINED:
                luaL_error(L, "BSON_TYPE_UNDEFINED not supported yet");
                break;
            case BSON_TYPE_OID: {
                char oid_str[25];
                bson_oid_to_string (value->value.v_oid.bytes, oid_str);
                generate_ObjectID(L, oid_str);
                break;
            };
            case BSON_TYPE_BOOL: {
                lua_pushboolean(L, value->value.v_bool);
                break;
            };
            case BSON_TYPE_DATE_TIME:
                luaL_error(L, "BSON_TYPE_DATE_TIME not supported yet");
                break;
            case BSON_TYPE_NULL: {
                generate_BSONNull(L);
                break;
            };
            case BSON_TYPE_REGEX:
                luaL_error(L, "BSON_TYPE_DATE_TIME not supported yet");
                break;
            case BSON_TYPE_DBPOINTER:
                luaL_error(L, "BSON_TYPE_DBPOINTER not supported yet");
                break;
            case BSON_TYPE_CODE:
                luaL_error(L, "BSON_TYPE_CODE not supported yet");
                break;
            case BSON_TYPE_SYMBOL:
                luaL_error(L, "BSON_TYPE_SYMBOL not supported yet");
                break;
            case BSON_TYPE_CODEWSCOPE:
                luaL_error(L, "BSON_TYPE_CODEWSCOPE not supported yet");
                break;
            case BSON_TYPE_INT32: {
                lua_pushnumber(L, value->value.v_int32);
                break;
            };
            case BSON_TYPE_TIMESTAMP:
                luaL_error(L, "not supported yet2\n");
                break;
            case BSON_TYPE_INT64: {
                lua_pushnumber(L, value->value.v_int64);
                break;
            };
            case BSON_TYPE_MAXKEY: {
                luaL_error(L, "BSON_TYPE_MAXKEY not supported yet\n");
                break;
            };
            case BSON_TYPE_MINKEY: {
                luaL_error(L, "BSON_TYPE_MINKEY not supported yet\n");
                break;
            };
            case BSON_TYPE_EOD: {
                luaL_error(L, "BSON_TYPE_EOD not supported yet\n");
                break;
            };
            default:
                break;
        }
    }
}