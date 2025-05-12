#include "Arduino.h"
#include "Configuration.h"
#include "Languages.h"
#include "Settings.h"
#include "MyTime.h"
#include "MyWifi.h"
#include "Html_content.h"

#define myDEBUG
#include "MyDebug.h"

#include "ESPAsyncWebServer.h"
#include <LittleFS.h>

static Settings *settings = Settings::getInstance();
static MyTime *mt = MyTime::getInstance();
static MyWifi *myWifi = MyWifi::getInstance();
static AsyncWebServer *webServer = myWifi->getServer();

// #####################################################################################################################
// FS Webseite
// Teil 1
const char fshtml1[] PROGMEM =  R"=====(
    document.addEventListener('DOMContentLoaded', () => {
    list(JSON.parse(localStorage.getItem('sortBy')));
    btn.addEventListener('click', () => {
      if (!confirm(LANG_FORMATCONF + "\n" )) event.preventDefault();
    });
    });
    function list(to){
    let myList = document.querySelector('main'), noted = '';
    fetch(`?sort=${to}`).then( (response) => {
          return response.json();
        }).then((json) => {
      myList.innerHTML = '<nav><input type="radio" id="/" name="group" checked="checked"><label for="/"> &#128193;</label><span id="cr">+&#128193;</nav></span><span id="si"></span>';
      document.querySelector('form').setAttribute('action', '/upload?f=');
      for (var i = 0; i < json.length - 1; i++) {
        let dir = '', f = json[i].folder, n = json[i].name;
        if (f != noted) {
        noted = f;
        dir = `<nav><input type="radio" id="${f}" name="group"><label for="${f}"></label> &#128193; ${f} <a href="?delete=/${f}">&#x1f5d1;&#xfe0f;</a></nav>`;
        }
        if (n != '') dir += `<li><a href="${f}/${n}">${n}</a><small> ${json[i].size}</small><a href="${f}/${n}"download="${n}"> Download</a> or<a href="?delete=${f}/${n}"> Delete</a>`;
        myList.insertAdjacentHTML('beforeend', dir);
      }
      myList.insertAdjacentHTML('beforeend', `<li><b id="so">${to ? '&#9660;' : '&#9650;'} LittleFS</b><small> ` + LANG_FSUSED + `: ${json[i].usedBytes.replace(".00", "")}/` + LANG_FSTOTAL + `: ${json[i].totalBytes.replace(".00", "")}</small>`);
      var free = json[i].freeBytes;
      cr.addEventListener('click', () => {
      document.getElementById('no').classList.toggle('no');
      });
      so.addEventListener('click', () => {
      list(to=++to%2);
      localStorage.setItem('sortBy', JSON.stringify(to));
      });
      document.addEventListener('change', (e) => {
        if (e.target.id == 'fs') { 
          for (var bytes = 0, i = 0; i < event.target.files.length; i++) bytes += event.target.files[i].size;
              for (var output = `${bytes} Byte`, i = 0, circa = bytes / 1024; circa > 1; circa /= 1024) output = circa.toFixed(2) + ['KB', 'MB', 'GB'][i++];
              if (bytes > free) {
                si.innerHTML = `<li><b> ${output}</b><strong> ` + LANG_NOSPACE + `</strong></li>`;
                up.setAttribute('disabled', 'disabled');
              } 
              else {
                si.innerHTML = `<li><b>` + LANG_FILESIZE + `:</b> ${output}</li>`;
                up.removeAttribute('disabled');
              }
       }
       document.querySelectorAll(`input[type=radio]`).forEach(el => { if (el.checked) document.querySelector('form').setAttribute('action', '/upload?f=' + el.id)});
     });
     document.querySelectorAll('[href^="?delete=/"]').forEach(node => {
       node.addEventListener('click', () => {
            if (!confirm(LANG_SURE)) event.preventDefault();
       });
     });
   });
 }
    
</script>
</head>
<body>
)=====";

// Teil 2
const char fshtml2[] PROGMEM =  R"=====(
  <form method="post" enctype="multipart/form-data">
    <input id="fs" type="file" name="up[]" multiple>
    <button id="up" disabled>Upload</button>
  </form>
  <div class="sanduhrcontainer" id="sanduhrcontainer" style="display: none">
  <img class="sanduhr" id="sanduhr" style="width: 150px" src="/sanduhr" alt="Sanduhr">
  </div>
  <script>
    const upbut = document.getElementById("up");
    upbut.addEventListener("click", function() {
    document.getElementById("sanduhrcontainer").style.display = "block";
  });
  </script>
  
  <form id="no" class="no" method="POST">
)=====";

