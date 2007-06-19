#pragma once

#include <windows.h>
#include <mmreg.h>

#include <io.h>


// QCLIWatcher 命令目标

class QCLIWatcher
{
public:
	QCLIWatcher();
	~QCLIWatcher();
public:
	void Start(HANDLE hToWatch, HANDLE hToClose);
	void Stop(void);
private:
	void _run();
	static DWORD WINAPI _thread_entry(void * ptr)
	{
		reinterpret_cast<QCLIWatcher *>(ptr)->_run();
		return 0;
	}
private:
	HANDLE m_hThread;

	HANDLE m_hToWatch;
	HANDLE m_hToClose;
};


// QCLIEncoder 命令目标

class QCLIEncoder
{
public:
	QCLIEncoder();
	virtual ~QCLIEncoder();

public:
	BOOL Initialize(HWND hwndParent = NULL, BOOL bNoWAVHeader = FALSE, BOOL bShowConsole = FALSE);
	BOOL Start(LPCTSTR lpszCommandLine, LPCTSTR lpszOutFileName, WAVEFORMATEX * wf);
	BOOL AddData(LPVOID lpBuffer, DWORD nNumberOfBytesToWrite);
	void Stop(void);
	BOOL IsRunning(void);

private:
	int _create_process(LPSTARTUPINFO lpsiStartupInfo);

	int _start_piped_encoder(void);
	int _add_data_to_pipe(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite);
	int _stop_piped_encoder(void);

	int _write_wav_header(WAVEFORMATEX * wf);

	int _start_file_encoder(void);
	int _add_data_to_file(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite);
	int _stop_file_encoder(void);
	int _fix_wav_file_header(void);

	void _error_msg(LPCTSTR lpText, ...);

private:
	BOOL m_bInitialized;
	QCLIWatcher m_WatcherThread;

	PROCESS_INFORMATION m_piProcInfo;
	HANDLE m_hStdinWrite;

	CString m_strCommandLine;

	CString m_strOutFileName;

	CString m_strTempFileName;
	FILE * m_tempFile;
	BOOL m_bTempMode;

	BOOL m_bNoWAVHeader;
	BOOL m_bShowConsole;
	HWND m_hwndParent;
};
/*
%s        Source filename
%d        Destination filename
%r        Bitrate ("32".."320")
%a        Album artist
%g        Album title
%t        Track title
%y        Year
%n        Track number
%m        MP3 music genre
%c        Comment
*/