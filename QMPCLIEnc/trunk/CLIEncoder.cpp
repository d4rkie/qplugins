// CLIEncoder.cpp : 实现文件
//

#include "stdafx.h"

#include ".\cliencoder.h"


// QCLIWatcher

QCLIWatcher::QCLIWatcher()
: m_hThread( INVALID_HANDLE_VALUE)
, m_hToWatch( INVALID_HANDLE_VALUE)
, m_hToClose( INVALID_HANDLE_VALUE)
{
}

QCLIWatcher::~QCLIWatcher()
{
	Stop();
}


// QCLIWatcher 成员函数

void QCLIWatcher::Start(HANDLE hToWatch, HANDLE hToClose)
{
	DWORD id;

	Stop();

	DuplicateHandle( GetCurrentProcess(), hToWatch, GetCurrentProcess(), &m_hToWatch, 0, FALSE, DUPLICATE_SAME_ACCESS );
	m_hToClose = hToClose;

	m_hThread = CreateThread( 0, 0, _thread_entry, reinterpret_cast<void *>(this), 0, &id);
}

void QCLIWatcher::Stop(void)
{
	if ( m_hThread != INVALID_HANDLE_VALUE) {
		//WaitForSingleObject( m_hThread, INFINITE);

		CloseHandle( m_hThread);
		m_hThread = INVALID_HANDLE_VALUE;
	}
}

void QCLIWatcher::_run()
{
    WaitForSingleObject( m_hToWatch, INFINITE);

	CloseHandle( m_hToClose);
	CloseHandle( m_hToWatch);
	m_hToClose = INVALID_HANDLE_VALUE;
	m_hToWatch = INVALID_HANDLE_VALUE;
}




// QCLIEncoder

QCLIEncoder::QCLIEncoder()
: m_bInitialized( FALSE)
, m_hStdinWrite( INVALID_HANDLE_VALUE)
, m_strCommandLine( "")
, m_strOutFileName( "")
, m_strTempFileName( "")
, m_bTempMode( FALSE)
, m_bNoWAVHeader( FALSE)
, m_bShowConsole( FALSE)
, m_hwndParent( NULL)
{
	m_piProcInfo.hProcess = INVALID_HANDLE_VALUE;
	m_piProcInfo.hThread = INVALID_HANDLE_VALUE;
}

QCLIEncoder::~QCLIEncoder()
{
	if ( m_bInitialized) {
		if ( !m_bTempMode)
			_stop_piped_encoder();
		else
			m_tempFile.Close();
	}
}


// QCLIEncoder 成员函数

// Initialize CLI encoder
// Remark: invoke Initialize before Start encoder to config common settings.
BOOL QCLIEncoder::Initialize(HWND hwndParent, BOOL bNoWAVHeader, BOOL bShowConsole)
{
	m_hwndParent = hwndParent;
	m_bNoWAVHeader = bNoWAVHeader;
	m_bShowConsole = bShowConsole;

	return TRUE;
//	return !m_bInitialized; // only one encoder per one processing.
}

// Start CLI encoder
BOOL QCLIEncoder::Start(LPCTSTR lpszCommandLine, LPCTSTR lpszOutFileName, WAVEFORMATEX * wf)
{
	BOOL ret;

	QFile::Remove(lpszOutFileName); // delete outfile to avoid warning from encoder.

	m_strCommandLine = lpszCommandLine;
	m_strOutFileName = lpszOutFileName;

	// process comandline
	CString tmp = '\"';
	tmp += m_strOutFileName + '\"';
	m_strCommandLine.Replace( _T("%d"), tmp);

	if ( m_strCommandLine.Find( _T("%s")) >= 0) { // is temp mode?
		m_bTempMode = TRUE;
		m_strTempFileName = m_strOutFileName + _T(".wav");
		tmp = '\"';
		tmp += m_strTempFileName + '\"';
		m_strCommandLine.Replace( _T("%s"), tmp);
	} else {
		m_bTempMode = FALSE;
		m_strTempFileName.Empty();
	}

	ret = TRUE;

	if ( !m_bTempMode) {
		if ( !_start_piped_encoder()) {
			Stop();
			ret = FALSE;
		}
	} else {
        if ( !_start_file_encoder())
			ret = FALSE;
	}

	if ( !m_bNoWAVHeader) {
		if ( !_write_wav_header( wf))
			ret = FALSE;
	}

	m_bInitialized = ret;

	return ret;
}

BOOL QCLIEncoder::AddData(LPVOID lpBuffer, DWORD nNumberOfBytesToWrite)
{
	if ( nNumberOfBytesToWrite > 0) {
		if ( !m_bTempMode)
			return _add_data_to_pipe( lpBuffer, nNumberOfBytesToWrite);
		else
			return _add_data_to_file( lpBuffer, nNumberOfBytesToWrite);
	} else {
		return FALSE;
	}
}

