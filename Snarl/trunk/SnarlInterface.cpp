// About:
//   Snarl C++ interface implementation
//   To understand what the different functions do and what they return, please
//   have a look at the API on http://www.fullphat.net/.
//   Also have a look at mSnarl_i.bas found in the CVS/SVN repository for Snarl.
//   
//
// Authors:
//   Written by Toke Noer and "Whitman"
// 
//
// License etc. :
//   Feel free to use this code and class as you see fit.
//   If you improve the code, it would be nice of you to take contact to the
//   authors, so all can get the benifit.
//
//   There is no guarantee that the code is correct or functional.
//   USE AT YOUR OWN RISK
//-----------------------------------------------------------------------------

// Last Update: 2007/03/04

// History
//  2007/03/04 : Added - snGetAppPath, snGetIconsPath, snGetVersionEx, 
//                       snSetTimeout, uSendEx


#include "SnarlInterface.h"

//-----------------------------------------------------------------------------
// Const strings
//-----------------------------------------------------------------------------
const LPCTSTR SnarlInterface::SNARL_GLOBAL_MSG = _T("SnarlGlobalEvent");


//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
SnarlInterface::SnarlInterface()
: m_hwndFrom(NULL)
{

}

SnarlInterface::~SnarlInterface()
{

}


//-----------------------------------------------------------------------------
// Public functions
//-----------------------------------------------------------------------------

LONG32 SnarlInterface::snShowMessage(LPCSTR szTitle, LPCSTR szText, LONG32 timeout, LPCSTR szIconPath, HWND hWndReply, WPARAM uReplyMsg)
{
	SNARLSTRUCT ss;
	ZeroMemory((void*)&ss, sizeof(ss));

	ss.Cmd = SNARL_SHOW;
	StringCbCopyA((LPSTR)&ss.Title, SNARL_STRING_LENGTH, szTitle);
	StringCbCopyA((LPSTR)&ss.Text,  SNARL_STRING_LENGTH, szText);
	StringCbCopyA((LPSTR)&ss.Icon,  SNARL_STRING_LENGTH, szIconPath);
	ss.Timeout = timeout;

    ss.LngData2 = reinterpret_cast<LONG32>(hWndReply);
    ss.Id = uReplyMsg;
    return uSend(ss);
}

//-----------------------------------------------------------------------------

LONG32 SnarlInterface::snShowMessageEx(LPCSTR szClass, LPCSTR szTitle, LPCSTR szText, LONG32 timeout, LPCSTR szIconPath, HWND hWndReply, WPARAM uReplyMsg, LPCSTR szSoundFile)
{
	SNARLSTRUCTEX ssex;
	ZeroMemory((void*)&ssex, sizeof(ssex));

	ssex.Cmd = SNARL_SHOW;
	ssex.Timeout = timeout;
    ssex.LngData2 = reinterpret_cast<LONG32>(hWndReply);
    ssex.Id = uReplyMsg;

	StringCbCopyA((LPSTR)&ssex.Class, SNARL_STRING_LENGTH, szClass);
	StringCbCopyA((LPSTR)&ssex.Title, SNARL_STRING_LENGTH, szTitle);
	StringCbCopyA((LPSTR)&ssex.Text,  SNARL_STRING_LENGTH, szText);
	StringCbCopyA((LPSTR)&ssex.Icon,  SNARL_STRING_LENGTH, szIconPath);
	StringCbCopyA((LPSTR)&ssex.Extra, SNARL_STRING_LENGTH, szSoundFile);

    return uSendEx(ssex);
}

//-----------------------------------------------------------------------------

BOOL SnarlInterface::snHideMessage(LONG32 id)
{
    SNARLSTRUCT ss;
    ss.Cmd = SNARL_HIDE;
	ss.Id = id;
    return static_cast<BOOL>(uSend(ss));
}

//-----------------------------------------------------------------------------

BOOL SnarlInterface::snIsMessageVisible(LONG32 id)
{
    SNARLSTRUCT ss;
    ss.Cmd = SNARL_IS_VISIBLE;
	ss.Id = id;
    return static_cast<BOOL>(uSend(ss));
}

//-----------------------------------------------------------------------------

