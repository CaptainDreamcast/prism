if "%1" == "" goto :over
echo %1

if "%2" == "" goto :no_directory
set directory=%2
goto :directory_over
:no_directory
set directory=.
:directory_over

set source_dir=%0\..\..\..\assets\template
set target_dir=%directory%\%1

xcopy /E /I /Y %source_dir% %target_dir%

set cur_file=%target_dir%\main.c
powershell -Command "(gc %cur_file%) -replace 'TEMPLATE', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\resource.h
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\Template.sln
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\Template.vcxproj
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\Template.vcxproj.filters
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"

rename %target_dir%\gitignore.txt .gitignore
rename %target_dir%\windows\Template.rc %1.rc
rename %target_dir%\windows\Template.sln %1.sln
rename %target_dir%\windows\Template.vcxproj %1.vcxproj
rename %target_dir%\windows\Template.vcxproj.filters %1.vcxproj.filters
rename %target_dir%\windows\Template.vcxproj.user %1.vcxproj.user

md %target_dir%\assets
md %target_dir%\concept

:over
