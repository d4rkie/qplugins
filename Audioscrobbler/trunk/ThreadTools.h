// http://msdn2.microsoft.com/en-us/library/xcb2z8hs(vs.71).aspx
//
// Usage: SetThreadName (-1, "MainThread");
//

#define MS_VC_EXCEPTION 0x406d1388

typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // must be 0x1000
   LPCSTR szName; // pointer to name (in user addr space)
   DWORD dwThreadID; // thread ID (-1=caller thread)
   DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
{
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = szThreadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
   }
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}