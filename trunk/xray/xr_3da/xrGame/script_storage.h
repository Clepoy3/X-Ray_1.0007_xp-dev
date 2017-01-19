////////////////////////////////////////////////////////////////////////////
//	Module 		: script_storage.h
//	Created 	: 01.04.2004
//  Modified 	: 01.04.2004
//	Author		: Dmitriy Iassenev
//	Description : XRay Script Storage
////////////////////////////////////////////////////////////////////////////
#pragma once

#include "pch_script.h"

namespace ScriptStorage {
	enum ELuaMessageType {
		eLuaMessageTypeInfo = u32(0),
		eLuaMessageTypeError,
		eLuaMessageTypeMessage,
		eLuaMessageTypeHookCall,
		eLuaMessageTypeHookReturn,
		eLuaMessageTypeHookLine,
		eLuaMessageTypeHookCount,
		eLuaMessageTypeHookTailReturn = u32(-1),
	};
}

struct lua_State;
class CScriptThread;

//KRodin: !!!НЕ ВКЛЮЧАТЬ!!! ТАМ ни один файл под новый луабинд не адаптирован!!!
//#define USE_DEBUGGER // alpet: полезно при отладке модов и интеграции 1.0007 (!). Позволяет использовать scriptDbgIde, не собирая ТОРМОЗНОЙ отладочный билд.

using namespace ScriptStorage;

class CScriptStorage {
private:
	lua_State					*m_virtual_machine	;
	CScriptThread				*m_current_thread	;
	BOOL						m_jit				;

	char *scriptBuffer = nullptr;
	size_t scriptBufferSize = 0;

protected:
	//bool parse_namespace(LPCSTR caNamespaceName, LPSTR b, u32 b_size, LPSTR c, u32 c_size);
	bool parse_namespace(LPCSTR caNamespaceName, LPSTR b, LPSTR c);
	bool				do_file						(LPCSTR	caScriptName, LPCSTR caNameSpaceName);
			void				reinit						(lua_State *LSVM);

public:
	//
	lua_State *lua() { return m_virtual_machine; };
	void current_thread(CScriptThread *thread)
	{
		VERIFY(thread && !m_current_thread || !thread);
		m_current_thread = thread;
	}
	CScriptThread *current_thread() const { return m_current_thread; }
	//
	CScriptStorage				();
	virtual						~CScriptStorage				();
			bool				load_buffer					(lua_State *L, LPCSTR caBuffer, size_t tSize, LPCSTR caScriptName, LPCSTR caNameSpaceName = 0);
			bool				load_file_into_namespace	(LPCSTR	caScriptName, LPCSTR caNamespaceName);
			bool				namespace_loaded			(LPCSTR	caName, bool remove_from_stack = true);
			bool				object						(LPCSTR	caIdentifier, int type);
			bool				object						(LPCSTR	caNamespaceName, LPCSTR	caIdentifier, int type);
			luabind/*::adl*/::object name_space					(LPCSTR	namespace_name);
	static	int	script_log					(ELuaMessageType message,	LPCSTR	caFormat, ...);
	static	bool				print_output				(lua_State *L,		LPCSTR	caScriptName,		int		iErorCode = 0);
	static const char *const GlobalNamespace;
	void print_stack();
};
