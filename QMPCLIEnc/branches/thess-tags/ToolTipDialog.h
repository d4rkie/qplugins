
#ifndef __TTDLG_H__
#define __TTDLG_H__

//
//		CToolTipDialog 
//
// Written by Alain Rist (ar@navpoch.com)
// Copyright (c) 2003 Alain Rist.
//
// This file is NOT a part of the Windows Template Library.
// The code and information is provided "as-is" without
// warranty of any kind, either expressed or implied.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the author written consent, and 
// providing that this notice and the author name is included. 
//
// Beware of bugs.
//

#pragma once

#ifndef __ATLCTRLS_H__
#error ToolTipDialog.h requires atlctrls.h to be included first
#endif


#ifndef TTS_BALLOON
#define TTS_BALLOON 0
#endif

namespace WTL
{
	template < class T ,class TT = CToolTipCtrl >
	class CToolTipDialog
	{
		// Data declarations and members
	public:
		TT& GetTT(){return m_TT;}
	protected:
		TT m_TT;
		UINT m_uTTStyle;
		UINT m_uToolFlags;
		CString m_strToolTip; // tool tip content
		// Construction
		CToolTipDialog( UINT uTTSTyle= TTS_NOPREFIX | TTS_BALLOON , UINT uToolFlags = TTF_IDISHWND | TTF_SUBCLASS ) 
			: m_TT( NULL ), m_uTTStyle( uTTSTyle ), 
			m_uToolFlags( uToolFlags | TTF_SUBCLASS )
		{}

		void TTInit()
		{
			T* pT= (T*)this;
			ATLASSERT( ::IsWindow( *pT ));
			m_TT.Create( *pT, NULL, NULL, m_uTTStyle );
			//CToolInfo ToolInfo( pT->m_uToolFlags, *pT , 0, 0, MAKEINTRESOURCE(pT->IDD) );
			CToolInfo ToolInfo( pT->m_uToolFlags, *pT ); // use TTN_GETDISPINFO to support long multi-line tool tip
			m_TT.AddTool( &ToolInfo );
			::EnumChildWindows( *pT, SetTool, (LPARAM)pT );
			TTSize( 0 );
			TTActivate( TRUE );
		}
		// Operations
	public:
		void TTActivate(BOOL bActivate)
		{ m_TT.Activate( bActivate ); }
		void TTSize( int nPixel )
		{ m_TT.SetMaxTipWidth( nPixel );}

		void TTSetTxt( HWND hTool, _U_STRINGorID text )
		{ m_TT.UpdateTipText( text, hTool);}
		void TTSetTxt( UINT idTool, _U_STRINGorID text )
		{ TTSetTxt( GetHWND( idTool ) , text);}

		BOOL TTAdd( HWND hTool )
		{ return SetTool( hTool, (LPARAM)(T*)this );}
		BOOL TTAdd( UINT idTool )
		{ return TTAdd( GetHWND( idTool ));}

		void TTRemove( HWND hTool )
		{ m_TT.DelTool( hTool );}
		void TTRemove( UINT idTool )
		{ TTRemove( GetHWND( idTool ));}

		void TTSetTitle( HWND hTool, UINT uIcon, LPCTSTR lpstrTitle)
		{ SetProp( hTool, _T("ICON"), (HANDLE)uIcon); SetProp( hTool, _T("TITLE"), (HANDLE)lpstrTitle);}
		void TTSetTitle( UINT idTool, UINT uIcon, LPCTSTR lpstrTitle)
		{ TTSetTitle( GetHWND( idTool), uIcon, lpstrTitle);}
		// Message map and handlers
		BEGIN_MSG_MAP_EX(CToolTipDialog)
			MSG_WM_INITDIALOG(OnInitDialog)
			MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouse)
			NOTIFY_CODE_HANDLER_EX(TTN_GETDISPINFO, OnToolTipNotify)
		END_MSG_MAP_EX()

		BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
		{
			TTInit();
			SetMsgHandled( FALSE);
			return TRUE;
		}

		LRESULT OnMouse(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			T* pT = (T*)this;
			SetMsgHandled( FALSE);
			if(m_TT.IsWindow())
				m_TT.RelayEvent((LPMSG)pT->GetCurrentMessage());
			return 0;
		}

		LRESULT OnToolTipNotify(LPNMHDR pnmh)
		{
			LPNMTTDISPINFO lpTTDI = (LPNMTTDISPINFO)pnmh;

			int idTool = ::GetWindowLong( (HWND)lpTTDI->hdr.idFrom, GWL_ID );
			m_strToolTip.LoadString( idTool );
			lpTTDI->lpszText = (LPTSTR)(LPCTSTR)m_strToolTip;

			LPTSTR title = (LPTSTR)GetProp( (HWND)lpTTDI->hdr.idFrom, _T("TITLE"));
			UINT icon = (UINT)GetProp( (HWND)lpTTDI->hdr.idFrom, _T("ICON"));
			if ( title)
				m_TT.SetTitle( icon, title);

			return TRUE;
		}

		// Implementation
	private:
		HWND GetHWND( UINT idTool )
		{ return ::GetDlgItem( *(T*)this, idTool );}

		static BOOL CALLBACK SetTool( HWND hTool, LPARAM pDlg)
		{
			TCHAR buf[2];
			T* pT = (T*)pDlg;
			int idTool = ::GetWindowLong(hTool, GWL_ID);
			if ( idTool != IDC_STATIC && LoadString( ModuleHelper::GetResourceInstance(), idTool, buf, 2))
			{
				//CToolInfo ToolInfo( pT->m_uToolFlags, hTool, 0, 0, (LPTSTR)idTool );
				CToolInfo ToolInfo( pT->m_uToolFlags, hTool ); // use TTN_GETDISPINFO to support long multi-line tool tip
				pT->m_TT.AddTool( &ToolInfo );
			}
			return TRUE;
		}
	};
} // namespace WTL
#endif // __TTDLG_H__

