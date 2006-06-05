//-----------------------------------------------------------------------------
//
// File:    WinLIRC.cpp
//
// About:   QCD Player WinLIRC client plug-in
//
// Authors: QCD plug-in by Toke Noer (toke@noer.it)
//          SDK written by Paul Quinn and Richard Carlson.
//
// License: This code may be freely modified, copied and distributed,
//			so long as no fee is charged for it. Also the source has to be
//			kept free if a binary version is distributed.
//
// Request: If anyone wants to make modifications/fixes/expansions based on
//			the code, please inform me first. (toke@noer.it) Maybe we can work
//			something out to keep a single "well written" plug-in.
//			
//-----------------------------------------------------------------------------
// QCD SDK notice:
// 
// QCD multimedia player application Software Development Kit Release 1.0.
//
// Copyright (C) 1997-2002 Quinnware
//
// This code is free.  If you redistribute it in any form, leave this notice here.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

/*----------------------------------------------------------------------------------------------------------------*\
 Version history:
  (   Date   ) Vers : By      : Description
  (2004-12-28) v1.2 : Toke    : *Default options button set to Add;
                                *Changed volume behaviour (uses getVolume+setVolume now);
								*Fixed mute behaviour when volume changed outside WinLIRC;
								+Added plug-in menu item to enable/disable plug-in;
								+Added options to suspend and hibernate the PC

  (2004-12-16) v1.1 : Toke    : *Only launch WinLIRC if not running;
                                +Added Shutdown PC option;
                                +WinLIRC now asks for path to WinLIRC if none is set;

  (2004-12-01) v1.0 : Toke    : Release


 Todo:

 Bugs:
	3. When 'send once' is ticked in action properties, it sends ones then pause then 6 times
	   more and only then stops sending.
	   I hava an idea here: 'send once' works as following: send once then pause, say for 500ms'
	   (changable), then continue sending till unpressed. Would be a more flexible control.	

\*----------------------------------------------------------------------------------------------------------------*/

#define _CREATELOG 1

#include "stdafx.h"

#define PLUGIN_MODULE_STRING	_T("WinLIRC v1.2")
#define CONNECT_STRING			_T("127.0.0.1")
#define CONNECT_PORT			8765

#include "..\..\includes\homemade\Log.h"

#include "..\..\includes\homemade\inifile.h"
#include "..\..\includes\homemade\ExitWindows.h"
#include "..\..\Includes\Win32Error.h"
#include "CfgFile.h"
#include "SocketError.h"
#include "MySocket.h"
#include "Config.h"
#include "About.h"
#include "WinLIRC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinLIRCApp

BEGIN_MESSAGE_MAP(CWinLIRCApp, CWinApp)
	//{{AFX_MSG_MAP(CWinLIRCApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinLIRCApp construction

CWinLIRCApp::CWinLIRCApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWinLIRCApp object
CWinLIRCApp theApp;

/////////////////////////////////////////////////////////////////////////////
// The log object
CLog* dlog;


/////////////////////////////////////////////////////////////////////////////
// CWinLIRCApp initialization

BOOL CWinLIRCApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	dlog = new CLog(_T("winlirc.log"));
	dlog->WriteLine("InitInstance");

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// QCD Interface
/////////////////////////////////////////////////////////////////////////////
HINSTANCE			hInstance;
HWND				hwndPlayer;
QCDModInitGen*		QCDCallbacks;
WNDPROC				QCDProc;

Settings			sSettings;

CMySocket*			pMySocket;
CSocketError*		pSocketError;
CPtrArray*			pArrCommands;
SpecialFunctions*	pSpecialFunc;
CConfig*			pConfigDlg;

PLUGIN_API BOOL GENERALDLL_ENTRY_POINT(QCDModInitGen *ModInit, QCDModInfo *ModInfo)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	ModInit->version				= PLUGIN_API_VERSION;
	ModInfo->moduleString			= const_cast<_TCHAR*>(PLUGIN_MODULE_STRING);

	ModInit->toModule.ShutDown		= ShutDown;
	ModInit->toModule.About			= About;
	ModInit->toModule.Configure		= Configure;

	QCDCallbacks = ModInit;

	hwndPlayer = (HWND)QCDCallbacks->Service(opGetParentWnd, 0, 0, 0);


	// TODO: all your plugin initialization here
	pSpecialFunc = new SpecialFunctions;
	pArrCommands = new CPtrArray();

	hInstance				= theApp.m_hInstance;
	pMySocket				= NULL;
	sSettings.bConnected	= FALSE;
	sSettings.bEnabled		= TRUE;
	pSpecialFunc->bMuted	= FALSE;

	ReadIniFile();
	ReadConfigFile();

	pSocketError = new CSocketError(hwndPlayer, sSettings.bShowErrorMessages, sSettings.bDebug);

	// Error check
	if (!pSocketError || !pArrCommands) {
		AfxMessageBox(_T("Init error!\nThe plug-in cannot start"));
		ShutDown(0);
		return FALSE;
	}

	// Subclass the player
	if ((QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDSubProc)) == 0) {
		AfxMessageBox(_T("Failed to subclass player!"), MB_OK, 0);
		return FALSE;
	}
	QCDCallbacks->Service(opSetPluginMenuItem, hInstance, ID_MENU, (long)"WinLIRC is enabled");
	
	pConfigDlg = new CConfig();		// Insert the preference page
	pConfigDlg->SetSettings(&sSettings);

	ConnectOnStart();	// Need to be called after socket etc. is created

	dlog->WriteLine("Init done");

	// return TRUE for successful initialization
	return TRUE;
}

