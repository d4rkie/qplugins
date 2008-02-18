#include <QCDModGeneral2.h>

#include "QMPGeneralDLL.h"
#include "ExportFunctions.h"
#include "PlayerSubclass.h"


//----------------------------------------------------------------------------
// PlayStarted

void CPlayerSubclass::PlayStarted()
{
	if (m_nPlayStartedTimer) {
		KillTimer(NULL, m_nPlayStartedTimer);
		m_nPlayStartedTimer = 0;
	}

	m_nPlayStartedTimer = SetTimer(NULL, NULL, 100, PlayStartedTimerProc);
}

VOID CALLBACK CPlayerSubclass::PlayStartedTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	// Clean timer
	CPlayerSubclass* ps = GetInstance();
	KillTimer(NULL, ps->m_nPlayStartedTimer);
	ps->m_nPlayStartedTimer = 0;

	ExportNowPlaying_Started();
}

//----------------------------------------------------------------------------
// PlayStopped
void CPlayerSubclass::PlayStopped()
{
	ExportNowPlaying_Ended();
}

//----------------------------------------------------------------------------
// PlayPaused
void CPlayerSubclass::PlayPaused()
{
	PlayStopped();
}

//----------------------------------------------------------------------------
// PlayerClosing
void CPlayerSubclass::PlayerClosing()
{
	PlayStopped();
}

//----------------------------------------------------------------------------

/*void CPlayerSubclass::Delay(void (CPlayerSubclass::*func)(), DWORD dwMilliseconds)
{
	if (m_Timer);
	std::set
	SetTimer(NULL, 0, dwMilliseconds, 
	//Sleep(dwMilliseconds);
	(*this.*func)();
}*/

//----------------------------------------------------------------------------

LRESULT CALLBACK CPlayerSubclass::QMPSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) 
{ 
	switch (msg)
	{
		case WM_PN_PLAYSTARTED :
		case WM_PN_TRACKCHANGED :
			//CPlayerSubclass* ps = CPlayerSubclass::GetInstance();
			//ps->Delay(&PlayStarted, 500);
			CPlayerSubclass::GetInstance()->PlayStarted();
			break;
		
		case WM_PN_PLAYSTOPPED :
			CPlayerSubclass::GetInstance()->PlayStopped();
			break;

		case WM_PN_PLAYPAUSED :
			CPlayerSubclass::GetInstance()->PlayPaused();
			break;

		//WM_PN_PLAYDONE
		//WM_PN_INFOCHANGED

	}
	
	return CallWindowProc(CPlayerSubclass::GetInstance()->m_oldQMPProc, hwnd, msg, wparam, lparam);
}

//----------------------------------------------------------------------------
// Implement Singleton pattern
//----------------------------------------------------------------------------
CPlayerSubclass* CPlayerSubclass::s_instance = 0;

CPlayerSubclass::CPlayerSubclass() 
	: m_hwndPlayer(0), m_oldQMPProc(0), m_nPlayStartedTimer(0)
{

}

CPlayerSubclass* CPlayerSubclass::GetInstance() 
{
	if (s_instance == 0)
		s_instance = new CPlayerSubclass();
	
	return s_instance;
}

void CPlayerSubclass::Destroy() 
{
	s_instance->PlayerClosing();
	delete s_instance;
	CPlayerSubclass::s_instance = 0;
}



//----------------------------------------------------------------------------
// Subclass (de)init functions
BOOL CPlayerSubclass::SubclassPlayer(HWND hwndPlayer)
{
	if (m_oldQMPProc != NULL || hwndPlayer == NULL)
		return FALSE;

	m_hwndPlayer = hwndPlayer;
	m_oldQMPProc = reinterpret_cast<WNDPROC>( SetWindowLongPtr(hwndPlayer, GWL_WNDPROC, (LONG_PTR)QMPSubProc) );
	if (m_oldQMPProc != 0)
		return TRUE;

	return FALSE;
}

//----------------------------------------------------------------------------

BOOL CPlayerSubclass::RemoveSubclass()
{
	if (m_oldQMPProc != NULL)
	{
		SetWindowLongPtr(m_hwndPlayer, GWL_WNDPROC, (LONG_PTR)m_oldQMPProc);
		m_oldQMPProc = NULL;
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
