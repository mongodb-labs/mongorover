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
#include "lua-version-compat.h"

int _convert_to_absolute_stack_index(lua_State *L,
                                     int index);

bool _add_lua_table_contents_to_bson_doc(lua_State *L,
                                         bson_t *bson_doc,
                                         int index,
                                         bool generate_id,
                                         int absolute_luaBSONObjects_index,
                                         bson_error_t *error);

bool _iterate_and_add_values_document_or_array_to_table(lua_State *L,
                                                        int index,
                                                        bson_t *bson_doc,
                                                        bson_iter_t *iter,
                                                        bool is_table,
                                                        int absolute_luaBSONObjects_index,
                                                        bson_error_t *error);

/**
 * _convert_to_absolute_stack_index:
 * @L: A lua_State.
 * @index: An int.
 *
 * Converts any stack index to an absolute index. A negative stack index is
 * relative to the top of the stack, so any objects placed on top of the
 * stack changes the relative index.
 *
 * Good explanation here: http://www.lua.org/pil/24.2.3.html. Particularly
 * this: "negative index -x is equivalent to the positive index gettop - x + 1".
 */

int
_convert_to_absolute_stack_index(lua_State *L,
                                 int index)
{
    return index > 0 ? index : lua_gettop(L) + index + 1;
}


/**
 * lua_table_is_array:
 * @L: A lua_State.
 * @index: An int.
 *
 * Takes a table at index on stack and iterates through it. If it only has
 * ascending number indexes starting at 1, then it is an array. If not, it is
 * a table.
 */

bool
lua_table_is_array(lua_State *L,
                   int index)
{
    int absolute_stack_index = _convert_to_absolute_stack_index(L, index);
    return lua_array_length(L, absolute_stack_index) > 0;
}

int
lua_array_length(lua_State *L,
                 int index)
{

    int num_keys;
    int absolute_stack_index = _convert_to_absolute_stack_index(L, index);

    lua_pushnil(L);

    for (num_keys = 0; lua_next(L, absolute_stack_index) != 0; num_keys++) {
        if ((lua_type(L, -2)) != LUA_TNUMBER) {
            lua_pop(L, 2);
            return false;
        }
        int index = lua_tonumber(L, -2);
        lua_pop(L, 1);
        if (index != num_keys + 1) {
            return -1;
        }
    }

    return num_keys;
}

/**
 * append_stack_value_to_bson_doc:
 * @L: A lua_State.
 * @bson_doc: A bson_t.
 * @key: A const char *.
 * @index: An int.
 * @absolute_luaBSONObjects_index: An int.
 * @error: A bson_error_t.
 *
 * This function takes the value at the given stack index and converts it
 * to its appropriate bson form and adds it to the bson_doc.
 *
 * Returns false if error occurred. Error propagated through the bson_error_t.
 */

