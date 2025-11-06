setlocal enabledelayedexpansion

call espenv.bat ESP32S3

@echo on

set LANGS=EN FR IT ES DE_DE
for %%L in (%LANGS%) do (
	echo Bearbeite Sprache %%L
	set FSFN=.\wordclock%VERS%.%TYPE%.%FS%.%%L.bin

	del /q %BASEDIR%\data\*.*
	del /q %BASEDIR%\data\web\*.*
	del /q %BASEDIR%\data\ico\*.*
	copy %BASEDIR%\data%%L\*.html %BASEDIR%\data\web
	copy %BASEDIR%\data%%L\*.json %BASEDIR%\data
	copy %BASEDIR%\dataGLB\*.* %BASEDIR%\data\web
	copy %BASEDIR%\dataICO\*.* %BASEDIR%\data\ico

	%MKSP% --create %BASEDIR%\data -b 4096 -p 256 -s %SPIFFS_SIZE% "!FSFN!"
	
)

pause