#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <TimeLib.h>

// Define pin connections
#define SS_PIN D2   // SDA on MFRC522
#define RST_PIN D1  // RST on MFRC522
#define RELAY_PIN D5

// Initialize MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Initialize LCD (address 0x27 for a 16 chars and 2 line display)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Admin Card UID (Set your admin card UID here)
const byte ADMIN_UID[] = {0xDE, 0xAD, 0xBE, 0xEF}; // Example UID, replace with your admin card's UID
const int ADMIN_UID_SIZE = sizeof(ADMIN_UID);

// EEPROM Addresses
const int USER_COUNT_ADDR = 0; // Address to store the number of users
const int USER_UID_START_ADDR = 1; // Starting address to store user UIDs

// Variables
bool adminMode = false;
int userCount = 0;

void setup() {
  Serial.begin(115200);
  SPI.begin(); // Initiate  SPI bus
  mfrc522.PCD_Init(); // Initiate MFRC522
  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Turn on the backlight

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Lock is initially engaged

  EEPROM.begin(512); // Initialize EEPROM
  userCount = EEPROM.read(USER_COUNT_ADDR);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RFID Lock System");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Read the UID
  byte readUID[4];
  for (byte i = 0; i < mfrc522.uid.size && i < 4; i++) {
    readUID[i] = mfrc522.uid.uidByte[i];
  }

  // Check if Admin Card
  if (isAdminCard(readUID)) {
    Serial.println("Admin Card Detected");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Admin Login");
    adminMode = true;
    delay(1000);
    addNewUser();
    adminMode = false;
    return;
  }

  // Check if User Card
  if (isUserCard(readUID)) {
    Serial.println("Access Granted");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Granted");
    unlockDoor();
  } else {
    Serial.println("Access Denied");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Denied");
    delay(2000);
  }

  mfrc522.PICC_HaltA(); // Halt PICC
}

// Function to compare two UIDs
bool compareUID(byte *uid1, byte *uid2, int size) {
  for(int i=0; i<size; i++) {
    if(uid1[i] != uid2[i]){
      return false;
    }
  }
  return true;
}

// Function to check if scanned card is Admin
bool isAdminCard(byte *scannedUID) {
  return compareUID(scannedUID, (byte*)ADMIN_UID, ADMIN_UID_SIZE);
}

// Function to check if scanned card is a User
bool isUserCard(byte *scannedUID) {
  for(int i=0; i<userCount; i++) {
    int addr = USER_UID_START_ADDR + i*4;
    byte storedUID[4];
    for(int j=0; j<4; j++) {
      storedUID[j] = EEPROM.read(addr + j);
    }
    if(compareUID(scannedUID, storedUID, 4)) {
      return true;
    }
  }
  return false;
}

// Function to add a new user
void addNewUser() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Add New User");
  lcd.setCursor(0,1);
  lcd.print("Scan User Card");
  
  // Wait for new card
  while (true) {
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      continue;
    }
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      continue;
    }
    
    byte newUID[4];
    for (byte i = 0; i < mfrc522.uid.size && i < 4; i++) {
      newUID[i] = mfrc522.uid.uidByte[i];
    }
    
    // Check if the card is already a user
    if(isUserCard(newUID) || isAdminCard(newUID)) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Card Exists");
      delay(2000);
      lcd.clear();
      return;
    }
    
    // Store the new UID in EEPROM
    int addr = USER_UID_START_ADDR + userCount*4;
    for(int i=0; i<4; i++) {
      EEPROM.write(addr + i, newUID[i]);
    }
    userCount++;
    EEPROM.write(USER_COUNT_ADDR, userCount);
    EEPROM.commit();
    
    Serial.println("New User Added");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("User Added");
    delay(2000);
    lcd.clear();
    return;
  }
}

// Function to unlock the door
void unlockDoor() {
  digitalWrite(RELAY_PIN, HIGH); // Unlock
  delay(5000); // Unlock duration (5 seconds)
  digitalWrite(RELAY_PIN, LOW); // Lock again
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Door Locked");
  delay(2000);
}

