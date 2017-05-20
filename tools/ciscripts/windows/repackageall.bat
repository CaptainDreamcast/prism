call setprojects.bat

setlocal enableDelayedExpansion

for /l %%x in (1, 1, %_libtari_projectamount%) do (
   set _libtari_index=%%x
   echo Repackaging project !_libtari_projects[%%x]!
   call removesingle.bat
   call checkoutsingle.bat
   call repackagesingle.bat
)

endlocal