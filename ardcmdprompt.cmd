@echo off

rem
rem Set root dir and remove the trailing \
rem
set ARDESCO_ROOT=%~dp0
set ARDESCO_ROOT=%ARDESCO_ROOT:~0,-1%

echo Ardesco Root: %ARDESCO_ROOT%

rem
rem See if ZEPHYR_BASE set. If so, use it.
rem
if .%ZEPHYR_BASE%==. goto nozb
echo.
echo Ardesco command prompt
echo.
echo ZEPHYR_BASE predefined as %ZEPHYR_BASE%

rem
rem the env.cmd file will be found using ZEPHYR_BASE.
rem
set envcmd=%ZEPHYR_BASE%\..\toolchain\cmd\env.cmd
set /p ncsver=<%ZEPHYR_BASE%\..\nrf\VERSION

goto callcmdfile


:nozb
rem
rem ZEPHYR_BASE not set. Set vars manually.
rem 
echo ZEPHYR_BASE not set. Attempt to find NCS tree within Ardesco directory.

if EXIST %ARDESCO_ROOT%\NCSDIR goto vfilefound
echo.
echo. NCSDIR file not found. File must contain name of NCS directory.
echo  For example, ncs or v1.3.0
echo.
goto startcmd
:vfilefound
set /p ncsver=<%ARDESCO_ROOT%\NCSDIR

rem echo %ncsver%

echo.
echo Ardesco command prompt
echo NCS version %ncsver%
echo.

set envcmd=%ARDESCO_ROOT%\%ncsver%\toolchain\cmd\env.cmd

:callcmdfile
if EXIST %envcmd% goto callcmdfilefound
echo Environment cmd file not found.
echo.Setup incomplete.
echo.
goto skipcmd

rem call the NCS setup cmd file
:callcmdfilefound
call %envcmd%
echo Configuration complete.

:startcmd
set envcmd=
cmd /Q /k title Ardesco Command Prompt  NCS %ncsver%
:skipcmd