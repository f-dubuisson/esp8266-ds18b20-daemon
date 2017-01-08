#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "Ds18b20.h"

const char* ssid = "xxxxxxxxxxxxx";
const char* password = "xxxxxxxxxxxx";

ESP8266WebServer server(80);

/** Fonction setup() **/
void setup() {
  Serial.begin(9600);
  Serial.println("Connecting...");
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

/** Fonction loop() **/
void loop() {
  server.handleClient();
}

void handleRoot() {
  float temperature;

  /* Lit la température ambiante à ~1Hz */
  if (getTemperature(&temperature, true) != READ_OK) {
    Serial.println("Erreur de lecture du capteur");
    return;
  }

  /* Affiche la température */
  String result = "";
  result += temperature;
  result += "°C";
  Serial.println(result);

  server.send(200, "text/plain", result);
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  server.send(404, "text/plain", message);
}
