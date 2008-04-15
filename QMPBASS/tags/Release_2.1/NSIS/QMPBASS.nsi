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
BrandingText "QPlug-in installer for QMP"

; The name of the installer
Name "BASS Sound System Plug-in"
; The file to write
OutFile "QMPBASS.exe"
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
  ; Fill default install path to Quinn Player
  ReadRegStr $INSTDIR HKLM "Software\Quinnware\Quintessential Media Player" ""
  ${Unless} ${FileExists} "$INSTDIR\QMPlayer.exe"
    ReadRegStr $INSTDIR HKLM "Software\Quinnware\Quintessential Player" ""
    ${Unless} ${FileExists} "$INSTDIR\QMPlayer.exe" ;Check here too
      Strcpy $INSTDIR "$PROGRAMFILES\Quintessential Media Player" ; set to default
    ${EndUnless}
  ${EndUnless}
FunctionEnd

;**********************
; Main
;**********************
Section "Main"
  SetOutPath $INSTDIR
  File "bass.dll"
  
  Call SetPluginPath
  Delete $OUTDIR\QCDBASS.dll
  File "QMPBASS.dll"
SectionEnd
