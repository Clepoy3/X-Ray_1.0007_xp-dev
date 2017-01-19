////////////////////////////////////////////////////////////////////////////
//	Module 		: script_engine.h
//	Created 	: 01.04.2004
//  Modified 	: 01.04.2004
//	Author		: Dmitriy Iassenev
//	Description : XRay Script Engine
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "pch_script.h"
#include "script_storage.h"
#include "script_export_space.h"
#include "associative_vector.h"

namespace ScriptEngine {
	enum EScriptProcessors {
		eScriptProcessorLevel = u32(0),
		eScriptProcessorGame,
		eScriptProcessorDummy = u32(-1),
	};
};

class CScriptProcess;
class CScriptThread;
struct lua_State;
struct lua_Debug;

#ifdef USE_DEBUGGER
	class CScriptDebugger;
#endif

class /*DLL_API*/ CScriptEngine : public CScriptStorage { //DLL_API - раскомментировать при выносе скриптового движка в отдельный dll
public:
	typedef CScriptStorage											inherited;
	typedef ScriptEngine::EScriptProcessors							EScriptProcessors;
	typedef associative_vector<EScriptProcessors,CScriptProcess*>	CScriptProcessStorage;

private:
	bool						m_reload_modules;

protected:
	CScriptProcessStorage		m_script_processes;
	int							m_stack_level;

protected:
#ifdef USE_DEBUGGER
	CScriptDebugger				*m_scriptDebugger;
#endif

private:
	string128					m_last_no_file;
	u32							m_last_no_file_length;

			bool				no_file_exists				(LPCSTR file_name, u32 string_length);
			void				add_no_file					(LPCSTR file_name, u32 string_length);

public:
								CScriptEngine				();
	virtual						~CScriptEngine				();
	/*lua_State**/ void init(/*bool loadGlobalNamespace*/);
	virtual	void				unload						();
	static	int					lua_panic					(lua_State *L);
	static	void				lua_error					(lua_State *L);
	static	int					lua_pcall_failed			(lua_State *L);
	static	void				lua_hook_call				(lua_State *L, lua_Debug *dbg);
			void				load_common_scripts			();
			bool				load_file(const char *scriptName, const char *namespaceName);
			CScriptProcess		*script_process				(const EScriptProcessors &process_id) const;
			void				add_script_process			(const EScriptProcessors &process_id, CScriptProcess *script_process);
			void				remove_script_process		(const EScriptProcessors &process_id);
			void				setup_auto_load				();
			bool				process_file_if_exists		(LPCSTR file_name, bool warn_if_not_exist);
			bool				process_file				(LPCSTR file_name);
			bool				process_file				(LPCSTR file_name, bool reload_modules);
			bool				function_object				(LPCSTR function_to_call, luabind::adl::object &object, int type = LUA_TFUNCTION);
			void				register_script_classes		();
			void				parse_script_namespace(const char *name, char *ns, u32 nsSize, char *func, u32 funcSize);

	template <typename TResult>
	IC		bool				functor						(LPCSTR function_to_call, luabind::functor<TResult> &lua_function);

#ifdef USE_DEBUGGER
			void				stopDebugger				();
			void				restartDebugger				();
			CScriptDebugger		*debugger					();
#endif
			void				collect_all_garbage			();
			LPCSTR				try_call					(LPCSTR func_name, LPCSTR param);

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
add_to_type_list(CScriptEngine)
#undef script_type_list
#define script_type_list save_type_list(CScriptEngine)

void log_script_error(LPCSTR format, ...);
extern DLL_API lua_State* game_lua();
extern DLL_API LPCSTR try_call_luafunc(LPCSTR func_name, LPCSTR param);

template <typename TResult>
IC bool CScriptEngine::functor(LPCSTR function_to_call, luabind::functor<TResult> &lua_function)
{
	luabind::object object;
	if (!function_object(function_to_call, object))
		return false;
	lua_function = object;
	return true;
}
