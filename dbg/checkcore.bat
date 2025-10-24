set CD=C:\Users\mz\.platformio\packages\framework-espidf\components\espcoredump
set PIO=C:\Users\mz\Documents\PlatformIO\Projects\Wortuhr_Tasks\.pio
set PATH=C:\Espressif\tools\xtensa-esp-elf-gdb\14.2_20240403\xtensa-esp-elf-gdb\bin;%PATH%
rem cd C:\Users\mz\Documents\ESP-IDF-Projects\esp-idf-master\components\espcoredump
rem pause
rem python C:\Users\mz\.platformio\packages\tool-esptoolpy\esptool.py read_flash 0x3f0000 0x10000 core.dmp
rem pause
rem python C:\Users\mz\Documents\ESP-IDF-Projects\esp-idf-master\components\espcoredump\espcoredump.py  info_corefile -t auto -c C:\Users\mz\Documents\ESP-IDF-Projects\esp-idf-master\components\espcoredump\core.dmp .\.pio\build\lolin_s3_mini\wordclock2000_SB_LDR.ESP32S3.D16.DE_DE.bin
rem pause
cd C:\Users\mz\Documents\PlatformIO\Projects\Wortuhr_Tasks\dbg

python "C:\Users\mz\.platformio\packages\tool-esptoolpy\esptool.py" ^
 --chip esp32s2 ^
 read_flash 0x3F0000 0x10000 coredump.bin
  
rem set ESPCOREDUMP_GDB=xtensa-esp32s3-elf-gdb.exe
rem set ESPCOREDUMP_GDB=C:\Espressif\tools\xtensa-esp-elf-gdb\14.2_20240403\xtensa-esp-elf-gdb\bin\xtensa-esp32s3-elf-gdb.exe

python %CD%/espcoredump.py ^
	--chip esp32s3 ^
	dbg_corefile ^
	--core coredump.bin ^
    ./wordclock2000_SB_LDR.ESP32S2.D16.DE_DE.elf
	
pause