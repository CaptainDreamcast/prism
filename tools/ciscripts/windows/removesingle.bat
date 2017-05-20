REM Expects _libtari_index index and setprojects to have been called before

set _libtari_projectpath=..\..\..\..\..\!_libtari_projects[%_libtari_index%]!
if exist %_libtari_projectpath% rd /s /q %_libtari_projectpath%
