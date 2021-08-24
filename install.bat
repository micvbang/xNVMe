@echo off
@setlocal enableextensions enabledelayedexpansion

set CC=clang
set TYPE=release
set PLATFORM_ID=Windows

set CXX=%CC%
if "%CC%"=="clang" set "CXX=clang++"

set LD=
if "%CC%"=="clang" set LD=lld-link

set ENV=CC='%CC%' CXX='%CXX%' LD='%LD%' PLATFORM_ID='%PLATFORM_ID%'

:: set msys2-shell
set SH=call msys2_shell -no-start -here -use-full-path -defterm

set "PATH=%ALLUSERSPROFILE%\chocolatey\bin;!PATH!"
set "PATH=%SystemDrive%\tools\msys64;!PATH!"
set "PATH=%ProgramFiles%\LLVM\bin;!PATH!"

%SH% -c "make install"