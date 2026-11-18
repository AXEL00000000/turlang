@echo off
rem Instala TurLang para el usuario actual (doble clic).
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0instalar.ps1"
set CODIGO=%ERRORLEVEL%
echo.
pause
exit /b %CODIGO%
