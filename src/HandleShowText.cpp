#include "Arduino.h"
#include "Configuration.h"
#include "Languages.h"
#include "Settings.h"
#include "Html_content.h"
#include "Helper.h"
#include "TaskStructs.h"

//#define myDEBUG
#include "MyDebug.h"

#include "ESPAsyncWebServer.h"
#include <LittleFS.h>


extern s_taskParams taskParams;
extern EventGroupHandle_t xEvent;

// Page showText
// Beispiel: http://<wortuhr-IP>/showText?buzzer=2&color=0&text=Das%20ist%20eine%20Nachricht%3A%20viel%20Spass%20mit%20der%20Wortuhr%2E
void handleShowText(AsyncWebServerRequest *request)
{
    taskParams.feedBuzzer = request->arg(F("buzzer")).toInt();
    String farbwert = request->arg(F("color"));
    farbwert.toUpperCase();
    taskParams.feedColor = string_to_num(farbwert);
    taskParams.feedText = "  " + request->arg(F("text")).substring(0, 80) + "   ";
    request->send(200, TEXT_PLAIN, F("OK."));

#ifdef DEBUG
    DEBUG_PRINT(F("Show text: "));
    DEBUG_PRINTLN(taskParams.feedText);
#endif


    xEventGroupSetBits(xEvent, MODE_FEED);
}