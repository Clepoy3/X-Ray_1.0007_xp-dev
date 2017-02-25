////////////////////////////////////////////////////////////////////////////
//	Module 		: script_storage.cpp
//	Created 	: 01.04.2004
//  Modified 	: 01.04.2004
//	Author		: Dmitriy Iassenev
//	Description : XRay Script Storage
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_storage.h"
#include "script_thread.h"
#include <stdarg.h>
#include "../lua_tools.h"
#include "../../build_config_defines.h"
#include "script_engine.h"
#include "ai_space.h"
#ifdef DEBUG
#	include "ai_debug.h" //Поглядеть, чего там
#endif
#ifdef USE_DEBUGGER
#	include "script_debugger.h"
#endif


#define SCRIPT_GLOBAL_NAMESPACE "_G"
const char *const CScriptStorage::GlobalNamespace = SCRIPT_GLOBAL_NAMESPACE;

LPCSTR file_header_old = "\
local function script_name() \
return \"%s\" \
end \
local this = {} \
%s this %s \
setmetatable(this, {__index = " SCRIPT_GLOBAL_NAMESPACE "}) \
setfenv(1, this) ";

LPCSTR file_header_new = "\
local function script_name() \
return \"%s\" \
end \
local this = {} \
this." SCRIPT_GLOBAL_NAMESPACE " = " SCRIPT_GLOBAL_NAMESPACE " \
%s this %s \
setfenv(1, this) ";


LPCSTR file_header = nullptr;

CScriptStorage::CScriptStorage() //fixed
{
	m_current_thread = nullptr;
	m_virtual_machine = nullptr;
}

CScriptStorage::~CScriptStorage() //fixed
{
	Msg("[CScriptStorage] Closing LuaJIT - start");
	if (m_virtual_machine)
		lua_close(m_virtual_machine); //Вот тут закрывается LuaJIT.

	if (scriptBuffer)
		xr_free(scriptBuffer);
	Msg("[CScriptStorage] Closing LuaJIT - end");
}

void CScriptStorage::reinit(lua_State *LSVM) //KRodin: fixed
{
	if (m_virtual_machine) //Как выяснилось, такое происходит при загрузке игры на этапе старта сервера 
	{
		Msg("[CScriptStorage] Found working LuaJIT WM! Close it!");
		lua_close(m_virtual_machine);
	}
	m_virtual_machine = LSVM;
#ifdef LUAICP_COMPAT
	#include "luaicp_attach.inc" // added by alpet 05.07.14
#endif
	if (strstr(Core.Params,"-_g"))
		file_header			= file_header_new;
	else
		file_header			= file_header_old;

	scriptBufferSize = 1024 * 1024;
	scriptBuffer = xr_alloc<char>(scriptBufferSize);
}

void CScriptStorage::print_stack() //KRodin: fixed
{
	lua_State *L = lua();
	LPCSTR traceback = get_lua_traceback(L, 0);
	Msg("%s", traceback);
}

int CScriptStorage::script_log(ScriptStorage::ELuaMessageType tLuaMessageType, LPCSTR caFormat, ...) //Используется в очень многих местах //Очень много пишет в лог, из многих мест. Наверное надо только в дебаге использовать
{
	va_list marker;
	va_start(marker, caFormat);
	//
	LPCSTR S = "";
	LPSTR S1;
	string4096 S2;
	switch (tLuaMessageType)
	{
		case ScriptStorage::eLuaMessageTypeInfo: {
			S = "[LUA INFO]";
			break;
		}
		case ScriptStorage::eLuaMessageTypeError: {
			S = "[LUA ERROR]";
			break;
		}
		case ScriptStorage::eLuaMessageTypeMessage: {
			S = "[LUA MESSAGE]";
			break;
		}
		case ScriptStorage::eLuaMessageTypeHookCall: {
			S = "[LUA HOOK_CALL]";
			break;
		}
		case ScriptStorage::eLuaMessageTypeHookReturn: {
			S = "[LUA HOOK_RETURN]";
			break;
		}
		case ScriptStorage::eLuaMessageTypeHookLine: {
			S = "[LUA HOOK_LINE]";
			break;
		}
		case ScriptStorage::eLuaMessageTypeHookCount: {
			S = "[LUA HOOK_COUNT]";
			break;
		}
		case ScriptStorage::eLuaMessageTypeHookTailReturn: {
			S = "[LUA HOOK_TAIL_RETURN]";
			break;
		}
		default: NODEFAULT;
	}
	xr_strcpy(S2, S);
	S1 = S2 + xr_strlen(S);
	int result = vsprintf(S1, caFormat, marker);
	Msg("-----------------------------------------");
	Msg("[script_log] %s", S2);
	ai().script_engine().print_stack();
	Msg("-----------------------------------------");
	va_end(marker);
	//
	return result;
}

