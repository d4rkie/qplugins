// About:
//   Snarl C++ interface implementation
//   To understand what the different functions do and what they return, please
//   have a look at the API on http://www.fullphat.net/dev/api.htm.
//   Also have a look at mSnarl_i.bas found in the CVS/SVN repository for Snarl.
//   
//
// Difference to VB implementation:
//   Please note that string functions return NULL when they fail and not an
//   empty string. So check for NULL...
//   Function names doesn't have the pre "sn".
//
// 
// Authors:
//   Written by Toke Noer and "Whitman"
//
// License etc. :
//   Feel free to use this code and class as you see fit.
//   If you improve the code, it would be nice of you to take contact to the
//   authors, so all can get the benifit.
//
//   There is no guarantee that the code is correct or functional.
//   USE AT YOUR OWN RISK
//-----------------------------------------------------------------------------

// History
//  2008/08/27 : x64 compiler fix
//  2008/08/24 : Renamed all functions to not have prepended "sn".
//             : Memory allocation functions added. (Use FreeString to free strings returned by Snarl)
//             : Added m_nLastMessageId to the class. (Use GetLastMessageId() to get it)
//             : Overloaded a few functions, so one don't have include the message id. (UpdateMessage f.i.)

//  2008/06/20 : Fixed snShowMessageEx so it actually sends the extended version - oops
//             : Fixed compiler warning C4800: forcing value to bool 'true' or 'false' (performance warning)

//  2008/05/19 : uSend and uSendEx would always set return value to M_OK on successfull call
//  2008/04/14 : Updated to follow (what should be) the final Snarl 2.0 API
//  2008/03/28 : Few fixes for Snarl 2.0
//  2007/05/23 : snGetGlobalMsg & snGetSnarlWindow made static
//  2007/03/25 : 1.6 RC1 fixup
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
: m_hwndFrom(NULL), m_nLastMessageId(0)
{

}

SnarlInterface::~SnarlInterface()
{

}


//-----------------------------------------------------------------------------
// snShowMessage()

/// Displays a message with Title and Text. Timeout controls how long the
/// message is displayed for (in seconds) (omitting this value means the message
/// is displayed indefinately). IconPath specifies the location of a PNG image
/// which will be displayed alongside the message text.
/// <returns>Message Id on success or M_RESULT on failure</returns>

LONG32 SnarlInterface::ShowMessage(LPCSTR szTitle, LPCSTR szText, LONG32 timeout, LPCSTR szIconPath, HWND hWndReply, WPARAM uReplyMsg)
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

	m_nLastMessageId = uSend(ss);
	return m_nLastMessageId;
}


//-----------------------------------------------------------------------------
// snShowMessageEx()

/// Displays a notification. This function is identical to snShowMessage()
/// except that Class specifies an alert previously registered with
/// snRegisterAlert() and SoundFile can optionally specify a WAV sound to play
/// when the notification is displayed on screen.

/// <returns>Message Id on success or M_RESULT on failure</returns>

LONG32 SnarlInterface::ShowMessageEx(LPCSTR szClass, LPCSTR szTitle, LPCSTR szText, LONG32 timeout, LPCSTR szIconPath, HWND hWndReply, WPARAM uReplyMsg, LPCSTR szSoundFile)
{
	SNARLSTRUCTEX ssex;
	ZeroMemory((void*)&ssex, sizeof(ssex));

	ssex.Cmd = SNARL_EX_SHOW;
	ssex.Timeout = timeout;
	ssex.LngData2 = reinterpret_cast<LONG32>(hWndReply);
	ssex.Id = uReplyMsg;

	StringCbCopyA((LPSTR)&ssex.Class, SNARL_STRING_LENGTH, szClass);
	StringCbCopyA((LPSTR)&ssex.Title, SNARL_STRING_LENGTH, szTitle);
	StringCbCopyA((LPSTR)&ssex.Text,  SNARL_STRING_LENGTH, szText);
	StringCbCopyA((LPSTR)&ssex.Icon,  SNARL_STRING_LENGTH, szIconPath);
	StringCbCopyA((LPSTR)&ssex.Extra, SNARL_STRING_LENGTH, szSoundFile);

	m_nLastMessageId = uSendEx(ssex);
	return m_nLastMessageId;
}


