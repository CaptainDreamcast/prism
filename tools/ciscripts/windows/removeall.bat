call setprojects.bat

setlocal enableDelayedExpansion

for /l %%x in (1, 1, %_libtari_projectamount%) do (
   set _libtari_index=%%x
   echo Removing project !_libtari_projects[%%x]!
   call removesingle.bat
)

endlocal