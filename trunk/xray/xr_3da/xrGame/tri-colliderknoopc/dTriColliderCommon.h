#pragma once

#include "../Level.h"
#include "../ode_include.h"
#include "../ExtendedGeom.h"
#include "dTriColliderMath.h"

extern xr_vector< flags8 > gl_cl_tries_state;
extern xr_vector<int>::iterator I,E,B;

#define NUMC_MASK (0xffff)

constexpr auto M_SIN_PI_3 = REAL(0.8660254037844386467637231707529362);
constexpr auto M_COS_PI_3 = REAL(0.5000000000000000000000000000000000);