//----------------------------------------------------------------------------

void ShutDown(int flags) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	QCDProc = (WNDPROC)SetWindowLong(hwndPlayer, GWL_WNDPROC, (LONG)QCDProc);
	
	SaveConfigFile();
	SaveIniFile();

	// Clean the Commands array
	for (int i = 0; i < pArrCommands->GetSize(); i++)
		delete ((Command*)pArrCommands->GetAt(i));

	delete pConfigDlg;
	delete pArrCommands;
	delete pMySocket;	// Closes open socket
	delete pSocketError;
	delete pSpecialFunc;

	dlog->WriteLine("Shutdown done");
	delete dlog;
}

//-----------------------------------------------------------------------------


void Configure(int flags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (flags == ID_MENU) {
		if (sSettings.bEnabled) {
			sSettings.bEnabled = FALSE;
			QCDCallbacks->Service(opSetPluginMenuState, hInstance, ID_MENU, 0);
			QCDCallbacks->Service(opSetPluginMenuItem, hInstance, ID_MENU, (long)"WinLIRC is disabled");
		}
		else {
			sSettings.bEnabled = TRUE;
			QCDCallbacks->Service(opSetPluginMenuState, hInstance, ID_MENU, 0);
			QCDCallbacks->Service(opSetPluginMenuItem, hInstance, ID_MENU, (long)"WinLIRC is enabled");
		}
	}
	else
		QCDCallbacks->Service(opShowPluginPage, theApp.m_hInstance, pConfigDlg->GetPageID(), 0);
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CAbout dlg;
	dlg.DoModal();
}

//-----------------------------------------------------------------------------
// LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
// QCD subclassing rutine
//-----------------------------------------------------------------------------

LRESULT CALLBACK QCDSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{ 
	switch (msg) {
		case WM_COMMAND : {
			if (wparam == QCD_COMMAND_VOLUP || wparam == QCD_COMMAND_VOLDOWN)
				pSpecialFunc->bMuted = FALSE;
			break;
		}
	}	// Switch
	return CallWindowProc(QCDProc, hwnd, msg, wparam, lparam); 
}

//-----------------------------------------------------------------------------
// BOOL Connect()
// Try to connect to WinLIRC
//-----------------------------------------------------------------------------

BOOL Connect()
{
	if (!sSettings.bConnected) {
		if (pMySocket)
			delete pMySocket;

		pMySocket = new CMySocket();

		if (!pMySocket->Create()) {
			CSocketError oTmpSockErr(hwndPlayer, true, sSettings.bDebug);
			pSocketError->DisplayError("MySocket->Create()", pMySocket->GetLastError());
			sSettings.bConnected = FALSE;
			delete pMySocket;
			pMySocket = NULL;

			return FALSE;
		}

		if (!pMySocket->Connect(CONNECT_STRING, CONNECT_PORT)) {
			pSocketError->DisplayError("Connect()", pMySocket->GetLastError());
			sSettings.bConnected = FALSE;
			delete pMySocket;
			pMySocket = NULL;

			return FALSE;
		}

		sSettings.bConnected = TRUE;
		return TRUE;
	}
	else
		return FALSE;
}

//-----------------------------------------------------------------------------
// ParseCommand(CString strCmd)
// The function that parses the command and sends it to QCD
//-----------------------------------------------------------------------------