/* //Эта функция ломает скрипты, вернул старую.
bool CScriptStorage::parse_namespace(LPCSTR caNamespaceName, LPSTR b, u32 b_size, LPSTR c, u32 c_size) //KRodin: fixed, но если будут проблемы, надо портировать STRCONCAT
{
	*b = 0;
	*c = 0;
	LPSTR S2;
	strcat(S2, caNamespaceName); //Может заменить на STRCONCAT ? Но это не так просто.
	LPSTR S = S2;
	for (int i = 0;; i++)
	{
		if (!xr_strlen(S))
		{
			script_log(ScriptStorage::eLuaMessageTypeError, "the namespace name %s is incorrect!", caNamespaceName);
			return false;
		}
		LPSTR S1 = strchr(S, '.');
		if (S1)
			*S1 = 0;
		if (i)
			xr_strcat(b, b_size, "{");
		xr_strcat(b, b_size, S);
		xr_strcat(b, b_size, "=");
		if (i)
			xr_strcat(c, c_size, "}");
		if (S1)
			S = ++S1;
		else
			break;
	}
	return true;
}
*/
bool CScriptStorage::parse_namespace(LPCSTR caNamespaceName, LPSTR b, LPSTR c)
{
	strcpy(b, "");
	strcpy(c, "");
	LPSTR S2 = xr_strdup(caNamespaceName);
	LPSTR S = S2;
	for (int i = 0;; ++i)
	{
		if (!xr_strlen(S))
		{
			script_log(ScriptStorage::eLuaMessageTypeError, "the namespace name %s is incorrect!", caNamespaceName);
			xr_free(S2);
			return false;
		}
		LPSTR S1 = strchr(S, '.');
		if (S1)
			*S1 = 0;

		if (i)
			strcat(b, "{");
		strcat(b, S);
		strcat(b, "=");
		if (i)
			strcat(c, "}");
		if (S1)
			S = ++S1;
		else
			break;
	}
	xr_free(S2);
	return true;
}

bool CScriptStorage::load_buffer(lua_State *L, LPCSTR caBuffer, size_t tSize, LPCSTR caScriptName, LPCSTR caNameSpaceName) //KRodin: fixed
{
	int l_iErrorCode;
	if (caNameSpaceName && xr_strcmp(GlobalNamespace, caNameSpaceName))
	{
		string512 insert, a, b;
		LPCSTR header = file_header;
		if (!parse_namespace(caNameSpaceName, a, /*sizeof(a),*/ b/*, sizeof(b)*/))
		{
			Msg("! [CScriptStorage::load_buffer] Can't parse namespace: [%s]", caNameSpaceName);
			return false;
		}
		//Msg("! [CScriptStorage::load_buffer] before xr_sprintf: insert: [%s], header: [%s], caNameSpaceName: [%s], a: [%s], b: [%s]", insert, header, caNameSpaceName, a, b);
		xr_sprintf(insert, header, caNameSpaceName, a, b);
		//Msg("! [CScriptStorage::load_buffer] insert after xr_sprintf: [%s]", insert);
		u32 str_len = xr_strlen(insert);
		u32 const total_size = str_len + tSize;
		if (total_size >= scriptBufferSize)
		{
			scriptBufferSize = total_size;
			scriptBuffer = (char *)xr_realloc(scriptBuffer, scriptBufferSize);
		}
		xr_strcpy(scriptBuffer, total_size, insert);
		CopyMemory(scriptBuffer + str_len, caBuffer, u32(tSize));
		//Msg("[CScriptStorage::load_buffer(1)] Loading buffer: %s", scriptBuffer);
		l_iErrorCode = luaL_loadbuffer(L, scriptBuffer, tSize + str_len, caScriptName);
	}
	else
		//Msg("[CScriptStorage::load_buffer(2)] Loading buffer: %s", caBuffer);
		l_iErrorCode = luaL_loadbuffer(L, caBuffer, tSize, caScriptName);
	if (l_iErrorCode)
	{
		print_output(L, caScriptName, l_iErrorCode);
		R_ASSERT(0);
		return false;
	}
	return true;
}

