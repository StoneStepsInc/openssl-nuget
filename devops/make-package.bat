@echo off

setlocal

if "%~1" == "" (
  echo Package revision must be provided as the first argument
  goto :EOF
)

set PKG_VER=3.38.2
set PKG_VER_ABBR=3380200
set PKG_REV=%~1

set SQLITE_FNAME=sqlite-amalgamation-%PKG_VER_ABBR%.zip
set SQLITE_DNAME=sqlite-amalgamation-%PKG_VER_ABBR%

rem original SQLite SHA3-256 hash: 00a008f1df87764c9ae794e9e7a68ae9a377f807d03c7a3ea9fc0ac3a1a1236f
set SQLITE_SHA256=8f766439c9fa1ae24ec1bdb71d7b58f0d9a90027cf03abdddb07b618e0a5332c

set SEVENZIP_EXE=c:\Program Files\7-Zip\7z.exe
set VCVARSALL=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall

curl --output %SQLITE_FNAME% https://www.sqlite.org/2022/%SQLITE_FNAME%

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
set VCVARSALL=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall

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

cl /c /O2 /Zi /DNDEBUG /FoRelease\ sqlite3.c
lib /MACHINE:X64 /OUT:Release\sqlite3.lib Release\sqlite3.obj

cl /O2 /DNDEBUG /FeRelease\sqlite3.exe sqlite3.c shell.c

mkdir Debug

cl /c /Od /Zi /FoDebug\ sqlite3.c 
lib /MACHINE:X64 /OUT:Debug\sqlite3.lib Debug\sqlite3.obj

rem
rem Create a Nuget package
rem
mkdir ..\nuget\build\native\include
copy *.h ..\nuget\build\native\include\

mkdir ..\nuget\build\native\bin
copy /Y Release\sqlite3.exe ..\nuget\build\native\bin

mkdir ..\nuget\build\native\lib\x64\Release
copy /Y Release\sqlite3.lib ..\nuget\build\native\lib\x64\Release

mkdir ..\nuget\build\native\lib\x64\Debug
copy /Y Debug\sqlite3.lib ..\nuget\build\native\lib\x64\Debug
copy /Y *.pdb ..\nuget\build\native\lib\x64\Debug

cd ..

nuget pack nuget\StoneSteps.SQLite.Static.nuspec -Version %PKG_VER%.%PKG_REV%
