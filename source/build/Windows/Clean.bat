@echo off

del ..\..\..\release\*.pdb
del ..\..\..\release\*.exp
del ..\..\..\release\*.lib
del ..\..\..\release\*.ilk
del ..\..\..\release\log.txt

del *.sdf
del *.suo
del log.txt

rd /S /Q Debug
rd /S /Q Release
rd /S /Q ipch