//################################################################
// FS Webseite CSS
// Teil 1

const char fsstyle1[] PROGMEM =  R"=====(
    body {
      font-family: sans-serif;
      background-color: #aaaaaa;
      display: flex;
      max-width : 500px;
      flex-flow: column;
      align-items: center;
    }
    h1,h2 {
      color: #e1e1e1;
      text-shadow: 2px 2px 2px black;
    }
    li {
      background-color: #feb1e2;
      list-style-type: none;
      margin-bottom: 10px;
      padding: 2px 5px 1px 0;
      box-shadow: 5px 5px 5px rgba(0,0,0,0.7);
    }
    li a:first-child, li b {
      background-color: #8f05a5;
      font-weight: bold;
      color: white;
      text-decoration:none;
      padding: 2px 5px;
      text-shadow: 2px 2px 1px black;
      cursor:pointer;
    }
    li strong {
      color: red;
    }
    input {
      height:35px;
      font-size:14px;
    }
    label + a {
      text-decoration: none;
    }
    h1 + main {
      display: flex;
    }  
    aside {
      display: flex;
      flex-direction: column;
      padding: 0.2em;
    }
    #left {
      align-items:flex-end;
      text-shadow: 0.5px 0.5px 1px #757474;
    }
    .note {
      background-color: #fecdee;
      padding: 0.5em;
      margin-top: 1em;
      text-align: center;
      max-width: 320px;
      border-radius: 0.5em;
    }
    [type=submit] {
      height:40px; 
      font-size: 14px;
    }
    .buttons {
      background-color: #353746;
      text-align:center;
      line-height: 22px;
      color:#FFFFFF;
      width:200px;
      height: 32px;
      padding:1px;
      border:2px solid #FFFFFF;
      font-size:14px;
      border-radius:15px;
      cursor: pointer;
    }
    form [title] {
      background-color: #353746;
      font-size: 14px;
      width: 150px;
    }
    )=====";
    
     // Teil 2
    const char fsstyle2[] PROGMEM =  R"=====(
    nav {
      display: flex;
      align-items: baseline;
      justify-content: space-between;
    }
    #left {
      align-items:flex-end;
      text-shadow: 0.5px 0.5px 1px #757474;
    }
    #cr {
      font-weight: bold;
      cursor:pointer;
      font-size: 1.5em;
    }
    #up {
      width: auto; 
    }
    .note {
      background-color: #fecdee;
      padding: 0.5em;
      margin-top: 1em;
      text-align: center;
      max-width: 320px;
      border-radius: 0.5em;
    }
    .no {
      display: none;
    }
    
    [value*=Format] {
      margin-top: 1em;
      box-shadow: 5px 5px 5px rgba(0,0,0,0.7);
    }
    [name="group"] {
      display: none;
    }
    [name="group"] + label {
      font-size: 1.5em;
      margin-right: 5px;
    }
    [name="group"] + label::before {
      content: "\002610";
    } 
    [name="group"]:checked + label::before {
      content: '\002611\0027A5';
    }
    .sanduhrcontainer {
      position: absolute;
        top: 130px;
        left: 180px;
    }
    .sanduhr {
        position: absolute;
          margin: 0px auto;   
    }
    )=====";
    // #####################################################################################################################
    
const String formatBytes(size_t const& bytes) {                                        // lesbare Anzeige der Speichergrößen
    return bytes < 1024 ? static_cast<String>(bytes) + " Byte" : bytes < 1048576 ? static_cast<String>(bytes / 1024.0) + "KB" : static_cast<String>(bytes / 1048576.0) + "MB";
}
      
