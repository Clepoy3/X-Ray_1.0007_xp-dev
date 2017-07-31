// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
// Third generation by Oles.

#ifndef stdafxH
#define stdafxH

#pragma once

#include "../xrCore/xrCore.h"

#include "include\DPlay\dplay8.h" //KRodin: добавил недостающий файл
#include "dxerr.h" //KRodin: перенёс сюда из NET_Client.cpp и NET_Server.cpp
#include "NET_Shared.h"

#define _RELEASE(x)			{ if(x) { (x)->Release();       (x)=NULL; } }
#define _SHOW_REF(msg, x)   { if(x) { x->AddRef(); Log(msg,u32(x->Release()));}}

#endif //stdafxH