bool CScriptStorage::do_file(LPCSTR caScriptName, LPCSTR caNameSpaceName) //KRodin: fixed
{
	int start = lua_gettop(lua());
	string_path l_caLuaFileName;
	IReader *l_tpFileReader = FS.r_open(caScriptName);
	if (!l_tpFileReader) {
		script_log(eLuaMessageTypeError,"Cannot open file \"%s\"",caScriptName);
		return false;
	}
	strconcat(sizeof(l_caLuaFileName), l_caLuaFileName, "@", caScriptName);
	if (!load_buffer(lua(), static_cast<LPCSTR>(l_tpFileReader->pointer()), (size_t)l_tpFileReader->length(), l_caLuaFileName, caNameSpaceName))
	{
//		VERIFY(lua_gettop(lua()) >= 4);
//		lua_pop(lua(),4);
//		VERIFY(lua_gettop(lua()) == start - 3);
		lua_settop(lua(),start);
		FS.r_close(l_tpFileReader);
		return false;
	}
	FS.r_close(l_tpFileReader);

	int errFuncId = -1;
#ifdef LUAICP_COMPAT2 // exception dangerous
	lua_getglobal(lua(), "AtPanicHandler");
	if ( lua_isfunction( lua(), -1) )
		errFuncId = lua_gettop(lua());
	else
	    lua_pop(lua(), 1);
#endif
#ifdef USE_DEBUGGER
	if( ai().script_engine().debugger() && errFuncId < 0 )
	    errFuncId = ai().script_engine().debugger()->PrepareLua(lua());
#endif
	int	l_iErrorCode = lua_pcall(lua(), 0, 0, (-1 == errFuncId) ? 0 : errFuncId);
#ifdef USE_DEBUGGER
	if( ai().script_engine().debugger() )
		ai().script_engine().debugger()->UnPrepareLua(lua(),errFuncId);
#endif
	if (l_iErrorCode)
	{
		print_output(lua(), caScriptName, l_iErrorCode);
		R_ASSERT(0);
		lua_settop(lua(),start);
		return false;
	}
	return true;
}

bool CScriptStorage::load_file_into_namespace(LPCSTR caScriptName, LPCSTR caNamespaceName) //KRodin: fixed
{
	int start = lua_gettop(lua());
	if (!do_file(caScriptName,caNamespaceName))
	{
		lua_settop (lua(),start);
		return false;
	}
	VERIFY (lua_gettop(lua()) == start);
	return true;
}

bool CScriptStorage::namespace_loaded(LPCSTR name, bool remove_from_stack) //KRodin: fixed
{
	int start = lua_gettop(lua());
	lua_pushstring(lua(), GlobalNamespace);
	lua_rawget(lua(), LUA_GLOBALSINDEX); 
	string256 S2;
	xr_strcpy(S2, name);
	LPSTR S = S2;
	for (;;)
	{
		if (!xr_strlen(S))
		{
			VERIFY(lua_gettop(lua()) >= 1); //кусок взят из OpenXray
			lua_pop(lua(), 1);
			VERIFY(start == lua_gettop(lua()));
			return false;
		}
		LPSTR S1 = strchr(S, '.');
		if (S1)
			*S1 = 0; 
		lua_pushstring(lua(),S); 
		lua_rawget(lua(),-2); 
		if (lua_isnil(lua(), -1))
		{ 
			//lua_settop(lua(),0);
			VERIFY(lua_gettop(lua()) >= 2);
			lua_pop(lua(),2); 
			VERIFY(start == lua_gettop(lua()));
			return false; //there is no namespace!
		}
		else if (!lua_istable(lua(),-1))
		{ 
			//lua_settop(lua(), 0);
			VERIFY(lua_gettop(lua()) >= 1);
			lua_pop(lua(), 1); 
			VERIFY(start == lua_gettop(lua()));
			Debug.fatal(DEBUG_INFO, " Error : the namespace name %s is already being used by the non-table object!\n", S);
			return false; 
		} 
		lua_remove(lua(), -2); 
		if (S1)
			S = ++S1; 
		else
			break; 
	} 
	if (!remove_from_stack)
		VERIFY(lua_gettop(lua()) == start + 1);
	else
	{
		VERIFY(lua_gettop(lua()) >= 1);
		lua_pop(lua(), 1);
		VERIFY(lua_gettop(lua()) == start);
	}
	return true; 
}

