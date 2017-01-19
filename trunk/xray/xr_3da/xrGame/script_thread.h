////////////////////////////////////////////////////////////////////////////
//	Module 		: script_thread.h
//	Created 	: 19.09.2003
//  Modified 	: 29.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Script thread class
////////////////////////////////////////////////////////////////////////////
#pragma once

#include "pch_script.h"
#include "script_engine.h"
#include "ai_space.h"
#ifdef USE_DEBUGGER
#	include "script_debugger.h"
#endif

struct lua_State;

class CScriptThread {
private:
	shared_str m_script_name;
	int m_thread_reference;
	bool m_active;
	lua_State *m_virtual_machine;
public:
	CScriptThread(LPCSTR caNamespaceName, bool do_string = false, bool reload = false);
	virtual ~CScriptThread();
	bool update();
	bool active() const { return m_active; }
	shared_str script_name() const { return m_script_name; }
	int thread_reference() const { return m_thread_reference; }
	lua_State *lua() const { return m_virtual_machine; }
};
