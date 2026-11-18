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
  int status = random(0,5); // 0 = STOPPED, 1 = RUNNING, 2 = ERROR, 3 = MAINTENANCE, 4 = IDLE

  switch (status)
  {
    case 0:
      return "STOPPED";
    case 1:
      return "RUNNING";
    case 2:
      return "ERROR";
    case 3:
      return "MAINTENANCE";
    case 4:
      return "IDLE";
    default:
      return "UNKNOWN";
  }
}

const char* getTimeStamp(){
  time_t timestamp;
  time(&timestamp);

  return ctime(&timestamp);
}

float randomFloat(float minVal, float maxVal) {
  return random(minVal * 10, maxVal * 10) / 10.0;
}

void populateSensors(JsonObject sensors) {
  sensors["temperature"] = randomFloat(20.0, 95.0);
  sensors["pressure"] = randomFloat(1.0, 8.5);
  sensors["rpm"] = random(0, 2000);
  sensors["tankLevel"] = random(0, 100);
}

void handleGetSensor() {
  JsonDocument doc;
  populateSensors(doc.to<JsonObject>());
  sendJson(200, doc);
}

void handlePLC(){
  JsonDocument doc;

  doc["plcId"] = "PLC_LINE_01";
  doc["timestamp"] = getTimeStamp();
  doc["status"] = fakePlcStatus();
  populateSensors(doc["sensors"].to<JsonObject>());

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
  server.on("/api/v1/status-esp32", HTTP_GET, handleStatus);
  server.on("/api/v1/status-plc", HTTP_GET, handlePLC);
  server.begin();
}

void loop() {
  server.handleClient();
}