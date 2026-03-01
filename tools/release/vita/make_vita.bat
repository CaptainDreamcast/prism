@ECHO OFF

IF [%1] == [] GOTO EMPTY
IF [%2] == [] GOTO EMPTY
IF [%3] == [] GOTO EMPTY

:EXECUTE
if exist build\ del /Q build\
mkdir build
external\vita-sdk\vita-mksfoex -s APP_VER=%3 -s TITLE_ID=%2 %1 build\param.sfo
external\vita-sdk\vita-pack-vpk -s build\param.sfo -b common\DolmexicaInfinite.self --add common/sce_sys/icon0.png=sce_sys/icon0.png --add common/sce_sys/livearea/contents/bg.png=sce_sys/livearea/contents/bg.png --add common/sce_sys/livearea/contents/startup.png=sce_sys/livearea/contents/startup.png --add common/sce_sys/livearea/contents/template.xml=sce_sys/livearea/contents/template.xml --add ../assets=assets %2.vpk 
GOTO END

:EMPTY
ECHO Usage: make_vita.bat GameName TitleID Version
GOTO END

:END