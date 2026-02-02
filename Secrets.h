#ifndef SECRETS_H
#define SECRETS_H

// --- Wi-Fi Configuration ---
// MODE: AP (Access Point) - Highly Recommended for Exhibitions
// The ESP32 will create its own Wi-Fi network.
#define USE_AP_MODE true

// AP Credentials (The Network You Create)
#define AP_SSID "Sara Samratah"
#define AP_PASS "12345678"

// Station Credentials (If you want to connect to School Wi-Fi)
// Set USE_AP_MODE to false to use this.
#define STA_SSID "pie"
#define STA_PASS "Joy@2026"

#endif
