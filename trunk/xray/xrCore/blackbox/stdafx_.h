#ifndef AFX_STDAFX_H__
#define AFX_STDAFX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "WarningsOff.h"
#define NOMINMAX
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include "PSAPI.h"
#include "WarningsOn.h"
#include "../xrCore.h"

namespace BlackBox {

bool isspace( int ch ); 

bool isdigit( int ch );

long atol( const char* nptr );

};

// #ifdef _EDITOR
#ifndef min
#   define min(a,b) ((a) < (b) ? (a) : (b))
#endif // _EDITOR

#endif //
