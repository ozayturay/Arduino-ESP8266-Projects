#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#define APNAME  "CFG-YouTubeCounter"
#define APIP    "192.168.4.1"
#define HOST    "YouTubeCounter"
#define PATH    "/update"

// #define USER    "admin"
// #define PASS    "DEADBEEF"

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void configModeCallback (WiFiManager *wifiManager)
{
  Serial.println();
  Serial.println("Entered Config Mode !!!");
  Serial.print("* Created Access Point ");
  Serial.println(wifiManager->getConfigPortalSSID());
  Serial.printf("* IP Address is %s\n", APIP);
  blinkStringCenter("CFG", font5x7, 500, 1);
  blinkStringCenter("192", font5x7, 500, 1);
  blinkStringCenter("168", font5x7, 500, 1);
  blinkStringCenter("4", font5x7, 500, 1);
  blinkStringCenter("1", font5x7, 500, 1);
  blinkStringCenter("CFG", font5x7, 500, 1);
}

void WiFiSetup()
{
  WiFiManager wifiManager;
  if (digitalRead(BTNPin) == LOW) wifiManager.resetSettings();
  wifiManager.setDebugOutput(false);
  wifiManager.setAPCallback(configModeCallback);
  blinkStringCenter("WiFi", font5x7, 500, 3);
  wifiManager.autoConnect(APNAME);

  Serial.println();
  Serial.println("Entered Operation Mode !!!");
  Serial.print("* Connected to ");
  Serial.println(WiFi.SSID());
  IPAddress IP = WiFi.localIP();
  Serial.print("* IP Address is ");
  Serial.println(IP);

  blinkStringCenter("OK", font5x7, 500, 1);
  blinkStringCenter(String(IP[0]).c_str(), font5x7, 250, 1);
  blinkStringCenter(String(IP[1]).c_str(), font5x7, 250, 1);
  blinkStringCenter(String(IP[2]).c_str(), font5x7, 250, 1);
  blinkStringCenter(String(IP[3]).c_str(), font5x7, 250, 1);
}

void OTASetup()
{
  ArduinoOTA.setHostname(HOST);
  ArduinoOTA.setPort(8266);
  // ArduinoOTA.setPassword(PASS);

  ArduinoOTA.onStart([]()
  {
    Serial.println();
    Serial.println("OTA Start . . .");
  });

  ArduinoOTA.onEnd([]()
  {
    Serial.println("\nOTA End");
  });

  ArduinoOTA.onProgress([](uint32_t progress, uint32_t total)
  {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error)
  {
    Serial.printf("OTA Error [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("OTA Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("OTA End Failed");
  });

  ArduinoOTA.begin();

  MDNS.begin(HOST);
  httpUpdater.setup(&httpServer, PATH);
  // httpUpdater.setup(&httpServer, PATH, USER, PASS);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);

  Serial.println();
  Serial.println("Update Server Ready !!!");
  Serial.printf("* Open http://%s.local%s in your browser\n", HOST, PATH);
  // Serial.printf("* Login with username %s and password %s\n", USER, PASS);
}

void OTALoop()
{
  ArduinoOTA.handle();
  httpServer.handleClient();
}


