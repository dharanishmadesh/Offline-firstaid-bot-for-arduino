#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// OLED Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SSD1306_I2C_ADDRESS 0x3C  // Set your I2C address here
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RFID settings
#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Keypad settings
const byte ROW_NUM = 4;
const byte COLUMN_NUM = 3;
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte pin_rows[ROW_NUM] = {9, 8, 7, 6};
byte pin_column[COLUMN_NUM] = {5, 4, 3};
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// GPS settings
TinyGPSPlus gps;
SoftwareSerial ss(4, 3); // RX, TX for GPS module

// DFPlayer Mini settings for Voice Assistant
SoftwareSerial mySoftwareSerial(11, 12);  // RX, TX for DFPlayer Mini
DFRobotDFPlayerMini myDFPlayer;

// Sample hospital data
const char hospitals[][30] PROGMEM = {
  "City Hospital - 3 km",
  "Green Health Clinic - 2 km",
  "Care Plus Medical Center - 5 km"
};

// First Aid Problem and Solution Database
const char problems[][50] PROGMEM = {
  "1. Bleeding: Apply pressure.",
  "2. Burn: Run under cold water.",
  "3. Choking: Heimlich maneuver."
};

const char solutions[][50] PROGMEM = {
  "Apply direct pressure to stop bleeding.",
  "Cool burn with water, don't apply ice.",
  "Perform Heimlich if airway is blocked."
};

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize OLED display
  if (!display.begin(SSD1306_I2C_ADDRESS, OLED_RESET)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);

  // Initialize RFID
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println(F("RFID scanner initialized."));

  // Initialize Keypad
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Initialize GPS
  ss.begin(9600); // GPS baud rate

  // Initialize DFPlayer Mini
  mySoftwareSerial.begin(9600);
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("DFPlayer Mini not found."));
    while (true);
  }
  myDFPlayer.volume(15); // Set volume (0 to 30)

  // Welcome Message on OLED and Voice Assistant
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Welcome to First Aid Bot!"));
  display.display();
  
  // Voice prompt
  myDFPlayer.play(1); // Play the first audio file (pre-recorded on SD card)
}

void loop() {
  // Read keypad input
  char key = keypad.getKey();
  if (key) {
    handleKeyPress(key);
  }

  // GPS reading
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }
  
  // If GPS has enough data, display location
  if (gps.location.isUpdated()) {
    displayLocation();
  }

  // RFID scanning
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      String rfidTag = getRFIDTag();
      displayRFIDInfo(rfidTag);
    }
  }
}

void handleKeyPress(char key) {
  display.clearDisplay();
  display.setCursor(0, 0);
  
  if (key >= '1' && key <= '9') {
    int index = key - '1';
    char problem[50];
    char solution[50];
    strcpy_P(problem, (char *)pgm_read_word(&(problems[index])));
    strcpy_P(solution, (char *)pgm_read_word(&(solutions[index])));
    display.println(problem);
    display.println(solution);
  }
  
  if (key == '1') {  // Show nearby hospitals on key press '1'
    display.clearDisplay();
    display.setCursor(0, 0);
    for (int i = 0; i < 3; i++) {
      char hospital[30];
      strcpy_P(hospital, (char *)pgm_read_word(&(hospitals[i])));
      display.println(hospital);
    }
    myDFPlayer.play(2); // Play the hospital information audio file
  }
  
  display.display();
}

String getRFIDTag() {
  String rfidTag = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidTag += String(mfrc522.uid.uidByte[i], HEX);
  }
  return rfidTag;
}

void displayRFIDInfo(String rfidTag) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("RFID Tag Scanned:"));
  display.println(rfidTag);  // Display the RFID tag
  display.display();
  delay(2000); // Pause for 2 seconds
}

void displayLocation() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("Lat: "));
  display.println(gps.location.lat(), 6); // Latitude
  display.print(F("Lng: "));
  display.println(gps.location.lng(), 6); // Longitude
  display.display();
  delay(2000); // Pause for 2 seconds
}
