;*********************
; Include header file
;*********************
!include "LogicLib.nsh"

;*********************
; Setup compiler
;*********************
SetCompressor /SOLID lzma
InstallColors /WINDOWS
Icon "qmp.ico"
XPStyle on
BrandingText "QCD/QMP Plug-in Installer"

; The name of the installer
Name "A-B Repeat"
; The file to write
OutFile "A-B_Repeat.exe"
; The default installation directory
InstallDir "$PROGRAMFILES\Quintessential Media Player"

; Select dir text
DirText "Select QCD/QMP install folder:"

;**********************
; Get QCD Plugin Folder
;**********************
Var QIsQMP ; is QMP or QCD?

;**********************
; Get QCD Plugin Folder
;**********************
Function SetPluginPath
  ${If} ${FileExists} "$INSTDIR\QMPlayer.exe" ; is QMP avaliable
    StrCpy $QIsQMP "True"
    StrCpy $1 "$INSTDIR\Plugins"
    ReadINIStr $9 "$INSTDIR\QMP.ini" "QMPlayer" "PluginFolder"
	${If} ${FileExists} $9
      ;if setting not empty, and is valid, set as folder
	  StrCpy $1 $9
	${EndIf}
  ${ElseIf} ${FileExists} "$INSTDIR\QCDPlayer.exe" ; is QCD avaliable
	StrCpy $QIsQMP "False"
    StrCpy $1 "$INSTDIR\Plugins"
    ReadINIStr $9 "$INSTDIR\QCD.ini" "QCD Player" "PluginFolder"
	${If} ${FileExists} $9
      ;if setting not empty, and is valid, set as folder
	  StrCpy $1 $9
	${EndIf}
  ${Else} ; set default when nothing
	StrCpy $QIsQMP "True";
    StrCpy $1 "$INSTDIR"
  ${EndIf}

  SetOutPath $1
FunctionEnd

Function .onInit
  ; Fill default install path to Quinn Player
  ReadRegStr $INSTDIR HKLM "Software\Quinnware\Quintessential Media Player" ""
  ${Unless} ${FileExists} "$INSTDIR\QMPlayer.exe" ; no QMP
    ReadRegStr $INSTDIR HKLM "Software\Quinnware\Quintessential Player" ""
    ${Unless} ${FileExists} "$INSTDIR\QCDPlayer.exe" ; no QCD
      Strcpy $INSTDIR "$PROGRAMFILES\Quintessential Media Player" ; set to default
    ${EndUnless}
  ${EndUnless}
FunctionEnd

;**********************
; Main
;**********************
Section "Main"
  Call SetPluginPath

  File "ABRepeat.dll"
SectionEnd