void QCLIEncoder::Stop(void)
{
	if ( m_bInitialized) {
		BOOL success = TRUE;

		if ( !m_bTempMode) {
			if ( !_stop_piped_encoder())
				success = FALSE;
		} else {
			if ( !_stop_file_encoder())
				success = FALSE;
		}

		// check finally dst. file
		if ( !QFile::FileExists(m_strOutFileName)) {
			_error_msg( _T( "Encoding failed!"));
			success = FALSE;
		}

		if (m_bTempMode)
			QFile::Remove(m_strTempFileName);
		/*
		// write tag
		*/

		if ( !success)
			QFile::Remove(m_strOutFileName);

		m_bInitialized = FALSE;
	}
}

BOOL QCLIEncoder::IsRunning(void)
{
	return m_piProcInfo.hProcess != INVALID_HANDLE_VALUE;
}

int QCLIEncoder::_write_wav_header(WAVEFORMATEX * wf)
{
	const int RIFFWAVHEADERSIZE = 44;
	LPDWORD pdwValue = NULL;
	BYTE pbtRiffWavHeader[RIFFWAVHEADERSIZE] = { 0,};
	unsigned int Bytes = (wf->wBitsPerSample+7) / 8;
	unsigned int word32;
	__int64 PCMdataLength = 0;

	//if ( m_preset.exact_length ) {
	//	__int64 samples = (__int64)(m_expected_length * (double)wf->nSamplesPerSec + 0.5);

	//	PCMdataLength = samples * wf->nChannels * Bytes;
	//}

	if ( PCMdataLength <= 0 || (PCMdataLength + (44-8)) > ((__int64)1<<31) ) {
		__int64 align = (__int64)(wf->nSamplesPerSec * wf->nChannels * Bytes);
		PCMdataLength = (((__int64)1<<31) - (44 - 8)) / align;
		PCMdataLength *= align;
	}

	// Write RIFF tag
	memcpy( &pbtRiffWavHeader[0], "RIFF", 4 );

	word32 = (unsigned int)(PCMdataLength + (44 - 8));
	pbtRiffWavHeader[4] = (unsigned char)(word32 >>  0);
	pbtRiffWavHeader[5] = (unsigned char)(word32 >>  8);
	pbtRiffWavHeader[6] = (unsigned char)(word32 >> 16);
	pbtRiffWavHeader[7] = (unsigned char)(word32 >> 24); // Size of the next chunk

	// Write WAVE tag
	memcpy( &pbtRiffWavHeader[8], "WAVE", 4);

	memcpy( &pbtRiffWavHeader[12], "fmt ", 4);
	pdwValue = (DWORD*)&pbtRiffWavHeader[16];
	*pdwValue = sizeof( WAVEFORMATEX) - sizeof( WORD); // length of the PCM data declaration = 2+2+4+4+2+2, don't write cbSize field

	memcpy( &pbtRiffWavHeader[20], wf, *pdwValue);

	memcpy( &pbtRiffWavHeader[RIFFWAVHEADERSIZE - 8], "data", 4);

	word32 = (unsigned int)PCMdataLength;
	pbtRiffWavHeader[RIFFWAVHEADERSIZE - 4] = (unsigned char)(word32 >>  0);
	pbtRiffWavHeader[RIFFWAVHEADERSIZE - 3] = (unsigned char)(word32 >>  8);
	pbtRiffWavHeader[RIFFWAVHEADERSIZE - 2] = (unsigned char)(word32 >> 16);
	pbtRiffWavHeader[RIFFWAVHEADERSIZE - 1] = (unsigned char)(word32 >> 24);

	if ( !m_bTempMode ) {
		if ( !_add_data_to_pipe( pbtRiffWavHeader, sizeof( pbtRiffWavHeader)))
			return 0;

		if ( WaitForSingleObject( m_piProcInfo.hProcess, 0) != WAIT_TIMEOUT ) {
			_error_msg( "Encoder process has terminated, incorrect parameters or input format!");
			return 0;
		}
	} else {
		if ( !_add_data_to_file( pbtRiffWavHeader, sizeof( pbtRiffWavHeader))) {
			_error_msg( "Error writing to temp file!" );
			return 0;
		}
	}

	return 1;
}


int QCLIEncoder::_create_process(LPSTARTUPINFO lpsiStartupInfo)
{
	return CreateProcess( 0, (LPTSTR)(LPCTSTR)m_strCommandLine, 0, 0, TRUE, 0, 0, 0, lpsiStartupInfo, &m_piProcInfo);
}

