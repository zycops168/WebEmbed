#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiAP.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#define SPI_FLASH_SEC_SIZE 1024

#define DEFAULT_SSID "NOEY_WIFI_2.4G";
#define DEFAULT_PASS "12341234"



String myAPssid = "ESP32-AP";
String myAPpassword = "12345678";

String apSsid = DEFAULT_SSID;
String apPass = DEFAULT_PASS;
bool STA_STATUS = false;

WebServer server(80);

const int led = LED_BUILTIN;

void handleRoot() {
  digitalWrite(led, LOW);
  server.send(200, "text/html", "<H1>hello from esp32!</H1>");
  digitalWrite(led, HIGH);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, HIGH);
}

void handleApDataPost() {
  if (server.args() != 1) {
    server.send(400, "text/plain", "argument error");
  } else {
    String jsonString = server.arg(0);
    StaticJsonDocument<100> doc;
    DeserializationError err = deserializeJson(doc, jsonString);
    if (err) {
      server.send(500, "text/plain", "server error");

    } else {
      String _apSsid = doc["ssid"].as<String>();
      String _apPass = doc["pass"].as<String>();
      server.send(200, "text/plain", "ok");

      // EEPROM Write
      // protect write duplicate data to EEPROM
      if (_apSsid != apSsid || _apPass != apPass) {
        apSsid = _apSsid;
        apPass = _apPass;
        eepromWrite();
      }
    }
  }
}

void handleApDataGet() {
  digitalWrite(led, LOW);
  String str = "{\"ssid\":\"" + apSsid + "\", \"pass\":\"" + apPass + "\"}";
  server.send(200, "text/json", str);
  digitalWrite(led, HIGH);

}

void handleApSetupGet() {
  digitalWrite(led, LOW);
  String str = "<html lang='en'><head> <meta charset='UTF-8'> <meta X-Content-Type-Options='nosniff'/><meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate'/> <meta http-equiv='Pragma' content='no-cache'/> <meta http-equiv='Expires' content='0'/> <meta name='viewport' content='width=device-width, initial-scale=1.0'> <title>AP Setup</title> <style>body{font-size: 28px; padding: 10px;}input{width: 100%; border: 1px solid gray; border-radius: 5px; font-size: 24px; outline: none;}label{font-weight: bold;}button{width: 31%; font-size: 24px; border: 1px solid gray; border-radius: 6px;}#button-group{margin-top: 20px; text-align: center;}</style></head><body> <center> <h1>AP Setup</h1> </center> <div> <label>SSID: </label> <input type='input' id='ssid' placeholder='<ssid>'><br><label>PASS:</label> <input type='password' id='pass' placeholder='<pass>' ><br></div><div id='button-group'> <button id='submit'>Submit</button> <button id='clear'>Clear</button> <button id='reload'>Reload</button> </div><script type='text/javascript'> document.getElementById('clear').onclick=()=>{document.getElementById('ssid').value=''; document.getElementById('pass').value='';}; document.getElementById('reload').onclick=()=>{console.log('reload button is clicked...'); var xmlHttp=new XMLHttpRequest(); xmlHttp.onreadystatechange=()=>{if (xmlHttp.readyState==XMLHttpRequest.DONE){/* state changed ( may success or error) */ /* - enable all object */ if (xmlHttp.status==200){console.log(xmlHttp.responseText); res=JSON.parse(xmlHttp.responseText); document.getElementById('ssid').value=res.ssid; document.getElementById('pass').value=res.pass;}else{console.log('fail');}}else{/* waiting for state change */ /* - should disable all object */}}; xmlHttp.open('GET', '/apData'); xmlHttp.send();}; document.getElementById('submit').onclick=()=>{console.log('submit button is clicked...'); var xmlHttp=new XMLHttpRequest(); xmlHttp.onreadystatechange=()=>{if (xmlHttp.readyState==XMLHttpRequest.DONE){/* state changed ( may success or error) */ /* - enable all object */ if (xmlHttp.status==200){alert('Success...')}else{alert('Fail...')}}else{/* waiting for state change */ /* - should disable all object */}}; var data=JSON.stringify({ssid: document.getElementById('ssid').value, pass: document.getElementById('pass').value}); xmlHttp.open('POST', '/apData'); xmlHttp.send(data);}; /* main */ document.getElementById('reload').click(); </script></body></html>";
  server.send(200, "text/html", str);
  digitalWrite(led, HIGH);
}

