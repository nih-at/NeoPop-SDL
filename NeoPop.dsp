# Microsoft Developer Studio Project File - Name="NeoPop" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=NEOPOP - WIN32 MAIN
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NeoPop.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NeoPop.mak" CFG="NEOPOP - WIN32 MAIN"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NeoPop - Win32 Debugger" (based on "Win32 (x86) Application")
!MESSAGE "NeoPop - Win32 Main" (based on "Win32 (x86) Application")
!MESSAGE "NeoPop - Win32 Debugger_DEBUG" (based on "Win32 (x86) Application")
!MESSAGE "NeoPop - Win32 Main_DEBUG" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NeoPop - Win32 Debugger"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "NeoPop___Win32_Debugger"
# PROP BASE Intermediate_Dir "NeoPop___Win32_Debugger"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj\System_Win32\Debugger\"
# PROP Intermediate_Dir "obj\System_Win32\Debugger\"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /Gr /W3 /O2 /Ob2 /I "..\zLIB\\" /I "Common\\" /I "TLCS900h\\" /I "Z80\\" /I "System_Win32\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT BASE CPP /Fr /YX
# ADD CPP /nologo /G5 /Gr /W3 /O2 /Ob2 /I "Core\\" /I "Core\TLCS-900h\\" /I "Core\Z80\\" /D "NEOPOP_DEBUG" /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /WX /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dxguid.lib dinput8.lib ddraw.lib dsound.lib kernel32.lib user32.lib comdlg32.lib gdi32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"..\bin\NeoPop.exe"
# SUBTRACT BASE LINK32 /map /debug
# ADD LINK32 dxguid.lib ddraw.lib dinput.lib dsound.lib kernel32.lib user32.lib comdlg32.lib gdi32.lib advapi32.lib ws2_32.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"NeoPopDebugger.exe"
# SUBTRACT LINK32 /map /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=upx -9 NeoPopDebugger.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "NeoPop - Win32 Main"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "NeoPop___Win32_Main"
# PROP BASE Intermediate_Dir "NeoPop___Win32_Main"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj\System_Win32\Main\"
# PROP Intermediate_Dir "obj\System_Win32\Main\"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /Gr /W3 /O2 /Ob2 /I "..\zLIB\\" /I "Common\\" /I "TLCS900h\\" /I "Z80\\" /I "System_Win32\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT BASE CPP /Fr /YX
# ADD CPP /nologo /G5 /Gr /W3 /O2 /Ob2 /I "Core\\" /I "Core\TLCS-900h\\" /I "Core\Z80\\" /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /FAs /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dxguid.lib dinput8.lib ddraw.lib dsound.lib kernel32.lib user32.lib comdlg32.lib gdi32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"..\bin\NeoPop.exe"
# SUBTRACT BASE LINK32 /map /debug
# ADD LINK32 dxguid.lib ddraw.lib dinput.lib dsound.lib kernel32.lib user32.lib comdlg32.lib gdi32.lib advapi32.lib ws2_32.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"NeoPop-Win32.exe"
# SUBTRACT LINK32 /map /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=upx -9 NeoPop-Win32.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "NeoPop - Win32 Debugger_DEBUG"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "NeoPop___Win32_Debugger_DEBUG"
# PROP BASE Intermediate_Dir "NeoPop___Win32_Debugger_DEBUG"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj\System_Win32\Debugger_DEBUG\"
# PROP Intermediate_Dir "obj\System_Win32\Debugger_DEBUG\"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /Gr /W3 /O2 /Ob2 /I "..\zLIB\\" /I "Common\\" /I "TLCS900h\\" /I "Z80\\" /I "System_Win32\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "NEOPOP_DEBUG" /FD /c
# SUBTRACT BASE CPP /Fr /YX
# ADD CPP /nologo /G5 /Gr /W3 /Zi /Od /I "Core\\" /I "Core\TLCS-900h\\" /I "Core\Z80\\" /D "NEOPOP_DEBUG" /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /Fr /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dxguid.lib dinput8.lib ddraw.lib dsound.lib kernel32.lib user32.lib comdlg32.lib gdi32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"..\bin\NPDebug.exe"
# SUBTRACT BASE LINK32 /map /debug
# ADD LINK32 dxguid.lib ddraw.lib dinput.lib dsound.lib kernel32.lib user32.lib comdlg32.lib gdi32.lib advapi32.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"NeoPopDebugger_DEBUG.exe"
# SUBTRACT LINK32 /pdb:none /map

