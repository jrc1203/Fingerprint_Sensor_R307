// Coding on ESP32-S3
#include "FS.h"
#include "Secrets.h"
#include "WebPage.h"
#include <Adafruit_Fingerprint.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <SPI.h>
#include <WiFi.h>

// --- CONFIGURATION ---
#define CANDIDATE_1 "Arjuna"
#define CANDIDATE_2 "Karna"
#define CANDIDATE_3 "Ekalavya"
#define CANDIDATE_4 "Abhimanyu"

// --- PINS (ESP32-S3 SAFE MAPPING) ---
#define PIN_RX2 16
#define PIN_TX2 17
#define PIN_BTN_A 18
#define PIN_BTN_B 14
#define PIN_BTN_C 4
#define PIN_BTN_RESULTS 5
#define PIN_BTN_D 19 // New Button D

#define PIN_SW_ANON 41

// --- LED PINS (Safe on S3) ---
#define PIN_LED_G 15
#define PIN_LED_R 7
#define PIN_LED_WIFI 8
#define PIN_LED_VOTE 21
#define PIN_LED_ANON 38
#define PIN_LED_ENROLL 39
#define PIN_BUZZ 40

// --- CANDIDATE LEDs ---
#define PIN_LED_A 20
#define PIN_LED_B 35
#define PIN_LED_C 36
#define PIN_LED_D 37
// TFT Display Pins (1.8" ST7735)
#define TFT_CS 10
#define TFT_RST 11
#define TFT_DC 9
#define TFT_MOSI 12
#define TFT_SCLK 13

// --- THEME COLOR DEFINITIONS (Ayodhya Dawn Match) ---
// Cream/White BG: #fffbf0 -> 0xFFFE
#define THEME_BG 0xFFFE
// Dark Brown/Red Text: #4a0404 -> 0x4800 (Maroon-ish)
#define THEME_TEXT 0x8000
// Dhwaj Saffron: #ff9933 -> 0xFC06
#define THEME_SAFFRON 0xFC06
// Metallic Gold: #c5a009 -> 0xC500
#define THEME_GOLD 0xFEA0
// Royal Red/Sindoor: #b71c1c -> 0xB0E0
#define THEME_RED 0xB800
// Success Green: #2e7d32 -> 0x2BE0
#define THEME_GREEN 0x2BE0

// HTML content is defined in WebPage.h

// --- GLOBAL OBJECTS ---
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
Adafruit_ST7735 tft =
    Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// --- DATA & STATE ---
struct User {
  uint16_t id;
  String name;
  bool voted;
};

User users[1001];
// Vote Counters (3 candidates)
uint32_t cntA = 0;
uint32_t cntB = 0;
uint32_t cntC = 0;
uint32_t cntD = 0;
bool sensorFound = false;

enum State { IDLE, ENROLL_WAIT_1, ENROLL_WAIT_2, WAIT_FOR_VOTE };
State currentState = IDLE;
uint16_t activeEnrollID = 0;
String activeEnrollName = "";
uint16_t activeVoterID = 0;
unsigned long scanTimer = 0;

// --- UTILS ---
void notifyClients(String msg = "") {
  JsonDocument doc;
  doc["cntA"] = cntA;
  doc["cntB"] = cntB;
  doc["cntC"] = cntC;
  doc["cntD"] = cntD;

  doc["nA"] = CANDIDATE_1;
  doc["nB"] = CANDIDATE_2;
  doc["nC"] = CANDIDATE_3;
  doc["nD"] = CANDIDATE_4;

  doc["anonMode"] = (digitalRead(PIN_SW_ANON) == LOW);

  if (msg != "")
    doc["status"] = msg;
  String output;
  serializeJson(doc, output);
  ws.textAll(output);
}

void notifyAlert(String msg) {
  String json = "{\"alert\":\"" + msg + "\"}";
  ws.textAll(json);
}

void broadcastUsersUpdate() {
  JsonDocument box;
  JsonArray usersArr = box.createNestedArray("users");

  for (int i = 1; i <= 1000; i++) {
    if (users[i].name != "") {
      JsonObject obj = usersArr.createNestedObject();
      obj["id"] = i;
      obj["name"] = users[i].name;
      obj["voted"] = users[i].voted;
    }
  }
  String out;
  serializeJson(box, out);
  ws.textAll(out);
}

