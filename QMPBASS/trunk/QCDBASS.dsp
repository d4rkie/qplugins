# Microsoft Developer Studio Project File - Name="QCDBASS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=QCDBASS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "QCDBASS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QCDBASS.mak" CFG="QCDBASS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QCDBASS - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "QCDBASS - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "QCDBASS"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "QCDBASS - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /ZI /Od /I "..\replaygain_synthesis" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /ZI /Od /I "..\replaygain_synthesis" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib replaygain_synthesis_static.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:IX86 /out:"D:\Media\Quintessential Player\Plugins\QCDBASS.dll" /pdbtype:sept /libpath:"..\replaygain_synthesis\Debug"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib replaygain_synthesis_static.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:IX86 /pdbtype:sept /libpath:"..\replaygain_synthesis\Debug"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "QCDBASS - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\replaygain_synthesis" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GF /c
# ADD CPP /nologo /MT /W3 /GX /Zd /O2 /I "..\replaygain_synthesis" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fr /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib replaygain_synthesis_static.lib /nologo /subsystem:windows /dll /machine:IX86 /pdbtype:sept /libpath:"..\replaygain_synthesis\Release"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib replaygain_synthesis_static.lib /nologo /subsystem:windows /dll /map /machine:IX86 /pdbtype:sept /libpath:"..\replaygain_synthesis\Release" /MAPINFO:LINES /MAPINFO:EXPORTS
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "QCDBASS - Win32 Debug"
# Name "QCDBASS - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bass_lib.cpp
DEP_CPP_BASS_=\
	"..\replaygain_synthesis\ordinals.h"\
	"..\replaygain_synthesis\replaygain_synthesis.h"\
	".\bass.h"\
	".\bass_lib.h"\
	".\BASSCfgUI.h"\
	".\cfg_var.h"\
	".\QCDBASS.h"\
	".\qcdhelper.h"\
	".\QCDInputDLL.h"\
	".\QCDModDefs.h"\
	".\QCDModInput.h"\
	".\tags.h"\
	
# End Source File
# Begin Source File

SOURCE=.\BASSCfgUI.cpp
DEP_CPP_BASSC=\
	".\bass.h"\
	".\BASSCfgUI.h"\
	".\cfg_var.h"\
	".\QCDBASS.h"\
	".\qcdhelper.h"\
	".\QCDInputDLL.h"\
	".\QCDModDefs.h"\
	".\QCDModInput.h"\
	
# End Source File
# Begin Source File

SOURCE=QCDBASS.cpp
DEP_CPP_QCDBA=\
	"..\replaygain_synthesis\ordinals.h"\
	"..\replaygain_synthesis\replaygain_synthesis.h"\
	".\bass.h"\
	".\bass_lib.h"\
	".\BASSCfgUI.h"\
	".\cfg_var.h"\
	".\QCDBASS.h"\
	".\qcdhelper.h"\
	".\QCDInputDLL.h"\
	".\QCDModDefs.h"\
	".\QCDModInput.h"\
	

!IF  "$(CFG)" == "QCDBASS - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "QCDBASS - Win32 Release"

# ADD CPP /nologo /GX /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\qcdhelper.cpp
DEP_CPP_QCDHE=\
	".\qcdhelper.h"\
	".\QCDModDefs.h"\
	".\QCDModInput.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tags.cpp
DEP_CPP_TAGS_=\
	".\qcdhelper.h"\
	".\QCDModDefs.h"\
	".\QCDModInput.h"\
	".\tags.h"\
	".\VorbisComment.h"\
	
# End Source File
# Begin Source File

SOURCE=.\VorbisComment.cpp
DEP_CPP_VORBI=\
	".\qcdhelper.h"\
	".\QCDModDefs.h"\
	".\QCDModInput.h"\
	".\tags.h"\
	".\VorbisComment.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bass_lib.h
# End Source File
# Begin Source File

SOURCE=.\BASSCfgUI.h
# End Source File
# Begin Source File

SOURCE=.\cfg_var.h
# End Source File
# Begin Source File

SOURCE=.\QCDBASS.h
# End Source File
# Begin Source File

SOURCE=.\qcdhelper.h
# End Source File
# Begin Source File

SOURCE=QCDInputDLL.h
# End Source File
# Begin Source File

SOURCE=QCDModDefs.h
# End Source File
# Begin Source File

SOURCE=QCDModInput.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\tags.h
# End Source File
# Begin Source File

SOURCE=.\VorbisComment.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\logo_bas.bmp
# End Source File
# Begin Source File

SOURCE=.\logo_bass.bmp
# End Source File
# Begin Source File

SOURCE=.\QCDBASS.rc
# End Source File
# Begin Source File

SOURCE=.\ss_bar.bmp
# End Source File
# Begin Source File

SOURCE=.\stream_s.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\qcdbass.nsi
# End Source File
# End Target
# End Project
