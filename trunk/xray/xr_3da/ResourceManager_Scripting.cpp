#include "stdafx.h"
#pragma hdrstop

#include "render.h"
#include "ResourceManager.h"
#include "tss.h"
#include "blenders\blender.h"
#include "blenders\blender_recorder.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
//KRodin: �� ������ ���� ����� ���� ���������� ������ ��� �������� ����.
#include "lua_tools.h"

bool CResourceManager::print_output(lua_State *L, const char* caScriptFileName, int errorCode)
{
	auto Prefix = "";
	if (errorCode) {
		switch (errorCode) {
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
	auto traceback = get_lua_traceback(L, 0);
	if (!lua_isstring(L, -1)) //�� �������! ����� ����� ���e�� ��� ����!
	{
		Msg("----------------------------------------------");
		Msg("[ResourceManager_Scripting.print_output(%s)] %s!\n%s", caScriptFileName, Prefix, traceback);
		Msg("----------------------------------------------");
		return false;
	}
	auto S = lua_tostring(L, -1);
	Msg("----------------------------------------------");
	Msg("[ResourceManager_Scripting.print_output(%s)] %s:\n%s\n%s", caScriptFileName, Prefix, S, traceback);
	Msg("----------------------------------------------");
	return true;
}

bool CResourceManager::load_buffer(lua_State *L, const char* caBuffer, size_t tSize, const char* caScriptName, const char* caNameSpaceName)
{
	//KRodin: ���������, �.�. � ��������� ��� ����������� �����, �� ��� ������, ��������.
	int buf_len = std::snprintf(nullptr, 0, FILE_HEADER, caNameSpaceName, caNameSpaceName, caBuffer);
	auto strBuf = std::make_unique<char[]>(buf_len + 1);
	std::snprintf(strBuf.get(), buf_len + 1, FILE_HEADER, caNameSpaceName, caNameSpaceName, caBuffer);
	//Log("[CResourceManager::load_buffer] Loading buffer:");
	//Log(strBuf.get());
	int l_iErrorCode = luaL_loadbuffer(L, strBuf.get(), buf_len /*+ 1 ����-���������� �� ����� ������ ������*/, caScriptName);
	if (l_iErrorCode)
	{
		print_output(L, caScriptName, l_iErrorCode);
		R_ASSERT(false); //�� ����������������!
		return false;
	}
	return true;
}

bool CResourceManager::do_file(lua_State* LSVM, const char* caScriptName, const char* caNameSpaceName) //KRodin: fixed
{
	string_path l_caLuaFileName;
	auto l_tpFileReader = FS.r_open(caScriptName);
	if (!l_tpFileReader) { //�������� �� ������?
		Msg("!![CResourceManager::do_file] Cannot open file [%s]", caScriptName);
		return false;
	}
	strconcat(sizeof(l_caLuaFileName), l_caLuaFileName, "@", caScriptName); //KRodin: �������� ���� � ���� @f:\games\s.t.a.l.k.e.r\gamedata\scripts\class_registrator.script
	//
	//KRodin: ����������. ������ ���������� ������� ����� �������� ���������, ��� ������ �� �����.
	auto strBuf = std::make_unique<char[]>(l_tpFileReader->length() + 1);
	strncpy(strBuf.get(), (const char*)l_tpFileReader->pointer(), l_tpFileReader->length());
	strBuf.get()[l_tpFileReader->length()] = 0;
	//
	load_buffer(LSVM, strBuf.get(), (size_t)l_tpFileReader->length(), l_caLuaFileName, caNameSpaceName);
	FS.r_close(l_tpFileReader);

	int	l_iErrorCode = lua_pcall(LSVM, 0, 0, 0); //KRodin: ��� ����� ������� �� ��������!
	if (l_iErrorCode)
	{
		print_output(LSVM, caScriptName, l_iErrorCode);
		R_ASSERT(false); //�� ����������������!
		return false;
	}
	return true;
}

bool CResourceManager::namespace_loaded(lua_State* LSVM, const char* name, bool remove_from_stack) //KRodin: fixed
{
	int start = lua_gettop(LSVM);
	lua_pushstring(LSVM, GlobalNamespace);
	lua_rawget(LSVM, LUA_GLOBALSINDEX);
	string256 S2;
	xr_strcpy(S2, name);
	auto S = S2;
	for (;;)
	{
		if (!xr_strlen(S))
		{
			VERIFY(lua_gettop(LSVM) >= 1); //����� �� ������ ���� �� OpenXray
			lua_pop(LSVM, 1);
			VERIFY(start == lua_gettop(LSVM));
			return false;
		}
		auto S1 = strchr(S, '.');
		if (S1)
			*S1 = 0;
		lua_pushstring(LSVM, S);
		lua_rawget(LSVM, -2);
		if (lua_isnil(LSVM, -1))
		{
			//lua_settop(LSVM,0);
			VERIFY(lua_gettop(LSVM) >= 2);
			lua_pop(LSVM, 2);
			VERIFY(start == lua_gettop(LSVM));
			return false; //there is no namespace!
		}
		else if (!lua_istable(LSVM, -1))
		{
			//lua_settop(LSVM, 0);
			VERIFY(lua_gettop(LSVM) >= 1);
			lua_pop(LSVM, 1);
			VERIFY(start == lua_gettop(LSVM));
			R_ASSERT3(false, "Error : the namespace is already being used by the non-table object! Name: ", S);
			return false;
		}
		lua_remove(LSVM, -2);
		if (S1)
			S = ++S1;
		else
			break;
	}
	if (!remove_from_stack)
		VERIFY(lua_gettop(LSVM) == start + 1);
	else
	{
		VERIFY(lua_gettop(LSVM) >= 1);
		lua_pop(LSVM, 1);
		VERIFY(lua_gettop(LSVM) == start);
	}
	return true;
}

bool CResourceManager::OBJECT_1(lua_State* LSVM, const char* identifier, int type)
{
	int start = lua_gettop(LSVM);
	lua_pushnil(LSVM);
	while (lua_next(LSVM, -2))
	{
		if (lua_type(LSVM, -1) == type && !xr_strcmp(identifier, lua_tostring(LSVM, -2)))
		{
			VERIFY(lua_gettop(LSVM) >= 3);
			lua_pop(LSVM, 3);
			VERIFY(lua_gettop(LSVM) == start - 1);
			return true;
		}
		lua_pop(LSVM, 1);
	}
	VERIFY(lua_gettop(LSVM) >= 1);
	lua_pop(LSVM, 1);
	VERIFY(lua_gettop(LSVM) == start - 1);
	return false;
}

bool CResourceManager::OBJECT_2(lua_State* LSVM, const char* namespace_name, const char* identifier, int type)
{
	int start = lua_gettop(LSVM);
	if (xr_strlen(namespace_name) && !namespace_loaded(LSVM, namespace_name, false))
	{
		VERIFY(lua_gettop(LSVM) == start);
		return false;
	}
	bool result = OBJECT_1(LSVM, identifier, type);
	VERIFY(lua_gettop(LSVM) == start);
	return result;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace luabind;

// wrapper
class adopt_sampler
{
	CBlender_Compile*		C;
	u32						stage;
public:
	adopt_sampler			(CBlender_Compile*	_C, u32 _stage)		: C(_C), stage(_stage)		{ if (u32(-1)==stage) C=0;		}
	adopt_sampler			(const adopt_sampler&	_C)				: C(_C.C), stage(_C.stage)	{ if (u32(-1)==stage) C=0;		}

	adopt_sampler&			_texture		(LPCSTR texture)		{ if (C) C->i_Texture	(stage,texture);											return *this;	}
	adopt_sampler&			_projective		(bool _b)				{ if (C) C->i_Projective(stage,_b);													return *this;	}
	adopt_sampler&			_clamp			()						{ if (C) C->i_Address	(stage,D3DTADDRESS_CLAMP);									return *this;	}
	adopt_sampler&			_wrap			()						{ if (C) C->i_Address	(stage,D3DTADDRESS_WRAP);									return *this;	}
	adopt_sampler&			_mirror			()						{ if (C) C->i_Address	(stage,D3DTADDRESS_MIRROR);									return *this;	}
	adopt_sampler&			_f_anisotropic	()						{ if (C) C->i_Filter	(stage,D3DTEXF_ANISOTROPIC,D3DTEXF_LINEAR,D3DTEXF_ANISOTROPIC);	return *this;	}
	adopt_sampler&			_f_trilinear	()						{ if (C) C->i_Filter	(stage,D3DTEXF_LINEAR,D3DTEXF_LINEAR,D3DTEXF_LINEAR);		return *this;	}
	adopt_sampler&			_f_bilinear		()						{ if (C) C->i_Filter	(stage,D3DTEXF_LINEAR,D3DTEXF_POINT, D3DTEXF_LINEAR);		return *this;	}
	adopt_sampler&			_f_linear		()						{ if (C) C->i_Filter	(stage,D3DTEXF_LINEAR,D3DTEXF_NONE,  D3DTEXF_LINEAR);		return *this;	}
	adopt_sampler&			_f_none			()						{ if (C) C->i_Filter	(stage,D3DTEXF_POINT, D3DTEXF_NONE,  D3DTEXF_POINT);		return *this;	}
	adopt_sampler&			_fmin_none		()						{ if (C) C->i_Filter_Min(stage,D3DTEXF_NONE);										return *this;	}
	adopt_sampler&			_fmin_point		()						{ if (C) C->i_Filter_Min(stage,D3DTEXF_POINT);										return *this;	}
	adopt_sampler&			_fmin_linear	()						{ if (C) C->i_Filter_Min(stage,D3DTEXF_LINEAR);										return *this;	}
	adopt_sampler&			_fmin_aniso		()						{ if (C) C->i_Filter_Min(stage,D3DTEXF_ANISOTROPIC);								return *this;	}
	adopt_sampler&			_fmip_none		()						{ if (C) C->i_Filter_Mip(stage,D3DTEXF_NONE);										return *this;	}
	adopt_sampler&			_fmip_point		()						{ if (C) C->i_Filter_Mip(stage,D3DTEXF_POINT);										return *this;	}
	adopt_sampler&			_fmip_linear	()						{ if (C) C->i_Filter_Mip(stage,D3DTEXF_LINEAR);										return *this;	}
	adopt_sampler&			_fmag_none		()						{ if (C) C->i_Filter_Mag(stage,D3DTEXF_NONE);										return *this;	}
	adopt_sampler&			_fmag_point		()						{ if (C) C->i_Filter_Mag(stage,D3DTEXF_POINT);										return *this;	}
	adopt_sampler&			_fmag_linear	()						{ if (C) C->i_Filter_Mag(stage,D3DTEXF_LINEAR);										return *this;	}
};																																							
																																							
// wrapper																																					
class	adopt_compiler																																		
{
	CBlender_Compile*		C;
public:
	adopt_compiler			(CBlender_Compile*	_C)	: C(_C)							{ }
	adopt_compiler			(const adopt_compiler&	_C)	: C(_C.C)					{ }

	adopt_compiler&			_options		(int	P,		bool	S)				{	C->SetParams		(P,S);					return	*this;		}
	adopt_compiler&			_o_emissive		(bool	E)								{	C->SH->flags.bEmissive=E;					return	*this;		}
	adopt_compiler&			_o_distort		(bool	E)								{	C->SH->flags.bDistort=E;					return	*this;		}
	adopt_compiler&			_o_wmark		(bool	E)								{	C->SH->flags.bWmark=E;						return	*this;		}
	adopt_compiler&			_pass			(LPCSTR	vs,		LPCSTR ps)				{	C->r_Pass			(vs,ps,true);			return	*this;		}
	adopt_compiler&			_fog			(bool	_fog)							{	C->PassSET_LightFog	(FALSE,_fog);			return	*this;		}
	adopt_compiler&			_ZB				(bool	_test,	bool _write)			{	C->PassSET_ZB		(_test,_write);			return	*this;		}
	adopt_compiler&			_blend			(bool	_blend, u32 abSRC, u32 abDST)	{	C->PassSET_ablend_mode(_blend,abSRC,abDST);	return 	*this;		}
	adopt_compiler&			_aref			(bool	_aref,  u32 aref)				{	C->PassSET_ablend_aref(_aref,aref);			return 	*this;		}
	adopt_sampler			_sampler		(LPCSTR _name)							{	u32 s = C->r_Sampler(_name,0);				return	adopt_sampler(C,s);	}
};

class	adopt_blend
{
public:
};

void CResourceManager::LuaError(lua_State* L)
{
	print_output(L, "", LUA_ERRRUN);
	const char *error = lua_tostring(L, -1);
	Debug.fatal(DEBUG_INFO,"LUA error: %s", error ? error : "NULL");
}

/*void CResourceManager::lua_cast_failed(lua_State *L, const luabind::type_id &info)
{
	print_output(L, "", LUA_ERRRUN);
	Debug.fatal(DEBUG_INFO, "LUA error: cannot cast lua value to %s", info.name());
}*/

int CResourceManager::lua_pcall_failed(lua_State *L)
{
	print_output(L,  "", LUA_ERRRUN);
#ifndef LUAICP_COMPAT   
	Debug.fatal(DEBUG_INFO, "LUA error: %s", lua_isstring(L, -1) ? lua_tostring(L, -1) : "");
#endif
	if (lua_isstring(L, -1))
		lua_pop(L, 1);
	return LUA_ERRRUN;
}

int CResourceManager::lua_panic(lua_State *L)
{
	print_output(L, "PANIC", LUA_ERRRUN);
	return 0;
}

////////////��������� ������ ��� ��������/////////////
static void *__cdecl luabind_allocator(luabind::memory_allocation_function_parameter, const void *pointer, size_t const size) //������ ����� �������� �����, ������� ����� ����� � �����
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
////////////////////////////////////////////////////////////

void CResourceManager::LS_Load()
{
//**************************************************************//
	//Msg("[CResourceManager] Starting LuaJIT");
	R_ASSERT2(!LSVM, "! LuaJIT is already running"); //�� ������ ������
	//
	luabind::allocator = &luabind_allocator; //��������� �������� ������ ����� � ������ ���� ���!
	luabind::allocator_parameter = nullptr;
	LSVM = luaL_newstate(); //��������� LuaJIT. ������ ���� �� ������� ���.
	luaL_openlibs(LSVM); //������������� ������� LuaJIT
	R_ASSERT2(LSVM, "! ERROR : Cannot initialize LUA VM!"); //���� ���������, ��������� �� �����.
	luabind::open(LSVM); //������ ��������
	//
//--------------��������� ��������------------------//
#ifdef LUABIND_NO_EXCEPTIONS
	luabind::set_error_callback(LuaError); //������ �� ������.
	//luabind::set_cast_failed_callback(lua_cast_failed);
#endif
	luabind::set_pcall_callback(lua_pcall_failed); //KRodin: �� ���������������� �� � ���� ������!!!
	lua_atpanic(LSVM, lua_panic);
	//Msg("[CResourceManager] LuaJIT Started!");
	//-----------------------------------------------------//
//***************************************************************//
	module(LSVM)
	[
		class_<adopt_sampler>("_sampler")
			.def(								constructor<const adopt_sampler&>())
			.def("texture",						&adopt_sampler::_texture		,return_reference_to<1>())
			.def("project",						&adopt_sampler::_projective		,return_reference_to<1>())
			.def("clamp",						&adopt_sampler::_clamp			,return_reference_to<1>())
			.def("wrap",						&adopt_sampler::_wrap			,return_reference_to<1>())
			.def("mirror",						&adopt_sampler::_mirror			,return_reference_to<1>())
			.def("f_anisotropic",				&adopt_sampler::_f_anisotropic	,return_reference_to<1>())
			.def("f_trilinear",					&adopt_sampler::_f_trilinear	,return_reference_to<1>())
			.def("f_bilinear",					&adopt_sampler::_f_bilinear		,return_reference_to<1>())
			.def("f_linear",					&adopt_sampler::_f_linear		,return_reference_to<1>())
			.def("f_none",						&adopt_sampler::_f_none			,return_reference_to<1>())
			.def("fmin_none",					&adopt_sampler::_fmin_none		,return_reference_to<1>())
			.def("fmin_point",					&adopt_sampler::_fmin_point		,return_reference_to<1>())
			.def("fmin_linear",					&adopt_sampler::_fmin_linear	,return_reference_to<1>())
			.def("fmin_aniso",					&adopt_sampler::_fmin_aniso		,return_reference_to<1>())
			.def("fmip_none",					&adopt_sampler::_fmip_none		,return_reference_to<1>())
			.def("fmip_point",					&adopt_sampler::_fmip_point		,return_reference_to<1>())
			.def("fmip_linear",					&adopt_sampler::_fmip_linear	,return_reference_to<1>())
			.def("fmag_none",					&adopt_sampler::_fmag_none		,return_reference_to<1>())
			.def("fmag_point",					&adopt_sampler::_fmag_point		,return_reference_to<1>())
			.def("fmag_linear",					&adopt_sampler::_fmag_linear	,return_reference_to<1>()),

		class_<adopt_compiler>("_compiler")
			.def(								constructor<const adopt_compiler&>())
			.def("begin",						&adopt_compiler::_pass			,return_reference_to<1>())
			.def("sorting",						&adopt_compiler::_options		,return_reference_to<1>())
			.def("emissive",					&adopt_compiler::_o_emissive	,return_reference_to<1>())
			.def("distort",						&adopt_compiler::_o_distort		,return_reference_to<1>())
			.def("wmark",						&adopt_compiler::_o_wmark		,return_reference_to<1>())
			.def("fog",							&adopt_compiler::_fog			,return_reference_to<1>())
			.def("zb",							&adopt_compiler::_ZB			,return_reference_to<1>())
			.def("blend",						&adopt_compiler::_blend			,return_reference_to<1>())
			.def("aref",						&adopt_compiler::_aref			,return_reference_to<1>())
			.def("sampler",						&adopt_compiler::_sampler		),	// returns sampler-object

		class_<adopt_blend>("blend")
			.enum_("blend")
			[
				value("zero",					int(D3DBLEND_ZERO)),
				value("one",					int(D3DBLEND_ONE)),
				value("srccolor",				int(D3DBLEND_SRCCOLOR)),
				value("invsrccolor",			int(D3DBLEND_INVSRCCOLOR)),
				value("srcalpha",				int(D3DBLEND_SRCALPHA)),
				value("invsrcalpha",			int(D3DBLEND_INVSRCALPHA)),
				value("destalpha",				int(D3DBLEND_DESTALPHA)),
				value("invdestalpha",			int(D3DBLEND_INVDESTALPHA)),
				value("destcolor",				int(D3DBLEND_DESTCOLOR)),
				value("invdestcolor",			int(D3DBLEND_INVDESTCOLOR)),
				value("srcalphasat",			int(D3DBLEND_SRCALPHASAT))
			]
	];

	// load shaders
	xr_vector<char*>*	folder			= FS.file_list_open	("$game_shaders$",::Render->getShaderPath(),FS_ListFiles|FS_RootOnly);
	xr_vector<char*>::iterator it;
	VERIFY								(folder);
	
	for (it = (*folder).begin(); it != (*folder).end(); it++)	{
		string_path						namesp,fn;
		strcpy_s							(namesp,*it);
		if	(0==strext(namesp) || 0!=xr_strcmp(strext(namesp),".s"))	continue;
		*strext	(namesp)=0;
		if		(0==namesp[0])			strcpy_s	(namesp, GlobalNamespace);
		strconcat						(sizeof(fn),fn,::Render->getShaderPath(),*it);
		FS.update_path					(fn,"$game_shaders$",fn);
		do_file(LSVM, fn, namesp);
	}
	FS.file_list_close			(folder);
}

void CResourceManager::LS_Unload()
{
	//Msg("[CResourceManager] Closing LuaJIT - start");
	lua_close(LSVM); //JIT ������ ��������
	LSVM = nullptr;
	//Msg("[CResourceManager] Closing LuaJIT - end");
}

BOOL	CResourceManager::_lua_HasShader	(LPCSTR s_shader)
{
	string256	undercorated;
	for (int i=0, l=xr_strlen(s_shader)+1; i<l; i++)
		undercorated[i]=('\\'==s_shader[i])?'_':s_shader[i];

#ifdef _EDITOR
	return OBJECT_2(LSVM, undercorated, "editor", LUA_TFUNCTION);
#else
	return OBJECT_2(LSVM, undercorated, "normal", LUA_TFUNCTION) || OBJECT_2(LSVM, undercorated, "l_special", LUA_TFUNCTION);
#endif
}

Shader*	CResourceManager::_lua_Create		(LPCSTR d_shader, LPCSTR s_textures)
{
	CBlender_Compile	C;
	Shader				S;

	// undecorate
	string256	undercorated;
	for (int i=0, l=xr_strlen(d_shader)+1; i<l; i++)
		undercorated[i]=('\\'==d_shader[i])?'_':d_shader[i];
	LPCSTR		s_shader = undercorated;

	// Access to template
	C.BT				= NULL;
	C.bEditor			= FALSE;
	C.bDetail			= FALSE;

	// Prepare
	_ParseList			(C.L_textures,	s_textures	);
	C.detail_texture	= NULL;
	C.detail_scaler		= NULL;

	// Compile element	(LOD0 - HQ)
	if (OBJECT_2(LSVM,s_shader,"normal_hq",LUA_TFUNCTION))
	{
		// Analyze possibility to detail this shader
		C.iElement			= 0;
//.		C.bDetail			= Device.Resources->_GetDetailTexture(*C.L_textures[0],C.detail_texture,C.detail_scaler);
		C.bDetail			= Device.Resources->m_textures_description.GetDetailTexture(C.L_textures[0],C.detail_texture,C.detail_scaler);

		if (C.bDetail)		S.E[0]	= C._lua_Compile(s_shader,"normal_hq");
		else				S.E[0]	= C._lua_Compile(s_shader,"normal");
	} else {
		if (OBJECT_2(LSVM,s_shader,"normal",LUA_TFUNCTION))
		{
			C.iElement			= 0;
//.			C.bDetail			= Device.Resources->_GetDetailTexture(*C.L_textures[0],C.detail_texture,C.detail_scaler);
			C.bDetail			= Device.Resources->m_textures_description.GetDetailTexture(C.L_textures[0],C.detail_texture,C.detail_scaler);
			S.E[0]				= C._lua_Compile(s_shader,"normal");
		}
	}

	// Compile element	(LOD1)
	if (OBJECT_2(LSVM,s_shader,"normal",LUA_TFUNCTION))
	{
		C.iElement			= 1;
//.		C.bDetail			= Device.Resources->_GetDetailTexture(*C.L_textures[0],C.detail_texture,C.detail_scaler);
		C.bDetail			= Device.Resources->m_textures_description.GetDetailTexture(C.L_textures[0],C.detail_texture,C.detail_scaler);
		S.E[1]				= C._lua_Compile(s_shader,"normal");
	}

	// Compile element
	if (OBJECT_2(LSVM,s_shader,"l_point",LUA_TFUNCTION))
	{
		C.iElement			= 2;
		C.bDetail			= FALSE;
		S.E[2]				= C._lua_Compile(s_shader,"l_point");;
	}

	// Compile element
	if (OBJECT_2(LSVM,s_shader,"l_spot",LUA_TFUNCTION))
	{
		C.iElement			= 3;
		C.bDetail			= FALSE;
		S.E[3]				= C._lua_Compile(s_shader,"l_spot");;
	}

	// Compile element
	if (OBJECT_2(LSVM,s_shader,"l_special",LUA_TFUNCTION))
	{
		C.iElement			= 4;
		C.bDetail			= FALSE;
		S.E[4]				= C._lua_Compile(s_shader,"l_special");
	}

	// Search equal in shaders array
	for (u32 it=0; it<v_shaders.size(); it++)
		if (S.equal(v_shaders[it]))	return v_shaders[it];

	// Create _new_ entry
	Shader*		N			=	xr_new<Shader>(S);
	N->dwFlags				|=	xr_resource_flagged::RF_REGISTERED;
	v_shaders.push_back		(N);
	return N;
}

ShaderElement*		CBlender_Compile::_lua_Compile	(LPCSTR namesp, LPCSTR name)
{
	ShaderElement		E;
	SH =				&E;
	RS.Invalidate		();

	// Compile
	LPCSTR				t_0		= *L_textures[0]			? *L_textures[0] : "null";
	LPCSTR				t_1		= (L_textures.size() > 1)	? *L_textures[1] : "null";
	LPCSTR				t_d		= detail_texture			? detail_texture : "null" ;
	lua_State*			LSVM	= Device.Resources->LSVM;
	object				shader  = get_globals(LSVM)[namesp];
	object				element = shader[name];
	adopt_compiler		ac		= adopt_compiler(this);
	element						(ac,t_0,t_1,t_d);
	r_End				();
	ShaderElement*	_r	= Device.Resources->_CreateElement(E);
	return			_r;
}
