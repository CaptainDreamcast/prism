@ECHO OFF
REM use with $(ProjectDir) as first param and path to index.html (default: ..\web\index.html) as second from Visual Studio
cd C:\emsdk && emsdk_env.bat && emrun "%1%2" & pause
