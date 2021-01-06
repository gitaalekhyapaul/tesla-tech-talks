#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

WiFiManager wifiManager;
ESP8266WebServer server(8080);

void handlePostLed();
void handleGetMillis();
void handle404NotFound();

void setup()
{
  Serial.begin(115200);
  wifiManager.autoConnect();
  Serial.printf("LOG:: Connected to WiFi\nLOG:: IP ");
  Serial.println(WiFi.localIP());
  pinMode(LED_BUILTIN, OUTPUT);
  server.on("/millis", HTTP_GET, handleGetMillis);
  server.on("/led", HTTP_POST, handlePostLed);
  server.onNotFound(handle404NotFound);
  server.begin();
  Serial.println("LOG:: Listening for requests on Port 8080");
}

void loop()
{
  server.handleClient();
}

void handleGetMillis()
{
  Serial.println("LOG:: GET /millis");
  DynamicJsonDocument res(1024);
  res["success"] = true;
  res["millis"] = millis();
  char response[1024];
  serializeJson(res, response);
  server.send(200, "application/json", response);
}

void handlePostLed()
{
  Serial.println("LOG:: POST /led");
  DynamicJsonDocument req(1024);
  deserializeJson(req, server.arg("plain"));
  const char *setLed = req["setLed"];
  DynamicJsonDocument res(1024);
  int s = -1;
  if (strcmp(setLed, "ON") == 0)
    s = 1;
  if (strcmp(setLed, "OFF") == 0)
    s = 0;
  switch (s)
  {
  case 1:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("LOG:: LED is switched ON");
    res["success"] = true;
    res["ledStatus"] = "ON";
    break;
  case 0:
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("LOG:: LED is switched OFF");
    res["success"] = true;
    res["ledStatus"] = "OFF";
    break;

  default:
    res["success"] = false;
    res["message"] = "Value of setLed should be ON or OFF";
    break;
  }
  char response[1024];
  serializeJson(res, response);
  bool success = res["success"];
  if (success)
  {
    server.send(200, "application/json", response);
  }
  else
  {
    Serial.println("API_ERROR:: 400 BAD REQUEST");
    server.send(400, "application/json", response);
  }
}

void handle404NotFound()
{
  Serial.println("API_ERROR:: 404 RESOURCE NOT FOUND");
  DynamicJsonDocument res(1024);
  res["success"] = false;
  res["message"] = "Resource Not Found";
  char response[1024];
  serializeJson(res, response);
  server.send(404, "application/json", response);
}