bool handleList(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    size_t usedBytes;
    size_t totalBytes;
    using namespace std;
    using records = tuple<String, String, int>;
    list<records> dirList;
#if defined(ESP8266)                                                             // Senden aller Daten an den Client
    FSInfo fs_info;  LittleFS.info(fs_info);                                             // Füllt FSInfo Struktur mit Informationen über das Dateisystem
    usedBytes=fs_info.usedBytes;
    totalBytes=fs_info.totalBytes;
    Dir dir = LittleFS.openDir("/");
#ifdef DEBUG_WEB
    DEBUG_PRINTF("LittleFS.handleList: %i Codezeile: %u\n", ESP.getMaxFreeBlockSize(),  __LINE__);
#endif

    while (dir.next()) {                                                                 // Ordner und Dateien zur Liste hinzufügen
        if (dir.isDirectory()) {
            uint8_t ran {0};
            Dir fold = LittleFS.openDir(dir.fileName());
            while (fold.next())  {
                ran++;
                dirList.emplace_back(dir.fileName(), fold.fileName(), fold.fileSize());
            }
            if (!ran) dirList.emplace_back(dir.fileName(), "", 0);
        }
        else 
        {
            dirList.emplace_back("", dir.fileName(), dir.fileSize());
        }
    }
#else
    usedBytes = LittleFS.usedBytes();
    totalBytes = LittleFS.totalBytes();
    File root = LittleFS.open("/");
    File dir = root.openNextFile();

    while(dir) {
        if(dir.isDirectory()) {
            uint8_t ran {0};
            String fn="/"+String(dir.name());
            File root2 = LittleFS.open(fn);
            File fold = root2.openNextFile();
            while (fold)  {
                ran++;
                dirList.emplace_back(fn, fold.name(), fold.size());
                fold = root2.openNextFile();
            }
            if (!ran) dirList.emplace_back(fn, "", 0);
        }
        else 
        {
            dirList.emplace_back("", dir.name(), dir.size());
        }
        dir = root.openNextFile();
    }

#endif

#ifdef DEBUG_WEB
#if defined(ESP8266)
    DEBUG_PRINTF("LittleFS.handleList: %i Codezeile: %u\n", ESP.getMaxFreeBlockSize(),  __LINE__);
#endif
#endif  

    dirList.sort([](const records & f, const records & l) {                              // Ordner sortieren
        if (get<0>(f)[0] != 0x00 || get<0>(l)[0] != 0x00) {
            for (uint8_t i = 0; i < 31; i++) {
                if (tolower(get<0>(f)[i]) < tolower(get<0>(l)[i])) return true;
                else if (tolower(get<0>(f)[i]) > tolower(get<0>(l)[i])) return false;
            }
        }
        return false;
    });

#ifdef DEBUG_WEB
#if defined(ESP8266)
    DEBUG_PRINTF("LittleFS: %i Codezeile: %u\n", ESP.getMaxFreeBlockSize(),  __LINE__);
#endif
#endif

    uint8_t jsonoutteil = 0;
    bool jsonoutfirst = true;
    String temp = F("[");
    for (auto& t : dirList) {
#if defined(ESP8266)
        if ( ESP.getMaxFreeBlockSize() < 1000) delay(10);
#endif
        if (temp != "[") temp += ',';
        temp += F("{\"folder\":\"");
        temp += get<0>(t);
        temp += F("\",\"name\":\"");
        temp += get<1>(t);
        temp += F("\",\"size\":\"");
        temp += formatBytes(get<2>(t));
        temp += F("\"}");
        jsonoutteil++;
        if ( jsonoutteil > 2 ) 
        {
            jsonoutteil = 0;
            if ( jsonoutfirst )
            {
                jsonoutfirst = false;
                response->print(temp);
            }
            else
            {
                response->print(temp);
            }
#if defined(ESP8266)
            if ( ESP.getMaxFreeBlockSize() < minFreeBlockSize )
            {
                minFreeBlockSize = ESP.getMaxFreeBlockSize();
                codeline = __LINE__;
                codetab = __NAME__;
#ifdef DEBUG_WEB
            DEBUG_PRINTF("minFreeBlockSize: %i Tab: %s Codezeile: %u\n", minFreeBlockSize, codetab.c_str(), codeline);
#endif
            }
#endif
#ifdef DEBUG_WEB
#if defined(ESP8266)
            DEBUG_PRINTF("LittleFS.handleList.MaxFreeBlockSize: %i Codezeile: %u\n", ESP.getMaxFreeBlockSize(),  __LINE__);
#endif
            DEBUG_PRINTLN(temp);
#endif

            temp = "";
            delay(0);
        } 
    }
    temp += F(",{\"usedBytes\":\"");
    temp += formatBytes(usedBytes);                      // Berechnet den verwendeten Speicherplatz
    temp += F("\",\"totalBytes\":\"");
    temp += formatBytes(totalBytes);                     // Zeigt die Größe des Speichers
    temp += F("\",\"freeBytes\":\""); 
    temp += (totalBytes - usedBytes);            // Berechnet den freien Speicherplatz
    temp += F("\"}]");   

    response->print(temp);
    request->send(response);
#ifdef DEBUG_WEB
    DEBUG_PRINTLN(temp);
#endif
    temp = "";

    return true;
}