// --- TFT DISPLAY HELPER (Updated for Theme) ---
// Default changed from WHITE to THEME_TEXT for visibility on Cream
void showTFT(String line1, String line2 = "", uint16_t color = THEME_TEXT) {
  // Use New Background Color
  tft.fillScreen(THEME_BG);

  // Draw Ornamental Border (Golden Age Theme)
  tft.drawRect(0, 0, 160, 128, THEME_GOLD);
  tft.drawRect(2, 2, 156, 124, THEME_GOLD);

  tft.setTextColor(THEME_SAFFRON); // Titles always Saffron now
  tft.setTextSize(2);
  tft.setCursor(5, 30);
  tft.println(line1);

  // Decorative divider
  tft.drawFastHLine(5, 55, 150, THEME_GOLD);

  if (line2 != "") {
    // Subtext always Royal Red/Brown
    tft.setTextColor(THEME_TEXT);
    tft.setTextSize(1);
    tft.setCursor(5, 70);
    tft.println(line2);
  }
}

// --- STORAGE LOADER ---
void saveDB() {
  File file = LittleFS.open("/db.json", "w");
  if (!file) {
    Serial.println("Failed to open DB");
    return;
  }

  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  // Only save used slots to save space
  for (int i = 1; i <= 1000; i++) {
    if (users[i].name != "") {
      JsonObject obj = arr.createNestedObject();
      obj["id"] = i;
      obj["n"] = users[i].name;
      if (users[i].voted)
        obj["v"] = 1;
    }
  }
  JsonObject meta = arr.createNestedObject();
  meta["meta"] = "counts";
  meta["a"] = cntA;
  meta["b"] = cntB;
  meta["c"] = cntC;
  meta["d"] = cntD;

  serializeJson(doc, file);
  file.close();
  Serial.println("DB Saved.");
}

void loadDB() {
  if (!LittleFS.exists("/db.json"))
    return;
  File file = LittleFS.open("/db.json", "r");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("DB Corrupt");
    return;
  }

  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject obj : arr) {
    if (obj.containsKey("meta")) {
      cntA = obj["a"];
      cntB = obj["b"];
      cntC = obj["c"];
      cntD = obj["d"];
    } else {
      int id = obj["id"];
      if (id > 0 && id <= 1000) {
        users[id].id = id;
        users[id].name = obj["n"].as<String>();
        users[id].voted = obj.containsKey("v");
      }
    }
  }
  file.close();
}

void sendFullDB(AsyncWebSocketClient *client) {
  // Send strictly the table data to one client
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  JsonDocument box;
  JsonArray usersArr = box.createNestedArray("users");

  for (int i = 1; i <= 1000; i++) {
    if (users[i].name != "") {
      JsonObject obj = usersArr.createNestedObject();
      obj["id"] = i;
      obj["name"] = users[i].name;
      obj["voted"] = users[i].voted;
    }
  }
  String out;
  serializeJson(box, out);
  client->text(out);
}

// --- FINGERPRINT LOGIC ---
int getFingerID() {
  if (!sensorFound)
    return -1;
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
    return -1; // No finger or error
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return -1;
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_NOTFOUND)
    return -2; // Finger found but NO MATCH
  if (p != FINGERPRINT_OK)
    return -1; // Communication error or other
  return finger.fingerID;
}

void feedback(bool success) {
  digitalWrite(PIN_LED_G, success);
  digitalWrite(PIN_LED_R, !success);
  digitalWrite(PIN_BUZZ, HIGH);
  delay(success ? 200 : 1000); // Longer beep for failure
  digitalWrite(PIN_BUZZ, LOW);
  delay(100);
  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_R, LOW);
}

