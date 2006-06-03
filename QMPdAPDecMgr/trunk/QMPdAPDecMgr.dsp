# Microsoft Developer Studio Project File - Name="QMPdAPDecMgr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=QMPdAPDecMgr - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "QMPdAPDecMgr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QMPdAPDecMgr.mak" CFG="QMPdAPDecMgr - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QMPdAPDecMgr - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "QMPdAPDecMgr - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "QMPdAPDecMgr"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "QMPdAPDecMgr - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GF /Gy /Fp".\Release/QMPdAPDecMgr.pch" /Fo".\Release/" /Fd".\Release/" /c /GX 
# ADD CPP /nologo /MT /W3 /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GF /Gy /Fp".\Release/QMPdAPDecMgr.pch" /Fo".\Release/" /Fd".\Release/" /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\Release\QMPdAPDecMgr.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\Release\QMPdAPDecMgr.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib /nologo /dll /out:".\Release\QMPdAPDecMgr.dll" /incremental:no /pdb:".\Release\QMPdAPDecMgr.pdb" /pdbtype:sept /subsystem:windows /implib:".\Release/QMPdAPDecMgr.lib" /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib /nologo /dll /out:".\Release\QMPdAPDecMgr.dll" /incremental:no /pdb:".\Release\QMPdAPDecMgr.pdb" /pdbtype:sept /subsystem:windows /implib:".\Release/QMPdAPDecMgr.lib" /machine:ix86 
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Compressing with UPX...
PostBuild_Cmds=D:\System\UPX\upx.exe -9 "$(TargetPath)"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "QMPdAPDecMgr - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp".\Debug/QMPdAPDecMgr.pch" /Fo".\Debug/" /Fd".\Debug/" /FR".\Debug/" /GZ /c /GX 
# ADD CPP /nologo /MTd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp".\Debug/QMPdAPDecMgr.pch" /Fo".\Debug/" /Fd".\Debug/" /FR".\Debug/" /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\Debug\QMPdAPDecMgr.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\Debug\QMPdAPDecMgr.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib /nologo /dll /out:"D:\Media\Quintessential Media Player\Plugins\QMPdAPDecMgr.dll" /incremental:no /debug /pdb:".\Debug\QMPdAPDecMgr.pdb" /pdbtype:sept /subsystem:windows /implib:".\Debug/QMPdAPDecMgr.lib" /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib /nologo /dll /out:"D:\Media\Quintessential Media Player\Plugins\QMPdAPDecMgr.dll" /incremental:no /debug /pdb:".\Debug\QMPdAPDecMgr.pdb" /pdbtype:sept /subsystem:windows /implib:".\Debug/QMPdAPDecMgr.lib" /machine:ix86 

!ENDIF

# Begin Target

# Name "QMPdAPDecMgr - Win32 Release"
# Name "QMPdAPDecMgr - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=QMPdAPDecMgr.cpp

!IF  "$(CFG)" == "QMPdAPDecMgr - Win32 Release"

# ADD CPP /nologo /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "QMPdAPDecMgr - Win32 Debug"

# ADD CPP /nologo /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /GZ /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\ConfigDlg.h
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

SOURCE=.\QMPdAPDecMgr.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\Wave.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\QMPdAPDecMgr.rc
# End Source File
# End Group
# End Target
# End Project

