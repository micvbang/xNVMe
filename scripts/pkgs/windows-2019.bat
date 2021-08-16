@echo off
@setlocal enableextensions enabledelayedexpansion

net session >nul: 2>&1
if errorlevel 1 (
	echo %0 must be run with Administrator privileges
	goto :eof
)

set "PATH=%ALLUSERSPROFILE%\chocolatey\bin;%PATH%"
where /q choco
if errorlevel 1 (
	echo Installing Chocolatey Manager ...
	powershell -NoProfile -InputFormat None -ExecutionPolicy Bypass -Command "[System.Net.ServicePointManager]::SecurityProtocol = 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"
	if errorlevel 1 goto :eof
)

set "PATH=%ProgramFiles%\LLVM\bin;%PATH%"
where /q clang
REM if errorlevel 1 if "%CLANG%"=="y" (
if errorlevel 1 (
	echo Installing Clang ...
	choco install llvm -y -r
	if errorlevel 1 goto :eof
)

set "PATH=%SystemDrive%\tools\msys64;%PATH%"
where /q msys2
if errorlevel 1 (
	echo Installing MSYS2 ...
	choco install msys2 -y -r
	if errorlevel 1 goto :eof
)

set MSYS2=call msys2_shell -no-start -here -use-full-path -defterm

set ver=
( for /f "tokens=*" %%f in ('%MSYS2% -c "make --version 2>/dev/null"') do (set ver=%%f) ) 2>nul:
if "%ver%"=="" (
	echo Installing make ...
	%MSYS2% -c "pacman --noconfirm -Syy msys/make"
)

set ver=
( for /f "tokens=*" %%f in ('%MSYS2% -c "cmp --version 2>/dev/null"') do (set ver=%%f) ) 2>nul:
if "%ver%"=="" (
	echo Installing diffutils ...
	%MSYS2% -c "pacman --noconfirm -Syy msys/diffutils"
)

set ver=
( for /f "tokens=*" %%f in ('%MSYS2% -c "autoconf --version 2>/dev/null"') do (set ver=%%f) ) 2>nul:
if "%ver%"=="" (
	echo Installing autoconf ...
	%MSYS2% -c "pacman --noconfirm -Syy msys/autoconf"
)