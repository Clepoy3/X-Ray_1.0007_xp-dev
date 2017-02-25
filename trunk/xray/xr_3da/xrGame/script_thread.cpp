////////////////////////////////////////////////////////////////////////////
//	Module 		: script_thread.cpp
//	Created 	: 19.09.2003
//  Modified 	: 29.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Script thread class
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_thread.h"

#define LUABIND_HAS_BUGS_WITH_LUA_THREADS //KRodin: надо попробовать с этим дефайном и без него.

const LPCSTR main_function = "console_command_run_string_main_thread_function";

CScriptThread::CScriptThread(LPCSTR caNamespaceName, bool do_string, bool reload) //KRodin: fixed
{
	m_virtual_machine = nullptr;
	m_active = false;
	lua_State *engineLua = ai().script_engine().lua();
	try {
		string256 S;
		if (!do_string)
		{
			m_script_name = caNamespaceName;
			ai().script_engine().process_file(caNamespaceName, reload);
		}
		else
		{
			m_script_name = "console command";
			xr_sprintf(S,"function %s()\n%s\nend\n", main_function, caNamespaceName);
			int l_iErrorCode = luaL_loadbuffer(engineLua, S, xr_strlen(S), "@console_command");
			if (!l_iErrorCode)
			{
				l_iErrorCode = lua_pcall(engineLua, 0, 0, 0);
				if (l_iErrorCode)
				{
					ai().script_engine().print_output(engineLua, *m_script_name, l_iErrorCode);
					return;
				}
			}
			else
			{
				ai().script_engine().print_output(engineLua, *m_script_name, l_iErrorCode);
				return;
			}
		}
		m_virtual_machine = lua_newthread(engineLua);
		VERIFY2(lua(), "Cannot create new Lua thread");		
#ifdef DEBUG
#	ifdef USE_DEBUGGER
		if (scriptEngine.debugger() && scriptEngine.debugger()->Active())
			lua_sethook(lua(), CDbgLuaHelper::hookLua, LUA_MASKLINE|LUA_MASKCALL|LUA_MASKRET, 0);
		else
#	endif
			lua_sethook(lua(), ai().script_engine().lua_hook_call, LUA_MASKLINE|LUA_MASKCALL|LUA_MASKRET, 0);
#endif
		if (!do_string)
			xr_sprintf(S, "%s.main()", caNamespaceName);
		else
			xr_sprintf(S, "%s()", main_function);
		if (!ai().script_engine().load_buffer(lua(), S, xr_strlen(S), "@_thread_main"))
			return;
		m_active = true;
	}
	catch(...) {
		m_active = false;
	}
}

CScriptThread::~CScriptThread() //KRodin: fixed
{
#ifndef LUABIND_HAS_BUGS_WITH_LUA_THREADS
//#ifdef DEBUG //KRodin: раскомментировал для отладки
	Msg("* Destroying script thread %s",*m_script_name);
//#endif
	try {
		luaL_unref(ai().script_engine().lua(),LUA_REGISTRYINDEX,m_thread_reference);
	}
	catch(...) {
	}
#endif
}

bool CScriptThread::update() //KRodin: fixed
{
	R_ASSERT2(m_active, "Cannot resume dead Lua thread!");
	try {
		ai().script_engine().current_thread(this);
		int l_iErrorCode = lua_resume(lua(), 0);
		if (l_iErrorCode && (l_iErrorCode != LUA_YIELD))
		{
			ai().script_engine().print_output(lua(), *script_name(), l_iErrorCode);
			m_active = false;
		}
		else
		{
			if (l_iErrorCode != LUA_YIELD)
			{
				ai().script_engine().print_output(lua(), *script_name(), l_iErrorCode);
				m_active = false;
				ai().script_engine().script_log(ScriptStorage::eLuaMessageTypeInfo, "Script %s is finished!", *m_script_name); //Убрать наверно надо
			}
			else
				VERIFY2(!lua_gettop(lua()), "Do not pass any value to coroutine.yield()!");
		}
		
		ai().script_engine().current_thread(nullptr);
	}
	catch(...) {
		ai().script_engine().current_thread(nullptr);
		m_active = false;
	}
	return m_active;
}
