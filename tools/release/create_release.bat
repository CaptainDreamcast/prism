@echo off

if "%~7"=="" (
    echo Usage:
    echo create_release.bat ^<project_name^> ^<target_folder^> ^<release_name^> ^<dreamcast_name^> ^<vita_title^> ^<vita_id^> ^<vita_version^>
	echo example: create_release.bat DolmexicaInfinite TestRelease "Dolmexica Infinite" DOLMEXICA_INFINITE "Dolmexica Infinite" DOLM00001 01.00
    exit /b 1
)

set "prism_folder=%~dp0\..\.."
set "project_name=%~1"
set "project_folder=C:\DEV\PROJECTS\%project_name%"
set "target_folder=%~2"
set "release_name=%~3"
set "dreamcast_name=%~4"
set "vita_title=%~5"
set "vita_id=%~6"
set "vita_version=%~7"

cd %project_folder%
mkdir release
cd release
rmdir /s /q "%target_folder%"
mkdir "%target_folder%"
cd ..
cd windows 
msbuild %project_name%All.sln /t:"%project_name%:clean" /p:Configuration="Release" /p:Platform="x64"
msbuild %project_name%All.sln /t:"%project_name%" /p:Configuration="Release" /p:Platform="x64"
cd ..
xcopy /i /y %project_name%.exe "release\%target_folder%\%release_name% for Windows.exe"*
xcopy /i /y fmod.dll "release\%target_folder%\fmod.dll"*
xcopy /s /e /i /y assets "release\%target_folder%\assets"

cd windows 
msbuild %project_name%All.sln /t:"%project_name%:clean" /p:Configuration="Release" /p:Platform="x64"
msbuild %project_name%All.sln /t:"%project_name%Dreamcast:clean" /p:Configuration="Release" /p:Platform="x64"
msbuild %project_name%All.sln /t:"%project_name%Dreamcast" /p:Configuration="Release" /p:Platform="x64"
cd ..

xcopy /s /e /i /y %prism_folder%\tools\release\dreamcast "release\%target_folder%\dreamcast"
mkdir "release\%target_folder%\dreamcast\filesystem"
mkdir "release\%target_folder%\dreamcast\ip"
xcopy /i /y boot\INSERT.png "release\%target_folder%\dreamcast\ip\INSERT.png"*
xcopy /i /y boot\ip.txt "release\%target_folder%\dreamcast\ip\ip.txt"*
%prism_folder%\tools\release\build_tools\dreamcast\scramble.exe 1ST_READ.BIN "release\%target_folder%\dreamcast\filesystem\1ST_READ.BIN"

cd windows 
msbuild %project_name%All.sln /t:"%project_name%Dreamcast:clean" /p:Configuration="Release" /p:Platform="x64"
msbuild %project_name%All.sln /t:"%project_name%Vita:clean" /p:Configuration="Release" /p:Platform="x64"
msbuild %project_name%All.sln /t:"%project_name%Vita" /p:Configuration="Release" /p:Platform="x64"
cd ..

xcopy /s /e /i /y %prism_folder%\tools\release\vita "release\%target_folder%\vita"
mkdir "release\%target_folder%\vita\common"
xcopy /s /e /i /y vita\sce_sys "release\%target_folder%\vita\common\sce_sys"
xcopy /i /y vita\build\%project_name%.self "release\%target_folder%\vita\common\%project_name%.self"*

cd windows 
msbuild %project_name%All.sln /t:"%project_name%Vita:clean" /p:Configuration="Release" /p:Platform="x64"
msbuild %project_name%All.sln /t:"%project_name%Web:clean" /p:Configuration="Release" /p:Platform="x64"
msbuild %project_name%All.sln /t:"%project_name%Web" /p:Configuration="Release" /p:Platform="x64"
cd ..

xcopy /s /e /i /y %prism_folder%\tools\release\web "release\%target_folder%\web"
mkdir "release\%target_folder%\web\common"
xcopy /i /y web\game.js "release\%target_folder%\web\common\game.js"*
xcopy /i /y web\game.wasm "release\%target_folder%\web\common\game.wasm"*
xcopy /i /y web\index.html "release\%target_folder%\web\common\index.html"*

cd windows 
msbuild %project_name%All.sln /t:"%project_name%Web:clean" /p:Configuration="Release" /p:Platform="x64"
cd ..

mkdir "release\%target_folder%\releases"
cd "release\%target_folder%\dreamcast"
call make_cdi.bat %dreamcast_name% Game 50000
@ECHO on
xcopy /i /y Game.cdi "..\releases\%release_name% for Dreamcast.cdi"*
del Game.cdi

cd ..
cd vita
call make_vita.bat "%vita_title%" %vita_id% %vita_version%
@ECHO on
xcopy /i /y %vita_id%.vpk "..\releases\%release_name% for Vita.vpk"*
del %vita_id%.vpk
rmdir /s /q build

cd ..
cd web
call make_web.bat Game
@ECHO on
xcopy /i /y Game.zip "..\releases\%release_name% for Web.zip"*
del Game.zip
rmdir /s /q build

cd ..\..\..