//-----------------------------------------------------------------------------
// snHideMessage()

/// Hides the notification specified by Id. Id is the value returned by
/// snShowMessage() or snShowMessageEx() when the notification was initially
/// created. This function returns True if the notification was successfully
/// hidden or False otherwise (for example, the notification may no longer exist).

BOOL SnarlInterface::HideMessage(LONG32 Id)
{
	SNARLSTRUCT ss;
	ss.Cmd = SNARL_HIDE;
	ss.Id = Id;

	return uSend(ss) != 0;
}

BOOL SnarlInterface::HideMessage()
{
	return HideMessage(m_nLastMessageId);
}

//-----------------------------------------------------------------------------
// snIsMessageVisible()

/// Returns True if the notification specified by Id is still visible, or
/// False if not. Id is the value returned by snShowMessage() or
/// snShowMessageEx() when the notification was initially created.

BOOL SnarlInterface::IsMessageVisible(LONG32 Id)
{
	SNARLSTRUCT ss;
	ss.Cmd = SNARL_IS_VISIBLE;
	ss.Id = Id;

	return uSend(ss) != 0;
}

BOOL SnarlInterface::IsMessageVisible()
{
	return IsMessageVisible(m_nLastMessageId);
}

//-----------------------------------------------------------------------------
// snUpdateMessage()

/// Changes the title and text in the message specified by Id to the values
/// specified by Title and Text respectively. Id is the value returned by 
/// snShowMessage() or snShowMessageEx() when the notification was originally
/// created. To change the timeout parameter of a notification, use snSetTimeout()

SnarlInterface::M_RESULT SnarlInterface::UpdateMessage(LONG32 id, LPCSTR szTitle, LPCSTR szText, LPCSTR szIconPath)
{
	SNARLSTRUCT ss;
	ZeroMemory((void*)&ss, sizeof(ss));

	ss.Cmd = SNARL_UPDATE;
	ss.Id = id;
	
	StringCbCopyA((LPSTR)&ss.Title, SNARL_STRING_LENGTH, szTitle);
	StringCbCopyA((LPSTR)&ss.Text,  SNARL_STRING_LENGTH, szText);
	StringCbCopyA((LPSTR)&ss.Icon,  SNARL_STRING_LENGTH, szIconPath);

	return static_cast<M_RESULT>(uSend(ss));
}

SnarlInterface::M_RESULT SnarlInterface::UpdateMessage(LPCSTR szTitle, LPCSTR szText, LPCSTR szIconPath)
{
	return UpdateMessage(szTitle, szText, szIconPath);
}

//-----------------------------------------------------------------------------
// snRegisterConfig

/// Registers an application's configuration interface with Snarl.
/// AppName is the text that's displayed in the Applications list so it should
/// be people friendly ("My cool app" rather than "my_cool_app").

SnarlInterface::M_RESULT SnarlInterface::RegisterConfig(HWND hWnd, LPCSTR szAppName, LONG32 replyMsg)
{
	return RegisterConfig2(hWnd, szAppName, replyMsg, "");
}


//-----------------------------------------------------------------------------
// snRegisterConfig2

/// Registers an application's configuration interface with Snarl.
/// This function is identical to snRegisterConfig() except that Icon can be
/// used to specify a PNG image which will be displayed against the
/// application's entry in Snarl's Preferences panel.

