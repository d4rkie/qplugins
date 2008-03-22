//-----------------------------------------------------------------------------
//
// File:	QMPAsioCongig.h
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

#pragma once
#include "afxwin.h"

#define MAX_DEVICE_NAME_LEN		64

// CQMPAsioConfig dialog

class CQMPAsioConfig : public CDialog
{
public:
	CQMPAsioConfig(CWnd* pParent = NULL);   // standard constructor
	virtual ~CQMPAsioConfig();

	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnAsioControlClicked();

	enum { IDD = IDD_CONFIGURE };

	long LoadResString(PluginServiceFunc serviceFunc, HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax);
	void LoadSettings(void);
	void SaveSettings(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

// Dialog Data
public:
	// ASIO device selection
	CComboBox m_pDeviceCombo;
	CComboBox m_pBufferSizeCombo;

	// Current device selection
	int m_nDevice;

	// Device buffer size
	long	m_nChannelBufferSize;
	long	m_nBufferSizeFactor;
	int		m_nBufferSizeIndex;

	// Seamless (no drain or stop processed)
	BOOL	m_bSeamless;
	// Enable player volume control (optional)
	BOOL	m_bVolumeEnable;
};
