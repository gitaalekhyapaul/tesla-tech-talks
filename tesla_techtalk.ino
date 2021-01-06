#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

const int redPin = D6;
const int yellowPin = D5;
const int greenPin = D1;

WiFiManager wifiManager;
ESP8266WebServer server(8080);

void handleGetUptime();
void handlePostChangeLed();
void handleGetSensor();
void handle404NotFound();

void setup()
{
  Serial.begin(115200);
  wifiManager.autoConnect();
  Serial.printf("LOG:: Connected to WiFi\nLOG:: IP ");
  Serial.println(WiFi.localIP());
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(A0, INPUT);
  server.on("/api/v1/getUptime", HTTP_GET, handleGetUptime);
  server.on("/api/v1/changeLed", HTTP_POST, handlePostChangeLed);
  server.on("/api/v1/sensor", HTTP_GET, handleGetSensor);
  server.onNotFound(handle404NotFound);
  server.begin();
  Serial.println("LOG:: Listening for requests on Port 8080");
}

void loop()
{
  server.handleClient();
}

void handleGetUptime()
{
  Serial.println("LOG:: GET /api/v1/getUptime");
  DynamicJsonDocument res(1024);
  res["success"] = true;
  res["uptime"] = millis();
  char response[1024];
  serializeJson(res, response);
  server.send(200, "application/json", response);
}

void handlePostChangeLed()
{
  Serial.println("LOG:: POST /api/v1/changeLed");
  DynamicJsonDocument req(1024);
  deserializeJson(req, server.arg("plain"));
  const char *setLed = req["setLed"];
  const char *ledType = req["ledType"];
  Serial.printf("LOG:: %s is to be set %s\n", ledType, setLed);
  DynamicJsonDocument res(1024);
  int ledPin;
  if (strcmp(ledType, "RED") == 0)
  {
    ledPin = redPin;
    res["ledType"] = "RED";
  }
  else if (strcmp(ledType, "YELLOW") == 0)
  {
    ledPin = yellowPin;
    res["ledType"] = "YELLOW";
  }
  else if (strcmp(ledType, "GREEN") == 0)
  {
    ledPin = greenPin;
    res["ledType"] = "GREEN";
  }
  else
  {
    res["success"] = false;
    res["message"] = "Value of ledType should be RED, YELLOW or GREEN.";
    char response[1024];
    serializeJson(res, response);
    server.send(400, "application/json", response);
    return;
  }
  int s = -1;
  if (strcmp(setLed, "ON") == 0)
    s = 1;
  if (strcmp(setLed, "OFF") == 0)
    s = 0;
  switch (s)
  {
  case 1:
    digitalWrite(ledPin, HIGH);
    Serial.println("LOG:: LED is switched ON");
    res["success"] = true;
    res["ledStatus"] = "ON";
    break;
  case 0:
    digitalWrite(ledPin, LOW);
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

void handleGetSensor()
{
  int sensorValue = analogRead(A0);
  Serial.println("LOG:: GET /sensor");
  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);
  DynamicJsonDocument res(1024);
  res["success"] = true;
  res["sensorValue"] = sensorValue;
  char response[1024];
  serializeJson(res, response);
  server.send(200, "application/json", response);
}