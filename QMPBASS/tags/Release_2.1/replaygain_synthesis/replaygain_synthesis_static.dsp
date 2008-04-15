# Microsoft Developer Studio Project File - Name="replaygain_synthesis_static" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=replaygain_synthesis_static - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "replaygain_synthesis_static.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "replaygain_synthesis_static.mak" CFG="replaygain_synthesis_static - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "replaygain_synthesis_static - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "replaygain_synthesis_static - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "replaygain_synthesis"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "replaygain_synthesis_static - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /I ".\include" /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_MBCS" /YX /Fp".\Debug/replaygain_synthesis_static.pch" /Fo".\Debug/" /Fd".\Debug/" /GZ /c /GX 
# ADD CPP /nologo /MTd /I ".\include" /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_MBCS" /YX /Fp".\Debug/replaygain_synthesis_static.pch" /Fo".\Debug/" /Fd".\Debug/" /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Debug\replaygain_synthesis_static.lib" 
# ADD LIB32 /nologo /out:".\Debug\replaygain_synthesis_static.lib" 

!ELSEIF  "$(CFG)" == "replaygain_synthesis_static - Win32 Release"

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
# ADD BASE CPP /nologo /MT /I ".\include" /W3 /O2 /Ob1 /Op /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_MBCS" /GF /Gy /YX /Fp".\Release/replaygain_synthesis_static.pch" /Fo".\Release/" /Fd".\Release/" /c /GX 
# ADD CPP /nologo /MT /I ".\include" /W3 /O2 /Ob1 /Op /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_MBCS" /GF /Gy /YX /Fp".\Release/replaygain_synthesis_static.pch" /Fo".\Release/" /Fd".\Release/" /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Release\replaygain_synthesis_static.lib" 
# ADD LIB32 /nologo /out:".\Release\replaygain_synthesis_static.lib" 

!ENDIF

# Begin Target

# Name "replaygain_synthesis_static - Win32 Debug"
# Name "replaygain_synthesis_static - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=replaygain_synthesis.cpp

!IF  "$(CFG)" == "replaygain_synthesis_static - Win32 Debug"

# ADD CPP /nologo /Od /GZ /GX 
!ELSEIF  "$(CFG)" == "replaygain_synthesis_static - Win32 Release"

# ADD CPP /nologo /O2 /GX 
!ENDIF

# End Source File
# End Group
# Begin Group "Private Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=include\private\fast_float_math_hack.h
# End Source File
# End Group
# Begin Group "Public Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=replaygain_synthesis.h
# End Source File
# End Group
# End Target
# End Project

