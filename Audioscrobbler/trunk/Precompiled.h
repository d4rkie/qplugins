#if !defined(AS_PRECOMPILED_HEADER_)
#define AS_PRECOMPILED_HEADER_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0400

// System includes
#include <TCHAR.h>
#include <Windows.h>
#include <time.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <vector>
#include <deque>


#define TIXML_USE_STL
#include "tinyxml\tinyxml.h"

#include <QString.h>


#ifdef _DEBUG
	#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#define new DEBUG_NEW
#endif

#include "md5.h"

#define CURL_NO_OLDIES
#define CURL_DISABLE_LDAP
#include "curl\include\curl\curl.h"

#include "Log.h"

#include <QCDCtrlMsgs.h>
#include <QCDModDefs.h>
#include <QCDModGeneral2.h>
#include <IQCDMediaInfo.h>
#include <IQCDTagInfo.h>


#endif // !defined(AS_PRECOMPILED_HEADER_)