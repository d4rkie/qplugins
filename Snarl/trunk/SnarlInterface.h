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

		static const LONG32 SNARL_NOTIFICATION_CLICKED = 32;   // notification was right-clicked by user
		static const LONG32 SNARL_NOTIFICATION_TIMED_OUT = 33;
		static const LONG32 SNARL_NOTIFICATION_ACK = 34;       // notification was left-clicked by user

		static const LONG32 SNARL_ASK_APPLET_VER = 42;         // Added in R1.5

		static const LONG32 SNARL_NOTIFICATION_CANCELLED = SNARL_NOTIFICATION_CLICKED;  // Added in R1.6

		static const DWORD WM_SNARLTEST = WM_USER + 237;       // note hardcoded WM_USER value!

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
			SNARLSTRUCT pss;
			char Extra[SNARL_STRING_LENGTH];
			char Extra2[SNARL_STRING_LENGTH];
			LONG32 Reserved1;
			LONG32 Reserved2;
		};


		SnarlInterface();
		~SnarlInterface();

		LONG32 snShowMessage(LPCSTR szTitle, LPCSTR szText, LONG32 timeout, LPCSTR szIconPath, HWND hWndReply, LONG32 uReplyMsg);
		BOOL snHideMessage(LONG32 id);
		BOOL snIsMessageVisible(LONG32 id);
		BOOL snUpdateMessage(LONG32 id, LPCSTR szTitle, LPCSTR szText, LPCSTR szIconPath = "");
		BOOL snRegisterConfig(HWND hWnd, LPCSTR szAppName, LONG32 replyMsg);
		BOOL snRegisterConfig2(HWND hWnd, LPCSTR szAppName, LONG32 replyMsg, LPCSTR szIcon);
		BOOL snRevokeConfig(HWND hWnd);
		BOOL snGetVersion(WORD* major, WORD* minor);
		HWND snGetSnarlWindow();
		UINT snGetGlobalMsg();

	private:
		LONG32 uSend(SNARLSTRUCT snarlStruct);
		HWND m_hwndFrom;
};

#endif // SNARL_INTERFACE
