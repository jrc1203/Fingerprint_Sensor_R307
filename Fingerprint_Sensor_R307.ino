/*
 * R307S Fingerprint Sensor Test with ESP32-WROOM
 * 
 * Wiring:
 *   R307S VCC  -> ESP32 3.3V
 *   R307S GND  -> ESP32 GND
 *   R307S TX   -> ESP32 GPIO16 (RX2)
 *   R307S RX   -> ESP32 GPIO17 (TX2)
 * 
 * Commands (via Serial Monitor at 115200 baud):
 *   'e' - Enroll a new fingerprint
 *   's' - Search for a fingerprint
 *   'd' - Delete all fingerprints
 *   'c' - Count stored fingerprints
 */

#include <Adafruit_Fingerprint.h>

// Use Hardware Serial2 on ESP32 (GPIO16=RX, GPIO17=TX)
#define RX_PIN 16
#define TX_PIN 17

HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t nextID = 1;  // Next ID to use for enrollment

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========================================");
  Serial.println("  R307S Fingerprint Sensor Test");
  Serial.println("========================================\n");

  // Initialize Serial2 for fingerprint sensor
  mySerial.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Initialize fingerprint sensor
  finger.begin(57600);
  delay(500);
  
  if (finger.verifyPassword()) {
    Serial.println("✓ Fingerprint sensor detected!");
  } else {
    Serial.println("✗ Fingerprint sensor NOT found!");
    Serial.println("  Check your wiring:");
    Serial.println("  - Is VCC connected to 3.3V?");
    Serial.println("  - Is TX->GPIO16 and RX->GPIO17?");
    Serial.println("  - Are TX and RX crossed (not straight)?");
    while (1) { delay(1); }
  }

  // Get sensor parameters
  finger.getParameters();
  Serial.print("  Capacity: ");
  Serial.print(finger.capacity);
  Serial.println(" fingerprints");
  Serial.print("  Security level: ");
  Serial.println(finger.security_level);
  
  // Count existing fingerprints
  finger.getTemplateCount();
  Serial.print("  Stored fingerprints: ");
  Serial.println(finger.templateCount);
  
  nextID = finger.templateCount + 1;
  
  Serial.println("\n----------------------------------------");
  Serial.println("Commands:");
  Serial.println("  'e' - Enroll new fingerprint");
  Serial.println("  's' - Search/match fingerprint");
  Serial.println("  'd' - Delete ALL fingerprints");
  Serial.println("  'c' - Count fingerprints");
  Serial.println("----------------------------------------\n");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case 'e':
      case 'E':
        enrollFingerprint();
        break;
        
      case 's':
      case 'S':
        searchFingerprint();
        break;
        
      case 'd':
      case 'D':
        deleteAllFingerprints();
        break;
        
      case 'c':
      case 'C':
        countFingerprints();
        break;
    }
  }
  
  delay(50);
}

// ============================================
// ENROLL A NEW FINGERPRINT
// ============================================
void enrollFingerprint() {
  Serial.println("\n>> ENROLL MODE");
  Serial.print("   Enrolling as ID #");
  Serial.println(nextID);
  Serial.println("   Place your finger on the sensor...");
  
  // Wait for finger
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("   ✓ Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("   ✗ Communication error");
        return;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("   ✗ Imaging error");
        return;
      default:
        Serial.println("   ✗ Unknown error");
        return;
    }
  }

  // Convert image to template (first scan)
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("   ✗ Could not process image");
    return;
  }

  Serial.println("   Remove finger...");
  delay(1000);
  while (finger.getImage() != FINGERPRINT_NOFINGER) {
    delay(100);
  }

  Serial.println("   Place SAME finger again...");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("   ✓ Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        break;
      default:
        return;
    }
  }

  // Convert image to template (second scan)
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("   ✗ Could not process image");
    return;
  }
  
  // Create model from the two templates
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("   ✓ Prints matched!");
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("   ✗ Fingerprints did not match!");
    return;
  } else {
    Serial.println("   ✗ Unknown error");
    return;
  }

  // Store the model
  p = finger.storeModel(nextID);
  if (p == FINGERPRINT_OK) {
    Serial.print("   ✓ SUCCESS! Stored as ID #");
    Serial.println(nextID);
    nextID++;
  } else {
    Serial.println("   ✗ Could not store fingerprint");
    return;
  }
  
  Serial.println();
}

// ============================================
// SEARCH FOR A FINGERPRINT
// ============================================
void searchFingerprint() {
  Serial.println("\n>> SEARCH MODE");
  Serial.println("   Place your finger on the sensor...");
  
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      continue;
    } else if (p != FINGERPRINT_OK) {
      Serial.println("   ✗ Could not take image");
      return;
    }
  }
  
  Serial.println("   ✓ Image taken");
  
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("   ✗ Could not process image");
    return;
  }
  
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("   ✓ MATCH FOUND! ID #");
    Serial.print(finger.fingerID);
    Serial.print(" (confidence: ");
    Serial.print(finger.confidence);
    Serial.println(")");
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("   ✗ NO MATCH - Fingerprint not registered");
  } else {
    Serial.println("   ✗ Search error");
  }
  
  Serial.println();
}

// ============================================
// DELETE ALL FINGERPRINTS
// ============================================
void deleteAllFingerprints() {
  Serial.println("\n>> DELETE ALL");
  Serial.println("   Are you sure? Send 'y' to confirm...");
  
  // Wait for confirmation
  while (!Serial.available()) {
    delay(100);
  }
  
  char confirm = Serial.read();
  if (confirm == 'y' || confirm == 'Y') {
    finger.emptyDatabase();
    Serial.println("   ✓ All fingerprints deleted!");
    nextID = 1;
  } else {
    Serial.println("   Cancelled.");
  }
  
  Serial.println();
}

// ============================================
// COUNT STORED FINGERPRINTS
// ============================================
void countFingerprints() {
  finger.getTemplateCount();
  Serial.print("\n>> Stored fingerprints: ");
  Serial.println(finger.templateCount);
  Serial.println();
}
