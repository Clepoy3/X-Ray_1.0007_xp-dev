////////////////////////////////////////////////////////////////////////////
//	Module 		: lua_tools.h
//	Created 	: 29.07.2014
//  Modified 	: 29.07.2014
//	Author		: Alexander Petrov
//	Description : Lua functionality extension
////////////////////////////////////////////////////////////////////////////
#pragma once
#include "stdafx.h"
#include "ai_script_space.h"

ENGINE_API const char* get_lua_traceback(lua_State *L, int);
ENGINE_API extern lua_State* g_game_lua;
