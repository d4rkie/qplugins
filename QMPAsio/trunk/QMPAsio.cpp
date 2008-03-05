//-----------------------------------------------------------------------------
//
// File:	QMPAsio.cpp
//
// About:	ASIO Playback interface for QMP 5.0
//
// Authors:	Written by Ted Hess
//
//	QMP multimedia player application Software Development Kit Release 5.0.
//
//	This code is free.  If you redistribute it in any form, leave this notice 
//	here.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "QMPAsio.h"

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

static TCHAR QMPAsioVersion[48];

CQMPAsioApp		asioApp;

BOOL CALLBACK ConfigDlgProcWrapper(HWND, UINT, WPARAM, LPARAM);

// CQMPAsioApp

BEGIN_MESSAGE_MAP(CQMPAsioApp, CWinApp)
END_MESSAGE_MAP()


// CQMPAsioApp construction

CQMPAsioApp::CQMPAsioApp()
{
	this->m_pQCDCallbacks = NULL;
	this->m_pConfig = NULL;
	this->m_pAbout = NULL;
	this->m_pAsioHost = NULL;
	this->m_hQCDWin = NULL;

	this->m_nPosition = 0;
}


// CQMPAsioApp initialization

BOOL CQMPAsioApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

//-----------------------------------------------------------------------------

PLUGIN_API QCDModInitPlay2* PLAYBACK2_DLL_ENTRY_POINT()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!asioApp.m_pQCDCallbacks)
		asioApp.m_pQCDCallbacks = new QCDModInitPlay2;

	asioApp.m_pQCDCallbacks->size						= sizeof(QCDModInitPlay2);
	asioApp.m_pQCDCallbacks->version					= PLUGIN_API_VERSION_UNICODE;

	asioApp.m_pQCDCallbacks->toModule.Initialize		= Initialize;
	asioApp.m_pQCDCallbacks->toModule.ShutDown			= ShutDown;
	asioApp.m_pQCDCallbacks->toModule.Open				= Open;
	asioApp.m_pQCDCallbacks->toModule.Write				= Write;
	asioApp.m_pQCDCallbacks->toModule.Flush				= Flush;
	asioApp.m_pQCDCallbacks->toModule.Pause				= Pause;
	asioApp.m_pQCDCallbacks->toModule.Stop				= Stop;
	asioApp.m_pQCDCallbacks->toModule.Drain				= Drain;
	asioApp.m_pQCDCallbacks->toModule.DrainCancel		= DrainCancel;
	asioApp.m_pQCDCallbacks->toModule.SetVolume			= SetVolume;
	asioApp.m_pQCDCallbacks->toModule.SetEQ				= NULL;
	asioApp.m_pQCDCallbacks->toModule.GetCurrentPosition= GetCurrentPosition;
	asioApp.m_pQCDCallbacks->toModule.About				= About;
	asioApp.m_pQCDCallbacks->toModule.Configure			= Configure;
	asioApp.m_pQCDCallbacks->toModule.TestFormat		= NULL;			// Unused?

	return asioApp.m_pQCDCallbacks;
}

int Initialize(QCDModInfo *ModInfo, int flags)
{
	PluginPrefPage prefPage;
	TCHAR	strBuf[128];

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	asioApp.m_pAsioDrivers = new AsioDrivers;
	asioApp.m_pAsioHost = new CAsioHost;
	
	asioApp.m_hQCDWin = (HWND)asioApp.m_pQCDCallbacks->Service(opGetParentWnd, NULL, 0, 0);

	asioApp.LoadResString(IDS_ASIOPLUGINTEXT, QMPAsioVersion, 48);

	// Note: ModInfo expects pointer to reference UNICODE when specified by PLUGIN_API_VERSION_UNICODE 
	ModInfo->moduleString      = (char *)QMPAsioVersion;
	ModInfo->moduleCategory    = (char *)_T("audio");
	
	// Need a config object for read/save params
	if (asioApp.m_pConfig == NULL)
	{
		asioApp.m_pConfig = new CQMPAsioConfig;
	}

	// Read .ini file settings
	asioApp.m_pConfig->LoadSettings();

	//
	// Build a preference page object
	//
	prefPage.struct_size = sizeof(prefPage);
	prefPage.hModule = asioApp.m_hInstance;
	prefPage.hModuleParent = 0;
	prefPage.lpTemplate = MAKEINTRESOURCEW(IDD_DIALOGFRAME);

	asioApp.LoadResString(IDS_PREFPAGEDISPLAYTEXT, strBuf, 128);
	prefPage.lpDisplayText = strBuf;

	prefPage.lpDialogFunc = ConfigDlgProcWrapper;
	prefPage.nCategory = PREFPAGE_CATEGORY_PLAYBACK;
	asioApp.m_nPrefPageID = asioApp.m_pQCDCallbacks->Service(opSetPluginPage, &prefPage, 0, 0);

#if defined(_DEBUG)
	// Easy access to config in DEBUG mode
	asioApp.LoadResString(IDS_PREFPAGEMENUTEXT, strBuf, 48);
	asioApp.m_pQCDCallbacks->Service(opSetPluginMenuItem, asioApp.m_hInstance, 0, (long)strBuf);
#endif

	// return TRUE for successful initialization
	return TRUE;
}

