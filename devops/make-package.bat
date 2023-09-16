@echo off

setlocal

if "%~1" == "" (
  echo Package revision must be provided as the first argument
  goto :EOF
)

set PKG_VER=3.43.0
set PKG_VER_ABBR=3430000
set PKG_REV=%~1

set SQLITE_FNAME=sqlite-amalgamation-%PKG_VER_ABBR%.zip
set SQLITE_DNAME=sqlite-amalgamation-%PKG_VER_ABBR%
set SQLITE_URL_BASE=https://www.sqlite.org/2023

rem original SQLite SHA3-256 hash: 1286856f5dff3b145f02dfe5aa626657c172a415889e5a265dc4dee83a799c03
set SQLITE_SHA256=bb5849ae4d7129c09d20596379a0b3f7b1ac59cf9998eba5ef283ea9b6c000a5

set SEVENZIP_EXE=c:\Program Files\7-Zip\7z.exe

curl --output %SQLITE_FNAME% %SQLITE_URL_BASE%/%SQLITE_FNAME%

"%SEVENZIP_EXE%" h -scrcSHA256 %SQLITE_FNAME% | findstr /C:"SHA256 for data" | call devops\check-sha256 "%SQLITE_SHA256%"

if ERRORLEVEL 1 (
  echo SHA-256 signature for %SQLITE_FNAME% does not match
  goto :EOF
)

"%SEVENZIP_EXE%" x %SQLITE_FNAME% 

cd %SQLITE_DNAME%

rem
rem Replace `Community` with `Enterprise` for Enterprise Edition
rem
set VCVARSALL=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall

rem
rem Set up environment for the VC++ x64 platform
rem
call "%VCVARSALL%" x64

rem
rem SQLite advises to build a DLL on this page:
rem
rem https://www.sqlite.org/howtocompile.html#building_a_windows_dll
rem
rem , but library functions are not exported, so a DLL without an
rem import library is created, which cannot be used for linking.
rem Static libraries are built instead in both configurations.
rem

mkdir Release

cl /c /MD /O2 /Zi /DNDEBUG /DSQLITE_ENABLE_MATH_FUNCTIONS /FoRelease\ /FdRelease\sqlite3.pdb sqlite3.c
lib /MACHINE:X64 /OUT:Release\sqlite3.lib Release\sqlite3.obj

cl /MD /O2 /DNDEBUG /DSQLITE_ENABLE_MATH_FUNCTIONS /FeRelease\sqlite3.exe sqlite3.c shell.c

mkdir Debug

cl /c /MDd /Od /Zi /DSQLITE_ENABLE_MATH_FUNCTIONS /FoDebug\ /FdDebug\sqlite3.pdb sqlite3.c 
lib /MACHINE:X64 /OUT:Debug\sqlite3.lib Debug\sqlite3.obj

rem
rem Create a Nuget package
rem
mkdir ..\nuget\build\native\include
copy /Y *.h ..\nuget\build\native\include\

mkdir ..\nuget\build\native\bin
copy /Y Release\sqlite3.exe ..\nuget\build\native\bin

mkdir ..\nuget\build\native\lib\x64\Release
copy /Y Release\sqlite3.lib ..\nuget\build\native\lib\x64\Release
copy /Y Release\sqlite3.pdb ..\nuget\build\native\lib\x64\Release

mkdir ..\nuget\build\native\lib\x64\Debug
copy /Y Debug\sqlite3.lib ..\nuget\build\native\lib\x64\Debug
copy /Y Debug\sqlite3.pdb ..\nuget\build\native\lib\x64\Debug

cd ..

nuget pack nuget\StoneSteps.SQLite.VS2022.Static.nuspec -Version %PKG_VER%.%PKG_REV%
