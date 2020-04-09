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

set cur_file=%target_dir%\main.cpp
powershell -Command "(gc %cur_file%) -replace 'TEMPLATE', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\resource.h
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\Template.sln
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\Template.vcxproj
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\Template.vcxproj.filters
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\Template.vcxproj.user
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\TemplateDreamcast.sln
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\TemplateDreamcast.vcxproj
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\TemplateDreamcast.vcxproj.filters
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\TemplateDreamcast.vcxproj.user
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\TemplateWeb.sln
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\TemplateWeb.vcxproj
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\TemplateWeb.vcxproj.filters
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\TemplateWeb.vcxproj.user
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"
set cur_file=%target_dir%\windows\TemplateAll.sln
powershell -Command "(gc %cur_file%) -replace 'Template', '%1' | Out-File %cur_file%"

rename %target_dir%\gitignore.txt .gitignore
rename %target_dir%\gitattributes.txt .gitattributes
rename %target_dir%\windows\Template.rc %1.rc
rename %target_dir%\windows\Template.sln %1.sln
rename %target_dir%\windows\Template.vcxproj %1.vcxproj
rename %target_dir%\windows\Template.vcxproj.filters %1.vcxproj.filters
rename %target_dir%\windows\Template.vcxproj.user %1.vcxproj.user
rename %target_dir%\windows\TemplateDreamcast.sln %1Dreamcast.sln
rename %target_dir%\windows\TemplateDreamcast.vcxproj %1Dreamcast.vcxproj
rename %target_dir%\windows\TemplateDreamcast.vcxproj.filters %1Dreamcast.vcxproj.filters
rename %target_dir%\windows\TemplateDreamcast.vcxproj.user %1Dreamcast.vcxproj.user
rename %target_dir%\windows\TemplateWeb.sln %1Web.sln
rename %target_dir%\windows\TemplateWeb.vcxproj %1Web.vcxproj
rename %target_dir%\windows\TemplateWeb.vcxproj.filters %1Web.vcxproj.filters
rename %target_dir%\windows\TemplateWeb.vcxproj.user %1Web.vcxproj.user
rename %target_dir%\windows\TemplateAll.sln %1All.sln

md %target_dir%\concept

powershell -Command "Get-Content %target_dir%\main.cpp | Set-Content -Encoding ascii %target_dir%\maintemp.cpp"
del -f %target_dir%\main.cpp
move %target_dir%\maintemp.cpp %target_dir%\main.cpp

:over
