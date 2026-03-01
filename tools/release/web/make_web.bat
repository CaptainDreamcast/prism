@ECHO OFF

IF [%1] == [] GOTO EMPTY

:EXECUTE
if exist build\ del /Q build\
mkdir build
external\python\python external/emscripten/tools/file_packager.py build/assets.data --use-preload-plugins --preload ../assets@assets --js-output=build/assets.js
copy common\index.html build\index.html
copy common\game.js build\game.js
copy common\game.wasm build\game.wasm
if exist %1.zip del /Q %1.zip
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('build', '%1.zip'); }"
GOTO END

:EMPTY
ECHO Usage: make_web.bat GameName
GOTO END

:END