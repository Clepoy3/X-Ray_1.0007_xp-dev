////////////////////////////////////////////////////////////////////////////
//	Module 		: script_engine.cpp
//	Created 	: 01.04.2004
//  Modified 	: 01.04.2004
//	Author		: Dmitriy Iassenev
//	Description : XRay Script Engine
//  KRodin: TODO: Проверить дебаггер и объединить всё это дело со script_storage.
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_engine.h"
#include "ai_space.h"
#include "object_factory.h"
#include "script_process.h"
#include "../lua_tools.h"
#include "../../build_config_defines.h"
#ifdef USE_DEBUGGER
#	include "script_debugger.h"
#endif
#ifndef XRSE_FACTORY_EXPORTS //это что?
#	ifdef DEBUG
#		include "ai_debug.h"
		extern Flags32 psAI_Flags;
#	endif
#endif

extern void export_classes(lua_State *L);

#ifdef LUAICP_COMPAT
void luaicp_error_handler(lua_State *L)
{
	lua_getglobal(L, "AtPanicHandler");
	if lua_isfunction(L, -1)
		lua_call(L, 0, 0);
	else
		lua_pop(L, 1);
}
#endif

/*////////////Аллокатор памяти для луабинда///////////// KRodin: Здесь закомментировано, т.к раньше всего инитится в файле ResourceManager_Scripting.cpp. Пусть пока там и будет.
static void *__cdecl luabind_allocator(void *context, const void *pointer, size_t const size) //Взято из OpenXray
{
	if (!size)
	{
		void *non_const_pointer = const_cast<LPVOID>(pointer);
		xr_free(non_const_pointer);
		return nullptr;
	}
	if (!pointer)
	{
#ifdef DEBUG
		return Memory.mem_alloc(size, "luabind");
#else
		return Memory.mem_alloc(size);
#endif
	}
	void *non_const_pointer = const_cast<LPVOID>(pointer);
#ifdef DEBUG
	return Memory.mem_realloc(non_const_pointer, size, "luabind");
#else
	return Memory.mem_realloc(non_const_pointer, size);
#endif
}
/*////////////////////////////////////////////////////////////

CScriptEngine::CScriptEngine() //fixed
{
	//luabind::allocator = &luabind_allocator; //Без этого игра валится при открытии луабинда //Отключено, т.к раньше всего инитится в ResourceManager_Scripting.cpp. Пусть пока там и будет.
	//luabind::allocator_context = nullptr;
	//
	m_stack_level = 0;
	m_reload_modules = false;
	m_last_no_file_length = 0;
	*m_last_no_file = 0;

#ifdef USE_DEBUGGER
	m_scriptDebugger = NULL;
	restartDebugger();	
#endif
}

CScriptEngine::~CScriptEngine() //fixed
{
	g_game_lua = nullptr;
	while (!m_script_processes.empty())
		remove_script_process(m_script_processes.begin()->first);

#ifdef USE_DEBUGGER
	xr_delete (m_scriptDebugger);
#endif
}

void CScriptEngine::unload() //fixed
{
	lua_settop(lua(), m_stack_level);
	m_last_no_file_length	= 0;
	*m_last_no_file			= 0;
}

int CScriptEngine::lua_panic(lua_State *L) //Fixed
{
	print_output(L, "PANIC", LUA_ERRRUN);
	return 0;
}

#ifdef LUABIND_NO_EXCEPTIONS
void CScriptEngine::lua_error(lua_State *L) //Fixed
{
	print_output(L, "", LUA_ERRRUN);

	#ifndef LUAICP_COMPAT   
		const char *error = lua_tostring(L, -1);
		Debug.fatal(DEBUG_INFO, "LUA error: %s", error ? error : "NULL");
	#endif
}
#endif

int CScriptEngine::lua_pcall_failed(lua_State *L) //Fixed
{
	print_output(L, "", LUA_ERRRUN);
	#ifndef LUAICP_COMPAT   
		Debug.fatal(DEBUG_INFO, "LUA error: %s", lua_isstring(L, -1) ? lua_tostring(L, -1) : "");
	#endif
	if (lua_isstring(L, -1))
		lua_pop(L, 1);
	return LUA_ERRRUN;
}

#ifdef LUABIND_NO_EXCEPTIONS
void lua_cast_failed(lua_State *L, LUABIND_TYPE_INFO info) //fixed
{
	CScriptEngine::print_output(L, "", LUA_ERRRUN);
	//Debug.fatal(DEBUG_INFO, "LUA error: cannot cast lua value to %s", info.name()); //KRodin: Тут наверное вылетать не надо.
}
#endif

#ifdef DEBUG //Надо изучить, что там
#	include "script_thread.h"
void CScriptEngine::lua_hook_call(lua_State *L, lua_Debug *dbg) //Это для чего??? Надо разобраться.
{
	if (ai().script_engine().current_thread())
		ai().script_engine().current_thread()->script_hook(L, dbg);
}
#endif

