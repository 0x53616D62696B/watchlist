@echo off
call C:\BuildTools\Common7\Tools\VsDevCmd.bat -arch=amd64
if errorlevel 1 exit /b %errorlevel%
powershell.exe -NoLogo -ExecutionPolicy Bypass -File "%~dp0verify-windows.ps1"
exit /b %errorlevel%
