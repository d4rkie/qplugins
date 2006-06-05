// MySocket.cpp : implementation file
//

#include "stdafx.h"
#include "WinLIRC.h"
#include "MySocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMySocket

CMySocket::CMySocket()
{
}

CMySocket::~CMySocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CMySocket, CSocket)
	//{{AFX_MSG_MAP(CMySocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CMySocket member functions

/*
Note	In CSocket, the OnConnect notification function is never called.
		For connections, you call Connect, which will return when the connection
		is completed (either successfully or in error).
		How connection notifications are handled is an MFC implementation detail.
*/

void CMySocket::OnClose(int nErrorCode) 
{
	// MessageBox(NULL, "OnClose()", "QCD WinLIRC", MB_OK);

	// This function is called on a socket to signal that the application on the other end
	// of the connection has closed its socket or that the connection has been lost.
	// This should be followed by closing the socket that received this notification.
	if (nErrorCode != 0)
	{
		CString strErr = "";
		switch (nErrorCode)
		{
			case WSAENETDOWN		: strErr = "WSAENETDOWN";
			case WSAECONNRESET		: strErr = "WSAECONNRESET";
			case WSAECONNABORTED	: strErr = "WSAECONNABORTED";
			default : strErr = "Unknown!";
		}

		if (strErr != "")
			AfxMessageBox("OnClose() Error: " + strErr);
	}				 

	sSettings.bConnected = FALSE;
	
	this->Close();
	CSocket::OnClose(nErrorCode);
}

void CMySocket::OnReceive(int nErrorCode)
{
	int iFind = 0, iFindNext = 0, iRcvd = 0, iCount = 0;
	CString strReceived, strTemp;
	_TCHAR strRcvd[4096];

	iRcvd = this->Receive(strRcvd, 4095);
	if (iRcvd == SOCKET_ERROR) {
		AfxMessageBox(_T("SOCKET_ERROR"));
		return;		
	}

	strRcvd[iRcvd] = NULL;

	// Make a CString object because it is easier to work with...
	strReceived = strRcvd;

	// Received string is in the format:
	// Keycode RepeatCount Button NameOfRemote
	// strReceived = _T("0000000000eab154 05 test1 myremote\n");
	
	iFind = strReceived.Find(_T(' '), iFind);

	if (iFind != -1) {
		iFindNext = strReceived.Find(_T(' '), iFind + 1);
		strTemp = strReceived.Mid(iFind + 1, iFindNext - iFind - 1);

		iCount = atoi(strTemp);

		iFind = iFindNext;
		iFindNext = strReceived.Find(_T(' '), iFind + 1);

		strTemp = strReceived.Mid(iFind + 1, iFindNext - iFind - 1);
		ParseCommand(strTemp, iCount);
	}

	CSocket::OnReceive(nErrorCode);
}