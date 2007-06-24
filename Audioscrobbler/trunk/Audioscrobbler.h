#ifndef AUDIOSCROBBLER_H_
#define AUDIOSCROBBLER_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//-----------------------------------------------------------------------------
enum AS_APM_COMMANDS {
	AS_MSG_PLAY_STARTED = WM_APP+10,
	AS_MSG_PLAY_STOPPED,
	AS_MSG_PLAY_DONE,
	AS_MSG_PLAY_PAUSED,
	AS_MSG_TRACK_CHANGED,
	AS_MSG_SETTINGS_CHANGED,
};

DWORD WINAPI AS_Main(LPVOID lpParameter);


//-----------------------------------------------------------------------------
// CQueueAPCItem Class
//-----------------------------------------------------------------------------
/*class CQueueAPCItem
{
private:
	const static DWORD DEFAULT_TIMEOUT = 500;

public:
	CQueueAPCItem(UINT nEvent, BOOL bCreateEvent = TRUE, void* pData = NULL)
		: m_nEvent(nEvent), m_pData(pData), m_hDoneEvent(NULL)
	{
		if (bCreateEvent)
			m_hDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	~CQueueAPCItem()
	{
		if (m_hDoneEvent)
			CloseHandle(m_hDoneEvent);
	}

	void PostMessage(ULONG nThread)
	{
		PostThreadMessage(nThread, m_nEvent, (WPARAM)this, 0);
	}

	void PostMessageAndWait(ULONG nThread)
	{
		PostMessage(nThread);
		if (m_hDoneEvent)
			WaitForSingleObject(m_hDoneEvent, DEFAULT_TIMEOUT);
	}

	void*      m_pData;
	HANDLE     m_hDoneEvent;
	UINT       m_nEvent;
};*/

//-----------------------------------------------------------------------------


#endif // AUDIOSCROBBLER_H_