// --- STATE MACHINE ---
void handleEnrollStep() {
  static int lastStep = 0; // 0=None, 1=Step1 Shown, 2=Step2 Shown

  if (!sensorFound) {
    notifyAlert("Sensor Disconnected!");
    currentState = IDLE;
    return;
  }

  if (currentState == ENROLL_WAIT_1) {
    if (lastStep != 1) {
      notifyClients("Place Finger (Step 1)...");
      showTFT("ENROLL", "Place Finger (1/2)", THEME_SAFFRON);
      lastStep = 1;
    }

    int p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      if (finger.image2Tz(1) == FINGERPRINT_OK) {
        notifyClients("Remove Finger");
        showTFT("GOOD!", "Remove Finger...", THEME_GREEN);
        feedback(true);
        delay(2000);
        currentState = ENROLL_WAIT_2;
        lastStep = 0;
        while (finger.getImage() != FINGERPRINT_NOFINGER)
          ;
      } else {
        feedback(false);
      }
    }
  } else if (currentState == ENROLL_WAIT_2) {
    if (lastStep != 2) {
      notifyClients("Place Same Finger Again (Step 2)...");
      showTFT("ENROLL", "Place Finger (2/2)", THEME_SAFFRON);
      lastStep = 2;
    }

    int p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      if (finger.image2Tz(2) == FINGERPRINT_OK) {
        if (finger.createModel() == FINGERPRINT_OK) {
          int id = activeEnrollID;
          if (finger.storeModel(id) == FINGERPRINT_OK) {
            // Success! Save to RAM
            users[id].name = activeEnrollName;
            users[id].id = id;
            users[id].voted = false;
            saveDB();
            digitalWrite(PIN_LED_ENROLL, LOW);
            feedback(true);
            notifyAlert("Enrolled ID #" + String(id) + ": " + activeEnrollName);
            broadcastUsersUpdate();
            showTFT("DHANYAVAD", activeEnrollName, THEME_GREEN);
            delay(1500);

            // AUTO-VOTE: Transition directly to voting state
            activeVoterID = id;
            currentState = WAIT_FOR_VOTE;
            notifyClients("Welcome " + activeEnrollName +
                          "! Now VOTE: A, B, C, D");
            showTFT("VOTE NOW", "Select Candidate", THEME_RED);
            lastStep = 0;
          } else {
            notifyAlert("Store Failed");
            currentState = IDLE;
            lastStep = 0;
          }
        } else {
          notifyAlert("Match Failed");
          currentState = IDLE;
          lastStep = 0;
        }
      }
    }
  }
}

void handleAuth() {
  int fid = getFingerID();
  if (fid > 0) {
    // Finger Found!
    if (users[fid].voted) {
      // ALREADY VOTED
      Serial.println("Already Voted!");
      notifyClients("User #" + String(fid) + " (" + users[fid].name +
                    ") ALREADY VOTED!");
      showTFT("DENIED!", users[fid].name + " Voted", THEME_RED);
      feedback(false); // Red LED + Long Beep
      delay(2000);
      showTFT("SARA SAMRATAH", "Place Finger to Vote", THEME_SAFFRON);
    } else {
      // ALLOW VOTE
      activeVoterID = fid;
      currentState = WAIT_FOR_VOTE;
      notifyClients("Welcome " + users[fid].name + "! Press A, B, C, D.");
      showTFT("NAMASTE", users[fid].name, THEME_GREEN);
      feedback(true);
      delay(1000);
      showTFT("VOTE NOW", "Select Candidate", THEME_RED);
    }
  } else if (fid == -2) {
    // UNKNOWN FINGER
    Serial.println("Unknown Finger");
    notifyClients("Unknown Finger Detected! Access Denied.");
    showTFT("ACCESS DENIED", "Unknown User", THEME_RED);
    feedback(false);
    delay(2000);
    showTFT("RAM RAJYA", "Place Finger to Vote", THEME_SAFFRON);
  }
}

void handleVotingButtons() {
  if (currentState == WAIT_FOR_VOTE) {
    // Blink vote LED
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 250) {
      digitalWrite(PIN_LED_VOTE, !digitalRead(PIN_LED_VOTE));
      lastBlink = millis();
    }

    bool A = !digitalRead(PIN_BTN_A);
    bool B = !digitalRead(PIN_BTN_B);
    bool C = !digitalRead(PIN_BTN_C);
    bool D = !digitalRead(PIN_BTN_D);

    if (A || B || C || D) {
      digitalWrite(PIN_LED_VOTE, LOW); // Stop blinking
      String voteName = "";
      int ledPin = -1;

      if (A) {
        cntA++;
        voteName = CANDIDATE_1;
        ledPin = PIN_LED_A;
      } else if (B) {
        cntB++;
        voteName = CANDIDATE_2;
        ledPin = PIN_LED_B;
      } else if (C) {
        cntC++;
        voteName = CANDIDATE_3;
        ledPin = PIN_LED_C;
      } else if (D) {
        cntD++;
        voteName = CANDIDATE_4;
        ledPin = PIN_LED_D;
      }

      if (activeVoterID > 0 && activeVoterID <= 1000) {
        users[activeVoterID].voted = true;
      }
      saveDB();
      feedback(true);

      // Candidate LED Feedback
      if (ledPin != -1) {
        digitalWrite(ledPin, HIGH);
        delay(1000); // Glow for 1 second
        digitalWrite(ledPin, LOW);
      }

      showTFT("SUCCESS!", "Voted: " + voteName.substring(0, 15), THEME_GREEN);
      notifyClients("Vote Cast Successfully!");
      broadcastUsersUpdate();
      delay(2000);
      notifyClients("System Ready");
      showTFT("SARA SAMRATAH", "Place Finger to Vote", THEME_SAFFRON);
      currentState = IDLE;
      activeVoterID = 0;
    }
  }
}