!ELSEIF  "$(CFG)" == "NeoPop - Win32 Main_DEBUG"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "NeoPop___Win32_Main_DEBUG"
# PROP BASE Intermediate_Dir "NeoPop___Win32_Main_DEBUG"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "obj\System_Win32\Main_DEBUG\"
# PROP Intermediate_Dir "obj\System_Win32\Main_DEBUG\"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /Gr /W3 /O2 /Ob2 /I "..\zLIB\\" /I "Common\\" /I "TLCS900h\\" /I "Z80\\" /I "System_Win32\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT BASE CPP /Fr /YX
# ADD CPP /nologo /G5 /Gr /W3 /Zi /Od /I "Core\\" /I "Core\TLCS-900h\\" /I "Core\Z80\\" /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /Fr /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dxguid.lib dinput8.lib ddraw.lib dsound.lib kernel32.lib user32.lib comdlg32.lib gdi32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"..\bin\NeoPop.exe"
# SUBTRACT BASE LINK32 /map /debug
# ADD LINK32 dxguid.lib ddraw.lib dinput.lib dsound.lib kernel32.lib user32.lib comdlg32.lib gdi32.lib advapi32.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"NeoPop-Win32_DEBUG.exe"
# SUBTRACT LINK32 /pdb:none /map

!ENDIF 

# Begin Target

# Name "NeoPop - Win32 Debugger"
# Name "NeoPop - Win32 Main"
# Name "NeoPop - Win32 Debugger_DEBUG"
# Name "NeoPop - Win32 Main_DEBUG"
# Begin Group "NeoPop (Core)"

# PROP Default_Filter ""
# Begin Group "Core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Core\bios.c
# End Source File
# Begin Source File

SOURCE=.\Core\bios.h
# End Source File
# Begin Source File

SOURCE=.\Core\biosHLE.c
# End Source File
# Begin Source File

SOURCE=.\Core\dma.c
# End Source File
# Begin Source File

SOURCE=.\Core\dma.h
# End Source File
# Begin Source File

SOURCE=.\Core\flash.c
# End Source File
# Begin Source File

SOURCE=.\Core\flash.h
# End Source File
# Begin Source File

SOURCE=.\Core\gfx.c
# End Source File
# Begin Source File

SOURCE=.\Core\gfx.h
# End Source File
# Begin Source File

SOURCE=.\Core\gfx_scanline_colour.c
# End Source File
# Begin Source File

SOURCE=.\Core\gfx_scanline_mono.c
# End Source File
# Begin Source File

SOURCE=.\Core\interrupt.c
# End Source File
# Begin Source File

SOURCE=.\Core\interrupt.h
# End Source File
# Begin Source File

SOURCE=.\Core\mem.c
# End Source File
# Begin Source File

SOURCE=.\Core\mem.h
# End Source File
# Begin Source File

SOURCE=.\Core\rom.c
# End Source File
# Begin Source File

SOURCE=.\Core\sound.c
# End Source File
# Begin Source File

SOURCE=.\Core\sound.h
# End Source File
# Begin Source File

SOURCE=.\Core\state.c
# End Source File
# Begin Source File

SOURCE=.\Core\state.h
# End Source File
# Begin Source File

SOURCE=.\Core\Z80_interface.c
# End Source File
# Begin Source File

