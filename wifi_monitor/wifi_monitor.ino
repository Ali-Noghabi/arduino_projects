#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WebServer.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Replace with your network credentials
const char* ssid     = "your-SSID";
const char* password = "your-PASSWORD";

// IP address of your router
const IPAddress routerIP(192, 168, 1, 1);

// Web server running on port 80
ESP8266WebServer server(80);

// Variables to store statistics
long signalStrength;
int pingCount = 0;
int failedPingCount = 0;
String espIP = "";

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  display.setCursor(0, 0);
  display.println("Connecting to WiFi...");
  display.display();
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
    display.display();
  }

  // Get the ESP IP address
  espIP = WiFi.localIP().toString();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected!");
  display.println("ESP IP: " + espIP);
  display.display();
  
  // Start the web server
  server.on("/", handleRoot);  // When accessing the root, show HTML page
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  // Update OLED display with WiFi and ping info
  display.clearDisplay();

  signalStrength = WiFi.RSSI();  // Get signal strength

  // Display on OLED
  display.setCursor(0, 0);
  display.println("WiFi Router Stats:");
  display.print("RSSI: ");
  display.print(signalStrength);
  display.println(" dBm");
  
  bool pingSuccess = Ping.ping(routerIP);
  display.print("Ping: ");
  display.println(pingSuccess ? "OK" : "Failed");

  display.print("ESP IP: ");
  display.println(espIP);
  display.display();

  // Serve data on the web page
  server.handleClient();

  // Wait 5 seconds
  delay(5000);
}

void handleRoot() {
  signalStrength = WiFi.RSSI();  // Get signal strength
  
  // Ping the router
  bool pingSuccess = Ping.ping(routerIP);
  if (!pingSuccess) {
    failedPingCount++;
  }

  // Calculate connection stability
  pingCount++;
  int stability = (pingCount - failedPingCount) * 100 / pingCount;

  // HTML page with data displayed
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>";
  html += "body { font-family: 'Arial', sans-serif; background-color: #f0f4f7; margin: 0; padding: 20px; }";
  html += ".container { max-width: 800px; margin: auto; padding: 20px; background-color: #fff; box-shadow: 0 4px 8px rgba(0,0,0,0.1); border-radius: 10px; }";
  html += ".header { font-size: 24px; font-weight: bold; text-align: center; padding: 10px 0; color: #4285f4; }";
  html += ".status { margin: 15px 0; font-size: 18px; color: #333; }";
  html += ".status span { font-weight: bold; color: #4285f4; }";
  html += "</style></head><body>";
  html += "<div class='container'><div class='header'>ESP8266 WiFi Status</div>";
  html += "<div class='status'>Signal Strength (RSSI): <span>" + String(signalStrength) + " dBm</span></div>";
  html += "<div class='status'>Ping to Router: <span>" + String(pingSuccess ? "Success" : "Failed") + "</span></div>";
  html += "<div class='status'>Total Pings: <span>" + String(pingCount) + "</span></div>";
  html += "<div class='status'>Failed Pings: <span>" + String(failedPingCount) + "</span></div>";
  html += "<div class='status'>Connection Stability: <span>" + String(stability) + "%</span></div>";
  html += "<div class='status'>ESP8266 IP Address: <span>" + espIP + "</span></div>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);  // Send the HTML response
}
