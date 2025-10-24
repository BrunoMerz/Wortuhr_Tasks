call espenv.bat ESP32S3



%ESPT% write_flash --flash_size %FLASH_SIZE% %SPIFFS_START% "%FSFN%"

pause "Jetzt neu starten!!!!!!"
