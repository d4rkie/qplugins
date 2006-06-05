// SocketError.cpp: implementation of the CSocketError class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WinLIRC.h"
#include "SocketError.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSocketError::CSocketError(HWND hwndParent, BOOL bShowErrorMsg, BOOL bDebug)
{
	m_hwndParent			= hwndParent;
	m_bDebug				= bDebug;
	m_bShowErrorMessages	= bShowErrorMsg;
}

CSocketError::~CSocketError()
{

}

//////////////////////////////////////////////////////////////////////

void CSocketError::DisplayError(const char* pFunc, int iError)
{
	if (m_bShowErrorMessages) {
		char strMsg[256];					// Declare a buffer to hold
		ZeroMemory(strMsg, 256);			// Automatically NULL-terminate the string

		if (m_bDebug) {
			switch (iError)
			{
				case WSAECONNREFUSED :
					sprintf(strMsg, "Call to %s failed!\nError is: Connection refused.\n\nErrorcode: %d", pFunc, iError);
					break;

				default :
					sprintf(strMsg, "Call to %s failed!\nError is: Unknown error!\n\nErrorcode: %d", pFunc, iError);
			}
		}
		else { // show friendly messages
			switch (iError)
			{
				case WSAECONNREFUSED :
					sprintf(strMsg, "Connection to WinLIRC refused.\nWinLIRC probably isn't running!");
					break;

				default :
					sprintf(strMsg, "There was an unknown error!");
			}
		}

		MessageBox(m_hwndParent, strMsg, "QCD WinLIRC error", MB_OK | MB_ICONWARNING);
	}
}