#define PIN_BTN_RESET 0

// --- SHOW RESULTS ON TFT (Modified for Theme Colors only) ---
void showResults() {
  // Use cream background and custom colors
  tft.fillScreen(THEME_BG);
  tft.drawRect(0, 0, 160, 128, THEME_GOLD);

  tft.setTextColor(THEME_SAFFRON);
  tft.setTextSize(1);
  tft.setCursor(5, 5);
  tft.println("=== CURRENT RESULTS ===");

  tft.setTextSize(2);

  // A - Saffron
  tft.setTextColor(THEME_SAFFRON);
  tft.setCursor(5, 25);
  tft.print("A: ");
  tft.println(cntA);

  // B - Gold
  tft.setTextColor(THEME_GOLD);
  tft.setCursor(5, 50);
  tft.print("B: ");
  tft.println(cntB);

  // C - Green
  tft.setTextColor(THEME_GREEN);
  tft.setCursor(5, 75);
  tft.print("C: ");
  tft.println(cntC);

  // D - Purple (Using THEME_TEXT for now or similar)
  tft.setTextColor(THEME_TEXT);
  tft.setCursor(80, 25); // Top right column
  tft.print("D: ");
  tft.println(cntD);

  // Total - Text color
  tft.setTextSize(1);
  tft.setTextColor(THEME_TEXT);
  tft.setCursor(5, 105);
  tft.print("Total Votes: ");
  tft.println(cntA + cntB + cntC + cntD);

  notifyClients("Showing Results on TFT...");
  delay(3000);
  showTFT("RAM RAJYA", "Place Finger to Vote", THEME_SAFFRON);
  notifyClients("System Ready");
}

