@echo off
rem Builds inventory.exe (requires Visual Studio Build Tools / cl.exe)
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo vcvars64.bat not found. Install Visual Studio Build Tools.
    exit /b 1
)

set CONN=%~dp0mysql-connector-c-6.1.11-winx64

cl /nologo /W3 inventory.c database.c /I"%CONN%\include" ^
   /link /LIBPATH:"%CONN%\lib" libmysql.lib
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

copy /y "%CONN%\lib\libmysql.dll" "%~dp0libmysql.dll" >nul
del /q *.obj 2>nul
echo Build OK: inventory.exe
exit /b 0