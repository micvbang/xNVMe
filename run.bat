@echo off
@setlocal enableextensions enabledelayedexpansion

set "PATH=%ALLUSERSPROFILE%\chocolatey\bin;!PATH!"
set "PATH=%SystemDrive%\tools\msys64;!PATH!"
set "PATH=%ProgramFiles%\LLVM\bin;!PATH!"
set "PATH=%SystemDrive%\tools\msys64\usr\local\bin;!PATH!"

xnvme.exe enum
xnvme.exe library-info