//-----------------------------------------------------------------------------
//
// File:	QMPAsioAbout.h
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


// CQMPAsioAbout dialog

class CQMPAsioAbout : public CDialog
{
public:
	CQMPAsioAbout(CWnd* pParent = NULL);   // standard constructor
	virtual ~CQMPAsioAbout();

// Dialog Data
	enum { IDD = IDD_ABOUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
public:

};
