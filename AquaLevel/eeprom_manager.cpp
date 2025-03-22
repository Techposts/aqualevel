#include <EEPROM.h>
#include <Arduino.h>
#include "config.h"
#include "eeprom_manager.h"

// 📌 Global Variables (defined in main file, declared in config.h)
float tankHeight = DEFAULT_TANK_HEIGHT;
float tankDiameter = DEFAULT_TANK_DIAMETER;
float tankVolume = DEFAULT_TANK_VOLUME;
float sensorOffset = DEFAULT_SENSOR_OFFSET;
float emptyDistance = DEFAULT_EMPTY_DISTANCE;
float fullDistance = DEFAULT_FULL_DISTANCE;
int measurementInterval = DEFAULT_MEASUREMENT_INTERVAL;
int readingSmoothing = DEFAULT_READING_SMOOTHING;
int alertLevelLow = DEFAULT_ALERT_LEVEL_LOW;
int alertLevelHigh = DEFAULT_ALERT_LEVEL_HIGH;
bool alertsEnabled = DEFAULT_ALERTS_ENABLED;

// Current readings
float currentDistance = 0.0;
float currentWaterLevel = 0.0;
float currentPercentage = 0.0;
float currentVolume = 0.0;

void setupEEPROM() {
  Serial.println("Initializing EEPROM...");
  
  // Initialize EEPROM with specified size
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Failed to initialize EEPROM!");
    delay(1000);
  }
  
  loadSettings();
}

// Simple CRC calculation
byte calculateCRC() {
  byte crc = 0;
  
  // XOR all setting values to form a simple checksum
  // For float values, we convert to int first to simplify
  crc ^= (int)tankHeight & 0xFF;
  crc ^= ((int)tankHeight >> 8) & 0xFF;
  crc ^= (int)tankDiameter & 0xFF;
  crc ^= ((int)tankDiameter >> 8) & 0xFF;
  crc ^= (int)tankVolume & 0xFF;
  crc ^= ((int)tankVolume >> 8) & 0xFF;
  crc ^= (int)sensorOffset & 0xFF;
  crc ^= ((int)sensorOffset >> 8) & 0xFF;
  crc ^= (int)emptyDistance & 0xFF;
  crc ^= ((int)emptyDistance >> 8) & 0xFF;
  crc ^= (int)fullDistance & 0xFF;
  crc ^= ((int)fullDistance >> 8) & 0xFF;
  crc ^= measurementInterval & 0xFF;
  crc ^= readingSmoothing & 0xFF;
  crc ^= alertLevelLow & 0xFF;
  crc ^= alertLevelHigh & 0xFF;
  crc ^= alertsEnabled ? 1 : 0;
  
  return crc;
}