BOOL SnarlInterface::snUpdateMessage(LONG32 id, LPCSTR szTitle, LPCSTR szText, LPCSTR szIconPath)
{
	SNARLSTRUCT ss;
    ss.Cmd = SNARL_UPDATE;
	ss.Id = id;

	StringCbCopyA((LPSTR)&ss.Title, SNARL_STRING_LENGTH, szTitle);
	StringCbCopyA((LPSTR)&ss.Text,  SNARL_STRING_LENGTH, szText);
	/* 1.6 Beta 4 */
    StringCbCopyA((LPSTR)&ss.Icon,  SNARL_STRING_LENGTH, szIconPath);
    return static_cast<BOOL>(uSend(ss));
}

//-----------------------------------------------------------------------------

BOOL SnarlInterface::snRegisterConfig(HWND hWnd, LPCSTR szAppName, LONG32 replyMsg)
{
	SNARLSTRUCT ss;

	m_hwndFrom = hWnd;
    
    ss.Cmd = SNARL_REGISTER_CONFIG_WINDOW;
    ss.LngData2 = reinterpret_cast<LONG32>(hWnd);
    ss.Id = replyMsg;
	StringCbCopyA((LPSTR)&ss.Title, SNARL_STRING_LENGTH, szAppName);
    return static_cast<BOOL>(uSend(ss));
}

//-----------------------------------------------------------------------------

BOOL SnarlInterface::snRegisterConfig2(HWND hWnd, LPCSTR szAppName, LONG32 replyMsg, LPCSTR szIcon)
{
	SNARLSTRUCT ss;

	m_hwndFrom = hWnd;

	ss.Cmd = SNARL_REGISTER_CONFIG_WINDOW_2;
	ss.LngData2 = reinterpret_cast<LONG32>(hWnd);
	ss.Id = replyMsg;
	StringCbCopyA((LPSTR)&ss.Title, SNARL_STRING_LENGTH, szAppName);
	StringCbCopyA((LPSTR)&ss.Icon, SNARL_STRING_LENGTH, szIcon);
	return uSend(ss);
}

//-----------------------------------------------------------------------------

BOOL SnarlInterface::snRevokeConfig(HWND hWnd)
{
	SNARLSTRUCT ss;
	
	m_hwndFrom = NULL;

    ss.Cmd = SNARL_REVOKE_CONFIG_WINDOW;
    ss.LngData2 = reinterpret_cast<LONG32>(hWnd);
    return static_cast<BOOL>(uSend(ss));
}

//-----------------------------------------------------------------------------

