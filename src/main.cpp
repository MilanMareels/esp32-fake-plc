#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <ctime>

const char* ssid = "telenet-2916D2E";
const char* password = "tanf6wdra8Ba";

WebServer server(80);

void sendJson(int code, JsonDocument& doc) {
  String body;
  serializeJson(doc, body);
  server.send(code, "application/json", body);
}

const char* fakePlcStatus(){
  int randomNumber = random(0,2); // 0 = off, 1 = on

  if(randomNumber == 0){
    return "OFF";
  }
  return "ON";
}

const char* getTimeStamp(){
  time_t timestamp;
  time(&timestamp);

  return ctime(&timestamp);
}

void handlePLC(){
  JsonDocument doc;

  doc["plcId"] = "PLC_LINE_01";
  doc["timestamp"] = getTimeStamp();
  doc["status"] = fakePlcStatus();

  sendJson(200, doc);
}

void handleStatus() {
  JsonDocument doc;

  doc["status"] = "healthy";
  doc["device"]  = "PLC_LINE_01";
  doc["uptime_s"] = millis() / 1000;
  doc["free_heap"] = ESP.getFreeHeap();
  doc["rssi"]    = WiFi.RSSI();
  doc["ip"]      = WiFi.localIP().toString();

  sendJson(200, doc);
}

void handleNotFound() {
  JsonDocument doc;
  doc["error"] = "not found";
  sendJson(404, doc);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.print("API at http://");
  Serial.println(WiFi.localIP());

  server.onNotFound(handleNotFound);
  server.on("/api/v1/status", HTTP_GET, handleStatus);
  server.on("/api/v1/plc", HTTP_GET, handlePLC);
  server.begin();
}

void loop() {
  server.handleClient();
}