#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

ESP8266WebServer server(80);

int relayPins[4] = {5, 4, 0, 2}; // GPIO pins for relay

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Initialize relay pins
  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // HIGH means OFF for relay module
  }

  // Define web server routes
  server.on("/relay/1/on", [](){ controlRelay(0, LOW); });
  server.on("/relay/1/off", [](){ controlRelay(0, HIGH); });
  server.on("/relay/2/on", [](){ controlRelay(1, LOW); });
  server.on("/relay/2/off", [](){ controlRelay(1, HIGH); });
  server.on("/relay/3/on", [](){ controlRelay(2, LOW); });
  server.on("/relay/3/off", [](){ controlRelay(2, HIGH); });
  server.on("/relay/4/on", [](){ controlRelay(3, LOW); });
  server.on("/relay/4/off", [](){ controlRelay(3, HIGH); });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void controlRelay(int relay, int state) {
  digitalWrite(relayPins[relay], state);
  server.send(200, "application/json", "{\"status\":\"success\"}");
}