SnarlInterface::M_RESULT SnarlInterface::RegisterConfig2(HWND hWnd, LPCSTR szAppName, LONG32 replyMsg, LPCSTR szIcon)
{
	if (!szAppName || !szIcon)
		return M_BAD_POINTER;

	SNARLSTRUCT ss;

	m_hwndFrom = hWnd;

	ss.Cmd = SNARL_REGISTER_CONFIG_WINDOW_2;
	ss.LngData2 = reinterpret_cast<LONG32>(hWnd);
	ss.Id = replyMsg;
	StringCbCopyA((LPSTR)&ss.Title, SNARL_STRING_LENGTH, szAppName);
	StringCbCopyA((LPSTR)&ss.Icon, SNARL_STRING_LENGTH, szIcon);

	return static_cast<M_RESULT>(uSend(ss));
}


//-----------------------------------------------------------------------------
// snRevokeConfig

/// Removes the application previously registered using snRegisterConfig() or
/// snRegisterConfig2(). hWnd should be the same as that used during registration.

SnarlInterface::M_RESULT SnarlInterface::RevokeConfig(HWND hWnd)
{
	SNARLSTRUCT ss;
	
	m_hwndFrom = NULL;

	ss.Cmd = SNARL_REVOKE_CONFIG_WINDOW;
	ss.LngData2 = reinterpret_cast<LONG32>(hWnd);

	return static_cast<M_RESULT>(uSend(ss));
}


//-----------------------------------------------------------------------------
// snGetVersion()

/// Checks if Snarl is currently running and, if it is, retrieves the major and
/// minor release version numbers in Major and Minor respectively.
/// Returns True if Snarl is running, False otherwise.

BOOL SnarlInterface::GetVersion(WORD* Major, WORD* Minor)
{
	SNARLSTRUCT ss;
	ss.Cmd = SNARL_GET_VERSION;
	LONG32 versionInfo = uSend(ss);
	if (versionInfo != M_FAILED && versionInfo != M_TIMED_OUT) {
		*Major = HIWORD(versionInfo);
		*Minor = LOWORD(versionInfo);
		return TRUE;
	}
	return FALSE;
}


//-----------------------------------------------------------------------------
// snGetVersionEx

/// Returns the Snarl system version number. This is an integer value which
/// represents the system build number and can be used to identify the specific
/// version of Snarl running

LONG32 SnarlInterface::GetVersionEx()
{
	SNARLSTRUCT ss;
	ss.Cmd = SNARL_GET_VERSION_EX;
	return uSend(ss);
}


//-----------------------------------------------------------------------------
// snSetTimeout()

/// Sets the timeout of existing notification Id to Timeout seconds. Id is the
/// value returned by snShowMessage() or snShowMessageEx() when the notification
/// was first created. 

SnarlInterface::M_RESULT SnarlInterface::SetTimeout(LONG32 Id, LONG32 Timeout)
{
	SNARLSTRUCT ss;
	ss.Cmd = SNARL_SET_TIMEOUT;
	ss.Id = Id;
	ss.LngData2 = Timeout;

	return static_cast<M_RESULT>(uSend(ss));
}

SnarlInterface::M_RESULT SnarlInterface::SetTimeout(LONG32 Timeout)
{
	return SetTimeout(m_nLastMessageId, Timeout);
}

//-----------------------------------------------------------------------------
// snRegisterAlert()

/// Registers an alert of Class for application AppName which must have previously
/// been registered with either snRegisterConfig() or snRegisterConfig2().

SnarlInterface::M_RESULT SnarlInterface::RegisterAlert(LPCSTR szAppName, LPCSTR szClass)
{
	SNARLSTRUCT ss;
	ss.Cmd = SNARL_REGISTER_ALERT;
	StringCbCopyA((LPSTR)&ss.Title, SNARL_STRING_LENGTH, szAppName);
	StringCbCopyA((LPSTR)&ss.Text, SNARL_STRING_LENGTH, szClass);

	return static_cast<M_RESULT>(uSend(ss));
}


