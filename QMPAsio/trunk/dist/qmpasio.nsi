;*********************
; Name and Setup
;*********************
Name "QMPAsio Playback"
OutFile "QMPAsio.exe"

SetCompressor zlib
AllowRootDirInstall false
InstallColors /WINDOWS
Icon "qmp.ico"

;*********************
; Get QCD Install Path
;*********************
InstallDir "$PROGRAMFILES\Quintessential Media Player"
InstallDirRegKey HKEY_LOCAL_MACHINE "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Quintessential Media Player" "UninstallString"
DirText "QMP Plugin folder selection"


;**********************
; Get QCD Plugin Folder
;**********************
Function SetPluginPath
 ;default install path
 StrCpy $1 "$INSTDIR"

 ;if this is qcd folder, default to plugin subfolder
 IfFileExists "$INSTDIR\QMPlayer.exe" 0 End
  StrCpy $1 "$INSTDIR\Plugins"

 ;read players plugin folder setting
 ReadINIStr $9 "$INSTDIR\qmp.ini" "Folders" "PluginFolder"

 ;if setting not empty, and is valid, set as folder
 StrCmp $9 "" End
 IfFileExists $9 0 End
 StrCpy $1 $9
  
 End:
 SetOutPath $1
FunctionEnd

;************************
; install your files here
;************************
Section
Call SetPluginPath

File "QMPAsio.dll"

SetOutPath $INSTDIR
File "QMPAsioReadme.txt"

SectionEnd
