/*
 * WaterTank Monitor v1.0 - ESP32 + HC-SR04 Ultrasonic Sensor
 * Adapted from Aqualevel project by Ravi Singh
 * 
 * Hardware: ESP32 + HC-SR04 Ultrasonic Sensor
 * 
 * Organization:
 * - config.h: Global constants and configuration
 * - eeprom_manager: Handles saving/loading settings
 * - sensor_manager: Handles ultrasonic sensor readings
 * - tank_calculator: Calculates water level and volume
 * - web_interface: Web server and UI
 * - wifi_manager: WiFi access point setup and mDNS support
 * 
 * Features:
 * - Accurate water level and volume monitoring
 * - Beautiful web interface with animations
 * - User-configurable tank parameters
 * - WiFi network connection with mDNS support
 */

#include <Arduino.h>
#include "config.h"
#include "eeprom_manager.h"
#include "sensor_manager.h"
#include "tank_calculator.h"
#include "web_interface.h"
#include "wifi_manager.h"


// For managing reading timing
unsigned long previousReadMillis = 0;
unsigned long previousUpdateMillis = 0;
unsigned long webUpdateInterval = 500;   // milliseconds between web updates

void setup() {
  // Initialize serial first for debugging
  Serial.begin(115200);
  Serial.println("\n\n");
  Serial.println("***************************************");
  Serial.println("* Aqualevel v1.0              *");
  Serial.println("* Ultrasonic Water Level Monitoring   *");
  Serial.println("* With WiFi & mDNS Support            *");
  Serial.println("***************************************");
  Serial.println("\n");
  
  // Critical initialization sequence:
  // 1. EEPROM first to load settings
  setupEEPROM();     
  
  // 2. Initialize WiFi manager (early for network setup)
  wifiManager.begin();
  
  // 3. Initialize tank calculator with loaded settings
  setupTankCalculator();
  
  // 4. Initialize ultrasonic sensor
  setupSensor();      
  
  // 5. Start web server
  setupWebServer();  
  
  Serial.println("Initialization complete. System ready.");
}

void loop() {
  // Process WiFi events and maintain connection
  wifiManager.process();
  
  // Handle web server requests
  handleWebServer();
  
  // Read sensor at regular intervals, using the user-defined interval
  unsigned long currentMillis = millis();
  if (currentMillis - previousReadMillis >= (measurementInterval * 1000)) {
    previousReadMillis = currentMillis;
    
    // Read water level from sensor
    readSensorDistance();
    
    // Calculate water level and volume based on sensor reading
    calculateWaterLevel();
  }
  
  // Update web clients at regular intervals (if needed)
  if (currentMillis - previousUpdateMillis >= webUpdateInterval) {
    previousUpdateMillis = currentMillis;
    
    // Any periodic web updates can go here
    // (The main sensor data is sent when clients request it)
  }
}