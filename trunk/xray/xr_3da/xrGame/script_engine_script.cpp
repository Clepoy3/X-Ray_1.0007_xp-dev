////////////////////////////////////////////////////////////////////////////
//	Module 		: script_engine_script.cpp
//	Created 	: 25.12.2002
//  Modified 	: 13.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife Simulator script engine export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_engine.h"
#include "ai_space.h"
//#include <ostream>
#include "xr_level_controller.h"
#include "../x_ray.h"

using namespace luabind;

bool editor()
{
#ifdef XRGAME_EXPORTS
	return false;
#else
	return true;
#endif
}

#ifdef XRGAME_EXPORTS
CRenderDevice *get_device() { return &Device; }
#endif

int bit_and(int i, int j) { return i & j; }
int bit_or(int i, int j) { return i | j; }
int bit_xor(int i, int j) { return i ^ j; }
int bit_not(int i) { return ~i; }
const char *user_name() { return Core.UserName; }
void prefetch_module(LPCSTR file_name) { ai().script_engine().process_file(file_name); }

struct profile_timer_script {
	u64							m_start_cpu_tick_count;
	u64							m_accumulator;
	u64							m_count;
	int							m_recurse_mark;
	
	ICF								profile_timer_script	()
	{
		m_start_cpu_tick_count	= 0;
		m_accumulator			= 0;
		m_count					= 0;
		m_recurse_mark			= 0;
	}

	ICF								profile_timer_script	(const profile_timer_script &profile_timer)
	{
		*this					= profile_timer;
	}

	ICF		profile_timer_script&	operator=				(const profile_timer_script &profile_timer)
	{
		m_start_cpu_tick_count	= profile_timer.m_start_cpu_tick_count;
		m_accumulator			= profile_timer.m_accumulator;
		m_count					= profile_timer.m_count;
		m_recurse_mark			= profile_timer.m_recurse_mark;
		return					(*this);
	}

	ICF		bool					operator<				(const profile_timer_script &profile_timer) const
	{
		return					(m_accumulator < profile_timer.m_accumulator);
	}

	ICF		void					start					()
	{
		if (m_recurse_mark) {
			++m_recurse_mark;
			return;
		}

		++m_recurse_mark;
		++m_count;
		m_start_cpu_tick_count	= CPU::GetCLK();
	}

	ICF		void					stop					()
	{
		if (!m_recurse_mark)
			return;
		--m_recurse_mark;
		
		if (m_recurse_mark)
			return;
		
		u64						finish = CPU::GetCLK();
		if (finish > m_start_cpu_tick_count)
			m_accumulator		+= finish - m_start_cpu_tick_count;
	}

	ICF		float					time					() const
	{
		FPU::m64r				();
		float					result = (float(double(m_accumulator)/double(CPU::clk_per_second))*1000000.f);
		FPU::m24r				();
		return					(result);
	}
};

ICF	profile_timer_script	operator+	(const profile_timer_script &portion0, const profile_timer_script &portion1)
{
	profile_timer_script	result;
	result.m_accumulator	= portion0.m_accumulator + portion1.m_accumulator;
	result.m_count			= portion0.m_count + portion1.m_count;
	return					(result);
}

//std::ostream& operator<<(std::ostream& os, const profile_timer_script& pt)
//{ return os << pt.time(); }

#ifdef XRGAME_EXPORTS
ICF	u32	script_time_global	()	{ return Device.dwTimeGlobal; }
#else
ICF	u32	script_time_global	()	{ return 0; }
#endif
extern int get_action_dik(EGameActions _action_id);
CApplication *get_application() { return pApp; }

#pragma optimize("s",on)
void CScriptEngine::script_register(lua_State *L)
{
	module(L)
	[
		def("log1",	(void(*)(const char*)) &Log), //RvP

		class_<profile_timer_script>("profile_timer")
			.def(constructor<>())
			.def(constructor<profile_timer_script&>())
			.def(const_self + profile_timer_script())
			.def(const_self < profile_timer_script())
			//.def(tostring(self))
			.def("start",&profile_timer_script::start)
			.def("stop",&profile_timer_script::stop)
			.def("time",&profile_timer_script::time)
		,
		class_<CApplication>("CApplication")
		.def("set_load_texture", &CApplication::SetLoadTexture),
		def("get_application", &get_application),
		def("prefetch", &prefetch_module),
		def("editor", &editor),
		def("bit_and", &bit_and),
		def("bit_or", &bit_or),
		def("bit_xor", &bit_xor),
		def("bit_not", &bit_not),
		def("user_name", &user_name),
		def("time_global", &script_time_global),
#ifdef XRGAME_EXPORTS
		def("device", &get_device),
#endif
		def("bind_to_dik", &get_action_dik)
	];
}