bool
append_stack_value_to_bson_doc(lua_State *L,
                               bson_t *bson_doc,
                               const char *key,
                               int index,
                               int absolute_luaBSONObjects_index,
                               bson_error_t *error)
{
    int absolute_stack_index = _convert_to_absolute_stack_index(L, index);
    switch (lua_type(L, absolute_stack_index)) {
        case LUA_TBOOLEAN: {
            bool bool_value = lua_toboolean(L, absolute_stack_index);
            bson_append_bool(bson_doc, key, -1, bool_value);
            break;
        };
        case LUA_TNUMBER: {
            lua_Number double_value = lua_tonumber(L, absolute_stack_index);
#if LUA_VERSION_NUM >= 503
            if (double_value == (int)double_value) {
                // unless specifically compiled for 32 bit, 64 bit is standard
                bson_append_int64(bson_doc, key, -1, (int) double_value);
            } else {
                bson_append_double(bson_doc, key, -1, double_value);
            }
#else
            bson_append_double(bson_doc, key, -1, double_value);
#endif
            break;
        };
        case LUA_TSTRING: {
            char *value;
            value = lua_tostring(L, absolute_stack_index);
            bson_append_utf8(bson_doc, key, -1, value, -1);
            break;
        };
        case LUA_TTABLE: {
            if (is_ObjectId(L, absolute_stack_index, absolute_luaBSONObjects_index)) {
                const char *object_id_key;
                bson_oid_t oid;

                lua_getfield(L, absolute_stack_index, "getKey");
                if (lua_isfunction(L, -1)) {

                    // Copy ObjectId on the stack as an input so we can use it as "self".
                    lua_pushvalue(L, absolute_stack_index);
                    if (lua_pcall(L, 1, 1, 0) != 0) {
                        strncpy (error->message,
                                 lua_tostring(L, -1),
                                 sizeof(error->message));
                        // Maintain stack integrity.
                        lua_pop(L, 1);
                        return false;
                    }

                    object_id_key = luaL_checkstring(L, -1);

                    // Pop off the returned string.
                    lua_pop(L, 1);

                    bson_oid_init_from_string(&oid, object_id_key);
                    bson_append_oid(bson_doc, key, -1, &oid);

                } else {
                    luaL_error(L, "ObjectId does not have method getKey");
                }
            } else if (is_BSONNull(L, absolute_stack_index, absolute_luaBSONObjects_index)) {
                bson_append_null(bson_doc, key, -1);
            } else {
                bson_t subdocument;
                if (lua_table_is_array(L, absolute_stack_index)) {
                    bson_append_array_begin(bson_doc, key, -1, &subdocument);
                    if (!(lua_table_to_bson(L, &subdocument, absolute_stack_index, false,
                                            absolute_luaBSONObjects_index, error))) {
                        return false;
                    }
                    bson_append_array_end(bson_doc, &subdocument);
                } else {
                    bson_append_document_begin(bson_doc, key, -1, &subdocument);
                    if (!(lua_table_to_bson(L, &subdocument, absolute_stack_index, false,
                                            absolute_luaBSONObjects_index, error))) {
                        return false;
                    }
                    bson_append_document_end(bson_doc, &subdocument);
                }
            }
            break;
        };
        default: {
            bson_snprintf(error->message, sizeof(error->message),
                          "invalid value type: %s",
                          lua_typename(L, lua_type(L, absolute_stack_index)));
            return false;
        }
    }

    return true;
}


/**
 * find_and_set_or_create_id:
 * @L: A lua_State.
 * @index: An int.
 * @bson_doc: A bson_t.
 * @absolute_luaBSONObjects_index: An int.
 * @error: A bson_error_t.
 *
 * Takes a table at given index and checks whether it has an _id field in it.
 * It will append the _id to the bson document given and also to the table on
 * the stack. If no _id exists, it will create one.
 */

bool
find_and_set_or_create_id(lua_State *L,
                          int index,
                          bson_t *bson_doc,
                          int absolute_luaBSONObjects_index,
                          bson_error_t *error)
{
    bson_oid_t oid;
    char str[25];
    int absolute_stack_index = _convert_to_absolute_stack_index(L, index);

    if (!(bson_empty(bson_doc))) {
        luaL_error(L, "bson document not empty. _id needs to be appended "
            "first");
    }

    lua_pushstring(L, "_id");
    lua_gettable(L, absolute_stack_index);
    if (!lua_isnil(L, -1)) {
        if (!(append_stack_value_to_bson_doc(L, bson_doc, "_id", -1,
                                             absolute_luaBSONObjects_index, error)))
        {
            lua_pop(L, 1);
            return false;
        }
        lua_pop(L, 1);
    } else {
        // Lingering nil value on the stack from the lua_gettable( ... ) call.
        lua_pop(L, 1);
        bson_oid_init(&oid, NULL);
        BSON_APPEND_OID (bson_doc, "_id", &oid);
        bson_oid_to_string(&oid, str);
        if (!(generate_ObjectID(L, str, absolute_luaBSONObjects_index, error))) {
            return false;
        }
        lua_setfield(L, absolute_stack_index, "_id");
    }

    return true;
}


/**
 * lua_table_to_bson:
 * @L: A lua_State.
 * @bson_doc: A bson_t.
 * @index: An int.
 * @_id_required: A boolean.
 * @absolute_luaBSONObjects_index: An int.
 * @error: A bson_error_t.
 *
 * Converts a lua table to bson. Pre-validation function for
 * _add_lua_table_contents_to_bson_doc( ... ) by checking that the bson document
 * passed in is empty.
 */
