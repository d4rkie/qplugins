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
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /I "..\replaygain_synthesis" /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp".\Debug/QCDBASS.pch" /Fo".\Debug/" /Fd".\Debug/" /FR".\Debug/" /GZ /c /GX 
# ADD CPP /nologo /MTd /I "..\replaygain_synthesis" /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp".\Debug/QCDBASS.pch" /Fo".\Debug/" /Fd".\Debug/" /FR".\Debug/" /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\Debug\QCDBASS.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\Debug\QCDBASS.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib replaygain_synthesis_static.lib /nologo /dll /out:"D:\Media\Quintessential Player\Plugins\QCDBASS.dll" /incremental:no /libpath:"..\replaygain_synthesis\Debug" /debug /pdb:".\Debug\QCDBASS.pdb" /pdbtype:sept /subsystem:windows /implib:".\Debug/QCDBASS.lib" /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib replaygain_synthesis_static.lib /nologo /dll /out:"D:\Media\Quintessential Player\Plugins\QCDBASS.dll" /incremental:no /libpath:"..\replaygain_synthesis\Debug" /debug /pdb:".\Debug\QCDBASS.pdb" /pdbtype:sept /subsystem:windows /implib:".\Debug/QCDBASS.lib" /machine:ix86 

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
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /I "..\replaygain_synthesis" /W3 /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GF /Gy /Fp".\Release/QCDBASS.pch" /Fo".\Release/" /Fd".\Release/" /c /GX 
# ADD CPP /nologo /MT /I "..\replaygain_synthesis" /W3 /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GF /Gy /Fp".\Release/QCDBASS.pch" /Fo".\Release/" /Fd".\Release/" /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\Release\QCDBASS.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\Release\QCDBASS.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib replaygain_synthesis_static.lib /nologo /dll /out:".\Release\QCDBASS.dll" /incremental:no /libpath:"..\replaygain_synthesis\Release" /pdb:".\Release\QCDBASS.pdb" /pdbtype:sept /subsystem:windows /implib:".\Release/QCDBASS.lib" /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib replaygain_synthesis_static.lib /nologo /dll /out:".\Release\QCDBASS.dll" /incremental:no /libpath:"..\replaygain_synthesis\Release" /pdb:".\Release\QCDBASS.pdb" /pdbtype:sept /subsystem:windows /implib:".\Release/QCDBASS.lib" /machine:ix86 
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Creating Installer...
PostBuild_Cmds=D:\system\nsis\makensis.exe $(TargetName).nsi
# End Special Build Tool

!ENDIF

# Begin Target

# Name "QCDBASS - Win32 Debug"
# Name "QCDBASS - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bass_lib.cpp
# End Source File
# Begin Source File

SOURCE=.\BASSCfgUI.cpp
# End Source File
# Begin Source File

SOURCE=.\cfg_var.cpp
# End Source File
# Begin Source File

SOURCE=QCDBASS.cpp

!IF  "$(CFG)" == "QCDBASS - Win32 Debug"

# ADD CPP /nologo /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /GZ /GX 
!ELSEIF  "$(CFG)" == "QCDBASS - Win32 Release"

# ADD CPP /nologo /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=.\qcdhelper.cpp
# End Source File
# Begin Source File

SOURCE=.\tags.cpp
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

