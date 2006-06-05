// ConfigOptions.cpp : implementation file
//

#include "stdafx.h"
#include "WinLIRC.h"
#include "..\..\Includes\Win32Error.h"
#include "..\..\Includes\Homemade\Inifile.h"
#include "ConfigOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigOptions dialog


CConfigOptions::CConfigOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigOptions)
	m_bDebugMessages = FALSE;
	m_bShowErrorMessages = FALSE;
	m_bLanchWinLIRC = FALSE;
	m_bConnectOnStart = FALSE;
	//}}AFX_DATA_INIT
}


void CConfigOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigOptions)
	DDX_Control(pDX, IDC_OPTIONS_PATH, m_cPath);
	DDX_Check(pDX, IDC_OPTIONS_DEBUGMESSAGES, m_bDebugMessages);
	DDX_Check(pDX, IDC_OPTIONS_SHOWERRORS, m_bShowErrorMessages);
	DDX_Check(pDX, IDC_OPTIONS_LAUNCH_WINLIRC, m_bLanchWinLIRC);
	DDX_Check(pDX, IDC_OPTIONS_CONNECT_ON_START, m_bConnectOnStart);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigOptions, CDialog)
	//{{AFX_MSG_MAP(CConfigOptions)
	ON_BN_CLICKED(IDC_OPTIONS_BROWSE_BTN, OnOptionsBrowseBtn)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigOptions message handlers

void CConfigOptions::OnOK() 
{
	UpdateData(TRUE);

	// Save options not in memory
	// Remember to add to SaveIniFile() in WinLIRC.cpp

	CHAR strInifile[MAX_PATH];
	QCDCallbacks->Service(opGetPluginSettingsFile, strInifile, MAX_PATH, 0);

	if (m_bLanchWinLIRC)
		WritePrivateProfileString(_T("WinLIRC"), "RunWinLIRC", _T("true"), strInifile);
	else
		WritePrivateProfileString(_T("WinLIRC"), "RunWinLIRC", _T("false"), strInifile);

	// Connect on start
	if (m_bConnectOnStart)
		WritePrivateProfileString(_T("WinLIRC"), "ConnectOnStart", _T("true"), strInifile);
	else
		WritePrivateProfileString(_T("WinLIRC"), "ConnectOnStart", _T("false"), strInifile);

	
	CDialog::OnOK();
}

void CConfigOptions::OnOptionsBrowseBtn() 
{
	OPENFILENAME ofn;
	_TCHAR strPath[_MAX_PATH];

	ZeroMemory(strPath, _MAX_PATH);
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize		= sizeof(ofn);
	ofn.hwndOwner		= hwndPlayer;
	ofn.lpstrFilter		= "winlirc.exe\0winlirc.exe\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrTitle		= _T("Specify location of winlirc.exe");
	ofn.Flags			= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrFile		= strPath;
	ofn.nMaxFile		= sizeof(strPath);

	if (GetOpenFileName(&ofn)) {
		_TCHAR* strFile = new _TCHAR[_tcslen(ofn.lpstrFile) - ofn.nFileOffset + sizeof(_TCHAR) + 1];
		_TCHAR* strDirectory = new _TCHAR[_tcslen(ofn.lpstrFile) + sizeof(_TCHAR)];
		
		ZeroMemory(strFile, _tcslen(ofn.lpstrFile) - ofn.nFileOffset + sizeof(_TCHAR));
		ZeroMemory(strDirectory, _tcslen(ofn.lpstrFile) + sizeof(_TCHAR));

		_tcsncpy(strDirectory, strPath, ofn.nFileOffset - 1);
		_tcscpy(strFile, strPath + ofn.nFileOffset);

		// Save to INI
		CHAR strInifile[MAX_PATH];
		CHAR strTmp[MAX_PATH];
		QCDCallbacks->Service(opGetPluginSettingsFile, strInifile, MAX_PATH, 0);

		wsprintf(strTmp, "%s", strFile);
		WritePrivateProfileString(_T("WinLIRC"), "WinLIRC_Filename", strTmp, strInifile);
		wsprintf(strTmp, "%s", strDirectory);
		WritePrivateProfileString(_T("WinLIRC"), "WinLIRC_Directory", strTmp, strInifile);

		// Show new path
		CString strPath;
		strPath = strDirectory;
		strPath += "\\";
		strPath += strFile;
		this->SetDlgItemText(IDC_OPTIONS_PATH, strPath);
		
		// Clean up
		delete [] strFile;
		delete [] strDirectory;
	}
	else {
		DWORD iExtErr = CommDlgExtendedError();
		if (iExtErr > 0) {	// Not sure this will work
			CWin32Error e;
			::MessageBox(hwndPlayer, e, _T("Error"), MB_OK);
		}
	}	
}

void CConfigOptions::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	// Todo
	_TCHAR strInifile[MAX_PATH];
	_TCHAR strTmp[MAX_PATH];
	CString strPath;

	QCDCallbacks->Service(opGetPluginSettingsFile, strInifile, MAX_PATH, 0);

	CIniFile objIni(strInifile, _T("WinLIRC"));

	// Path of WinLIRC
	objIni.GetString(_T("WinLIRC_Directory"), strTmp, _T("none"), MAX_PATH);
	strPath = strTmp;
	strPath += "\\";
	objIni.GetString(_T("WinLIRC_Filename"), strTmp, _T("none"), MAX_PATH);	
	strPath += strTmp;
	if (strPath.CompareNoCase("none\\none") != 0)
		this->SetDlgItemText(IDC_OPTIONS_PATH, strPath);
	else
		this->SetDlgItemText(IDC_OPTIONS_PATH, _T(""));

	m_bLanchWinLIRC		= objIni.GetBool(_T("RunWinLIRC"), false);	
	m_bConnectOnStart	= objIni.GetBool(_T("ConnectOnStart"), true);	
	
	UpdateData(FALSE);
}

void CConfigOptions::BrowseForWinLIRC()
{
	OnOptionsBrowseBtn();
}