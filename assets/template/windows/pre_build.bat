..\..\tools\genromfs-win\x64\Debug\genromfs.exe -d ..\assets -f %1\assets.pak
..\..\tools\bin2c\x64\Debug\bin2c.exe %1\assets.pak ..\assets.cpp romdisk_buffer
set cur_file=..\assets.cpp
%SYSTEMROOT%\System32\WindowsPowerShell\v1.0\powershell.exe -Command "(gc %cur_file%) -replace 'const char', 'char' | Out-File %cur_file%"
set cur_file=..\assets.cpp
%SYSTEMROOT%\System32\WindowsPowerShell\v1.0\powershell.exe -Command "(gc %cur_file%) -replace 'const int', 'int' | Out-File %cur_file%"
