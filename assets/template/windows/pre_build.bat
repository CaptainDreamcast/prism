del ..\assets\*.exe
del ..\assets\*.dll
del ..\assets\*.exp
del ..\assets\*.iobj
del ..\assets\*.ipdb
del ..\assets\*.lib
del ..\assets\*.pdb
..\..\tools\genromfs-win\x64\Debug\genromfs.exe -d ..\assets -f x64\%1\assets.pak
..\..\tools\bin2c\x64\Debug\bin2c.exe x64\%1\assets.pak ..\assets.cpp romdisk_buffer
set cur_file=..\assets.cpp
%SYSTEMROOT%\System32\WindowsPowerShell\v1.0\powershell.exe -Command "(gc %cur_file%) -replace 'const char', 'char' | Out-File %cur_file%"
set cur_file=..\assets.cpp
%SYSTEMROOT%\System32\WindowsPowerShell\v1.0\powershell.exe -Command "(gc %cur_file%) -replace 'const int', 'int' | Out-File %cur_file%"
