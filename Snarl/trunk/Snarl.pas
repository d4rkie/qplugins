unit Snarl;

{$ifdef FPC}
  {$mode delphi}
{$endif}

interface

uses
  Windows, Messages;
  
(*
 * Registered window message and event identifiers (passed in wParam when either SNARL_GLOBAL_MSG or ReplyMsg is received)
 *)
const
	SNARL_GLOBAL_MSG 							= 'SnarlGlobalEvent';
	SNARL_NOTIFICATION_CANCELLED 	= 0;
	SNARL_LAUNCHED 								= 1;
	SNARL_QUIT 										= 2;

	SNARL_NOTIFICATION_CLICKED 		= 32;            // notification was right-clicked by user
	SNARL_NOTIFICATION_TIMED_OUT 	= 33;
	SNARL_NOTIFICATION_ACK 				= 34;            // notification was left-clicked by user

(*
 * Snarl Data Types
 *)
type
  SNARL_COMMANDS = (
    SNARL_SHOW        = 1,
    SNARL_HIDE        = 2,
    SNARL_UPDATE      = 3,
    SNARL_IS_VISIBLE  = 4,
    SNARL_GET_VERSION = 5,
    SNARL_REGISTER_CONFIG_WINDOW = 6,
    SNARL_REVOKE_CONFIG_WINDOW = 7
  );

  TSnarlCommands = SNARL_COMMANDS;

  TSnarlBuffer = array[0..1023] of Byte;

  SNARLSTRUCT =  record
    Cmd:      TSnarlCommands;	// What to do...
    Id:       Integer;					// Message ID (returned by snShowMessage())
    Timeout:  Integer;					// Timeout in seconds (0=sticky)
    LngData2: Integer;    				// Reserved
    Title:    TSnarlBuffer;
    Text:     TSnarlBuffer;
    Icon:     TSnarlBuffer;
  end;
  TSnarlStruct = SNARLSTRUCT;

(*
 * Snarl Helper Functions
 *)
function snShowMessage(ATitle, AText: String; ATimeout: Integer = 0;
  AIconPath: String = ''; AhwndReply: Integer = 0; AReplyMsg: Integer = 0): Integer;
function snUpdateMessage(AId: Integer; ATitle, AText: String): Boolean;
function snHideMessage(AId: Integer): Boolean;
function snIsMessageVisible(AId: Integer): Boolean;
function snGetVersion(var Major, Minor: Word): Boolean;
function snGetGlobalMsg: Integer;
function snRegisterConfig(AHandle: HWND; AAppName: String; AReplyMsg: Integer): Integer;
function snRevokeConfig(AHandle: HWND): Integer;

implementation

(*
 * Private utility functions:
 * 	_Send(TSnarlStruct) 
 *			Used by most public helper functions to send the WM_COPYDATA message.
 *		_Clear(TSnarlStruct) 
 *			Clears all data in the structure
 *)
function _Send(pss: TSnarlStruct): Integer;
var
  hwnd: THandle;
  pcd: TCopyDataStruct;
begin
  { WIll get a window class when snarl is released } 
  hwnd := FindWindow(nil, 'Snarl');
  if not IsWindow(hwnd) then
    Result := 0
  else
  begin
    pcd.dwData := 2;
    pcd.cbData := Sizeof(pss);
    pcd.lpData := @pss;
    Result := Integer(SendMessage(hwnd, WM_COPYDATA, 0, Integer(@pcd)));
  end;
end;

procedure _Clear(var pss: TSnarlStruct); inline;
begin
  FillChar(pss, Sizeof(pss), 0);
end;

(************************************************************
 * The Helper Functions
 ************************************************************)
 
function snShowMessage(ATitle, AText: String; ATimeout: Integer = 0;
  AIconPath: String = ''; AhwndReply: Integer = 0; AReplyMsg: Integer = 0): Integer;
var
  pss: TSnarlStruct;
begin
  _Clear(pss);
  pss.Cmd := SNARL_SHOW;
  CopyMemory(@pss.Title, PChar(ATitle), 1023);
  CopyMemory(@pss.Text, PChar(AText), 1023);
  CopyMemory(@pss.Icon, PChar(AIconPath), 1023);
  pss.Timeout := ATimeout;
  { R0.3 }
  pss.LngData2 := AhwndReply;
  pss.Id := AReplyMsg;
  Result := _Send(pss);
end;

function snUpdateMessage(AId: Integer; ATitle, AText: String): Boolean;
var
  pss: TSnarlStruct;
begin
  _Clear(pss);
  pss.Id := AId;
  pss.Cmd := SNARL_UPDATE;
  CopyMemory(@pss.Title, PChar(ATitle), 1023);
  CopyMemory(@pss.Text, PChar(AText), 1023);
  Result := Boolean(_Send(pss));
end;

function snHideMessage(AId: Integer): Boolean;
var
  pss: TSnarlStruct;
begin
  _Clear(pss);
  pss.Id := AId;
  pss.Cmd := SNARL_HIDE;
  Result := Boolean(_Send(pss));
end;

function snIsMessageVisible(AId: Integer): Boolean;
var
  pss: TSnarlStruct;
begin
  _Clear(pss);
  pss.Id := AId;
  pss.Cmd := SNARL_IS_VISIBLE;
  Result := Boolean(_Send(pss));
end;

function snGetVersion(var Major, Minor: Word): Boolean;
var
  pss: TSnarlStruct;
  hr: Integer;
begin
  _Clear(pss);
  pss.Cmd := SNARL_GET_VERSION;
  hr := Integer(_Send(pss));
  Result := hr <> 0;
  if Result then
  begin
    Major := HiWord(hr);
    Minor := LoWord(hr);
  end;
end;

function snGetGlobalMsg(): Integer;
begin
	Result := RegisterWindowMessage(SNARL_GLOBAL_MSG);
end;

function snRegisterConfig(AHandle: HWND; AAppName: String; AReplyMsg: Integer): Integer;
var
	pss: TSnarlStruct;
begin
	_Clear(pss);
	pss.Cmd := SNARL_REGISTER_CONFIG_WINDOW;
	pss.LngData2 := AHandle;
	pss.Id := AReplyMsg;
	CopyMemory(@pss.Title, PChar(AAppName), 1023);
	Result := _Send(pss);
end;

function snRevokeConfig(AHandle: HWND): Integer;
var
	pss: TSnarlStruct;
begin
	_Clear(pss);
	pss.Cmd := SNARL_REVOKE_CONFIG_WINDOW;
	pss.LngData2 := AHandle;
	Result := _Send(pss);
end;


end.
