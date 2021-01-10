/*Importing Libraries*/
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

/*Declaring Pin Numbers for the LEDs*/
const int redPin = D6;
const int yellowPin = D5;
const int greenPin = D1;

/*Initialising the WiFiManager Module*/
WiFiManager wifiManager;

/*Starting a Web Server at Port 8080*/
ESP8266WebServer server(8080);

/*Registering event handlers*/
void handleGetUptime();
void handlePostChangeLed();
void handleGetSensorValue();
void handle404NotFound();

/*Setup only runs once*/
void setup()
{
  /*Starting the serial connection and checking for past WiFi connections to connect to*/
  Serial.begin(115200);
  wifiManager.autoConnect();
  Serial.printf("LOG:: Connected to WiFi\nLOG:: IP ");
  Serial.println(WiFi.localIP());

  /*Specifying the Digital and Analog Pins*/
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(A0, INPUT);

  /*Server Route Handlers*/
  server.on("/api/v1/getUptime", HTTP_GET, handleGetUptime);
  server.on("/api/v1/changeLed", HTTP_POST, handlePostChangeLed);
  server.on("/api/v1/getSensorValue", HTTP_GET, handleGetSensorValue);
  server.onNotFound(handle404NotFound);

  /*Starting the server*/
  server.begin();
  Serial.println("LOG:: Listening for requests on Port 8080");
}

/*Loop goes on executing for the lifetime*/
void loop()
{
  server.handleClient();
}

/*GET - /api/v1/getUptime*/
void handleGetUptime()
{
  Serial.println("LOG:: GET /api/v1/getUptime");
  /*We use ArduinoJson Library to create and parse JSON objects*/
  DynamicJsonDocument res(1024);
  res["success"] = true;
  res["uptime"] = millis();
  char response[1024];
  serializeJson(res, response);
  server.send(200, "application/json", response);
}

/*POST - /api/v1/changeLed*/
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
    /*Switching an LED on by pulling it high*/
    digitalWrite(ledPin, HIGH);
    Serial.println("LOG:: LED is switched ON");
    res["success"] = true;
    res["ledStatus"] = "ON";
    break;
  case 0:
    /*Switching an LED off by pulling it low*/
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

/*GET - /api/v1/getSensorValue*/
void handleGetSensorValue()
{
  /*
  NodeMCU has a 10-bit DAC. Thus the analogRead values range from 0 -1023
  for 0 - 3.3 volts. On top of that we use map() to convert the above value to a
  scale of 0 -100
  */
  int sensorValue = map(analogRead(A0), 0, 1024, 0, 100);
  Serial.println("LOG:: GET /api/v1/getSensorValue");
  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);
  DynamicJsonDocument res(1024);
  res["success"] = true;
  res["sensorValue"] = sensorValue;
  char response[1024];
  serializeJson(res, response);
  server.send(200, "application/json", response);
}

/*404 Route Handler*/
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