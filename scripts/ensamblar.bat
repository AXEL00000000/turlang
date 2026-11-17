@echo off
rem Ensambla y enlaza un .asm generado por turlang: programa.asm -> programa.exe
rem Usa el NASM y el gcc de 32 bits que trae SASM (carpeta en SASM_DIR, si existe).
setlocal

if "%~1"=="" goto uso
if not exist "%~1" goto noExiste

set "SASM=C:\Program Files (x86)\SASM"
if defined SASM_DIR set "SASM=%SASM_DIR%"
set "NASM=%SASM%\NASM\nasm.exe"
set "GCCDIR=%SASM%\MinGW\bin"

if not exist "%NASM%" goto sinNasm
if not exist "%GCCDIR%\gcc.exe" goto sinGcc

set "PATH=%GCCDIR%;%PATH%"
"%NASM%" -f win32 "%~1" -o "%~dpn1.obj"
if errorlevel 1 goto falloNasm
gcc -m32 "%~dpn1.obj" -o "%~dpn1.exe"
if errorlevel 1 goto falloGcc
echo generado: %~dpn1.exe
exit /b 0

:uso
echo uso: ensamblar.bat programa.asm
exit /b 2

:noExiste
echo ensamblar: no existe el archivo %~1
exit /b 2

:sinNasm
echo ensamblar: no se encontro nasm.exe en "%SASM%\NASM". Instala SASM o define SASM_DIR.
exit /b 2

:sinGcc
echo ensamblar: no se encontro el gcc de 32 bits en "%GCCDIR%". Instala SASM o define SASM_DIR.
exit /b 2

:falloNasm
echo ensamblar: NASM no pudo ensamblar %~1
exit /b 1

:falloGcc
echo ensamblar: gcc no pudo enlazar %~dpn1.obj
exit /b 1
