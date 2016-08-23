--[[

Copyright 2015 MongoDB, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

--]]


--- Constants for defining cursor behavior.
-- @module mongorover.CursorType

--- Constants for defining cursor behavior.
-- @type mongorover.CursorTypes

local _QUERY_OPTIONS = {
	tailable_cursor = 2,
	slave_okay = 4,
	oplog_replay = 8,
	no_timeout = 16,
	await_data = 32,
	exhaust = 64,
	partial = 128
}

--- Constants for defining cursor behavior.
-- @field NON_TAILABLE The standard cursor type.
-- @field TAILABLE The tailable cursor type. Tailable cursors are only for use
-- with capped collections. They are not closed when the last data is
-- retrieved but are kept open and the cursor location marks the final
-- document position. If more data is received iteration of the cursor will
-- continue from the last document received.
-- @field TAILABLE_AWAIT A tailable cursor with the await option set. Creates a
-- tailable cursor that will wait for a few seconds after returning the full
-- result set so that it can capture and return additional data added during
-- the query.
-- @field EXHAUST An exhaust cursor. MongoDB will stream batched results to the
-- client without waiting for the client to request each batch, reducing
-- latency.
-- @field _QUERY_OPTIONS All query options for for use by
-- @{mongorover.MongoCollection:find}.
-- @table CursorType

local CursorType = {
	NON_TAILABLE = 0,
	TAILABLE = _QUERY_OPTIONS.tailable_cursor,
	-- bitwise support for Lua5.1, 5.2, and 5.3 is a nightmare
	-- TAILABLE_AWAIT = _QUERY_OPTIONS.tailable_cursor | _QUERY_OPTIONS .await_data
	TAILABLE_AWAIT = 34,
	EXHAUST = _QUERY_OPTIONS.exhaust,
	_QUERY_OPTIONS = _QUERY_OPTIONS
}

return CursorType