void deleteRecursive(const String &path) {
if (LittleFS.remove(path)) {
    LittleFS.open(path.substring(0, path.lastIndexOf('/')) + "/", "w");
    return;
}
#if defined(ESP8266)
Dir dir = LittleFS.openDir(path);
while (dir.next()) {
    deleteRecursive(path + '/' + dir.fileName());
}
LittleFS.rmdir(path);
#else
File dir = LittleFS.open(path);
File file = dir.openNextFile();

while(file) {
    deleteRecursive(path + '/' + file.name());
    file = dir.openNextFile();
}
LittleFS.rmdir(path);
#endif
}

void sendResponse(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(303, "message/http");
    response->addHeader("Location", "fs");
    request->send(response);
}


void handleFSExplorer(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream(TEXT_HTML);
    String message;
    if (request->hasArg("new")) 
    {
      String folderName {request->arg("new")};
      for (auto& c : {34, 37, 38, 47, 58, 59, 92}) for (auto& e : folderName) if (e == c) e = 95;    // Ersetzen der nicht erlaubten Zeichen
      folderName = "/" + folderName;
      DEBUG_PRINTF("mkdir: %s\n",folderName);
      if(!LittleFS.mkdir(folderName)) {
        DEBUG_PRINTLN("mkdir error");
      }
      sendResponse(request);
    }
    else if (request->hasArg("sort")) 
    {
      bool x=handleList(request);
    }
    else if (request->hasArg("delete")) 
    {
      deleteRecursive(request->arg("delete"));
      sendResponse(request);
    }
    else
    {
      message =  F("<!DOCTYPE HTML>");
      message += F("<html lang=\"");
      message += LANG_HTMLLANG;
      message += F("\">\n");
      message += F("<head>");
      message += F("<meta charset=\"UTF-8\">");
      message += F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
      message += F("<link rel=\"stylesheet\" href=\"fsstyle.css\">");
      message += F("<title>");
      message += LANG_EXPLORER;
      message += F("</title>\n");
      message += F("<script>");
      message += F("const LANG_FORMATCONF = \"" LANG_FORMATCONF "\";\n");
      message += F("const LANG_NOSPACE = \"" LANG_NOSPACE "\";\n");
      message += F("const LANG_FILESIZE= \"" LANG_FILESIZE "\";\n");
      message += F("const LANG_SURE= \"" LANG_SURE "\";\n");
      message += F("const LANG_FSUSED= \"" LANG_FSUSED "\";\n");
      message += F("const LANG_FSTOTAL= \"" LANG_FSTOTAL "\";\n");
      response->print(message);
      response->print(FPSTR(fshtml1));
      message = F("<h2>" LANG_EXPLORER "</h2>");
      response->print(message);
      response->print(FPSTR(fshtml2));
      message = F("<input name=\"new\" placeholder=\"" LANG_FOLDER "\"");    
      //message += F(" pattern=\"[^\x22/%&\\:;]{1,31}\" title=\"");
      message += F(" title=\"");
      message += F(LANG_CHAR_NOT_ALLOW);
      message += F("\" required=\"\">");
      message += F("<button>Create</button>\n");
      message += F("</form>");
      message += F("<main></main>\n");
      message += F("<form action=\"/format\" method=\"POST\">\n");            
      message += F("<button class=\"buttons\" title=\"" LANG_BACK "\" id=\"back\" type=\"button\" onclick=\"window.location.href=\'\\/\'\">&#128281; " LANG_BACK "</button>\n");
      message += F("<button class=\"buttons\" title=\"Format LittleFS\" id=\"btn\" type=\"submit\" value=\"Format LittleFS\">&#10060;Format LittleFS</button>\n");
      message += F("</form>\n");
      message += F("</body>\n");
      message += F("</html>\n");
      response->print(message);
      request->send(response);
    }
  
  }
  
  void handleFSExplorerCSS(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream(TEXT_CSS);
    response->print(FPSTR(fsstyle1));
    response->print(FPSTR(fsstyle2));
    request->send(response);
  }