//-----------------------------------------------------------------------------
// snGetGlobalMsg()

/// Returns the atom that corresponds to the "SnarlGlobalEvent" registered
/// Windows message. This message is sent by Snarl when it is first starts and
/// when it shuts down.

LONG32 SnarlInterface::GetGlobalMsg()
{
	return RegisterWindowMessage(SNARL_GLOBAL_MSG);
}


//-----------------------------------------------------------------------------
// snGetSnarlWindow

HWND SnarlInterface::GetSnarlWindow()
{
	return FindWindow(NULL, _T("Snarl"));
}


//-----------------------------------------------------------------------------
// snGetAppPath()

/// Returns a pointer to the path.
/// ** Remember to call FreeString

LPCTSTR SnarlInterface::GetAppPath()
{
	HWND hWnd = GetSnarlWindow();
	if (hWnd)
	{
		HWND hWndPath = FindWindowEx(hWnd, 0, _T("static"), NULL);
		if (hWndPath)
		{
			TCHAR strTmp[MAX_PATH] = {0};
			int nReturn = GetWindowText(hWndPath, strTmp, MAX_PATH);
			if (nReturn > 0) {
				TCHAR* strReturn = AllocateString(nReturn + 1);
				StringCchCopy(strReturn, nReturn + 1, strTmp);
				return strReturn;
			}
		}
	}
	return NULL;
}


//-----------------------------------------------------------------------------
// snGetIconsPath()

/// Returns a pointer to the iconpath.
/// ** Remember to call FreeString when done with the string

LPCTSTR SnarlInterface::GetIconsPath()
{
	TCHAR* szIconPath = NULL;
	LPCTSTR szPath = GetAppPath();
	if (!szPath)
		return NULL;

	size_t nLen = 0;
	if (SUCCEEDED(StringCbLength(szPath, MAX_PATH, &nLen)))
	{
		nLen += 10 + 1; // etc\\icons\\ + NULL
		szIconPath = AllocateString(nLen);

		StringCbCopy(szIconPath, nLen * sizeof(TCHAR), szPath);
		StringCbCat(szIconPath, nLen * sizeof(TCHAR), _T("etc\\icons\\"));
	}
	
	FreeString(szPath);

	return szIconPath;
}



//-----------------------------------------------------------------------------
// Private functions
//-----------------------------------------------------------------------------

LONG32 SnarlInterface::uSend(SNARLSTRUCT ss)
{
	DWORD_PTR nReturn = M_FAILED;

	HWND hWnd = GetSnarlWindow();
	if (IsWindow(hWnd))
	{
		COPYDATASTRUCT cds;
		cds.dwData = 2;
		cds.cbData = sizeof(ss);
		cds.lpData = &ss;

		if (SendMessageTimeout(hWnd, WM_COPYDATA, (WPARAM)m_hwndFrom, (LPARAM)&cds, SMTO_NORMAL, 1000, &nReturn) == 0)
		{
			if (GetLastError() == ERROR_TIMEOUT)
				nReturn = M_TIMED_OUT;
		}
	}

	return nReturn;
}


//-----------------------------------------------------------------------------

LONG32 SnarlInterface::uSendEx(SNARLSTRUCTEX ssex)
{
	DWORD_PTR nReturn = M_FAILED;

	HWND hWnd = GetSnarlWindow();
	if (IsWindow(hWnd))
	{
		COPYDATASTRUCT cds;
		cds.dwData = 2;
		cds.cbData = sizeof(ssex);
		cds.lpData = &ssex;

		if (SendMessageTimeout(hWnd, WM_COPYDATA, (WPARAM)m_hwndFrom, (LPARAM)&cds, SMTO_NORMAL, 1000, &nReturn) == 0)
		{
			if (GetLastError() == ERROR_TIMEOUT)
				nReturn = M_TIMED_OUT;
		}
	}

	return nReturn;
}


//-----------------------------------------------------------------------------