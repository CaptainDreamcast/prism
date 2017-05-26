REM Expects _libtari_index index and setprojects to have been called before

set _libtari_projectpath=..\..\..\..\..\!_libtari_projects[%_libtari_index%]!
set _libtari_solpath=%_libtari_projectpath%\windows\!_libtari_projects[%_libtari_index%]!.sln

git clone !_libtari_repos[%_libtari_index%]!
move !_libtari_projects[%_libtari_index%]! %_libtari_projectpath%
"C:\Program Files (x86)\MSBuild\14.0\Bin\msbuild" %_libtari_solpath% /t:build /p:Configuration=Release;Platform=x86