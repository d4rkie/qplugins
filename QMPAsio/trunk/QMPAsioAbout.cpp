//-----------------------------------------------------------------------------
//
// File:	QMPAsioAbout.cpp
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

// QMPAsioAbout.cpp : implementation file
//

#include "stdafx.h"
#include "QMPAsio.h"
#include "QMPAsioAbout.h"


// CQMPAsioAbout dialog

CQMPAsioAbout::CQMPAsioAbout(CWnd* pParent /*=NULL*/)
	: CDialog(CQMPAsioAbout::IDD, pParent)
{

}

CQMPAsioAbout::~CQMPAsioAbout()
{
}

void CQMPAsioAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CQMPAsioAbout, CDialog)
END_MESSAGE_MAP()


// CQMPAsioAbout message handlers

BOOL CQMPAsioAbout::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	//
	// Load dialog text using QMP Resource manager
	//
	TCHAR	strBuf[128];

	asioApp.LoadResString(IDS_ASIOPLUGINTEXT, strBuf, 128);
	this->SetDlgItemText(IDC_ASIOPLUGINTEXT, strBuf);

	asioApp.LoadResString(IDS_STEINBERGTEXT, strBuf, 128);
	this->SetDlgItemText(IDC_STEINBERGTEXT, strBuf);

	asioApp.LoadResString(IDS_AUTHORTEXT, strBuf, 128);
	this->SetDlgItemText(IDC_AUTHORTEXT, strBuf);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQMPAsioAbout::PostNcDestroy()
{
	// Poof!
	delete this;
	asioApp.m_pAbout = NULL;

	return;
}

void CQMPAsioAbout::OnCancel()
{
	// Don't leak window handles
	this->DestroyWindow();

	return;
}

void CQMPAsioAbout::OnOK()
{
	// Don't leak window handles
	this->DestroyWindow();

	return;
}
