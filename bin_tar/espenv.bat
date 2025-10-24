@echo off

echo %1%

rem set ESPT=python C:\Users\mz\Documents\ESP-Tools\esptool-master\esptool.py
set ESPT=python C:\Users\mz\.platformio\packages\tool-esptoolpy\esptool.py
rem set ESPT=C:\Users\mz\Documents\ESP-Tools\esptool\esptool.exe

set BASEDIR=..

rem set MKSP=C:\Users\mz\Documents\ESP-Tools\mkspiffs\mkspiffs.exe
set MKSP=C:\Users\mz\AppData\Local\Arduino15\packages\esp8266\tools\mklittlefs\3.1.0-gcc10.3-e5f9fec\mklittlefs.exe
set MKSP1=C:\Users\mz\AppData\Local\Arduino15\packages\esp8266\tools\mklittlefs\3.1.0-gcc10.3-e5f9fec\mklittlefs.exe
set VERS=2000
set VARIANTE=_SB_LDR
set LIC=
set PIN=D4
set LANG=DE_DE
set FS=littlefs

if %1%==D1MINI (
	echo "Settings for D1 Mini"
	set MKSP=C:\Users\mz\.platformio\packages\tool-mklittlefs\mklittlefs.exe
	set BINDIR=C:\Users\mz\Documents\PlatformIO\Projects\Wortuhr_ESP8266-ESP32\bin-tar
	set FLASH_SIZE=4MB
	set SPIFFS_START=0x00200000 
	set SPIFFS_SIZE=0x1FA000
	set TYPE=D1R2
	set APP0=0x0
)

if %1%==D1PRO (
	echo "Settings for D1 Mini Pro"
	set MKSP=C:\Users\mz\.platformio\packages\tool-mklittlefs\mklittlefs.exe
	set BINDIR=C:\Users\mz\Documents\PlatformIO\Projects\Wortuhr_ESP8266-ESP32\bin-tar
	set FLASH_SIZE=16MB
	set SPIFFS_START=0x00200000
	set SPIFFS_SIZE=0xDFA000
	set TYPE=D1R2PRO
	set APP0=0x0
)

if %1%==T-HMI (
	echo "Settings for LILYGO T-HMI"
	set BINDIR=C:\Users\mz\Documents\PlatformIO\Projects\Wortuhr_ESP8266-ESP32\bin-tar
	set FLASH_SIZE=16MB
	set SPIFFS_START=0xc90000
	set SPIFFS_SIZE=0x360000
	set TYPE=LILYGO
	set APP0=0x10000
	set PIN=15
	set MKSP=C:\Users\mz\.platformio\packages\tool-mklittlefs\mklittlefs.exe
)

if %1%==T7-S3 (
	echo "Settings for LILYGO T7-S3"
	set BINDIR=C:\Users\mz\Documents\PlatformIO\Projects\Wortuhr_ESP8266-ESP32\bin-tar
	set FLASH_SIZE=16MB
	set SPIFFS_START=0xc90000
	set SPIFFS_SIZE=0x360000
	set TYPE=t7s3
	set APP0=0x10000
	set PIN=12
	set MKSP=C:\Users\mz\.platformio\packages\tool-mklittlefs\mklittlefs.exe
)

if %1%==D1ESP32 (
	echo "Settings for D1 ESP32"
	set BINDIR=C:\Users\mz\Documents\PlatformIO\Projects\Wortuhr_ESP8266-ESP32\bin-tar
	set FLASH_SIZE=4MB
	set SPIFFS_START=0x250000
	set SPIFFS_SIZE=0x1B0000
	set TYPE=ESP32
	set APP0=0x10000
	set PIN=04
	set MKSP=C:\Users\mz\.platformio\packages\tool-mklittlefs\mklittlefs.exe
)

if %1%==ESP32S3 (
	echo "Settings for ESP32S3"
	set BINDIR=C:\Users\mz\Documents\PlatformIO\Projects\Wortuhr_ESP8266-ESP32\bin-tar
	set FLASH_SIZE=4MB
	set SPIFFS_START=0x250000
	set SPIFFS_SIZE=0x1B0000
	set TYPE=ESP32S3
	set APP0=0x10000
	set PIN=D16
	set MKSP=C:\Users\mz\.platformio\packages\tool-mklittlefs\mklittlefs.exe
)

if %1%==ESP32S2 (
	echo "Settings for ESP32S2"
	set BINDIR=C:\Users\mz\Documents\PlatformIO\Projects\Wortuhr_ESP8266-ESP32\bin-tar
	set FLASH_SIZE=4MB
	set SPIFFS_START=0x250000
	set SPIFFS_SIZE=0x1B0000
	set TYPE=ESP32S2
	set APP0=0x10000
	set PIN=D16
	set MKSP=C:\Users\mz\.platformio\packages\tool-mklittlefs\mklittlefs.exe
)

set PGMFN=%BINDIR%\wordclock%VERS%%VARIANTE%.%TYPE%%LIC%.%PIN%.%LANG%.bin
set FSFN=%BASEDIR%\bin-tar\wordclock%VERS%.%TYPE%.%FS%.%LANG%.bin
set UPFSFN=%BASEDIR%\bin-tar\unpack\wordclock%VERS%.%TYPE%.%FS%.bin


@echo on