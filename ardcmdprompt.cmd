@echo off
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
goto callcmdfile


:nozb
rem
rem ZEPHYR_BASE not set. Set vars manually.
rem 
if EXIST NCSDIR goto vfilefound
echo.
echo. NCSDIR file not found. File must contain name of NCS directory.
echo  For example, ncs or v1.3.0
echo.
goto startcmd
:vfilefound
set /p ncsver=<NCSDIR

rem Set root dir and remove the trailing \
set ARDESCO_ROOT=%~dp0
set ARDESCO_ROOT=%ARDESCO_ROOT:~0,-1%

rem echo %ncsver%
echo %ARDESCO_ROOT%

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
goto startcmd

rem call the NCS setup cmd file
:callcmdfilefound
call %envcmd%
echo Configuration complete.

:startcmd
set envcmd=
cmd /Q /k title Ardesco Command Prompt