// --- ANONYMOUS MODE HANDLER ---
void handleAnonMode() {
  int p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    feedback(true);
    showTFT("ANON VOTE", "Select A, B, C, D", THEME_SAFFRON);
    notifyClients("Anonymous Voter - Select Candidate!");
    activeVoterID = 0;
    currentState = WAIT_FOR_VOTE;
  }
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);

  // Initialize TFT Display
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(THEME_BG); // Init with Cream
  showTFT("BOOTING...", "Sara Samratah", THEME_SAFFRON);

  pinMode(PIN_BTN_A, INPUT_PULLUP);
  pinMode(PIN_BTN_B, INPUT_PULLUP);
  pinMode(PIN_BTN_C, INPUT_PULLUP);
  pinMode(PIN_BTN_D, INPUT_PULLUP);
  pinMode(PIN_BTN_RESULTS, INPUT_PULLUP);
  pinMode(PIN_SW_ANON, INPUT_PULLUP);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);

  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_WIFI, OUTPUT);
  pinMode(PIN_LED_VOTE, OUTPUT);
  pinMode(PIN_LED_ANON, OUTPUT);
  pinMode(PIN_LED_ENROLL, OUTPUT);
  pinMode(PIN_BUZZ, OUTPUT);

  pinMode(PIN_LED_A, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_LED_C, OUTPUT);
  pinMode(PIN_LED_D, OUTPUT);

  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_R, LOW);
  digitalWrite(PIN_LED_WIFI, LOW);
  digitalWrite(PIN_LED_VOTE, LOW);
  digitalWrite(PIN_LED_ANON, LOW);
  digitalWrite(PIN_LED_ENROLL, LOW);

  if (!LittleFS.begin(true))
    Serial.println("LittleFS Error");
  loadDB();

  mySerial.begin(57600, SERIAL_8N1, PIN_RX2, PIN_TX2);
  delay(100);
  if (finger.verifyPassword()) {
    Serial.println("Sensor Found!");
    sensorFound = true;
    showTFT("Sensor OK", "", THEME_GREEN);
  } else {
    Serial.println("Sensor NOT Found :(");
    sensorFound = false;
    showTFT("NO SENSOR!", "Check wiring", THEME_RED);
  }
  delay(1000);

  // Wi-Fi
  if (USE_AP_MODE) {
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    digitalWrite(PIN_LED_WIFI, HIGH);
  } else {
    WiFi.begin(STA_SSID, STA_PASS);
    while (WiFi.status() != WL_CONNECTED)
      delay(500);
    Serial.println(WiFi.localIP());
    digitalWrite(PIN_LED_WIFI, HIGH);
  }

  // Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/enroll", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("name")) {
      String n = request->getParam("name")->value();
      int slot = 0;
      for (int i = 1; i <= 1000; i++) {
        if (users[i].name == "") {
          slot = i;
          break;
        }
      }
      if (slot > 0) {
        activeEnrollID = slot;
        activeEnrollName = n;
        currentState = ENROLL_WAIT_1;
        request->send(200, "text/plain", "OK. Place Finger...");
      } else {
        request->send(200, "text/plain", "Full!");
      }
    }
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    for (int i = 1; i <= 1000; i++)
      users[i].voted = false;
    cntA = 0;
    cntB = 0;
    cntC = 0;
    cntD = 0;
    saveDB();
    notifyClients("Election Reset!");
    broadcastUsersUpdate();
    request->send(200, "text/plain", "OK");
  });

  server.on("/wipe", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (sensorFound)
      finger.emptyDatabase();
    LittleFS.remove("/db.json");
    ESP.restart();
    request->send(200);
  });

  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("id")) {
      int id = request->getParam("id")->value().toInt();
      if (sensorFound)
        finger.deleteModel(id);
      users[id].name = "";
      users[id].voted = false;
      saveDB();
      notifyClients("Deleted ID " + String(id));
      broadcastUsersUpdate();
    }
    request->send(200);
  });

  // WebSocket
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      sendFullDB(client);
      notifyClients();
    }
  });
  server.addHandler(&ws);
  server.begin();

  // Show Ready Screen on TFT
  showTFT("RAM RAJYA", "Place Finger to Vote", THEME_SAFFRON);
}

void loop() {
  ws.cleanupClients();

  bool anonModeOn = (digitalRead(PIN_SW_ANON) == LOW);
  digitalWrite(PIN_LED_ANON, anonModeOn);

  static unsigned long lastResultsPress = 0;
  if (digitalRead(PIN_BTN_RESULTS) == LOW && currentState == IDLE) {
    if (millis() - lastResultsPress > 500) {
      lastResultsPress = millis();
      showResults();
    }
  }

  if (digitalRead(PIN_BTN_RESET) == LOW) {
    delay(50);
    if (digitalRead(PIN_BTN_RESET) == LOW) {
      Serial.println("HARDWARE RESET TRIGGERED!");
      showTFT("RESETTING...", "Please Wait", THEME_RED);

      for (int i = 1; i <= 1000; i++)
        users[i].voted = false;
      cntA = 0;
      cntB = 0;
      cntC = 0;
      cntD = 0;
      saveDB();

      notifyClients("Election Reset by Admin Button!");
      broadcastUsersUpdate();

      feedback(false);
      delay(1000);
      showTFT("SARA SAMRATAH", "Place Finger to Vote", THEME_SAFFRON);
      currentState = IDLE;

      while (digitalRead(PIN_BTN_RESET) == LOW)
        ;
    }
  }

  // Main Logic
  if (currentState == IDLE) {
    if (millis() - scanTimer > 100) {
      if (sensorFound) {
        if (anonModeOn) {
          handleAnonMode();
        } else {
          handleAuth();
        }
      }
      scanTimer = millis();
    }
  } else if (currentState == ENROLL_WAIT_1 || currentState == ENROLL_WAIT_2) {
    digitalWrite(PIN_LED_ENROLL, HIGH);
    handleEnrollStep();
  } else if (currentState == WAIT_FOR_VOTE) {
    handleVotingButtons();
  }
}