int auto_load(lua_State *L) //fixed //KRodin: Вот это - гигантские тормоза. Надо думать, что с ними делать. Может просто перед регистрацией классов (или в какой-то более подходящщий момент) сканировать всю папку скриптов и их все компилить? Чтобы обойтись как-то без этих тормозов. Но делать это аккуратно конечно, т.к скрипты могут себя неадекватно вести при пих компиляции раньше какого-то момента.
{
	if ((lua_gettop(L) < 2) || !lua_istable(L,1) || !lua_isstring(L,2))
	{
		lua_pushnil(L);
		return 1;
	}
	ai().script_engine().process_file_if_exists(lua_tostring(L, 2), false);
	lua_rawget(L, 1);
	return 1;
}

void CScriptEngine::setup_auto_load() //fixed
/*{  //Вариант из OpenXRay
	luaL_newmetatable(lua(), "XRAY_AutoLoadMetaTable");
	lua_pushstring(lua(), "__index");
	lua_pushcfunction(lua(), auto_load);
	lua_settable(lua(), -3);
	lua_pushstring(lua(), GlobalNamespace);
	lua_gettable(lua(), LUA_GLOBALSINDEX);
	luaL_getmetatable(lua(), "XRAY_AutoLoadMetaTable");
	lua_setmetatable(lua(), -2);
	//. ??????????
	// lua_settop(lua(), 0);
}*/
{ //старый вариант --Может всё таки его использовать?
	lua_pushstring(lua(), GlobalNamespace);
	lua_gettable(lua(), LUA_GLOBALSINDEX); 
	int value_index	= lua_gettop(lua());  // alpet: во избежания оставления в стеке лишней метатаблицы
	luaL_newmetatable(lua(), "XRAY_AutoLoadMetaTable");
	lua_pushstring(lua(), "__index");
	lua_pushcfunction(lua(), auto_load);
	lua_settable(lua(), -3);
	// luaL_getmetatable(lua(), "XRAY_AutoLoadMetaTable");
	lua_setmetatable(lua(), value_index);
}

/*lua_State**/ void CScriptEngine::init(/*bool loadGlobalNamespace*/) //Fixed
{
	Msg("[CScriptEngine::init] Starting LuaJIT!");
	lua_State* LSVM = luaL_newstate(); //Запускаем LuaJIT. Память себе он выделит сам.
	R_ASSERT2(LSVM, "! ERROR : Cannot initialize LUA VM!"); //Надо проверить, случается ли такое.
	reinit(LSVM);
	luabind::open(LSVM); //Запуск луабинда
//--------------Установка калбеков------------------//
#ifdef LUABIND_NO_EXCEPTIONS
	luabind::set_error_callback(lua_error);
	luabind::set_cast_failed_callback(lua_cast_failed);
#endif
	luabind::set_pcall_callback(lua_pcall_failed); //KRodin: НЕ ЗАКОММЕНТИРОВАТЬ НИ В КОЕМ СЛУЧАЕ!!!
	lua_atpanic(LSVM, lua_panic);
//-----------------------------------------------------//
	export_classes(LSVM); //Тут регистрируются все движковые функции, импортированные в скрипты
	luaL_openlibs(LSVM); //Инициализация функций LuaJIT

	//if (loadGlobalNamespace) //Сделано для того, чтобы не надо было городить кучу функций для запуска нескольких копий LuaJIT в будущем (для шейдеров и скриптов например.)
	//{
#ifdef USE_DEBUGGER
		if (debugger())
			debugger()->PrepareLuaBind();
#endif
		g_game_lua = LSVM; //KRodin: не трогать, там намудрили так, что хрен разбрешься. Пусть так и будет.
		setup_auto_load(); //Построение метатаблиц
#ifdef DEBUG //Это надо убрать наверно?
#ifdef USE_DEBUGGER
		if( !debugger() || !debugger()->Active()  )
#endif
			lua_sethook(lua(),lua_hook_call, LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET, 0);
#endif
		bool save = m_reload_modules;
		m_reload_modules = true;
		process_file_if_exists(GlobalNamespace, false);
		m_reload_modules = save;

		m_stack_level = lua_gettop(LSVM);

		register_script_classes(); //Походу, запуск class_registrator.script
		object_factory().register_script(); //Тоже какая-то регистрация классов
		load_common_scripts(); //Ещё загрузка каких-то скриптов //Вообще это всё надо пересмотреть и убрать зависимость от конфигов. И может даже вынести отсюда.
	//}
	Msg("[CScriptEngine::init] LuaJIT Started!");
	//return LSVM; //На будущее
}

