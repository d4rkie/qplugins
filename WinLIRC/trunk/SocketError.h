// SocketError.h: interface for the CSocketError class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SocketError_H__F1A01AD7_37E7_46BE_B7C4_7B27ED30A0D3__INCLUDED_)
#define AFX_SocketError_H__F1A01AD7_37E7_46BE_B7C4_7B27ED30A0D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSocketError
{
public:
	CSocketError(HWND hwndParent, BOOL bShowErrorMsg, BOOL bDebug);
	virtual ~CSocketError();

	void DisplayError(const char* pFunc, int iError);

private:
	BOOL m_bDebug;
	BOOL m_bShowErrorMessages;
	HWND m_hwndParent;
};

#endif // !defined(AFX_SocketError_H__F1A01AD7_37E7_46BE_B7C4_7B27ED30A0D3__INCLUDED_)
