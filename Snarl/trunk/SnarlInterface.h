#ifndef SNARL_INTERFACE
#define SNARL_INTERFACE

#include <tchar.h>
#include <windows.h>
#include <strsafe.h>


class SnarlInterface {
	public:
		static const LPCTSTR SNARL_GLOBAL_MSG;

		static const int SNARL_STRING_LENGTH = 1024;

		static const LONG32 SNARL_LAUNCHED = 1;                // Snarl has just started running
		static const LONG32 SNARL_QUIT = 2;                    // Snarl is about to stop running
		static const LONG32 SNARL_ASK_APPLET_VER = 3;          // (R1.5) Reserved for future use
		static const LONG32 SNARL_SHOW_APP_UI = 4;             // (R1.6) Application should show its UI

		static const LONG32 SNARL_NOTIFICATION_CLICKED = 32;   // notification was right-clicked by user
		static const LONG32 SNARL_NOTIFICATION_TIMED_OUT = 33;
		static const LONG32 SNARL_NOTIFICATION_ACK = 34;       // notification was left-clicked by user

		static const LONG32 SNARL_NOTIFICATION_CANCELLED = SNARL_NOTIFICATION_CLICKED;  // Added in R1.6

		static const DWORD WM_SNARLTEST    = WM_USER + 237;    // note hardcoded WM_USER value!
		static const DWORD WM_MANAGE_SNARL = WM_USER + 238; 
		
		typedef enum M_RESULT {
			M_ABORTED         = 0x80000007,
			M_ACCESS_DENIED   = 0x80000009,
			M_ALREADY_EXISTS  = 0x8000000C,
			M_BAD_HANDLE      = 0x80000006,
			M_BAD_POINTER     = 0x80000005,
			M_FAILED          = 0x80000008,
			M_INVALID_ARGS    = 0x80000003,
			M_NO_INTERFACE    = 0x80000004,
			M_NOT_FOUND       = 0x8000000B,
			M_NOT_IMPLEMENTED = 0x80000001,
			M_OK              = 0x00000000,
			M_OUT_OF_MEMORY   = 0x80000002,
			M_TIMED_OUT       = 0x8000000A
		};

		enum SNARL_COMMANDS {
			SNARL_SHOW = 1,
			SNARL_HIDE,
			SNARL_UPDATE,
			SNARL_IS_VISIBLE,
			SNARL_GET_VERSION,
			SNARL_REGISTER_CONFIG_WINDOW,
			SNARL_REVOKE_CONFIG_WINDOW,

			// R1.6 onwards
			SNARL_REGISTER_ALERT,
			SNARL_REVOKE_ALERT,   // for future use
			SNARL_REGISTER_CONFIG_WINDOW_2,
			SNARL_GET_VERSION_EX,
			SNARL_SET_TIMEOUT,

			// extended commands (all use SNARLSTRUCTEX)
			SNARL_EX_SHOW = 0x20
		};

		struct SNARLSTRUCT {
			SNARL_COMMANDS Cmd;
			LONG32 Id;
			LONG32 Timeout;
			LONG32 LngData2;
			char Title[SNARL_STRING_LENGTH];
			char Text[SNARL_STRING_LENGTH];
			char Icon[SNARL_STRING_LENGTH];
		};

		struct SNARLSTRUCTEX {
			SNARL_COMMANDS Cmd;
			LONG32 Id;
			LONG32 Timeout;
			LONG32 LngData2;
			char Title[SNARL_STRING_LENGTH];
			char Text[SNARL_STRING_LENGTH];
			char Icon[SNARL_STRING_LENGTH];

			char Class[SNARL_STRING_LENGTH];
			char Extra[SNARL_STRING_LENGTH];
			char Extra2[SNARL_STRING_LENGTH];
			LONG32 Reserved1;
			LONG32 Reserved2;
		};


		SnarlInterface();
		~SnarlInterface();

		static HWND   GetSnarlWindow();		
		static LONG32 GetGlobalMsg();

		
		LPTSTR AllocateString(size_t n) { return new TCHAR[n]; }
		void FreeString(LPCTSTR str)    { delete [] str; str = NULL; }
		

		LONG32  ShowMessage(LPCSTR szTitle, LPCSTR szText, LONG32 timeout = 0, LPCSTR szIconPath = "", HWND hWndReply = NULL, WPARAM uReplyMsg = 0);
		LONG32  ShowMessageEx(LPCSTR szClass, LPCSTR szTitle, LPCSTR szText, LONG32 timeout = 0, LPCSTR szIconPath = "", HWND hWndReply = NULL, WPARAM uReplyMsg = 0, LPCSTR szSoundFile = "");

		LPCTSTR GetAppPath();    // ** Remember to FreeString when finished with the string !
		LPCTSTR GetIconsPath();  // ** Remember to FreeString when finished with the string !

		BOOL      GetVersion(WORD* Major, WORD* Minor);
		LONG32    GetVersionEx();
		BOOL      HideMessage();
		BOOL      HideMessage(LONG32 Id);
		BOOL      IsMessageVisible();
		BOOL      IsMessageVisible(LONG32 Id);
		M_RESULT  RegisterAlert(LPCSTR szAppName, LPCSTR szClass);
		M_RESULT  RegisterConfig(HWND hWnd, LPCSTR szAppName, LONG32 replyMsg);
		M_RESULT  RegisterConfig2(HWND hWnd, LPCSTR szAppName, LONG32 replyMsg, LPCSTR szIcon);
		M_RESULT  RevokeConfig(HWND hWnd);
		M_RESULT  SetTimeout(LONG32 Timeout);
		M_RESULT  SetTimeout(LONG32 Id, LONG32 Timeout);
		M_RESULT  UpdateMessage(LPCSTR szTitle, LPCSTR szText, LPCSTR szIconPath = "");
		M_RESULT  UpdateMessage(LONG32 Id, LPCSTR szTitle, LPCSTR szText, LPCSTR szIconPath = "");
		
		LONG32    GetLastMessageId() { return m_nLastMessageId; }

	private:
		LONG32 uSend(SNARLSTRUCT ss);
		LONG32 uSendEx(SNARLSTRUCTEX ssex);
		
		LONG32 m_nLastMessageId;
		HWND   m_hwndFrom; // set during snRegisterConfig() or snRegisterConfig2()
};

#endif // SNARL_INTERFACE
