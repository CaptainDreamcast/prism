@ECHO OFF

IF [%1] == [] GOTO EMPTY
IF [%2] == [] GOTO EMPTY
IF [%3] == [] GOTO INIT_LBA
set /a audio_lba = %3
GOTO EXECUTE

:INIT_LBA
set /a audio_lba = 302

:EXECUTE
rmdir /s /q filesystem\assets
xcopy /s /e /i /y ..\assets filesystem\assets
rem move into folder so makeip finds IP.TMPL
cd tools && makeip ..\ip\ip.txt ..\filesystem\IP.BIN && cd ..
tools\pngtomr ip\INSERT.png ip\INSERT.mr
tools\logoinsert ip\INSERT.mr filesystem\IP.BIN
set /a data_start_lba = audio_lba + 11400
tools\mkisofs -C 0,%data_start_lba% -V %1 -G filesystem\IP.BIN -l -o %2.iso filesystem
tools\cdi4dc %2.iso %2.cdi -a %audio_lba%
del ip\INSERT.mr
del filesystem\IP.BIN
del %2.iso
rmdir /s /q filesystem\assets
GOTO END

:EMPTY
ECHO Usage: make_cdi.bat CD_NAME CdiName [AudioSectorCount]
GOTO END

:END