bool
lua_table_to_bson(lua_State *L,
                  bson_t *bson_doc,
                  int index,
                  bool _id_required,
                  int absolute_luaBSONObjects_index,
                  bson_error_t *error)
{
    if (!(bson_empty(bson_doc))) {
        strncpy(error->message,
                "bson_document passed to lua_table_to_bson is not empty. To "
                    "append a table's contents to a bson document, use "
                    "add_lua_table_contents_to_bson_doc",
                sizeof(error->message));
        return false;
    }

    if (!(_add_lua_table_contents_to_bson_doc(L, bson_doc, index, _id_required,
                                              absolute_luaBSONObjects_index, error)))
    {
        return false;
    }

    return true;
}


/**
 * add_lua_table_contents_to_bson_doc:
 * @L: A lua_State.
 * @bson_doc: A bson_t.
 * @index: An int.
 * @_id_required: A boolean.
 * @absolute_luaBSONObjects_index: An int.
 * @error: A bson_error_t.
 *
 * Appends lua table contents to bson document, regardless of it the bson
 * document is empty or not. Checks that the bson document is valid before
 * returning.
 */
bool
add_lua_table_contents_to_bson_doc(lua_State *L,
                                   bson_t *bson_doc,
                                   int index,
                                   bool _id_required,
                                   int absolute_luaBSONObjects_index,
                                   bson_error_t *error)
{
    size_t offset;

    if (!(_add_lua_table_contents_to_bson_doc(L, bson_doc, index, _id_required,
                                              absolute_luaBSONObjects_index, error)))
    {
        return false;
    }

    if (!(bson_validate(bson_doc, BSON_VALIDATE_NONE, &offset))) {
        strncpy(error->message,
                "adding lua table to bson document caused malformed bson "
                    "document",
                sizeof(error->message));
        return false;
    }

    return true;
}


/**
 * _add_lua_table_contents_to_bson_doc:
 * @L: A lua_State.
 * @bson_doc: A bson_t.
 * @index: An int.
 * @_id_required: A boolean.
 * @absolute_luaBSONObjects_index: An int.
 * @error: A bson_error_t.
 *
 * Takes lua table at the very top of the stack and iterates through it,
 * converting its contents into the bson_doc.
 *
 * If _id_required is true, an _id with ObjectId will be created at the
 * beginning of the object if it is not present already in the document. It
 * will be skipped when it is iterated over again while going through all the
 * keys in the lua table.
 *
 * Returns false if error occurred. Error propagated through the bson_error_t.
 */

bool
_add_lua_table_contents_to_bson_doc(lua_State *L,
                                    bson_t *bson_doc,
                                    int index,
                                    bool _id_required,
                                    int absolute_luaBSONObjects_index,
                                    bson_error_t *error)
{
    // index: relative or absolute position of table on the stack being
    // converted
    // absolute_stack_index: index converted to an absolute stack index
    // array_index: Potentially altered lua index (since lua indexes start at 1)

    int absolute_stack_index = _convert_to_absolute_stack_index(L, index);
    bool is_array;
    int number_key;

    if (!lua_istable(L, absolute_stack_index)) {
        bson_snprintf(error->message, sizeof(error->message),
                      "value at index %d is not a table", index);
        return false;
    }

    if (!(lua_checkstack(L, 3))) {
        strncpy(error->message,
                "too many levels of embedded arrays, would cause stack "
                    "overflow in C API",
                sizeof(error->message));

        return false;
    }

    if (_id_required) {
        if (!(find_and_set_or_create_id(L, absolute_stack_index, bson_doc,
                                        absolute_luaBSONObjects_index, error)))
        {
            return false;
        }
    }
    is_array = lua_table_is_array(L, absolute_stack_index);

    lua_pushnil(L);
    while (lua_next(L, absolute_stack_index) != 0) {
        bool number_string_as_index = false;
        switch (lua_type(L, -2)) {

            case LUA_TNUMBER:
                // Occurs when a string containing a number is used as a key
                // in document opposed to index in an array.
                number_string_as_index = true;

                // FALL THROUGH

            case LUA_TSTRING: {
                const char *key;

                if (!number_string_as_index) {
                    key = lua_tostring(L, -2);
                } else {
                    number_key = lua_tonumber(L, -2);

                    // Lua indices start at 1, so decrement by one if it is
                    // an array.
                    if (is_array) {
                        number_key--;
                    }

                    char buffer[100];
                    sprintf(buffer, "%d", (int) number_key);
                    key = buffer;
                }

                if (_id_required) {
                    // _id was already appended to the beginning of the
                    // document, as per MongoDB specification.
                    if (strcmp(key, "_id") == 0) {
                        lua_pop(L, 1);
                        continue;
                    }
                }

                if (!(append_stack_value_to_bson_doc(L, bson_doc, key, -1,
                                                     absolute_luaBSONObjects_index, error)))
                {
                    lua_pop(L, 1);
                    return false;
                }

                break;
            }
            default: {
                bson_snprintf(error->message, sizeof(error->message),
                              "invalid key type: %s",
                              lua_typename(L, lua_type(L, -2)));
                return false;
            }
        }
        lua_pop(L, 1);
    }

    return true;
}


