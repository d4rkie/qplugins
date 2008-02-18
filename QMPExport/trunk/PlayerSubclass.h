#ifndef QMPSubclass_H
#define QMPSubclass_H

#include <Windows.h>


class CPlayerSubclass
{
public:
	static CPlayerSubclass* GetInstance();
	static void Destroy();

	BOOL SubclassPlayer(HWND hwndPlayer);
	BOOL RemoveSubclass();

	void PlayStarted();
	void PlayStopped();
	void PlayPaused();

	void PlayerClosing();

	//void Delay(void (CPlayerSubclass::*func)(), DWORD dwMilliseconds);

protected:
	CPlayerSubclass();
   // CPlayerSubclass& operator= (const CPlayerSubclass&);


private:
	static LRESULT CALLBACK QMPSubProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static VOID CALLBACK PlayStartedTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	HWND     m_hwndPlayer;
	WNDPROC  m_oldQMPProc;
	UINT_PTR m_nPlayStartedTimer;


	//----------------------------------------------------------------------------
	// Singleton members
	//----------------------------------------------------------------------------
	static CPlayerSubclass* s_instance;
	CPlayerSubclass(const CPlayerSubclass& obj) { throw "Copy constructor called in singleton class."; };
};


#endif // QMPSubclass_H