int QCLIEncoder::_start_piped_encoder(void)
{
	ZeroMemory( &m_piProcInfo, sizeof( PROCESS_INFORMATION));
	HANDLE handle;  // temporary handle
	HANDLE stdinput;
	SECURITY_ATTRIBUTES sa = { sizeof( SECURITY_ATTRIBUTES), NULL, TRUE };

	HANDLE original_stdinput = GetStdHandle( STD_INPUT_HANDLE);
	CreatePipe( &stdinput, &handle, &sa, 0);
	SetStdHandle( STD_INPUT_HANDLE, stdinput);
	DuplicateHandle( GetCurrentProcess(), handle, GetCurrentProcess(), &m_hStdinWrite, 0, FALSE, DUPLICATE_SAME_ACCESS);
	CloseHandle ( handle);

	STARTUPINFO si;
	ZeroMemory( &si, sizeof( si));
	si.cb = sizeof( si);

	si.hStdInput   = stdinput;
	si.hStdOutput  = GetStdHandle( STD_OUTPUT_HANDLE); //StdoutWrite;
	si.hStdError   = GetStdHandle( STD_ERROR_HANDLE); //StderrWrite;
	si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE; // never show for piped encoder

	int ret = 1;

	if ( !_create_process( &si)) {
		_error_msg( "Unable to start the encoder!");
		ret = 0;
	}

	// Restore the original stdin
	SetStdHandle( STD_INPUT_HANDLE, original_stdinput);

	if ( ret != 0) {
		if ( WaitForSingleObject( m_piProcInfo.hProcess, 0) != WAIT_TIMEOUT ) {
			_error_msg( "Encoder process has terminated, incorrect parameters!");
			ret = 0;
		}
	}

	if ( ret)
		m_WatcherThread.Start( m_piProcInfo.hProcess, stdinput);
	else
		CloseHandle( stdinput);

	return ret;
}

int QCLIEncoder::_add_data_to_pipe(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite)
{
	unsigned long written;
	if ( WaitForSingleObject( m_piProcInfo.hProcess, 0) != WAIT_TIMEOUT)
		return 0;
	if ( !WriteFile( m_hStdinWrite, lpBuffer, nNumberOfBytesToWrite, &written, 0) || written != nNumberOfBytesToWrite ) {
		_error_msg( "Writing to encoder failed!");
		return 0;
	}

	return 1;
}

int QCLIEncoder::_stop_piped_encoder(void)
{
	if ( m_hStdinWrite != INVALID_HANDLE_VALUE ) {
		CloseHandle( m_hStdinWrite);
		m_hStdinWrite = INVALID_HANDLE_VALUE;
	}
	if ( m_piProcInfo.hProcess != INVALID_HANDLE_VALUE) {
		CloseHandle( m_piProcInfo.hProcess);
		m_piProcInfo.hProcess = INVALID_HANDLE_VALUE;
	}
	if ( m_piProcInfo.hThread != INVALID_HANDLE_VALUE) {
		CloseHandle( m_piProcInfo.hThread);
		m_piProcInfo.hThread = INVALID_HANDLE_VALUE;
	}

	m_WatcherThread.Stop();

	return 1;
}

int QCLIEncoder::_start_file_encoder(void)
{
	return m_tempFile.Open(m_strTempFileName, QFile::modeCreate | QFile::modeWrite);
}

int QCLIEncoder::_add_data_to_file(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite)
{
	m_tempFile.Write(lpBuffer, nNumberOfBytesToWrite);
	return 1;
}

int QCLIEncoder::_stop_file_encoder(void)
{
	if ( !_fix_wav_file_header())
		return 0;

	m_tempFile.Close();

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = m_bShowConsole ? SW_SHOWMINNOACTIVE : SW_HIDE;
	if ( !_create_process(&si)) {
		_error_msg ( "Unable to execute: %s", m_strCommandLine);

		return 0;
	} else {
		do {
			Sleep( 10);
		} while ( WaitForSingleObject(m_piProcInfo.hProcess, 0) == WAIT_TIMEOUT);

		return 1;
	}
}

int QCLIEncoder::_fix_wav_file_header(void)
{
	unsigned char tmp[4];
	unsigned long t;
	__int64 size = m_tempFile.GetLength();
	if ( size <= 0 ) return 0;

	if ( !m_tempFile.Seek(4))
		return 0;
	t = (unsigned long)(size - 8);
	tmp[0] = (unsigned char)(t >>  0);
	tmp[1] = (unsigned char)(t >>  8);
	tmp[2] = (unsigned char)(t >> 16);
	tmp[3] = (unsigned char)(t >> 24);
	if ( !m_tempFile.Write(tmp, 4))
		return 0;

	if ( !m_tempFile.Seek(40))
		return 0;
	t = (unsigned long)(size - 44);
	tmp[0] = (unsigned char)(t >>  0);
	tmp[1] = (unsigned char)(t >>  8);
	tmp[2] = (unsigned char)(t >> 16);
	tmp[3] = (unsigned char)(t >> 24);
	if ( !m_tempFile.Write(tmp, 4))
		return 0;

	return 1;
}

void QCLIEncoder::_error_msg(LPCTSTR lpText, ...)
{
	CString foo( _T( ""));
	va_list args;
	va_start( args, lpText);
	foo.FormatV( lpText, args);
	va_end( args);

	MessageBox( m_hwndParent, foo, _T( "CommandLine Encoder Error!"), MB_OK | MB_ICONERROR);
}