void ParseCommand(CString strCmd, UINT iCount)
{
	if (!sSettings.bEnabled)
		return;

	dlog->Write(_T("Command: "));
	dlog->Write(const_cast<char*>(LPCTSTR(strCmd)));
	dlog->Write(_T("\tCount: "));
	dlog->Write(iCount);
	dlog->Flush();

	Command* pCmd;

	for (int i = 0; i < pArrCommands->GetSize(); i++)
	{
		pCmd = (Command*)pArrCommands->GetAt(i);
		if (strCmd.CompareNoCase(pCmd->strButton) == 0) {
			// The message is recognized
			if (pCmd->bSendOnce && iCount > 0)	// Lets do an early exit
				return;

			if (pCmd->iCommand == 0) {
			// Speciel functions

				// Volume commands made custom, bacause QCD_COMMAND_VOLUP isn't always 1%
				// and it needs to unset mute
				if (pCmd->strCommand.CompareNoCase(_T("Volume Up (+1)")) == 0 || 
						pCmd->strCommand.CompareNoCase(_T("Volume Up (+2)")) == 0 ||
						pCmd->strCommand.CompareNoCase(_T("Volume Up (+5)")) == 0 ||
						pCmd->strCommand.CompareNoCase(_T("Volume Up (+10)")) == 0)
				{
					pSpecialFunc->bMuted = FALSE;

					long iVol = QCDCallbacks->Service(opGetVolume, 0, 0, 0);
					iVol += pCmd->iRepeatCount;
					QCDCallbacks->Service(opSetVolume, 0, iVol, 0);
				}
				if (pCmd->strCommand.CompareNoCase(_T("Volume Down (-1)")) == 0 || 
						pCmd->strCommand.CompareNoCase(_T("Volume Down (-2)")) == 0 ||
						pCmd->strCommand.CompareNoCase(_T("Volume Down (-5)")) == 0 ||
						pCmd->strCommand.CompareNoCase(_T("Volume Down (-10)")) == 0)
				{
					pSpecialFunc->bMuted = FALSE;

					long iVol = QCDCallbacks->Service(opGetVolume, 0, 0, 0);
					iVol -= pCmd->iRepeatCount;
					QCDCallbacks->Service(opSetVolume, 0, iVol, 0);
				}

				// Mute
				if (pCmd->strCommand.CompareNoCase(_T("Mute")) == 0 && iCount == 0) {
					if (pSpecialFunc->bMuted) {		// Turn up volume
						QCDCallbacks->Service(opSetVolume, 0, pSpecialFunc->iVolume, 0);
						pSpecialFunc->bMuted = FALSE;
					}
					else {	// Turn down volume
						pSpecialFunc->iVolume = QCDCallbacks->Service(opGetVolume, 0, 0, 0);
						QCDCallbacks->Service(opSetVolume, 0, 0, 0);
						pSpecialFunc->bMuted = TRUE;
					}
					return;
				}

				// Shutdown PC
				if (pCmd->strCommand.CompareNoCase(_T("Shutdown PC")) == 0) {
					CExitWindows ew;
					if (ew.m_bWinNT)
						ew.ExitWindowsEx(EWX_POWEROFF | 0x10, 0);	// 0x10 == EWX_FORCEIFHUNG
					else {
						// Close the connection to WinLIRC
						pMySocket->Close();
						ew.ExitWindowsEx(EWX_SHUTDOWN, 0);
						// Exit QCD
						PostMessage(hwndPlayer, WM_COMMAND, 40003, 0);	// ID_MENU_EXIT
					}
				}

				// Suspend
				if (pCmd->strCommand.CompareNoCase(_T("Suspend PC")) == 0) {
					int iRet = (int)ShellExecute(hwndPlayer, "open", "rundll32.exe", "powrprof.dll,SetSuspendState", "%windir%\\System32", SW_SHOWNORMAL);
					if (iRet < 33)
						AfxMessageBox("Failed to suspend!");
				}

				// Hibernate
				if (pCmd->strCommand.CompareNoCase(_T("Hibernate PC")) == 0) {
					int iRet = (int)ShellExecute(hwndPlayer, "open", "rundll32.exe", "powrprof.dll,SetSuspendState Hibernate", "%windir%\\System32", SW_SHOWNORMAL);
					if (iRet < 33)
						AfxMessageBox("Failed to hibernate!");
				}
			}
			else {
				// Standard functions
				for (int j = 0; j < pCmd->iRepeatCount; j++)
					PostMessage(hwndPlayer, WM_COMMAND, pCmd->iCommand, 0);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// ReadConfigFile()
//-----------------------------------------------------------------------------

void ReadConfigFile()
{
	_TCHAR strTemp[MAX_PATH];
	
	QCDCallbacks->Service(opGetPluginFolder, strTemp, MAX_PATH, 0);
	_tcscat(strTemp, "\\winlirc.cfg");
	
	CCfgFile oFile(strTemp);
	oFile.Read();
}

//-----------------------------------------------------------------------------
// SaveConfigFile()
//-----------------------------------------------------------------------------

void SaveConfigFile()
{
	_TCHAR strTemp[MAX_PATH];
	
	QCDCallbacks->Service(opGetPluginFolder, strTemp, MAX_PATH, 0);
	_tcscat(strTemp, "\\winlirc.cfg");
	
	CCfgFile oFile(strTemp);
	oFile.Save();
}

//-----------------------------------------------------------------------------
// ReadIniFile()
//-----------------------------------------------------------------------------

void ReadIniFile()
{
	CHAR strIniFile[MAX_PATH];

	QCDCallbacks->Service(opGetPluginSettingsFile, &strIniFile, MAX_PATH, 0);
	CIniFile objIni(strIniFile, _T("WinLIRC"));

	sSettings.bShowErrorMessages	= objIni.GetBool(_T("ShowErrorMessages"), true);
	sSettings.bDebug				= objIni.GetBool(_T("Debug"), false);

	// Launch WinLIRC ???
	if (objIni.GetBool(_T("RunWinLIRC"), false)) {
		if (!IsWinLIRCRunning())
			LaunchWinLIRC();
	}
}

//-----------------------------------------------------------------------------
// SaveIniFile()
//-----------------------------------------------------------------------------

void SaveIniFile()
{
	CHAR strIniFile[MAX_PATH];

	QCDCallbacks->Service(opGetPluginSettingsFile, &strIniFile, MAX_PATH, 0);
	CIniFile objIni(strIniFile, _T("WinLIRC"));

	objIni.SetBool(_T("ShowErrorMessages"), sSettings.bShowErrorMessages);
	objIni.SetBool(_T("Debug"), sSettings.bDebug);

	// Save options not in memory
	_TCHAR strTmp[MAX_PATH];
	BOOL bTmp;
	objIni.GetString(_T("WinLIRC_Filename"), strTmp, _T("none"), MAX_PATH);
	objIni.SetString(_T("WinLIRC_Filename"), strTmp);
	objIni.GetString(_T("WinLIRC_Directory"), strTmp, _T("none"), MAX_PATH);
	objIni.SetString(_T("WinLIRC_Directory"), strTmp);

	bTmp = objIni.GetBool(_T("RunWinLIRC"), false);
	objIni.SetBool(_T("RunWinLIRC"), bTmp);

	bTmp = objIni.GetBool(_T("ConnectOnStart"), true);
	objIni.SetBool(_T("ConnectOnStart"), bTmp);

	objIni.Flush();
}

void ConnectOnStart()
{
	CHAR strIniFile[MAX_PATH];

	QCDCallbacks->Service(opGetPluginSettingsFile, &strIniFile, MAX_PATH, 0);
	CIniFile objIni(strIniFile, _T("WinLIRC"));

	if (objIni.GetBool(_T("ConnectOnStart"), true))
		Connect();
}

//-----------------------------------------------------------------------------
// Launch WinLIRC
// Returns: 1 = Success
//          0 = Failed to launch
//         -1 = Path not found
//-----------------------------------------------------------------------------

INT LaunchWinLIRC()
{
	_TCHAR strIniFile[MAX_PATH];
	_TCHAR strFile[MAX_PATH], strDir[MAX_PATH];

	QCDCallbacks->Service(opGetPluginSettingsFile, &strIniFile, MAX_PATH, 0);
	CIniFile objIni(strIniFile, _T("WinLIRC"));
	
	objIni.GetString(_T("WinLIRC_Filename"), strFile, _T("none"), MAX_PATH);
	objIni.GetString(_T("WinLIRC_Directory"), strDir, _T("none"), MAX_PATH);

	if (strFile && strDir && (_tcsncmp(_T("none"), strFile, MAX_PATH) != 0)
		&& (_tcsncmp(_T("none"), strDir, MAX_PATH) != 0))
	{
		INT iErr = (INT)ShellExecute(NULL, NULL, strFile, NULL, strDir, SW_SHOWDEFAULT);

		if (iErr < 33 && sSettings.bDebug) {
			DWORD iExtErr = CommDlgExtendedError();
			if (iExtErr > 0) {	// Not sure this will work
				CWin32Error e;
				::MessageBox(hwndPlayer, e, _T("Error"), MB_OK);
			}
			return 0;
		}
		else
			return 1;
	}
	else
		return -1;
}

//-----------------------------------------------------------------------------
// Find out if WinLIRC is running
//-----------------------------------------------------------------------------

BOOL IsWinLIRCRunning()
{
	FARPROC EnumProcInstance = MakeProcInstance((FARPROC)EnumWindowsProc, hInstance);

	BOOL bSuccess = false;
	// Call the EnumWindows function to start the iteration
	EnumWindows((WNDENUMPROC)EnumProcInstance, (LPARAM)&bSuccess);

	// Free up the allocated memory handle
	FreeProcInstance(EnumProcInstance);

	return bSuccess;
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
   	_TCHAR strBuffer[256];
	_TCHAR* strWinLIRC = _T("WinLIRC");

    // Get the window text to insert    
    GetWindowText(hWnd, strBuffer, 256);

    // Check if they match
	if (_tcsnicmp(strWinLIRC, strBuffer, 256) == 0) {
		*(BOOL*)lParam = TRUE;
		return FALSE;	// Stop iteration
	}

    return TRUE;	// Continue
}