BOOL SnarlInterface::snGetVersion(WORD* major, WORD* minor)
{
	SNARLSTRUCT ss;
    ss.Cmd = SNARL_GET_VERSION;
    LONG32 versionInfo = uSend(ss);
	if (versionInfo) {
		int maj = static_cast<int>(HIWORD(versionInfo));
		*major = maj;
		int min = static_cast<int>(LOWORD(versionInfo));
		*minor = min;
        return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------

LONG32 SnarlInterface::snGetVersionEx()
{
	SNARLSTRUCT ss;
    ss.Cmd = SNARL_GET_VERSION_EX;
	return uSend(ss);
}

//-----------------------------------------------------------------------------

BOOL SnarlInterface::snSetTimeout(LONG32 Id, LONG32 Timeout)
{
	SNARLSTRUCT ss;
    ss.Cmd = SNARL_SET_TIMEOUT;
    ss.Id = Id;
    ss.LngData2 = Timeout;
    return uSend(ss);
}

//-----------------------------------------------------------------------------

LONG32 SnarlInterface::snRegisterAlert(LPCSTR szAppName, LPCSTR szClass)
{
	SNARLSTRUCT ss;
    ss.Cmd = SNARL_REGISTER_ALERT;
	StringCbCopyA((LPSTR)&ss.Title, SNARL_STRING_LENGTH, szAppName);
	StringCbCopyA((LPSTR)&ss.Text, SNARL_STRING_LENGTH, szClass);
    return uSend(ss);
}
//-----------------------------------------------------------------------------

UINT SnarlInterface::snGetGlobalMsg()
{
	return RegisterWindowMessage(SNARL_GLOBAL_MSG);
}

//-----------------------------------------------------------------------------

HWND SnarlInterface::snGetSnarlWindow()
{
	return FindWindow(NULL, _T("Snarl"));
}


//-----------------------------------------------------------------------------
// Returns a pointer to the path.
// delete [] when finished !
LPCTSTR SnarlInterface::snGetAppPath()
{
	DWORD nDataSize = 1; // Query data size in bytes
	LONG nRet = RegGetValue(HKEY_LOCAL_MACHINE, 
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Snarl.exe"), 
		NULL, RRF_RT_REG_SZ, NULL, NULL, &nDataSize);
	
	if (nRet == ERROR_SUCCESS || nRet == ERROR_MORE_DATA)
	{
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR* szFile = new TCHAR[nDataSize / sizeof(TCHAR)];
		TCHAR* szPath = new TCHAR[nDataSize / sizeof(TCHAR)];

		nRet = RegGetValue(HKEY_LOCAL_MACHINE, 
			_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Snarl.exe"), 
			NULL, REG_SZ, NULL, szFile, &nDataSize);

		if (_tsplitpath_s(szFile, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0) == 0) { // success
			StringCbCopy(szPath, nDataSize, szDrive);
			StringCbCat(szPath, nDataSize, szDir);
			// StringCbCat(szPath, nDataSize, _T("\\"));
		}
		else {
			delete [] szPath;
			szPath = NULL;
		}

		delete [] szFile;
		return szPath;
	}
	else {
		TCHAR szError[1024] = {0};
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, nRet, 0, szError, 1024, 0);
		MessageBox(NULL, szError, _T("SnarlInterface error"), 0);
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Returns a pointer to the iconpath.
// delete [] when finished !
LPCTSTR SnarlInterface::snGetIconsPath()
{
	TCHAR* szIconPath = NULL;
	LPCTSTR szPath = snGetAppPath();
	if (!szPath)
		return NULL;

	size_t nLen = _tcsnlen(szPath, MAX_PATH);
	if (nLen > 0)
	{
		nLen += 10 + 1; // etc\\icons\\ + NULL
		szIconPath = new TCHAR[nLen];

		StringCbCopy(szIconPath, nLen * sizeof(TCHAR), szPath);
		StringCbCat(szIconPath, nLen * sizeof(TCHAR), _T("etc\\icons\\"));
	}
	
	delete [] szPath;

	return szIconPath;
}



//-----------------------------------------------------------------------------
// Private functions
//-----------------------------------------------------------------------------

LONG32 SnarlInterface::uSend(SNARLSTRUCT ss)
{
	DWORD nReturn = M_FAILED;
	HWND hWnd = snGetSnarlWindow();
	if (IsWindow(hWnd))
	{
		
		COPYDATASTRUCT cds;
		cds.dwData = 2;
		cds.cbData = sizeof(ss);
		cds.lpData = &ss;
		if (!SendMessageTimeout(hWnd, WM_COPYDATA, (WPARAM)m_hwndFrom, (LPARAM)&cds, SMTO_NORMAL, 2000, &nReturn))
		{
			OutputDebugString(_T("QMPSnarl: uSend::SendMessageTimeout"));
			nReturn = M_TIMED_OUT;
		}
	}
	return nReturn;
}

//-----------------------------------------------------------------------------

LONG32 SnarlInterface::uSendEx(SNARLSTRUCTEX ssex)
{
	DWORD nReturn = M_FAILED;
	HWND hWnd = snGetSnarlWindow();
	if (IsWindow(hWnd))
	{
		COPYDATASTRUCT cds;
		cds.dwData = 2;
		cds.cbData = sizeof(ssex);
		cds.lpData = &ssex;
		if (!SendMessageTimeout(hWnd, WM_COPYDATA, (WPARAM)m_hwndFrom, (LPARAM)&cds, SMTO_NORMAL, 2000, &nReturn))
		{
			OutputDebugString(_T("QMPSnarl: uSend::SendMessageTimeout"));
			nReturn = M_TIMED_OUT;
		}
	}
	return nReturn;
}

//-----------------------------------------------------------------------------