/**
 * bson_is_array:
 * @L: A lua_State.
 * @iter: A bson_iter_t.
 *
 * Takes in bson_iter_t pointing at the head of a document and traverses it's
 * keys to determine whether the bson object is an array or a document.
 * Returns true if it is an array, returns false if it is not.
 */

bool
bson_is_array(bson_iter_t iter)
{
    int i;
    char *end;
    long ret;
    for (i = 0; bson_iter_next(&iter); i++) {
        const char *key = bson_iter_key(&iter);
        ret = strtol(key, &end, 10);
        if (ret != i || *end) {
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
 * @absolute_luaBSONObjects_index: An int.
 * @error: A bson_error_t.
 *
 * Takes in iter pointing at a document, subdocument, or subarray. Depending on
 * if it is a subarray or a document it will call
 * _iterate_and_add_values_document_or_array_to_table( ... ) to push the values
 * on the stack accordingly.
 *
 * Returns false if error occurred. Error propagated through bson_error_t.
 */

bool
bson_subdocument_or_subarray_to_table(lua_State *L,
                                      bson_t *bson_doc,
                                      bson_iter_t *iter,
                                      int absolute_luaBSONObjects_index,
                                      bson_error_t *error)
{

    bson_iter_t child;
    bool is_array;
    lua_newtable (L);

    if (bson_iter_recurse(iter, &child)) {
        is_array = bson_is_array(child);

        if (!(_iterate_and_add_values_document_or_array_to_table(L, -1, bson_doc,
                                                                 &child, !is_array,
                                                                 absolute_luaBSONObjects_index, &error)))
        {
            return false;
        }
    }

    return true;
}


/**
 * _iterate_and_add_values_document_or_array_to_table:
 * @L: A lua_State.
 * @index: Index of table on stack.
 * @bson_doc: BSON document that is being parsed.
 * @iter: A bson_iter_t.
 * @is_table: Whether the bson_doc passed in is a table or an array
 * @absolute_luaBSONObjects_index: An int.
 * @error: A bson_error_t.
 *
 * This will iterate through a bson document and add its values to a table that
 * is pre-existing on the top of the stack. If it is an "array" it will be a
 * lua table with number indices beginning at index 1.
 *
 * Returns false if an error occurred. Error propagated through bson_error_t.
 */

bool
_iterate_and_add_values_document_or_array_to_table(lua_State *L,
                                                   int index,
                                                   bson_t *bson_doc,
                                                   bson_iter_t *iter,
                                                   bool is_table,
                                                   int absolute_luaBSONObjects_index,
                                                   bson_error_t *error)
{
    int absolute_stack_index = _convert_to_absolute_stack_index(L, index);
    int i;
    for (i = 0; bson_iter_next(iter); i++) {
        const char *key;
        const bson_value_t *value;

        key = bson_iter_key(iter);
        value = bson_iter_value(iter);
        switch (value->value_type) {
            case BSON_TYPE_DOUBLE: {
                lua_Number val = value->value.v_double;
                if (is_table) {
                    lua_pushnumber(L, val);
                    lua_setfield(L, absolute_stack_index, key);
                } else {
                    lua_pushnumber(L, val);
                    // Lua indices start at 1.
                    lua_rawseti(L, absolute_stack_index, i + 1);
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
                    if (!(bson_subdocument_or_subarray_to_table(L, bson_doc, iter,
                                                                absolute_luaBSONObjects_index, error)))
                    {
                        lua_pop(L, 1);
                        return false;
                    }

                    lua_setfield(L, absolute_stack_index, key);
                } else {
                    if (!(bson_subdocument_or_subarray_to_table(L, bson_doc, iter,
                                                                absolute_luaBSONObjects_index, error)))
                    {
                        lua_pop(L, 1);
                        return false;
                    }
                    lua_rawseti(L, absolute_stack_index, i + 1);
                }
            };
            case BSON_TYPE_ARRAY: {
                if (is_table) {
                    if (!(bson_subdocument_or_subarray_to_table(L, bson_doc, iter,
                                                                absolute_luaBSONObjects_index, error)))
                    {
                        lua_pop(L, 1);
                        return false;
                    }
                    lua_setfield(L, absolute_stack_index, key);
                } else {
                    if (!(bson_subdocument_or_subarray_to_table(L, bson_doc, iter,
                                                                absolute_luaBSONObjects_index, error)))
                    {
                        lua_pop(L, 1);
                        return false;
                    }
                    lua_rawseti(L, absolute_stack_index, i + 1);
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
                char oid_str[25];
                bson_oid_to_string(value->value.v_oid.bytes, oid_str);

                if (!(generate_ObjectID(L, oid_str, absolute_luaBSONObjects_index, error))) {
                    return false;
                }

                lua_setfield(L, absolute_stack_index, key);
                break;
            };
            case BSON_TYPE_BOOL: {
                if (is_table) {
                    lua_pushboolean(L, value->value.v_bool);
                    lua_setfield(L, absolute_stack_index, key);
                } else {
                    lua_pushboolean(L, value->value.v_bool);
                    lua_rawseti(L, absolute_stack_index, i + 1);
                }
                break;
            };
            case BSON_TYPE_DATE_TIME:
                luaL_error(L, "BSON_TYPE_DATE_TIME not supported yet");
                break;
            case BSON_TYPE_NULL: {
                if (!(generate_BSONNull(L, absolute_luaBSONObjects_index, error))) {
                    return false;
                }
                lua_setfield(L, absolute_stack_index, key);
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
                lua_pushinteger_compat(L, value->value.v_int32);

                if (is_table) {
                    lua_setfield(L, absolute_stack_index, key);
                } else {
                    lua_rawseti(L, absolute_stack_index, i + 1);
                }
                break;
            };
            case BSON_TYPE_TIMESTAMP:
                luaL_error(L, "BSON_TYPE_TIMESTAMP not supported yet\n");
                break;
            case BSON_TYPE_INT64: {
                lua_pushinteger_compat(L, value->value.v_int64);

                if (is_table) {
                    lua_setfield(L, absolute_stack_index, key);
                } else {
                    lua_rawseti(L, absolute_stack_index, i + 1);
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

    return true;
}


/**
 * bson_document_or_array_to_table:
 * @L: A lua_State.
 * @bson_doc: A bson_t.
 * @is_table: A bool.
 * @absolute_luaBSONObjects_index: An int.
 * @error: a bson_error_t.
 *
 * Converts bson document into a lua table and places it on the top of the
 * stack.
 *
 * Returns false if an error occurred. Error propagated through bson_error_t.
 *
 */

bool
bson_document_or_array_to_table(lua_State *L,
                                bson_t *bson_doc,
                                bool is_table,
                                int absolute_luaBSONObjects_index,
                                bson_error_t *error)
{
    bson_iter_t iter;
    lua_newtable (L);
    if (bson_iter_init(&iter, bson_doc)) {
        if (!(_iterate_and_add_values_document_or_array_to_table(L, -1, bson_doc,
                                                                 &iter, is_table,
                                                                 absolute_luaBSONObjects_index,
                                                                 error)))
        {
            return false;
        }
    }

    return true;
}
