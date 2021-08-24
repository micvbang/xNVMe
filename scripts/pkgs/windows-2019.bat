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
if errorlevel 1 (
	echo Installing Clang ...
	choco install llvm -y -r
	if errorlevel 1 goto :eof
)

set "PATH=%ProgramFiles%\CMake\bin;%PATH%"
where /q cmake
if errorlevel 1 (
	echo Installing cmake ...
	choco install cmake -y -r
	if errorlevel 1 goto :eof
)

where /q make
if errorlevel 1 (
	echo Installing make ...
	choco install make -y -r
	if errorlevel 1 goto :eof
)

set msvc=y
set vc=
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%vswhere%" for /f "tokens=*" %%i in ('"%vswhere%" -latest -find VC') do set "vc=%%i"
if "%msvc%%vc%"=="y" (
	echo Installing Visual Studio ...
	choco install visualstudio2019community -y -r
	if errorlevel 1 goto :eof
	choco install visualstudio2019-workload-nativedesktop -y -r
	if errorlevel 1 goto :eof
)

set "PATH=%SystemDrive%\tools\msys64;%PATH%"
where /q msys2
if errorlevel 1 (
	echo Installing MSYS2 ...
	choco install msys2 -y -r
	if errorlevel 1 goto :eof
)