void saveSettings() {
  // First byte as an initialization marker
  EEPROM.write(EEPROM_ADDR_MARKER, EEPROM_INITIALIZED_MARKER);
  
  // Store actual values - converting floats to integers (x100) for storage
  int tankHeightInt = (int)(tankHeight * 100);
  EEPROM.write(EEPROM_ADDR_TANK_HEIGHT_L, tankHeightInt & 0xFF);
  EEPROM.write(EEPROM_ADDR_TANK_HEIGHT_H, (tankHeightInt >> 8) & 0xFF);
  
  int tankDiameterInt = (int)(tankDiameter * 100);
  EEPROM.write(EEPROM_ADDR_TANK_DIAMETER_L, tankDiameterInt & 0xFF);
  EEPROM.write(EEPROM_ADDR_TANK_DIAMETER_H, (tankDiameterInt >> 8) & 0xFF);
  
  int tankVolumeInt = (int)(tankVolume * 10); // 1 decimal place precision
  EEPROM.write(EEPROM_ADDR_TANK_VOLUME_L, tankVolumeInt & 0xFF);
  EEPROM.write(EEPROM_ADDR_TANK_VOLUME_H, (tankVolumeInt >> 8) & 0xFF);
  
  int sensorOffsetInt = (int)(sensorOffset * 100);
  EEPROM.write(EEPROM_ADDR_SENSOR_OFFSET_L, sensorOffsetInt & 0xFF);
  EEPROM.write(EEPROM_ADDR_SENSOR_OFFSET_H, (sensorOffsetInt >> 8) & 0xFF);
  
  int emptyDistanceInt = (int)(emptyDistance * 100);
  EEPROM.write(EEPROM_ADDR_EMPTY_DISTANCE_L, emptyDistanceInt & 0xFF);
  EEPROM.write(EEPROM_ADDR_EMPTY_DISTANCE_H, (emptyDistanceInt >> 8) & 0xFF);
  
  int fullDistanceInt = (int)(fullDistance * 100);
  EEPROM.write(EEPROM_ADDR_FULL_DISTANCE_L, fullDistanceInt & 0xFF);
  EEPROM.write(EEPROM_ADDR_FULL_DISTANCE_H, (fullDistanceInt >> 8) & 0xFF);
  
  EEPROM.write(EEPROM_ADDR_MEASUREMENT_INTERVAL, measurementInterval);
  EEPROM.write(EEPROM_ADDR_READING_SMOOTHING, readingSmoothing);
  EEPROM.write(EEPROM_ADDR_ALERT_LEVEL_LOW, alertLevelLow);
  EEPROM.write(EEPROM_ADDR_ALERT_LEVEL_HIGH, alertLevelHigh);
  EEPROM.write(EEPROM_ADDR_ALERTS_ENABLED, alertsEnabled ? 1 : 0);
  
  // Calculate and store CRC
  byte crc = calculateCRC();
  EEPROM.write(EEPROM_ADDR_CRC, crc);
  
  // Commit the data to flash
  // THIS IS CRITICAL FOR ESP32 - without this, data isn't actually saved to flash
  if (EEPROM.commit()) {
    Serial.println("EEPROM committed successfully");
  } else {
    Serial.println("ERROR: EEPROM commit failed");
  }
  
  Serial.println("Settings saved to EEPROM:");
  Serial.println("Tank Height: " + String(tankHeight) + " cm");
  Serial.println("Tank Diameter: " + String(tankDiameter) + " cm");
  Serial.println("Tank Volume: " + String(tankVolume) + " L");
  Serial.println("Sensor Offset: " + String(sensorOffset) + " cm");
  Serial.println("Empty Distance: " + String(emptyDistance) + " cm");
  Serial.println("Full Distance: " + String(fullDistance) + " cm");
  Serial.println("Measurement Interval: " + String(measurementInterval) + " s");
  Serial.println("Reading Smoothing: " + String(readingSmoothing));
  Serial.println("Alert Level Low: " + String(alertLevelLow) + "%");
  Serial.println("Alert Level High: " + String(alertLevelHigh) + "%");
  Serial.println("Alerts Enabled: " + String(alertsEnabled ? "Yes" : "No"));
}