bool CScriptStorage::object(LPCSTR identifier, int type) //KRodin: fixed
{
	int start = lua_gettop(lua());
	lua_pushnil(lua()); 
	while (lua_next(lua(), -2))
	{ 
		if (lua_type(lua(), -1) == type && !xr_strcmp(identifier, lua_tostring(lua(), -2)))
		{ 
			VERIFY(lua_gettop(lua()) >= 3);
			lua_pop(lua(), 3); 
			VERIFY(lua_gettop(lua()) == start - 1);
			return true; 
		} 
		lua_pop(lua(), 1); 
	} 
	VERIFY(lua_gettop(lua()) >= 1);
	lua_pop(lua(), 1); 
	VERIFY(lua_gettop(lua()) == start - 1);
	return false; 
}

bool CScriptStorage::object(LPCSTR namespace_name, LPCSTR identifier, int type) //KRodin: fixed
{
	int start = lua_gettop(lua());
	if (xr_strlen(namespace_name) && !namespace_loaded(namespace_name, false))
	{
		VERIFY(lua_gettop(lua()) == start);
		return false; 
	}
	bool result = object(identifier, type);
	VERIFY(lua_gettop(lua()) == start);
	return result; 
}

luabind::object CScriptStorage::name_space(LPCSTR namespace_name) //KRodin: fixed
{
	string256 S1;
	xr_strcpy(S1,namespace_name);
	LPSTR S = S1;
	luabind::object lua_namespace = luabind::get_globals(lua());
	for (;;)
	{
		if (!xr_strlen(S))
			return lua_namespace;
		LPSTR I = strchr(S, '.');
		if (!I)
			return lua_namespace[/*(const char*)*/S];
		*I = 0;
		lua_namespace = lua_namespace[/*(const char*)*/S];
		S = I + 1;
	}
}

bool CScriptStorage::print_output(lua_State *L, LPCSTR caScriptFileName, int errorCode) //KRodin: fixed, вызывается из нескольких мест, в т.ч. из калбеков lua_error, lua_pcall_failed, lua_cast_failed, lua_panic
{
	LPCSTR Prefix = "";
	if (errorCode) {
		switch (errorCode){
			case LUA_ERRRUN: {
				Prefix = "SCRIPT RUNTIME ERROR";
				break;
			}
			case LUA_ERRMEM: {
				Prefix = "SCRIPT ERROR (memory allocation)";
				break;
			}
			case LUA_ERRERR: {
				Prefix = "SCRIPT ERROR (while running the error handler function)";
				break;
			}
			case LUA_ERRFILE: {
				Prefix = "SCRIPT ERROR (while running file)";
				break;
			}
			case LUA_ERRSYNTAX: {
				Prefix = "SCRIPT SYNTAX ERROR";
				break;
			}
			case LUA_YIELD: {
				Prefix = "Thread is yielded";
				break;
			}
			default: NODEFAULT;
		}
	}

	LPCSTR traceback = get_lua_traceback(L, 0);

	if (!lua_isstring(L, -1)) //НЕ УДАЛЯТЬ! Иначе будут вылeты без лога!
	{
		Msg("----------------------------------------------");
		Msg("[print_output(%s)] %s!\n%s", caScriptFileName, Prefix, traceback);
		Msg("----------------------------------------------");
		return false;
	}

	LPCSTR S = lua_tostring(L, -1);
	//if (strstr(S, "cannot resume dead coroutine")) //???
	//	R_ASSERT2(0, "[print_output] Please do not return any values from main!!!"); //Сюда надо добавить больше инфы, наверно.
	//else
	//{
		//if (strstr(S, "[print_output] no overload of 'net_packet:r_vec3'"))  // при загрузке серверных объектов выполняются их корявые скрипты с тыщей таких ошибок :( //KRodin: лично я таких ошиюок не вижу, по крайней мере на кордоне в начале игры.
		//	 return true;

		Msg("----------------------------------------------");
		Msg("[print_output(%s)] %s:\n%s\n%s", caScriptFileName, Prefix, S, traceback);
		Msg("----------------------------------------------");
#ifdef LUAICP_COMPAT
		lua_getglobal(L, "DebugDumpAll");
		lua_pcall(L, 0, 0, -1);
#endif
#ifdef USE_DEBUGGER //Проверить надо чего тут внутри происходит
		if (ai().script_engine().debugger() && ai().script_engine().debugger()->Active())
		{
			ai().script_engine().debugger()->Write		(S);
			ai().script_engine().debugger()->ErrorBreak	();
		}
#endif
	//}
	return true;
}
