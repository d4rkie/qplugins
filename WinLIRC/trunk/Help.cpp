// Help.cpp : implementation file
//

#include "stdafx.h"
#include "WinLIRC.h"
#include "Help.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHelp dialog


CHelp::CHelp(CWnd* pParent /*=NULL*/)
	: CDialog(CHelp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHelp)
	m_strEdit1 = _T("");
	//}}AFX_DATA_INIT
}


void CHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHelp)
	DDX_Text(pDX, IDC_HELP_EDIT1, m_strEdit1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHelp, CDialog)
	//{{AFX_MSG_MAP(CHelp)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHelp message handlers

void CHelp::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	m_strEdit1 = _T(
		"Status:\r\n"
		"The status panel shows the current status of the connection and if WinLIRC is detected to run.\r\n"
		"If QCD isn't connected to WinLIRC and WinLIRC is running the Connect button can be used to connect.\r\n"
		"If the path to WinLIRC is probably set in the Options dialog, the Launch WinLIRC button can be used to start WinLIRC.\r\n\r\n\r\n"

		"Assigned buttons:\r\n"
		"The assigned buttons list show all the commands that QCD will listen for.\r\n"
		"To add new remote buttons press the Add button. "
		"Use the Remove button to remove commands and use the Modify (or doubleclick an item) to modify a command.\r\n\r\n\r\n"
		
		"Options:\r\n"
		"The options dialog shows the path to WinLIRC if set. "
		"This path has to be correct for the Launch WinLIRC button and the Launch WinLIRC on start option to work.\r\n"
		"If Connect on start is checked the plug-in will try to connect to the WinLIRC server on start.\r\n"
		"Unchecking the Show error messages checkbox will disable most error messages from the plug-in.\r\n"
		"Check the Debug messages if you want some more technical error messages. "
		"This can be usefull for people with connection problems.\r\n"
		"The Show error messages needs to be enabled for Debug messages to have an effect.\r\n\r\n\r\n"

		"Help to custom commands:\r\n"
		"When a custom command is made the plug-in will send the command present in the WM_COMMAND field to QCD when the assigned remote button is pressed.\r\n"
		"This is usefull if a function is present in the QCD SDK but the plug-in hasn't support for it.\r\n\r\n"
		"The Remote button field has to be set to the string that WinLIRC sends when a given button is pressed.\r\n"
		"The Command name should be set to a descriptive text of the command.\r\n"
		"Repeat count tells the plug-in how many times the command should be send every time the custom button is pressed.\r\n"
		"The Send once property means that the message is only sent to QCD one time if you hold down the remote button."
		"This is usefull to enable for functions like track forward etc. while a function like volume up works better if it is disabled."
		);
	UpdateData(FALSE);
}