// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
#ifndef WINVER
#define WINVER		0x0500
#endif
//#define _WIN32_WINNT	0x0400
#ifndef _WIN32_IE
#define _WIN32_IE	0x0400
#endif


#define _WTL_USE_CSTRING
#define _WTL_NEW_PAGE_NOTIFY_HANDLERS

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>

#include <atlcrack.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atldlgs.h>
#include <atlddx.h>

#include "resource.h" // resource

