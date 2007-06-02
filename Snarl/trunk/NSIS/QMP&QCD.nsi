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
BrandingText "QMP Plug-in Installer"

; The name of the installer
Name "Snarl"
; The file to write
OutFile "QMPSnarl.exe"
; The default installation directory
InstallDir "$PROGRAMFILES\Quintessential Media Player"

; Select dir text
DirText "Select QMP install folder:"


;**********************
; Get QCD Plugin Folder
;**********************
Function SetPluginPath
  ${If} ${FileExists} "$INSTDIR\QMPlayer.exe" ; is QMP avaliable
    StrCpy $1 "$INSTDIR\Plugins"
    ReadINIStr $9 "$INSTDIR\QMP.ini" "QMPlayer" "PluginFolder"
	${If} ${FileExists} $9
      ;if setting not empty, and is valid, set as folder
	  StrCpy $1 $9
	${EndIf}
  ${Else} ; set default when nothing
    StrCpy $1 "$INSTDIR\Plugins"
  ${EndIf}

  SetOutPath $1
FunctionEnd

Function .onInit
  ; Fill default install path to Quinn Player
  ReadRegStr $INSTDIR HKLM "Software\Quinnware\Quintessential Media Player" ""
  ${Unless} ${FileExists} "$INSTDIR\QMPlayer.exe" ; no QMP
    Strcpy $INSTDIR "$PROGRAMFILES\Quintessential Media Player" ; set to default
  ${EndUnless}
FunctionEnd

;**********************
; Main
;**********************
Section "Main"
  Call SetPluginPath

  File "Snarl.dll"
  File "Snarl.png"
SectionEnd
