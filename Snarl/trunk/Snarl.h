// Snarl.h
// Headerfile for Snarl - Ported by Toke Noer (toke@noer.it)
// API: http://www.k23productions.com/haiku/snarl/dev/

#if !defined(SNARL_H_INCLUDED_)
#define SNARL_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Windows.h>

// Registered window message and event identifiers (passed in wParam when either SNARL_GLOBAL_MSG or ReplyMsg is received)
#define SNARL_GLOBAL_MSG				_T("SnarlGlobalEvent")
#define SNARL_NOTIFICATION_CANCELLED	0
#define SNARL_LAUNCHED					1
#define SNARL_QUIT 						2

#define SNARL_NOTIFICATION_CLICKED 		32            // notification was right-clicked by user
#define SNARL_NOTIFICATION_TIMED_OUT 	33
#define SNARL_NOTIFICATION_ACK 			34            // notification was left-clicked by user

#define SNARL_BUFF_SIZE 1024

//Snarl Data Types
enum SNARL_COMMANDS {
    SNARL_SHOW        = 1,
    SNARL_HIDE        = 2,
    SNARL_UPDATE      = 3,
    SNARL_IS_VISIBLE  = 4,
    SNARL_GET_VERSION = 5,
    SNARL_REGISTER_CONFIG_WINDOW = 6,
    SNARL_REVOKE_CONFIG_WINDOW   = 7
};

struct SNARLSTRUCT {
    SNARL_COMMANDS	Cmd;				// What to do...
    int				Id;					// Message ID (returned by snShowMessage())
    int				Timeout;			// Timeout in seconds (0=sticky)
    int				LngData2;			// Reserved
    char			Title[SNARL_BUFF_SIZE];
    char			Text[SNARL_BUFF_SIZE];
    char			Icon[SNARL_BUFF_SIZE];
};

/*
 * Private utility functions:
 * 	_Send(TSnarlStruct) 
 *			Used by most public helper functions to send the WM_COPYDATA message.
 */
int _Send(SNARLSTRUCT pss)
{
	HWND hwnd;
	COPYDATASTRUCT pcd;
	
	// Will get a window class when snarl is released
	hwnd = FindWindow(NULL, _T("Snarl"));
	if (!hwnd)
		return NULL;

	pcd.dwData = 2;
    pcd.cbData = sizeof(pss);
    pcd.lpData = &pss;
    return SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM)&pcd);
}

/************************************************************
 * The Helper Functions
 ************************************************************/
 
int snShowMessage(char* strTitle, char* strText, int nTimeout = 0, char* strIconPath = "", HWND hwndReply = NULL, int nReplyMsg = 0)
{
	SNARLSTRUCT pss;
	ZeroMemory((void*)&pss, sizeof(pss));

	pss.Cmd = SNARL_SHOW;
	CopyMemory(&pss.Title, strTitle,    SNARL_BUFF_SIZE);
	CopyMemory(&pss.Text,  strText,     SNARL_BUFF_SIZE);
	CopyMemory(&pss.Icon,  strIconPath, SNARL_BUFF_SIZE);
	pss.Timeout = nTimeout;

	// R0.3
	pss.LngData2 = (int)hwndReply;
	pss.Id = nReplyMsg;
	return _Send(pss);
}

bool snUpdateMessage(int AId, char* strTitle, char* strText)
{
	SNARLSTRUCT pss;
	ZeroMemory((void*)&pss, sizeof(pss));

	pss.Id  = AId;
	pss.Cmd = SNARL_UPDATE;
	CopyMemory(&pss.Title, strTitle, SNARL_BUFF_SIZE);
	CopyMemory(&pss.Text,  strText,  SNARL_BUFF_SIZE);
	return _Send(pss);
}

bool snHideMessage(int AId)
{
	SNARLSTRUCT pss;
	ZeroMemory((void*)&pss, sizeof(pss));

	pss.Id  = AId;
	pss.Cmd = SNARL_HIDE;
	return _Send(pss);
}

bool snIsMessageVisible(int AId)
{
	SNARLSTRUCT pss;
	ZeroMemory((void*)&pss, sizeof(pss));

	pss.Id  = AId;
	pss.Cmd = SNARL_IS_VISIBLE;
	return _Send(pss);
}

bool snGetVersion(WORD* nMajor, WORD* nMinor)
{
	int hr = 0;
	SNARLSTRUCT pss;
	ZeroMemory((void*)&pss, sizeof(pss));

	pss.Cmd = SNARL_GET_VERSION;
	hr = _Send(pss);
	if (hr)
	{
		*nMajor = HIWORD(hr);
		*nMinor = LOWORD(hr);
	}
	return hr;
}

int snGetGlobalMsg()
{
	return RegisterWindowMessage(SNARL_GLOBAL_MSG);
}

int snRegisterConfig(HWND hwnd, char* strAppName, int nReplyMsg)
{
	SNARLSTRUCT pss;
	ZeroMemory((void*)&pss, sizeof(pss));

	pss.Cmd      = SNARL_REGISTER_CONFIG_WINDOW;
	pss.LngData2 = (int)hwnd;
	pss.Id       = nReplyMsg;
	CopyMemory(&pss.Title, strAppName, SNARL_BUFF_SIZE);
	return _Send(pss);
}

int snRevokeConfig(HWND hwnd)
{
	SNARLSTRUCT pss;
	ZeroMemory((void*)&pss, sizeof(pss));

	pss.Cmd      = SNARL_REVOKE_CONFIG_WINDOW;
	pss.LngData2 = (int)hwnd;
	return _Send(pss);
}

#endif // #if !defined(SNARL_H_INCLUDED_)