void CScriptEngine::remove_script_process(const EScriptProcessors &process_id) //fixed
{
	CScriptProcessStorage::iterator	I = m_script_processes.find(process_id);
	if (I != m_script_processes.end())
	{
		xr_delete((*I).second);
		m_script_processes.erase(I);
	}
}

bool CScriptEngine::load_file(const char *scriptName, const char *namespaceName) //fixed
{
	if (!process_file(scriptName))
		return false;
	string1024 initializerName;
	xr_strcpy(initializerName, scriptName);
	xr_strcat(initializerName, "_initialize");
	if (object(namespaceName, initializerName, LUA_TFUNCTION))
	{
		// lua_dostring(lua(), xr_strcat(initializerName, "()"));
		luabind::functor<void> f;
		R_ASSERT(functor(initializerName, f));
		f();
	}
	return true;
}

void CScriptEngine::add_script_process(const EScriptProcessors &process_id, CScriptProcess *script_process) //fixed
{
	VERIFY(m_script_processes.find(process_id) == m_script_processes.end());
	m_script_processes.insert(std::make_pair(process_id, script_process));
}

CScriptProcess *CScriptEngine::script_process(const EScriptProcessors &process_id) const //fixed
{
	auto it = m_script_processes.find(process_id);
	if (it != m_script_processes.end())
		return it->second;
	return nullptr;
}

void CScriptEngine::parse_script_namespace(const char *name, char *ns, u32 nsSize, char *func, u32 funcSize) //Fixed
{
	auto p = strrchr(name, '.');
	if (!p)
	{
		xr_strcpy(ns, nsSize, GlobalNamespace);
		p = name - 1;
	}
	else
	{
		VERIFY(u32(p - name + 1) <= nsSize);
		strncpy(ns, name, p - name);
		ns[p - name] = 0;
	}
	xr_strcpy(func, funcSize, p + 1);
}

void CScriptEngine::load_common_scripts() //Надо бы вынести отсюда?
{
	string_path		S;
	FS.update_path	(S,"$game_config$","script.ltx");
	CInifile		*l_tpIniFile = xr_new<CInifile>(S);
	R_ASSERT		(l_tpIniFile);
	if (!l_tpIniFile->section_exist("common"))
	{
		xr_delete			(l_tpIniFile);
		return;
	}
	if (l_tpIniFile->line_exist("common","script"))
	{
		shared_str caScriptString = l_tpIniFile->r_string("common","script");
		u32				n = _GetItemCount(*caScriptString);
		string256		I;
		for (u32 i=0; i<n; ++i)
		{
			_GetItem(*caScriptString,i,I);
			load_file(I, GlobalNamespace);
		}
	}
	xr_delete			(l_tpIniFile);
}

bool CScriptEngine::process_file_if_exists(LPCSTR file_name, bool warn_if_not_exist) //fixed
{
#ifdef DEBUG
	Msg("[CScriptEngine::process_file_if_exists] loading file: [%s]", file_name); //Довольно часто вызывается... Надо что-то с этим делать.
#endif
	u32 string_length = xr_strlen(file_name);
	if (!warn_if_not_exist && no_file_exists(file_name, string_length))
		return false;
	string_path S,S1;
	if (m_reload_modules || (*file_name && !namespace_loaded(file_name)))
	{
		FS.update_path(S, "$game_scripts$", strconcat(sizeof(S1), S1, file_name, ".script"));
		if (!warn_if_not_exist && !FS.exist(S))
		{
#ifdef DEBUG
			Msg("-------------------------");
			Msg("[CScriptEngine::process_file_if_exists] WARNING: Access to nonexistent variable '%s' or loading nonexistent script '%s'", file_name, S1);
			print_stack();
			Msg("-------------------------");
#endif
			add_no_file(file_name, string_length);
			return false;
		}
#ifdef DEBUG
		Msg("[CScriptEngine::process_file_if_exists] loading script: [%s]", S1);
#endif
		m_reload_modules = false;
		load_file_into_namespace(S, *file_name ? file_name : GlobalNamespace);
	}
	return true;
}

bool CScriptEngine::process_file(LPCSTR file_name) //fixed
{ return process_file_if_exists(file_name, true); }

bool CScriptEngine::process_file(LPCSTR file_name, bool reload_modules) //fixed
{
	m_reload_modules = reload_modules;
	bool result = process_file_if_exists(file_name, true);
	m_reload_modules = false;
	return result;
}

