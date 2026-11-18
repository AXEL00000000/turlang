@echo off
rem Desinstala TurLang del usuario actual (doble clic).
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0desinstalar.ps1"
set CODIGO=%ERRORLEVEL%
echo.
pause
exit /b %CODIGO%
