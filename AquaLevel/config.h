#ifndef CONFIG_H
#define CONFIG_H

/*
 * WaterTank Monitor - ESP32 + HC-SR04 Ultrasonic Sensor
 * Adapted from AmbiSense project
 */

// 🛠 Sensor Config
#define TRIGGER_PIN 1    // HC-SR04 Trigger pin
#define ECHO_PIN 3    // HC-SR04 Echo pin
#define EEPROM_INITIALIZED_MARKER 123
#define EEPROM_SIZE 512  // Space for settings

// Tank default parameters (cm for dimensions)
#define DEFAULT_TANK_HEIGHT 100         // cm
#define DEFAULT_TANK_DIAMETER 50        // cm
#define DEFAULT_TANK_VOLUME 200         // liters
#define DEFAULT_SENSOR_OFFSET 5         // cm (distance from sensor to max water level)
#define DEFAULT_EMPTY_DISTANCE 95       // cm (distance from sensor to bottom when empty)
#define DEFAULT_FULL_DISTANCE 5         // cm (distance from sensor to water when full)
#define DEFAULT_MEASUREMENT_INTERVAL 5  // seconds between measurements
#define DEFAULT_READING_SMOOTHING 5     // number of readings to average
#define DEFAULT_ALERT_LEVEL_LOW 10      // percentage for low water alert
#define DEFAULT_ALERT_LEVEL_HIGH 90     // percentage for high water alert
#define DEFAULT_ALERTS_ENABLED true     // enable/disable alerts

// 📡 Wi-Fi Access Point
#define WIFI_AP_SSID "Aqualevel"
#define WIFI_AP_PASSWORD "12345678"

// 📡 Web Server
#define WEB_SERVER_PORT 80

// Global variables for tank parameters
extern float tankHeight;         // Height of water tank in cm
extern float tankDiameter;       // Diameter of cylindrical tank in cm
extern float tankVolume;         // Max volume in liters
extern float sensorOffset;       // Distance from sensor to max water level
extern float emptyDistance;      // Distance reading when tank is empty
extern float fullDistance;       // Distance reading when tank is full
extern int measurementInterval;  // Time between measurements in seconds
extern int readingSmoothing;     // Number of readings to average
extern int alertLevelLow;        // Low water alert percentage
extern int alertLevelHigh;       // High water alert percentage
extern bool alertsEnabled;       // Enable/disable alerts

// Global variables for current readings
extern float currentDistance;    // Current distance reading from sensor
extern float currentWaterLevel;  // Current water level in cm
extern float currentPercentage;  // Current water percentage (0-100)
extern float currentVolume;      // Current water volume in liters

// EEPROM memory layout
#define EEPROM_SYSTEM_START     0
#define EEPROM_ADDR_MARKER      (EEPROM_SYSTEM_START + 0)
#define EEPROM_ADDR_TANK_HEIGHT_L (EEPROM_SYSTEM_START + 1)
#define EEPROM_ADDR_TANK_HEIGHT_H (EEPROM_SYSTEM_START + 2)
#define EEPROM_ADDR_TANK_DIAMETER_L (EEPROM_SYSTEM_START + 3)
#define EEPROM_ADDR_TANK_DIAMETER_H (EEPROM_SYSTEM_START + 4)
#define EEPROM_ADDR_TANK_VOLUME_L (EEPROM_SYSTEM_START + 5)
#define EEPROM_ADDR_TANK_VOLUME_H (EEPROM_SYSTEM_START + 6)
#define EEPROM_ADDR_SENSOR_OFFSET_L (EEPROM_SYSTEM_START + 7)
#define EEPROM_ADDR_SENSOR_OFFSET_H (EEPROM_SYSTEM_START + 8)
#define EEPROM_ADDR_EMPTY_DISTANCE_L (EEPROM_SYSTEM_START + 9)
#define EEPROM_ADDR_EMPTY_DISTANCE_H (EEPROM_SYSTEM_START + 10)
#define EEPROM_ADDR_FULL_DISTANCE_L (EEPROM_SYSTEM_START + 11)
#define EEPROM_ADDR_FULL_DISTANCE_H (EEPROM_SYSTEM_START + 12)
#define EEPROM_ADDR_MEASUREMENT_INTERVAL (EEPROM_SYSTEM_START + 13)
#define EEPROM_ADDR_READING_SMOOTHING (EEPROM_SYSTEM_START + 14)
#define EEPROM_ADDR_ALERT_LEVEL_LOW (EEPROM_SYSTEM_START + 15)
#define EEPROM_ADDR_ALERT_LEVEL_HIGH (EEPROM_SYSTEM_START + 16)
#define EEPROM_ADDR_ALERTS_ENABLED (EEPROM_SYSTEM_START + 17)
#define EEPROM_ADDR_CRC (EEPROM_SYSTEM_START + 18)

// WiFi credentials section (100-299) - using the same layout as original project
#define EEPROM_WIFI_START        100
#define EEPROM_WIFI_SSID_ADDR    (EEPROM_WIFI_START)
#define EEPROM_WIFI_PASS_ADDR    (EEPROM_WIFI_SSID_ADDR + MAX_SSID_LENGTH)
#define EEPROM_DEVICE_NAME_ADDR  (EEPROM_WIFI_PASS_ADDR + MAX_PASSWORD_LENGTH)
#define EEPROM_WIFI_MODE_ADDR    (EEPROM_DEVICE_NAME_ADDR + MAX_DEVICE_NAME_LENGTH)

#endif // CONFIG_H