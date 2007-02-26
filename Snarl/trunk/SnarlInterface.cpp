// About:	Snarl C++ interface implementation
//
// Authors:	Written by Toke Noer and "Whitman"
//
// License etc. :
//   Feel free to use this code and class as you see fit.
//   If you improve the code, it would be nice of you to take contact to the
//   authors, so all can get the benifit.
//
//   There is no guarantee that the code is correct or functional.
//   USE AT YOUR OWN RISK
//-----------------------------------------------------------------------------


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

LONG32 SnarlInterface::snShowMessage(LPCSTR szTitle, LPCSTR szText, LONG32 timeout, LPCSTR szIconPath, HWND hWndReply, LONG32 uReplyMsg)
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

LONG32 SnarlInterface::uSend(SNARLSTRUCT snarlStruct)
{
	HWND hWnd = snGetSnarlWindow();
	if (IsWindow(hWnd))
	{
		DWORD nReturn = 0;
		COPYDATASTRUCT cds;
		cds.dwData = 2;
		cds.cbData = sizeof(snarlStruct);
		cds.lpData = &snarlStruct;
		if (SendMessageTimeout(hWnd, WM_COPYDATA, (WPARAM)m_hwndFrom, (LPARAM)&cds, SMTO_NORMAL, 2000, &nReturn))
			return nReturn;
		else {
			OutputDebugString(_T("QMPSnarl: uSend::SendMessageTimeout"));
			return 0;
		}
	}
	return -1;
}