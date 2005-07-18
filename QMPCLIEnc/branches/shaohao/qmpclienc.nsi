;*********************
; Include header file
;*********************
!include "LogicLib.nsh"

;*********************
; Name and Setup
;*********************
Name "QMP Command Line Encoder(CLI) Plug-in"
OutFile "enc_qmpclienc.exe"

SetCompressor lzma
InstallColors /WINDOWS
Icon "qmp.ico"
XPStyle on
BrandingText "QMP Plug-in Installer"

;*********************
; Get QMP Install Path
;*********************
InstallDir "$PROGRAMFILES\Quintessential Media Player"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Quinnware\Quintessential Media Player" ""
;DirShow show
DirText "Select the folder to install this plug-in:"


;**********************
; Get QMP Plugin Folder
;**********************
Function SetPluginPath
;default install path
  StrCpy $1 "$INSTDIR"

;if this is qmp folder, default to plugin subfolder
  ${If} ${FileExists} "$INSTDIR\qmplayer.exe"
    StrCpy $1 "$INSTDIR\Plugins"

;read players plugin folder setting
    ReadINIStr $9 "$INSTDIR\QMP.ini" "QMPlayer" "PluginFolder"

;if setting not empty, and is valid, set as folder
    ${If} $9 != ""
    ${AndIf} ${FileExists} $9
      StrCpy $1 $9
    ${EndIf}

  ${EndIf}

;set final out path
  SetOutPath $1
FunctionEnd


;************************
; install your files here
;************************
Section
  Call SetPluginPath

  File "Release\QMPCLIEnc.dll"
  File "QMPCLIEnc.ep"
SectionEnd
