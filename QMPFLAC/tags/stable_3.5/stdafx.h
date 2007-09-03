// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

#pragma warning ( disable : 4996 )

// Change these values to use different versions
#ifndef WINVER
#define WINVER			0x0500
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0x0501
#endif
#ifndef _WIN32_IE
#define _WIN32_IE		0x0501
#endif


#include <assert.h>


#define _WTL_NO_CSTRING
#define _WTL_NEW_PAGE_NOTIFY_HANDLERS

#include <atlbase.h>

#include <atlstr.h> // ATL CString

#include <atlapp.h>
#include <atlwin.h>

#include <atlframe.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atldlgs.h>
#include <atluser.h>
#include <atlddx.h>

