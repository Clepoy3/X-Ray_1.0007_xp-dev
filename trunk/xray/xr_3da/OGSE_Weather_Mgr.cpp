//‘ункци€ дл€ полного скриптового управлени€ погодой. —делано на основе исходников ogse.dll от KD. KRodin (c) 2017

#include "stdafx.h"
#include "OGSE_Weather_Mgr.hpp"

#include "IGame_Persistent.h"
#include "Environment.h"
#include "xr_efflensflare.h"
#include "thunderbolt.h"

bool str2vect2(const char* s, _vector2<float> &v)
{
	return 2 == sscanf(s, "%f,%f", &v.x, &v.y);
}
bool str2vect3(const char* s, _vector3<float> &v)
{
	return 3 == sscanf(s, "%f,%f,%f", &v.x, &v.y, &v.z);
}

bool str2vect4(const char* s, _vector4<float> &v)
{
	v.w = 0;
	return sscanf(s, "%f,%f,%f,%f", &v.x, &v.y, &v.z, &v.w) >= 3;
}

void InitEnvDescriptor(CEnvDescriptor *env_desc, luabind::object const& table)
{
	R_ASSERT2(env_desc, "InitEnvDescriptor: 'env_desc' is a null pointer!");

	CEnvironment &env = g_pGamePersistent->Environment(); // получаем объект погоды

	using namespace luabind;

	env_desc->sky_rotation = 0;
	env_desc->bolt_period = 0;
	env_desc->bolt_duration = 0;
	env_desc->env_ambient = 0;

	env_desc->exec_time = fmod( object_cast<float>(table["exec_time"]), (lua_Number)DAY_LENGTH ); //KRodin: как вы€снилось, object_cast<float> нормально читает float из строки, что хорошо!
	env_desc->exec_time_loaded = env_desc->exec_time;

	auto v = object_cast<const char*>(table["sky_texture"]);
	//Msg("sky_texture: %s", v);
	static char buf[1024];
	strcpy_s(buf, v);
	env_desc->sky_texture_name._set(buf);
	strncat_s(buf, sizeof(buf), "#small", 6);
	env_desc->sky_texture_env_name._set(buf);

	env_desc->clouds_texture_name._set(object_cast<const char*>(table["clouds_texture"]));

	float multiplier = 0;
	int res = sscanf( object_cast<const char*>(table["clouds_color"]), "%f,%f,%f,%f,%f",
		&env_desc->clouds_color.x,
		&env_desc->clouds_color.y,
		&env_desc->clouds_color.z,
		&env_desc->clouds_color.w,
		&multiplier);
	R_ASSERT2( res >= 3, "SetEnvDescData: invalid 'clouds_color' parameter!" );
	float save = env_desc->clouds_color.w;
	env_desc->clouds_color.mul(.5f*multiplier);
	env_desc->clouds_color.w = save;

	R_ASSERT2( str2vect3( object_cast<const char*>(table["sky_color"]), env_desc->sky_color), "SetEnvDescData: invalid 'sky_color' parameter!" );
	env_desc->sky_color.mul(.5f);

	env_desc->sky_rotation = deg2rad(object_cast<float>(table["sky_rotation"]));

	env_desc->far_plane = object_cast<float>(table["far_plane"]);
	//Msg("Far_plane: %f", object_cast<float>(table["far_plane"]));
	R_ASSERT2( str2vect3(object_cast<const char*>(table["fog_color"]), env_desc->fog_color), "SetEnvDescData: invalid 'fog_color' parameter!" );

	env_desc->fog_density = object_cast<float>(table["fog_density"]);

	env_desc->fog_distance = object_cast<float>(table["fog_distance"]);

	env_desc->rain_density = object_cast<float>(table["rain_density"]);

	//KRodin: params_ex закомментированы, т.к в этом движке это всЄ не поддерживаетс€.
	//R_ASSERT2(str2vect3(object_cast<const char*>(table["rain_color"]), env_desc->params_ex->rain_color), "SetEnvDescData: invalid 'rain_color' parameter!");

	//env_desc->params_ex->rain_max_drop_angle = deg2rad(object_cast<float>(table["rain_max_drop_angle"]));

	//env_desc->params_ex->sun_shafts = object_cast<float>(table["sun_shafts"]);

	//env_desc->params_ex->sun_shafts_length = object_cast<float>(table["sun_shafts_length"]);

	//env_desc->params_ex->rain_increase_speed = object_cast<float>(table["rain_increase_speed"]);

	//env_desc->params_ex->moon_road_intensity = object_cast<float>(table["moon_road_intensity"]);

	env_desc->wind_velocity = object_cast<float>(table["wind_velocity"]);

	env_desc->wind_direction = deg2rad(object_cast<float>(table["wind_direction"]));

	R_ASSERT2( str2vect3(object_cast<const char*>(table["ambient"]), env_desc->ambient), "SetEnvDescData: invalid 'ambient' parameter!" );

	R_ASSERT2( str2vect4(object_cast<const char*>(table["hemi_color"]), env_desc->hemi_color), "SetEnvDescData: invalid 'hemi_color' parameter!" );

	R_ASSERT2( str2vect3(object_cast<const char*>(table["sun_color"]), env_desc->sun_color), "SetEnvDescData: invalid 'sun_color' parameter!" );

	_vector2<float> vector;
	R_ASSERT2( str2vect2(object_cast<const char*>(table["sun_dir"]), vector), "SetEnvDescData: invalid 'sun_dir' parameter!" );
	env_desc->sun_dir.setHP(deg2rad(vector.y), deg2rad(vector.x));

	auto flares_name = object_cast<const char*>(table["flares"]);
	//Msg("flares_name: %s", flares_name);
	env_desc->lens_flare_id = env.eff_LensFlare->AppendDef(pSettings, flares_name);
	//Msg("lens_flare_id: %d", env_desc->lens_flare_id);

	auto thunderbolt_name = object_cast<const char*>(table["thunderbolt"]);
	//Msg("thunderbolt_name: %s", thunderbolt_name);
	env_desc->tb_id = env.eff_Thunderbolt->AppendDef(pSettings, thunderbolt_name);
	//Msg("thunderbolt_id: %d", env_desc->tb_id);

	env_desc->bolt_period = object_cast<float>(table["bolt_period"]);

	env_desc->bolt_duration = object_cast<float>(table["bolt_duration"]);

	auto env_sect = object_cast<const char*>(table["env_ambient"]);
	if (env_sect && strcmp(env_sect, ""))
	{
		//Msg("env_ambient: %s", env_sect);
		//shared_str str;
		//str.set(env_sect);
		env_desc->env_ambient = env.AppendEnvAmb(env_sect);
	}

	env_desc->on_device_destroy();
	env_desc->on_device_create();
}

