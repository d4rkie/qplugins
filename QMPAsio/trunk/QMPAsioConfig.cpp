//-----------------------------------------------------------------------------
//
// File:	QMPAsioConfig.cpp
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

// QMPAsioConfig.cpp : implementation file
//

#include "stdafx.h"
#include "QMPAsio.h"
#include "QMPAsioConfig.h"

static short BufferSizeChoices[] = { 512, 1024, 2048, 4096, 8192, 16384 };

#define BUFSIZE_CHOICES	(sizeof(BufferSizeChoices) / sizeof(short))
// From BufferSizeChoices array
#define BUFSIZE_DEFAULT	2048
#define BUFSIZE_INDEX_DEFAULT	2

// CQMPAsioConfig dialog

CQMPAsioConfig::CQMPAsioConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CQMPAsioConfig::IDD, pParent)
	, m_nDevice(0)
	, m_nChannelBufferSize(BUFSIZE_DEFAULT)
	, m_nBufferSizeIndex(BUFSIZE_INDEX_DEFAULT)
	, m_bSeamless(FALSE)
	, m_bVolumeEnable(TRUE)
{
	m_nBufferSizeFactor = 0;
}

CQMPAsioConfig::~CQMPAsioConfig()
{
}

void CQMPAsioConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICECOMBO, m_pDeviceCombo);
	DDX_CBIndex(pDX, IDC_DEVICECOMBO, m_nDevice);
	DDX_Control(pDX, IDC_BUFFERSIZECOMBO, m_pBufferSizeCombo);
	DDX_CBIndex(pDX, IDC_BUFFERSIZECOMBO, m_nBufferSizeIndex);
	DDX_Check(pDX, IDC_SEAMLESS, m_bSeamless);
	DDX_Check(pDX, IDC_VOLUMEENABLE, m_bVolumeEnable);
}


BEGIN_MESSAGE_MAP(CQMPAsioConfig, CDialog)
	ON_WM_ACTIVATE()
	ON_BN_CLICKED(IDC_ASIOCONTROLBUTTON, &CQMPAsioConfig::OnAsioControlClicked)
END_MESSAGE_MAP()

// CQMPAsioConfig message handlers

BOOL CQMPAsioConfig::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	//
	// Load dialog text using QMP Resource manager
	//
	TCHAR	strBuf[128];

	asioApp.LoadResString(IDS_ASIOPLAYBACKCONFIG, strBuf, 128);
	this->SetDlgItemText(IDC_ASIOPLAYBACKCONFIG, strBuf);

	asioApp.LoadResString(IDS_CHANNELBUFFERSIZETEXT, strBuf, 128);
	this->SetDlgItemText(IDC_CHANNELBUFFERSIZETEXT, strBuf);

	asioApp.LoadResString(IDS_CHOOSEDEVICETEXT, strBuf, 128);
	this->SetDlgItemText(IDC_CHOOSEDEVICETEXT, strBuf);

	asioApp.LoadResString(IDS_ASIOCONTROLBUTTONTEXT, strBuf, 128);
	this->SetDlgItemText(IDC_ASIOCONTROLBUTTON, strBuf);

	asioApp.LoadResString(IDS_SEAMLESSTEXT, strBuf, 128);
	this->SetDlgItemText(IDC_SEAMLESS, strBuf);

	asioApp.LoadResString(IDS_VOLUMEENABLETEXT, strBuf, 128);
	this->SetDlgItemText(IDC_VOLUMEENABLE, strBuf);

	const int nMaxDevice = asioApp.m_pAsioDrivers->asioGetNumDev();

	for(int idx = 0; idx < nMaxDevice; idx++)
	{
		char	sDeviceName[MAX_DEVICE_NAME_LEN];
		TCHAR	sDeviceNameW[MAX_DEVICE_NAME_LEN];
		size_t	res;

		asioApp.m_pAsioDrivers->asioGetDriverName(idx, sDeviceName, MAX_DEVICE_NAME_LEN);
		mbstowcs_s(&res, sDeviceNameW, MAX_DEVICE_NAME_LEN, sDeviceName, _TRUNCATE);
		m_pDeviceCombo.AddString(sDeviceNameW);
	}

	// Populate buffer size pick-list
	for (int k = 0; k < BUFSIZE_CHOICES; k++)
	{
		TCHAR	buf[10];
	
		_itot_s(BufferSizeChoices[k], buf, sizeof(buf)/sizeof(TCHAR), 10);
		this->m_pBufferSizeCombo.AddString(buf);
	}

	this->m_pDeviceCombo.SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQMPAsioConfig::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);

	this->m_pDeviceCombo.SetCurSel(m_nDevice);

	this->m_pBufferSizeCombo.SetCurSel(m_nBufferSizeIndex);

	CString	fmtMsg((LPCTSTR)IDS_UNDERRUNSMSG);
	CString msgBuf;
	msgBuf.Format(fmtMsg, asioApp.m_pAsioHost->GetUnderrunCount());
	this->SetDlgItemText(IDC_UNDERRUNS, msgBuf);

	fmtMsg.LoadString(IDS_LATENCYMSG);
	// Calc ms from samples
	msgBuf.Format(fmtMsg, (1000 * asioApp.m_pAsioHost->GetDeviceLatency()) / asioApp.m_pAsioHost->GetSampleRate());
	this->SetDlgItemText(IDC_ASIOLATENCY, msgBuf);

	// Update dialog with current info
	this->UpdateData(FALSE);

	return;
}

