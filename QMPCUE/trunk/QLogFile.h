#include <stdio.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////

class QLogFile
{
public:
	static QLogFile & GetInst()
	{
		static QLogFile inst;

		return inst;
	}

	void OutputLogStr(const TCHAR * mod, const TCHAR * format, ...)
	{
		va_list args;
		int     len;
		TCHAR   *buffer;
		__time64_t ltime;
		TCHAR tbuffer[26];
		errno_t err;
		FILE * fpLog;

		// open file first
		_tfopen_s( &fpLog, _T("debug.log"), _T("at"));

		// retrieve the variable arguments
		va_start( args, format);

		len = _vsctprintf( format, args) // _vsctprintf doesn't count
			+ 1; // terminating '\0'

		buffer = new TCHAR[len * sizeof(TCHAR)];

		_vstprintf( buffer, format, args);

		// get current system time
		_time64( &ltime);
		_tctime64_s( tbuffer, 26, &ltime);
		tbuffer[24] = _T('\0');

		// output debug string
		_ftprintf( fpLog, _T("[%s]%s> %s\n"), tbuffer, mod, buffer);
		_get_errno( &err);

		delete [] buffer;

		// close file
		fclose( fpLog);
	}

protected:
	QLogFile()
	{
	}
	~QLogFile()
	{
	}
};