void CScriptEngine::register_script_classes() //Может вынести отсюда?
{
	string_path					S;
	FS.update_path				(S,"$game_config$","script.ltx");
	CInifile					*l_tpIniFile = xr_new<CInifile>(S);
	R_ASSERT					(l_tpIniFile);

	if (!l_tpIniFile->section_exist("common")) {
		xr_delete				(l_tpIniFile);
		return;
	}

	shared_str m_class_registrators		= READ_IF_EXISTS(l_tpIniFile,r_string,"common","class_registrators","");
	xr_delete					(l_tpIniFile);

	u32							n = _GetItemCount(*m_class_registrators);
	string256					I;
	for (u32 i=0; i<n; ++i) {
		_GetItem				(*m_class_registrators,i,I);
		luabind::functor<void>	result;
		if (!functor(I,result)) {
			script_log			(eLuaMessageTypeError,"Cannot load class registrator %s!",I);
			continue;
		}
		result					(const_cast<CObjectFactory*>(&object_factory()));
	}
}

bool CScriptEngine::function_object(LPCSTR function_to_call, luabind::object &object, int type) //fixed
{
	if (!xr_strlen(function_to_call))
		return false;
	string256 name_space, function;
	parse_script_namespace(function_to_call, name_space, sizeof(name_space), function, sizeof(function));
	if (xr_strcmp(name_space, GlobalNamespace))
	{
		LPSTR file_name = strchr(name_space, '.');
		if (!file_name)
			process_file(name_space);
		else
		{
			*file_name = 0;
			process_file(name_space);
			*file_name = '.';
		}
	}

	if (!this->object(name_space, function, type))
		return false;
	luabind::object lua_namespace = this->name_space(name_space);
	object = lua_namespace[function];
	return true;
}


#ifdef USE_DEBUGGER //Надо разобраться что за дебаггер и для чего он нужен.
CScriptDebugger *CScriptEngine::debugger()
{
	return m_scriptDebugger;
}

void CScriptEngine::stopDebugger()
{
	if (debugger()){
		xr_delete	(m_scriptDebugger);
		Msg			("Script debugger succesfully stoped.");
	}
	else
		Msg			("Script debugger not present.");
}

void CScriptEngine::restartDebugger()
{
	if(debugger())
		stopDebugger();

	m_scriptDebugger = xr_new<CScriptDebugger>();
	debugger()->PrepareLuaBind();
	Msg				("Script debugger succesfully restarted.");
}
#endif


bool CScriptEngine::no_file_exists(LPCSTR file_name, u32 string_length) //fixed
{
	if (m_last_no_file_length != string_length)
		return false;
	return !memcmp(m_last_no_file, file_name, string_length);
}

void CScriptEngine::add_no_file(LPCSTR file_name, u32 string_length) //fixed
{
	m_last_no_file_length = string_length;
	std::memcpy(m_last_no_file, file_name, string_length + 1);
}

void CScriptEngine::collect_all_garbage() //fixed //Это сборщик мусора, вызывается при дестрое уровня, видимо.
{
	lua_gc(lua(), LUA_GCCOLLECT, 0);
	//lua_gc(lua(), LUA_GCCOLLECT, 0); //зачем два раза вызывать?
}


void log_script_error(LPCSTR format, ...) //Взызвается из трех мест, надо б удалить наверно. Но пока пусть останется.
{
	string1024 line_buf;
	va_list mark;	
	va_start(mark, format);
	int sz = _vsnprintf(line_buf, sizeof(line_buf)-1, format, mark); 	
	line_buf[sizeof(line_buf) - 1] = 0;
	va_end(mark);

	ai().script_engine().script_log(ScriptStorage::ELuaMessageType::eLuaMessageTypeError, line_buf);
}
 
DLL_API lua_State* game_lua() //Fixed
{
	R_ASSERT(g_game_lua);
	return g_game_lua;
}

////////////////////////////////////////////////////////////////////////////
LPCSTR CScriptEngine::try_call(LPCSTR func_name, LPCSTR param) //В данный момент нужно для калбека смены загрузочных экранов.
{
	if (NULL == this || NULL == lua())
		return "#ERROR: Script engine not ready";
	// максимально быстрый вызов функции
	int save_top = lua_gettop(lua());
	lua_getglobal(lua(), func_name);
	if (lua_isfunction(lua(), -1))
	{
		int args = 0;
		if (param)
		{
			args++;
			lua_pushstring(lua(), param);
		}

		if (0 != lua_pcall(lua(), args, LUA_MULTRET, 0))
			lua_pcall_failed(lua());

		static string1024 result;
		strcpy_s(result, "#OK");
		if (lua_isstring(lua(), -1))
			strcpy_s(result, 1023, lua_tostring(lua(), -1));
		lua_settop(lua(), save_top);
		return result;
	}
	else
	{
		lua_pop(lua(), 1);
		return "#ERROR: function not found";
	}
}

DLL_API LPCSTR try_call_luafunc(LPCSTR func_name, LPCSTR param) //В данный момент нужно для калбека смены загрузочных экранов.
{
	return ai().script_engine().try_call(func_name, param);
}
////////////////////////////////////////////////////////////////////////////