// Load module config settings
void CQMPAsioConfig::LoadSettings(void)
{
	TCHAR IniName[MAX_PATH], appname[MAX_PATH];

	asioApp.LoadResString(IDS_MODULENAME, appname, MAX_PATH);
	asioApp.m_pQCDCallbacks->Service(opGetPluginSettingsFile, IniName, sizeof(IniName), 0);

	this->m_nDevice = GetPrivateProfileInt(appname, _T("ASIODevice"), 0, IniName);
	this->m_nChannelBufferSize = GetPrivateProfileInt(appname, _T("BufferSize"), BUFSIZE_DEFAULT, IniName);
	this->m_bSeamless = GetPrivateProfileInt(appname, _T("EnableSeamless"), 0, IniName);
	this->m_bVolumeEnable = GetPrivateProfileInt(appname, _T("EnableVolumeControl"), 1, IniName);

	// Lookup in table for index
	for (int k = 0; k < BUFSIZE_CHOICES; k++)
	{
		if (this->m_nChannelBufferSize == BufferSizeChoices[k])
		{
			this->m_nBufferSizeIndex = k;
			this->m_nBufferSizeFactor = this->m_nChannelBufferSize / 512;
			break;
		}
	}

	return;
}

// Save module config settings
void CQMPAsioConfig::SaveSettings(void)
{
	TCHAR IniName[MAX_PATH], appname[MAX_PATH], buf[32];

	asioApp.LoadResString(IDS_MODULENAME, appname, MAX_PATH);
	asioApp.m_pQCDCallbacks->Service(opGetPluginSettingsFile, IniName, sizeof(IniName), 0);

	_itot_s(this->m_nDevice, buf, sizeof(buf)/sizeof(TCHAR), 10);
	WritePrivateProfileString(appname, _T("ASIODevice"), buf, IniName);

	this->m_nChannelBufferSize = BufferSizeChoices[this->m_nBufferSizeIndex];
	this->m_nBufferSizeFactor = this->m_nChannelBufferSize / 512;
	_itot_s(this->m_nChannelBufferSize, buf, sizeof(buf)/sizeof(TCHAR), 10);
	WritePrivateProfileString(appname, _T("BufferSize"), buf, IniName);

	_itot_s(this->m_bSeamless, buf, 2, 10);
	WritePrivateProfileString(appname, _T("EnableSeamless"), buf, IniName);

	_itot_s(this->m_bVolumeEnable, buf, 2, 10);
	WritePrivateProfileString(appname, _T("EnableVolumeControl"), buf, IniName);

	return;
}

//
// Open ASIO control panel
//
void CQMPAsioConfig::OnAsioControlClicked()
{
	if (asioApp.m_pAsioHost)
	{
		// Open device if necessary
		if (!asioApp.m_pAsioHost->IsDeviceInited())
			asioApp.m_pAsioHost->OpenDevice();

		// Bring up control panel
		ASIOControlPanel();	
	}
}
