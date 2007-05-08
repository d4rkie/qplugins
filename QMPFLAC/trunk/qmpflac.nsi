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
Name "Free Lossless Audio Codec (FLAC) Plug-in"
; The file to write
OutFile "in_FLAC.exe"
; The default installation directory
InstallDir "$PROGRAMFILES\Quintessential Media Player"

; Select dir text
DirText "Select QMP install folder:"

;**********************
; Get QMP Plugin Folder
;**********************
Function SetPluginPath
  ${If} ${FileExists} "$INSTDIR\QMPlayer.exe" ; is QMP avaliable
    StrCpy $1 "$INSTDIR\Plugins"
    ReadINIStr $9 "$INSTDIR\QMP.ini" "Folders" "PluginFolder"
    ${If} ${FileExists} $9
      ;if setting not empty, and is valid, set as folder
      StrCpy $1 $9
    ${EndIf}
  ${Else} ; set default when nothing
    StrCpy $1 "$INSTDIR"
  ${EndIf}

  SetOutPath $1
FunctionEnd

Function .onInit
  ; Read the default install path from register
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

  ; Put file there
  File "/oname=QMPFLAC.dll" "Release\QMPFLAC.dll"

SectionEnd