//-----------------------------------------------------------------------------

void ShutDown(int flags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Unload driver
	asioApp.m_pAsioHost->CloseDevice(TRUE);

	// Save settings and destroy config page
	if (asioApp.m_pConfig)
	{
		asioApp.m_pConfig->SaveSettings();

		delete asioApp.m_pConfig;
		asioApp.m_pConfig = NULL;
	}

	return;
}

//-----------------------------------------------------------------------------

//Open and init ASIO
BOOL Open(LPCSTR medianame, WAVEFORMATEX *wf)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
#if defined(_DEBUG)
	CString fmtMsg;
	fmtMsg.Format(_T("-> Open: Rate = %d, Size = %d, Format = %d\n"), wf->nSamplesPerSec, wf->wBitsPerSample, wf->wFormatTag);
	OutputDebugString((LPCTSTR)fmtMsg);
#endif

	// Sync volume with player
	int leftVol = asioApp.m_pQCDCallbacks->Service(opGetVolume, NULL, 1, 0);
	int rightVol = asioApp.m_pQCDCallbacks->Service(opGetVolume, NULL, 2, 0);
	asioApp.m_pAsioHost->DoSetVolume(leftVol, rightVol);

	// Starting at 0
	asioApp.m_pQCDCallbacks->toPlayer.PositionUpdate(0);

	return asioApp.m_pAsioHost->DoOpen(wf->nSamplesPerSec, wf->wBitsPerSample, wf->nChannels, wf->wFormatTag);
}

//-----------------------------------------------------------------------------

