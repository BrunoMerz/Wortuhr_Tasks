call espenv.bat ESP32S3

@echo on

set LANG=DE_DE
set FSFN=%BASEDIR%\bin-tar\wordclock%VERS%.%TYPE%.%FS%.%LANG%.bin
del %BASEDIR%\data\*.json
copy %BASEDIR%\data%LANG%\*.html %BASEDIR%\data\web
copy %BASEDIR%\data%LANG%\*.json %BASEDIR%\data
%MKSP% --create %BASEDIR%\data -b 4096 -p 256 -s %SPIFFS_SIZE% "%FSFN%"

set LANG=EN
set FSFN=%BASEDIR%\bin-tar\wordclock%VERS%.%TYPE%.%FS%.%LANG%.bin
del %BASEDIR%\data\*.json
copy %BASEDIR%\data%LANG%\*.html %BASEDIR%\data\web
copy %BASEDIR%\data%LANG%\*.json %BASEDIR%\data
%MKSP% --create %BASEDIR%\data -b 4096 -p 256 -s %SPIFFS_SIZE% "%FSFN%"

set LANG=FR
set FSFN=%BASEDIR%\bin-tar\wordclock%VERS%.%TYPE%.%FS%.%LANG%.bin
del %BASEDIR%\data\*.json
copy %BASEDIR%\data%LANG%\*.html %BASEDIR%\data\web
copy %BASEDIR%\data%LANG%\*.json %BASEDIR%\data
%MKSP% --create %BASEDIR%\data -b 4096 -p 256 -s %SPIFFS_SIZE% "%FSFN%"

set LANG=IT
set FSFN=%BASEDIR%\bin-tar\wordclock%VERS%.%TYPE%.%FS%.%LANG%.bin
del %BASEDIR%\data\*.json
copy %BASEDIR%\data%LANG%\*.html %BASEDIR%\data\web
copy %BASEDIR%\data%LANG%\*.json %BASEDIR%\data
%MKSP% --create %BASEDIR%\data -b 4096 -p 256 -s %SPIFFS_SIZE% "%FSFN%"

set LANG=ES
set FSFN=%BASEDIR%\bin-tar\wordclock%VERS%.%TYPE%.%FS%.%LANG%.bin
del %BASEDIR%\data\*.json
copy %BASEDIR%\data%LANG%\*.html %BASEDIR%\data\web
copy %BASEDIR%\data%LANG%\*.json %BASEDIR%\data
%MKSP% --create %BASEDIR%\data -b 4096 -p 256 -s %SPIFFS_SIZE% "%FSFN%"
del %BASEDIR%\data\*.json

pause