void loadSettings() {
  // Check if EEPROM has been initialized with our marker
  byte initialized = EEPROM.read(EEPROM_ADDR_MARKER);
  
  if (initialized == EEPROM_INITIALIZED_MARKER) {
    // EEPROM has been initialized, load values
    
    // Read tank height (using 2 bytes for values that can exceed 255)
    int tankHeightInt = EEPROM.read(EEPROM_ADDR_TANK_HEIGHT_L) | (EEPROM.read(EEPROM_ADDR_TANK_HEIGHT_H) << 8);
    tankHeight = tankHeightInt / 100.0;
    
    // Read tank diameter (using 2 bytes)
    int tankDiameterInt = EEPROM.read(EEPROM_ADDR_TANK_DIAMETER_L) | (EEPROM.read(EEPROM_ADDR_TANK_DIAMETER_H) << 8);
    tankDiameter = tankDiameterInt / 100.0;
    
    // Read tank volume (using 2 bytes)
    int tankVolumeInt = EEPROM.read(EEPROM_ADDR_TANK_VOLUME_L) | (EEPROM.read(EEPROM_ADDR_TANK_VOLUME_H) << 8);
    tankVolume = tankVolumeInt / 10.0;
    
    // Read sensor offset (using 2 bytes)
    int sensorOffsetInt = EEPROM.read(EEPROM_ADDR_SENSOR_OFFSET_L) | (EEPROM.read(EEPROM_ADDR_SENSOR_OFFSET_H) << 8);
    sensorOffset = sensorOffsetInt / 100.0;
    
    // Read empty distance (using 2 bytes)
    int emptyDistanceInt = EEPROM.read(EEPROM_ADDR_EMPTY_DISTANCE_L) | (EEPROM.read(EEPROM_ADDR_EMPTY_DISTANCE_H) << 8);
    emptyDistance = emptyDistanceInt / 100.0;
    
    // Read full distance (using 2 bytes)
    int fullDistanceInt = EEPROM.read(EEPROM_ADDR_FULL_DISTANCE_L) | (EEPROM.read(EEPROM_ADDR_FULL_DISTANCE_H) << 8);
    fullDistance = fullDistanceInt / 100.0;
    
    measurementInterval = EEPROM.read(EEPROM_ADDR_MEASUREMENT_INTERVAL);
    readingSmoothing = EEPROM.read(EEPROM_ADDR_READING_SMOOTHING);
    alertLevelLow = EEPROM.read(EEPROM_ADDR_ALERT_LEVEL_LOW);
    alertLevelHigh = EEPROM.read(EEPROM_ADDR_ALERT_LEVEL_HIGH);
    alertsEnabled = EEPROM.read(EEPROM_ADDR_ALERTS_ENABLED) == 1;
    
    // Read stored CRC
    byte storedCRC = EEPROM.read(EEPROM_ADDR_CRC);
    
    // Calculate CRC based on loaded values
    byte calculatedCRC = calculateCRC();
    
    // Verify CRC
    if (storedCRC == calculatedCRC) {
      Serial.println("Settings loaded from EEPROM (CRC valid):");
    } else {
      Serial.println("WARNING: CRC mismatch, possible EEPROM corruption!");
      Serial.println("Stored CRC: " + String(storedCRC) + ", Calculated: " + String(calculatedCRC));
      // Continue using the loaded values, but warn the user
    }
    
    Serial.println("Tank Height: " + String(tankHeight) + " cm");
    Serial.println("Tank Diameter: " + String(tankDiameter) + " cm");
    Serial.println("Tank Volume: " + String(tankVolume) + " L");
    Serial.println("Sensor Offset: " + String(sensorOffset) + " cm");
    Serial.println("Empty Distance: " + String(emptyDistance) + " cm");
    Serial.println("Full Distance: " + String(fullDistance) + " cm");
    Serial.println("Measurement Interval: " + String(measurementInterval) + " s");
    Serial.println("Reading Smoothing: " + String(readingSmoothing));
    Serial.println("Alert Level Low: " + String(alertLevelLow) + "%");
    Serial.println("Alert Level High: " + String(alertLevelHigh) + "%");
    Serial.println("Alerts Enabled: " + String(alertsEnabled ? "Yes" : "No"));
  } else {
    // EEPROM hasn't been initialized, set defaults
    tankHeight = DEFAULT_TANK_HEIGHT;
    tankDiameter = DEFAULT_TANK_DIAMETER;
    tankVolume = DEFAULT_TANK_VOLUME;
    sensorOffset = DEFAULT_SENSOR_OFFSET;
    emptyDistance = DEFAULT_EMPTY_DISTANCE;
    fullDistance = DEFAULT_FULL_DISTANCE;
    measurementInterval = DEFAULT_MEASUREMENT_INTERVAL;
    readingSmoothing = DEFAULT_READING_SMOOTHING;
    alertLevelLow = DEFAULT_ALERT_LEVEL_LOW;
    alertLevelHigh = DEFAULT_ALERT_LEVEL_HIGH;
    alertsEnabled = DEFAULT_ALERTS_ENABLED;
    
    Serial.println("Using default settings (EEPROM not initialized or corrupted)");
    Serial.println("EEPROM marker: " + String(initialized) + " (expected: " + String(EEPROM_INITIALIZED_MARKER) + ")");
    
    // Save defaults to EEPROM for future use
    saveSettings();
  }
  
  // Validate values to prevent issues
  if (tankHeight <= 0 || tankHeight > 1000) tankHeight = DEFAULT_TANK_HEIGHT;
  if (tankDiameter <= 0 || tankDiameter > 1000) tankDiameter = DEFAULT_TANK_DIAMETER;
  if (tankVolume <= 0 || tankVolume > 100000) tankVolume = DEFAULT_TANK_VOLUME;
  if (sensorOffset < 0 || sensorOffset > 100) sensorOffset = DEFAULT_SENSOR_OFFSET;
  if (emptyDistance <= 0 || emptyDistance > 500) emptyDistance = DEFAULT_EMPTY_DISTANCE;
  if (fullDistance < 0 || fullDistance > emptyDistance) fullDistance = DEFAULT_FULL_DISTANCE;
  if (measurementInterval < 1 || measurementInterval > 3600) measurementInterval = DEFAULT_MEASUREMENT_INTERVAL;
  if (readingSmoothing < 1 || readingSmoothing > 50) readingSmoothing = DEFAULT_READING_SMOOTHING;
  if (alertLevelLow < 0 || alertLevelLow > 100) alertLevelLow = DEFAULT_ALERT_LEVEL_LOW;
  if (alertLevelHigh < 0 || alertLevelHigh > 100) alertLevelHigh = DEFAULT_ALERT_LEVEL_HIGH;
}