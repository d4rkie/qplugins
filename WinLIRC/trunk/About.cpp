// About.cpp : implementation file
//

#include "stdafx.h"
#include "WinLIRC.h"
#include "About.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAbout dialog


CAbout::CAbout(CWnd* pParent /*=NULL*/)
	: CDialog(CAbout::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAbout)
	m_strEdit1 = _T("");
	//}}AFX_DATA_INIT
}


void CAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAbout)
	DDX_Text(pDX, IDC_ABOUT_EDIT1, m_strEdit1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAbout, CDialog)
	//{{AFX_MSG_MAP(CAbout)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAbout message handlers

void CAbout::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	m_strEdit1 =
		_T(
		"WinLIRC plug-in by\r\nToke Noer\r\n"
		"toke@noer.it\r\n\r\n"
		"Feel free to write to the above address if you find bugs or have feature requests."
		"The forum on the QCD site is also a good place to post bug reports etc.!\r\n\r\n"
		"This plug-in is created on request from some of the QCD forum users."
		"I do not own an IR receiver myself at the moment, so please report all bugs found so they can be fixed.\r\n\r\n"
		"I hope someone finds the plug-in usefull! :-)"
		);
	UpdateData(FALSE);
}