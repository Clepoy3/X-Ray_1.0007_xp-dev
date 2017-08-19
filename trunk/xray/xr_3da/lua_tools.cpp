#include "stdafx.h"
#include "lua_tools.h"

lua_State* g_game_lua = nullptr;

ENGINE_API const char* get_lua_traceback(lua_State *L, int)
{
	if (L && !g_game_lua)
		g_game_lua = L;

	luaL_traceback(g_game_lua, g_game_lua, nullptr, 0);
	auto tb = lua_tostring(g_game_lua, -1);
	lua_pop(g_game_lua, 1);
	return tb;
}
