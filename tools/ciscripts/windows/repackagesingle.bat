REM Expects _libtari_index index and setprojects to have been called before
 
set _libtari_projectpath=..\..\..\..\..\!_libtari_projects[%_libtari_index%]!
set _libtari_solpath=%_libtari_projectpath%\windows\!_libtari_projects[%_libtari_index%]!.sln
set _libtari_releasepath=%_libtari_projectpath%\windows\Release
set "_libtari_packagepath=%_libtari_projectpath%\packaged\windows\!_libtari_names[%_libtari_index%]! for Windows.zip"

"C:\Program Files (x86)\MSBuild\14.0\Bin\msbuild" %_libtari_solpath% /t:build /p:Configuration=Release;Platform=x86
For /F "tokens=*" %%I in ('dir %_libtari_releasepath%\*.obj /s /b') do del "%%~fI"
For /F "tokens=*" %%I in ('dir %_libtari_releasepath%\*.pdb /s /b') do del "%%~fI"
For /F "tokens=*" %%I in ('dir %_libtari_releasepath%\*.exp /s /b') do del "%%~fI"
For /F "tokens=*" %%I in ('dir %_libtari_releasepath%\*.ipdb /s /b') do del "%%~fI"
For /F "tokens=*" %%I in ('dir %_libtari_releasepath%\*.iexp /s /b') do del "%%~fI"
For /F "tokens=*" %%I in ('dir %_libtari_releasepath%\*.res /s /b') do del "%%~fI"
For /F "tokens=*" %%I in ('dir %_libtari_releasepath%\*.iobj /s /b') do del "%%~fI"
For /F "tokens=*" %%I in ('dir %_libtari_releasepath%\*.tlog /s /b') do rd /s /q "%%~fI"

echo Updating package "%_libtari_packagepath%"
del "%_libtari_packagepath%"
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('%_libtari_releasepath%', '%_libtari_packagepath%'); }"

set _libtari_originalpath=%CD%
cd %_libtari_projectpath%
git add .
git commit -m "[Update] update packaged zip for #Windows"
git push origin
cd %_libtari_originalpath%