void handleLed() {
  uint32_t cur = millis();
  static uint32_t next = cur + 1000;
  if (cur >= next) {
    digitalWrite(led, !digitalRead(led));
    next = cur + 1000;
  }
}

void eepromWrite() {
  char c;
  int addr = 0;
  unsigned char s, i;

  EEPROM.begin(SPI_FLASH_SEC_SIZE);

  // write "@$"
  c = '@'; EEPROM.put(addr, c); addr ++;
  c = '$'; EEPROM.put(addr, c); addr ++;

  // ssid
  s = (unsigned char)apSsid.length(); EEPROM.put(addr, s); addr ++;
  for (i = 0; i < s; i ++) {
    c = apSsid[i]; EEPROM.put(addr, c); addr ++;
  }

  // password
  s = (unsigned char)apPass.length(); EEPROM.put(addr, s); addr ++;
  for (i = 0; i < s; i ++) {
    c = apPass[i]; EEPROM.put(addr, c); addr ++;
  }
  // write EEPROM
  EEPROM.end();
}

void eepromRead() {
  char c;
  int addr = 0;
  unsigned char s, i;

  EEPROM.begin(SPI_FLASH_SEC_SIZE);
  // read and check "@$"
  char header[3] = {' ', ' ', '\0'};
  EEPROM.get(addr, header[0]); addr ++;
  EEPROM.get(addr, header[1]); addr ++;

  if (strcmp(header, "@$") != 0) {
    eepromWrite();
  } else {
    // ssid
    EEPROM.get(addr, s); addr ++;
    apSsid = "";
    for (i = 0; i < s; i++) {
      EEPROM.get(addr, c); apSsid = apSsid + c; addr ++;
    }

    // pass
    EEPROM.get(addr, s); addr ++;
    apPass = "";
    for (i = 0; i < s; i++) {
      EEPROM.get(addr, c); apPass = apPass + c; addr ++;
    }
  }
}

void setup(void) {
  // h/w pin
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW); // trun on LED

  Serial.begin(115200);

  // read EEPROM
  eepromRead();

  // try to connect as sta (station mode)
  WiFi.mode(WIFI_STA);
  WiFi.begin(apSsid.c_str(), apPass.c_str());
  Serial.print("Connect to " + apSsid + " ");

  int counter = 0;
  do {
    delay(500);
    digitalWrite(led, !digitalRead(led));
    delay(500);
    digitalWrite(led, !digitalRead(led));
    Serial.print(".");
    counter += 1;
  } while (WiFi.status() != WL_CONNECTED && counter < 10);

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    STA_STATUS = false;
    Serial.println("fail");
  } else {
    Serial.println("success");
    STA_STATUS = true;
    Serial.println("ip (STA): " + (WiFi.localIP()).toString());
  }

  // get chip ID
  char temp[10];
  uint64_t id = ESP.getEfuseMac();
  sprintf(temp, "%04X", (uint16_t)(id >> 32));
  myAPssid = myAPssid + "-" + String(temp);
  sprintf(temp, "%08X", (uint32_t)id);
  myAPssid = myAPssid + String(temp);
  Serial.println("myAPssid: " + myAPssid);

  // ap mode (software)
  WiFi.softAP(myAPssid.c_str(), myAPpassword.c_str());
  Serial.println("ip (AP): " + (WiFi.softAPIP()).toString());


  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  // web service
  server.on("/", handleRoot);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.on("/apSetup", HTTP_GET, handleApSetupGet);
  server.on("/apData", HTTP_GET, handleApDataGet);
  server.on("/apData", HTTP_POST, handleApDataPost);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // turn led off
  digitalWrite(led, HIGH);
  delay(500);
}

void loop(void) {
  if (STA_STATUS == false){
    handleLed();
  }
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
}
