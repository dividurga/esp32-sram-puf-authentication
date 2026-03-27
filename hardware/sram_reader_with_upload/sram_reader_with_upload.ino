/*
 * ESP32 RTC SLOW Memory Reader with Server Upload
 * 
 * IMPORTANT: SRAM PUF only changes on POWER CYCLE!
 * - Soft reset (reset button) does NOT change SRAM
 * - For new measurements: Disconnect power, wait 1-2s, reconnect
 */

#include <WiFi.h>
#include <HTTPClient.h>

#if __has_include("config.h")
    #include "config.h"
#endif

// ============================================
// CONFIGURATION - Edit if NOT using config.h
// ============================================
#ifndef WIFI_SSID
    #define WIFI_SSID "Divi iPhone"
#endif

#ifndef WIFI_PASSWORD
    #define WIFI_PASSWORD "test1234"
#endif

#ifndef MEASUREMENT_SERVER_URL
  #define MEASUREMENT_SERVER_URL "http://172.20.10.3:3000/api/sram"
#endif

#ifndef ESP_DEVICE_ID
    #define ESP_DEVICE_ID 1
#endif

#ifndef SRAM_ADDRESS
    #define SRAM_ADDRESS 0x50000000
#endif

#ifndef SRAM_READ_SIZE
    #define SRAM_READ_SIZE 128
#endif

// ============================================
// GLOBAL VARIABLES
// ============================================
String sramData = "";
bool uploadDone = false;

// ============================================
// FUNCTIONS
// ============================================

void readSRAM() {
    unsigned char *ram_buffer = (unsigned char *) ((uint32_t)SRAM_ADDRESS);
    sramData = "";
    
    Serial.println("[INFO] Reading RTC SLOW Memory...");
    
    for (size_t i = 0; i < (size_t)SRAM_READ_SIZE; i++) {
        if (ram_buffer[i] < 0x10) {
            sramData += "0";
        }
        sramData += String(ram_buffer[i], HEX);
    }
    
    Serial.print("      Read ");
    Serial.print((size_t)SRAM_READ_SIZE);
    Serial.println(" bytes from RTC SLOW");
    Serial.print("      Data: ");
    Serial.println(sramData);
}

bool uploadToServer() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[ERROR] WiFi not connected");
        return false;
    }
    
    WiFiClient client;
    HTTPClient http;
    
    Serial.println("[INFO] Uploading to server...");
    
    if (!http.begin(client, MEASUREMENT_SERVER_URL)) {
        Serial.println("      x Failed to connect");
        return false;
    }
    
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Basic ZXNwOnBhc3N3b3Jk");
    
    String jsonPayload = "{";
    jsonPayload += "\"espid\":" + String(ESP_DEVICE_ID) + ",";
    jsonPayload += "\"data\":\"" + sramData + "\"";
    jsonPayload += "}";
    
    Serial.print("      Payload size: ");
    Serial.print(jsonPayload.length());
    Serial.println(" bytes");
    
    int httpResponseCode = http.POST(jsonPayload);
    
    Serial.print("      Response Code: ");
    Serial.println(httpResponseCode);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        if (httpResponseCode == 200) {
            Serial.println("      Upload successful");
        } else {
            Serial.println("      x Upload failed");
            Serial.print("      Response: ");
            Serial.println(response);
        }
    } else {
        Serial.print("      x HTTP Error: ");
        Serial.println(http.errorToString(httpResponseCode));
    }
    
    http.end();
    delay(100);
    
    return (httpResponseCode == 200);
}

// ============================================
// MAIN PROGRAM
// ============================================

void setup() {
    Serial.begin(115200);
    delay(1000); // FIX 1: longer delay for Serial to initialize

    Serial.println("\n========================================");
    Serial.println("ESP32 RTC SLOW Memory Reader with Upload");
    Serial.println("========================================");
    Serial.println("IMPORTANT: SRAM PUF only changes on POWER CYCLE!");
    Serial.println("           Soft reset does NOT change SRAM values.");
    Serial.println("========================================\n");

    // Read SRAM before WiFi to avoid interference
    readSRAM();

    // FIX 2: correct WiFi init order
    Serial.println("[INFO] Connecting to WiFi...");
    Serial.print("      SSID: ");
    Serial.println(WIFI_SSID);

    WiFi.disconnect(true, true);
    delay(1000);
    WiFi.persistent(false);   // FIX 3: don't use stale cached credentials
    WiFi.mode(WIFI_STA);
    delay(100);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 60) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n      Connected!");
        Serial.print("      IP: ");
        Serial.println(WiFi.localIP());

        if (uploadToServer()) {
            uploadDone = true;
        }
    } else {
        Serial.println("\n[ERROR] WiFi connection failed after 30s!");
        Serial.println("        Check SSID, password, and hotspot IP.");
    }

    Serial.println("\n========================================");
    Serial.println("Done! For next measurement:");
    Serial.println("  1. Disconnect ESP32 power");
    Serial.println("  2. Wait 1-2 seconds");
    Serial.println("  3. Reconnect power");
    Serial.println("========================================\n");
}

void loop() {
    if (!uploadDone && WiFi.status() == WL_CONNECTED) {
        if (uploadToServer()) {
            uploadDone = true;
        }
    }
    delay(5000);
}