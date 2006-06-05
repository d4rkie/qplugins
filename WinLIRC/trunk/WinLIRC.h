// WinLIRC.h : main header file for the WINLIRC DLL
//

#if !defined(AFX_WINLIRC_H__E47103E9_6296_407F_B45B_3FCCD1FFB81D__INCLUDED_)
#define AFX_WINLIRC_H__E47103E9_6296_407F_B45B_3FCCD1FFB81D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CWinLIRCApp
// See WinLIRC.cpp for the implementation of this class
//

class CWinLIRCApp : public CWinApp
{
public:
	CWinLIRCApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinLIRCApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CWinLIRCApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// QCD
/////////////////////////////////////////////////////////////////////////////

#ifndef QCDGeneralDLL_H
#define QCDGeneralDLL_H

#include "..\qcdpdk\QCDGeneral\QCDModGeneral.h"
#include "..\qcdpdk\QCDGeneral\QCDCtrlMsgs.h"

extern HINSTANCE		hInstance;
extern HWND				hwndPlayer;
extern QCDModInitGen*	QCDCallbacks;
extern CPtrArray*		pArrCommands;

// The variable struct
struct Settings
{
	BOOL	bConnected;
	BOOL	bDebug;	
	BOOL	bShowErrorMessages;
	BOOL	bEnabled;
};

struct SpecialFunctions
{
	// Mute
	BOOL	bMuted;
	INT		iVolume;
};

struct Command
{
	BOOL	bSendOnce;		// If this is true, commands should only be send when iCount == 0
	BOOL	bCustom;
	INT		iRepeatCount;	// Times to send to message. For example volume commands might wanna send more than once
	UINT	iCommand;		// The QCD_COMMAND_?
	CString strButton;		// The button as send by WinLIRC server
	CString strCommand;		// The internal name of the command
};

extern Settings sSettings;

// Calls from the Player
void Configure(int flags);
void About(int flags);
void ShutDown(int flags);

BOOL Connect();
void ParseCommand(CString strCmd, UINT iCount);
void ReadConfigFile();
void SaveConfigFile();
void ReadIniFile();
void SaveIniFile();
void ConnectOnStart();
INT LaunchWinLIRC();
BOOL IsWinLIRCRunning();
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#endif //QCDGeneralDLL_H


/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINLIRC_H__E47103E9_6296_407F_B45B_3FCCD1FFB81D__INCLUDED_)