void ENGINE_API SetEnvDescData(luabind::object const& t1, luabind::object const& t2)
{
#ifdef LUABIND_09
	R_ASSERT(LUA_TTABLE == luabind::type(t1));
	R_ASSERT(LUA_TTABLE == luabind::type(t2));
#else
	R_ASSERT(LUA_TTABLE == t1.type());
	R_ASSERT(LUA_TTABLE == t2.type());
#endif

	CEnvironment &env = g_pGamePersistent->Environment(); // получаем объект погоды

	InitEnvDescriptor((*env.CurrentWeather)[0], t1);
	InitEnvDescriptor((*env.CurrentWeather)[1], t2);

	CEnvDescriptor* *env_desc0 = &(*env.CurrentWeather)[0];
	CEnvDescriptor* *env_desc1 = &(*env.CurrentWeather)[1];

	//Msg("1 - env0: %f, env1: %f", (*env_desc0)->exec_time, (*env_desc1)->exec_time);
	//Msg("1 - envd0: %f, envd1: %f", env.Current[0]->exec_time, env.Current[1]->exec_time);

	if ((*env_desc0)->exec_time > (*env_desc1)->exec_time)
	{
		CEnvDescriptor *tmp_desc = *env_desc0;
		*env_desc0 = *env_desc1;
		*env_desc1 = tmp_desc;
	}

	//Msg("2 - env0: %f, env1: %f", (*env_desc0)->exec_time, (*env_desc1)->exec_time);
	//Msg("2 - envd0: %f, envd1: %f", env.Current[0]->exec_time, env.Current[1]->exec_time);

	env.SelectEnvs(env.CurrentWeather, env.Current[0], env.Current[1], env.fGameTime);

	//Msg("3 - env0: %f, env1: %f", (*env_desc0)->exec_time, (*env_desc1)->exec_time);
	//Msg("3 - envd0: %f, envd1: %f", env.Current[0]->exec_time, env.Current[1]->exec_time);
}
