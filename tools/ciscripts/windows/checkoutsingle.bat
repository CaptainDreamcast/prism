REM Expects _libtari_index index and setprojects to have been called before

set _libtari_projectpath=..\..\..\..\..\!_libtari_projects[%_libtari_index%]!
git clone !_libtari_repos[%_libtari_index%]!
move !_libtari_projects[%_libtari_index%]! %_libtari_projectpath%