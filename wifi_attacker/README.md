# ESP8266 WiFi Scanner & Attacker Tool

## Overview

This project utilizes the ESP8266 module to scan for available Wi-Fi networks, deauthenticate clients from networks, and create multiple fake access points (APs). It serves as both a network monitoring tool and a potential security tester. The web-based interface allows users to view available networks and perform deauthentication and fake AP creation directly from a browser.

![alt text](image.png)
**⚠️ Legal Concerns:**
This tool is intended for **robust network testing** and **security audits** only. The unauthorized use of deauthentication attacks and the creation of fake access points is illegal in many regions. Ensure you have permission from network administrators or legal authorities before using this tool.

## Features

### 1. **Wi-Fi Network Scanner**
   - Scans and lists all available Wi-Fi networks in the vicinity.
   - Displays details like SSID, BSSID, RSSI (signal strength), encryption type, channel, frequency, hidden status, and maximum data rate.
   
   **How it works:**
   - The ESP8266 performs a passive scan of Wi-Fi networks. The networks are displayed on a web page that can be accessed via a browser connected to the ESP8266’s access point.

### 2. **Deauthentication Attack**
   - Allows the user to select a specific network and send deauthentication packets to disconnect all clients from the selected access point.
   
   **How it works:**
   - The ESP8266 sends deauth packets with a specific BSSID to kick connected devices off the network. This exploits a vulnerability in older Wi-Fi standards (like WPA2 or open networks).
   
   **Use case:**
   - This feature can be used to test the resilience of a network to deauth attacks. However, this should **only** be used with explicit permission for network security testing.

### 3. **Fake Access Point Creation**
   - Creates multiple fake access points with predefined SSIDs and passwords.
   
   **How it works:**
   - Using the `WiFi.softAP()` function, the ESP8266 can create up to 10 fake access points. These APs appear in the Wi-Fi list of nearby devices, potentially causing confusion or noise in a network-rich environment.

   **Use case:**
   - This feature is useful for testing network congestion or device behavior in environments with multiple access points. It can also be used to simulate AP interference for security and network performance tests.

### 4. **Web Interface**
   - A simple web-based interface that allows users to view available networks and interact with the deauth and fake AP creation features.

   **How it works:**
   - The ESP8266 runs a small web server, accessible via the browser at the default IP address `192.168.4.1`. The interface displays the scanned networks and provides buttons to perform actions.

## Setup Guide

### Prerequisites
   - ESP8266 (e.g., NodeMCU, Wemos D1 mini, etc.)
   - Arduino IDE with ESP8266 support installed
   - Required Arduino libraries:
     - `ESP8266WiFi.h`
     - `ESP8266WebServer.h`

### Installation Steps

1. **Install ESP8266 Support in Arduino IDE:**
   - In Arduino IDE, go to `File` → `Preferences`.
   - In the `Additional Boards Manager URLs` field, add the following URL:  
     `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   - Go to `Tools` → `Board` → `Boards Manager`, search for **ESP8266** and install the latest version.

2. **Flash the Code to ESP8266:**
   - Connect your ESP8266 to your computer via a USB cable.
   - Select the correct board in `Tools` → `Board` (e.g., NodeMCU 1.0 or Wemos D1 Mini).
   - Select the correct COM port in `Tools` → `Port`.
   - Hit the **Upload** button to flash the code to the ESP8266.

3. **Connect to the ESP8266 Access Point:**
   - Once flashed, the ESP8266 will start broadcasting an access point named `ESP8266_WiFi_Scanner` with the password `password123`.
   - Connect your device to this Wi-Fi network.

4. **Access the Web Interface:**
   - Open a browser and navigate to `http://192.168.4.1`.
   - You should see the list of available Wi-Fi networks along with buttons for deauthentication and creating fake APs.


### Important Notes:
- **Deauth Packets**: Modern networks (e.g., WPA3) may be less susceptible to deauth attacks. This tool works best with older WPA2 networks or open Wi-Fi networks.
- **AP Limitations**: ESP8266 is a low-power device and might not handle too many concurrent access points. Monitor stability when creating many APs.

---

Feel free to modify or enhance this project based on your testing needs. Happy hacking (ethically)!