// Output data - Start ASIO device if idle
BOOL Write(WriteDataStruct* writeData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (asioApp.m_pAsioHost->IsDeviceOpen())
	{
		// We are open - is there data?
		if (writeData->bytelen)
		{
			// Return when all data sent to device (or at lease queued up)
			asioApp.m_pAsioHost->DoWrite(writeData);
			// Keep track of position ignoring driver latency
			asioApp.m_nPosition = writeData->markerstart;
			asioApp.m_pQCDCallbacks->toPlayer.PositionUpdate(asioApp.m_nPosition);
		}

		// Can display now ignoring driver latency
		return OUTPUT_OK_TO_VIS;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------

BOOL Flush(UINT marker)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

#if defined(_DEBUG)
	OutputDebugString(_T("-> Flush called\n"));
#endif

	// Reset all buffers pending next Write
	asioApp.m_pAsioHost->DoFlush(marker);

	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL Drain(int flags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

#if defined(_DEBUG)
	OutputDebugString(_T("-> Drain called\n"));
#endif

	// Wait for all data to be rendered
	asioApp.m_pAsioHost->DoDrain(flags);

	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL DrainCancel(int flags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

#if defined(_DEBUG)
	OutputDebugString(_T("-> DrainCancel called\n"));
#endif

	// Cancel drain and return
	asioApp.m_pAsioHost->DoDrainCancel(flags);

	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL Stop(int flags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

#if defined(_DEBUG)
	CString fmtMsg;
	fmtMsg.Format(_T("-> Stop(%d) called\n"), flags);
	OutputDebugString((LPCTSTR)fmtMsg);
#endif

	// Special handling of PLAYDONE for seamless playback
	if (asioApp.m_pConfig->m_bSeamless && (flags == STOPFLAG_PLAYDONE))
		return FALSE;

#if 0
	// Get a bunch of player state information
	int	didCycle;
	DWORD nTracks = asioApp.m_pQCDCallbacks->Service(opGetNumTracks, NULL, 0, 0);
	DWORD nIndex = asioApp.m_pQCDCallbacks->Service(opGetCurrentIndex, NULL, 0, 0) + 1;
	DWORD nNext = asioApp.m_pQCDCallbacks->Service(opGetNextIndex, NULL, -1, (long)&didCycle);
	BOOL bRepeatOff = (asioApp.m_pQCDCallbacks->Service(opGetRepeatState, NULL, 0, 0) == 0);
	BOOL bShuffleOn = (asioApp.m_pQCDCallbacks->Service(opGetShuffleState, NULL, 0, 0) != 0);

	// Check for last track or cycled and repeat off
	if (flags == STOPFLAG_PLAYDONE)
	{
		// Not repeating
		if (bRepeatOff)
		{
			// If shuffle, didCycle shows we are done
			if (bShuffleOn)
			{
				if (didCycle == 1)
				{
					flags = STOPFLAG_ALLDONE;
				}
			} else {
				// Otherwise, are we at end of list
				if (nTracks == nIndex)
				{
					flags = STOPFLAG_ALLDONE;
				}
			}
		}
	}
#endif

	// Stop ASIO driver or just stop processing buffers (checks flags)
	asioApp.m_pAsioHost->DoStop(flags);

	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL Pause(int flag)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Stop or resume playback
	asioApp.m_pAsioHost->DoPause(flag);

	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL SetVolume(int levelleft, int levelright, int flags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	//
	// Set volume and balance level (levelleft, levelright = 0 --> 100)
	// Balance is determined by ratio of two levels
	//
	asioApp.m_pAsioHost->DoSetVolume(levelleft, levelright);

	return TRUE;
}

//-----------------------------------------------------------------------------

BOOL GetCurrentPosition(UINT *position, int flags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	*position = asioApp.m_nPosition;

	return TRUE;
}

//-----------------------------------------------------------------------------

void Configure(int flags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (asioApp.m_nPrefPageID != 0)
		asioApp.m_pQCDCallbacks->Service(opShowPluginPage, asioApp.m_hInstance, asioApp.m_nPrefPageID, 0);

	return;
}

//-----------------------------------------------------------------------------

void About(int flags)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HWND hwndParent = (HWND)asioApp.m_pQCDCallbacks->Service(opGetPropertiesWnd, 0, 0, 0);

	// Create about dialog if none
	if (asioApp.m_pAbout == NULL)
	{
		asioApp.m_pAbout = new CQMPAsioAbout();

		ResInfo resInfo = { sizeof(ResInfo), asioApp.m_hInstance, MAKEINTRESOURCEW(IDD_ABOUT), 0, 0 };
		LPCDLGTEMPLATE hTemplate = (LPCDLGTEMPLATE)asioApp.m_pQCDCallbacks->Service(opLoadResDialog, (void*)&resInfo, 0, 0);
		_ASSERT(hTemplate);

		asioApp.m_pAbout->CreateIndirect(hTemplate, CWnd::FromHandle(hwndParent));
	}

	// Show modeless (will self destruct)
	asioApp.m_pAbout->ShowWindow(SW_SHOW);

	return;
}

//-----------------------------------------------------------------------------

//
// LoadResString
//
// Fetch string resource from module
//
void CQMPAsioApp::LoadResString(UINT uID, LPTSTR lpBuffer, int nBufferMax)
{
	_ASSERT(lpBuffer);
	_ASSERT(nBufferMax > 0);

	ResInfo resInfo = { sizeof(ResInfo), this->m_hInstance, MAKEINTRESOURCEW(uID), 0, 0 };
	lpBuffer[0] = L'\0';
	this->m_pQCDCallbacks->Service(opLoadResString, (void*)lpBuffer, (long)(nBufferMax * sizeof(TCHAR)), (long)&resInfo);

	return;
}

//-----------------------------------------------------------------------------

BOOL CALLBACK ConfigDlgProcWrapper(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	switch (msg)
	{
	case WM_INITDIALOG:
		if (asioApp.m_pConfig == NULL)
		{
			asioApp.m_pConfig = new CQMPAsioConfig();
			asioApp.m_pConfig->LoadSettings();
		}

		if (asioApp.m_pConfig->m_hWnd == NULL)
		{
			ResInfo resInfo = { sizeof(ResInfo), asioApp.m_hInstance, MAKEINTRESOURCEW(IDD_CONFIGURE), 0, 0 };
			LPCDLGTEMPLATE hTemplate = (LPCDLGTEMPLATE)asioApp.m_pQCDCallbacks->Service(opLoadResDialog, (void*)&resInfo, 0, 0);
			_ASSERT(hTemplate);

			asioApp.m_pConfig->CreateIndirect(hTemplate, CWnd::FromHandle(hwnd));
		}

		asioApp.m_pConfig->ShowWindow(SW_SHOW);

		return TRUE;
	
	case WM_CHILDACTIVATE:
		asioApp.m_pConfig->OnActivate(LOWORD(wParam), NULL, FALSE);
		return TRUE;

	case WM_PN_DIALOGSAVE:
		if (asioApp.m_pConfig)
		{
			asioApp.m_pConfig->UpdateData(TRUE);

			asioApp.m_pConfig->SaveSettings();

			 // Tear down window here
			if (asioApp.m_pConfig->m_hWnd)
				asioApp.m_pConfig->DestroyWindow();
		}

		return TRUE;
	}

	// Drain it
	return FALSE;
}