SOURCE=.\Core\Z80_interface.h
# End Source File
# End Group
# Begin Group "TLCS-900h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_disassemble.c"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_disassemble_dst.c"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_disassemble_extra.c"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_disassemble_reg.c"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_disassemble_src.c"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_interpret.c"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_interpret_dst.c"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_interpret_reg.c"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_interpret_single.c"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_interpret_src.c"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers.c"
# End Source File
# End Group
# Begin Group "TLCS-900h (Headers)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_disassemble.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_interpret.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_interpret_dst.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_interpret_reg.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_interpret_single.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_interpret_src.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapB.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeB0.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeB1.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeB2.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeB3.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeL0.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeL1.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeL2.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeL3.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeW0.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeW1.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeW2.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapCodeW3.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapL.h"
# End Source File
# Begin Source File

SOURCE=".\Core\TLCS-900h\TLCS900h_registers_mapW.h"
# End Source File
# End Group
# Begin Group "z80"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Core\z80\Codes.h
# End Source File
# Begin Source File

SOURCE=.\Core\z80\CodesCB.h
# End Source File
# Begin Source File

SOURCE=.\Core\z80\CodesED.h
# End Source File
# Begin Source File

SOURCE=.\Core\z80\CodesXCB.h
# End Source File
# Begin Source File

SOURCE=.\Core\z80\CodesXX.h
# End Source File
# Begin Source File

SOURCE=.\Core\z80\dasm.c
# End Source File
# Begin Source File

SOURCE=.\Core\z80\Tables.h
# End Source File
# Begin Source File

SOURCE=.\Core\z80\Z80.c
# End Source File
# Begin Source File

SOURCE=.\Core\z80\Z80.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Core\Docs\changelog.txt
# End Source File
# Begin Source File

SOURCE=".\Core\Docs\Memory Map.txt"
# End Source File
# Begin Source File

SOURCE=.\Core\neopop.c
# End Source File
# Begin Source File

SOURCE=.\Core\neopop.h
# End Source File
# Begin Source File

SOURCE=".\Core\Docs\source-readme.txt"
# End Source File
# Begin Source File

SOURCE=.\Core\Docs\todo.txt
# End Source File
# End Group
# Begin Group "System_Win32"

# PROP Default_Filter ""
# Begin Group "Debugger"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\System_Win32\Debugger\debug_breakpoint.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\Debugger\debug_codeview.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\Debugger\debug_memview.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\Debugger\debug_regview.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\Debugger\debug_watch.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\Debugger\debug_z80regview.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\Debugger\system_debug.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\Debugger\system_debug.h
# End Source File
# End Group
# Begin Group "docs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\System_Win32\Docs\changelog.txt
# End Source File
# Begin Source File

SOURCE=".\System_Win32\docs\NeoPop DirectSound.txt"
# End Source File
# Begin Source File

SOURCE=.\System_Win32\Docs\todo.txt
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "ico"
# Begin Source File

SOURCE=.\System_Win32\Go.ico
# End Source File
# Begin Source File

SOURCE=.\System_Win32\icon.ico
# End Source File
# Begin Source File

SOURCE=.\System_Win32\neopop.bmp
# End Source File
# Begin Source File

SOURCE=.\System_Win32\resource.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\Resource.rc
# End Source File
# Begin Source File

SOURCE=.\System_Win32\stop.ico
# End Source File
# End Group
# Begin Group "zLIB"

# PROP Default_Filter ""
# Begin Source File

SOURCE=System_Win32\zLIB\adler32.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\crc32.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\infblock.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\infblock.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\infcodes.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\infcodes.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\inffast.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\inffast.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\inffixed.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\inflate.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\inftrees.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\infutil.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\infutil.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\unzip.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\unzip.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\zconf.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\zlib.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\zLIB\zutil.c
# End Source File
# Begin Source File

SOURCE=System_Win32\zLIB\zutil.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\System_Win32\system_comms.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_comms.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_config.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_config.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_graphics.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_graphics.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_input.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_input.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_input_dialog.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_io.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_language.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_language.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_main.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_main.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_rom.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_rom.h
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_sound.c
# End Source File
# Begin Source File

SOURCE=.\System_Win32\system_sound.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\bugs.html
# End Source File
# Begin Source File

SOURCE=.\License.txt
# End Source File
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Target
# End Project
