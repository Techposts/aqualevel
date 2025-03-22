
#include <Arduino.h>
#include "config.h"
#include "tank_calculator.h"

// Variables to track alert states to avoid repeated alerts
bool lowAlertActive = false;
bool highAlertActive = false;

// Function to send alert (can be expanded to support additional notification methods)
void sendAlert(const char* alertType, float level) {
  // Log to serial console
  Serial.print("ALERT: ");
  Serial.print(alertType);
  Serial.print(" water level! Current: ");
  Serial.print(level);
  Serial.println("%");
  
  // Add additional alert methods here:
  // - For example, trigger a GPIO pin for an external buzzer or LED
  // - Or implement a push notification system via MQTT or similar
}

void setupTankCalculator() {
  Serial.println("Initializing tank calculator...");
  
  // Validate tank geometry for volume calculations
  if (tankHeight <= 0 || tankDiameter <= 0) {
    Serial.println("WARNING: Invalid tank dimensions. Using defaults.");
    tankHeight = DEFAULT_TANK_HEIGHT;
    tankDiameter = DEFAULT_TANK_DIAMETER;
  }
  
  // Calculate tank volume if needed (cylindrical tank formula: V = π * r² * h)
  float calculatedVolume = PI * pow(tankDiameter / 2, 2) * tankHeight / 1000; // Volume in liters
  
  // If user-set volume is very different from calculated volume, warn but respect user's value
  if (abs(tankVolume - calculatedVolume) > calculatedVolume * 0.2) { // If difference is more than 20%
    Serial.println("WARNING: User-specified volume (" + String(tankVolume) + 
                  " L) differs significantly from calculated volume (" + 
                  String(calculatedVolume) + " L)");
  }
  
  Serial.println("Tank calculator initialized with dimensions:");
  Serial.println("Height: " + String(tankHeight) + " cm");
  Serial.println("Diameter: " + String(tankDiameter) + " cm");
  Serial.println("Volume: " + String(tankVolume) + " L");
  Serial.println("Empty distance: " + String(emptyDistance) + " cm");
  Serial.println("Full distance: " + String(fullDistance) + " cm");
  
  // Initialize alert states
  lowAlertActive = false;
  highAlertActive = false;
}

void calculateWaterLevel() {
  // Skip calculation if distance reading is invalid
  if (currentDistance <= 0) {
    return;
  }
  
  // Calculate water level based on distance
  // When the sensor reads emptyDistance, the tank is empty (0%)
  // When the sensor reads fullDistance, the tank is full (100%)
  
  // Check if the distance reading is within the expected range
  if (currentDistance > emptyDistance) {
    // Reading is greater than max empty distance, cap at empty
    currentWaterLevel = 0;
    currentPercentage = 0;
    currentVolume = 0;
  } else if (currentDistance < fullDistance) {
    // Reading is less than min full distance, cap at full
    currentWaterLevel = tankHeight;
    currentPercentage = 100;
    currentVolume = tankVolume;
  } else {
    // Normal calculation in the valid range
    float distanceRange = emptyDistance - fullDistance;
    
    // Calculate percentage full (0-100%)
    currentPercentage = ((emptyDistance - currentDistance) / distanceRange) * 100.0;
    
    // Calculate water level (in cm)
    currentWaterLevel = (currentPercentage / 100.0) * tankHeight;
    
    // Calculate volume (in liters)
    currentVolume = (currentPercentage / 100.0) * tankVolume;
  }
  
  // Round values for display
  currentPercentage = round(currentPercentage * 10) / 10.0; // One decimal place
  currentWaterLevel = round(currentWaterLevel * 10) / 10.0; // One decimal place
  currentVolume = round(currentVolume * 10) / 10.0;         // One decimal place
  
  // Debug output
  Serial.print("Water level: ");
  Serial.print(currentWaterLevel);
  Serial.print(" cm (");
  Serial.print(currentPercentage);
  Serial.print("%), Volume: ");
  Serial.print(currentVolume);
  Serial.println(" L");
  
  // Enhanced alert processing
  if (alertsEnabled) {
    // Low water alert
    if (currentPercentage <= alertLevelLow) {
      if (!lowAlertActive) {
        sendAlert("LOW", currentPercentage);
        lowAlertActive = true;
      }
    } else {
      lowAlertActive = false;
    }
    
    // High water alert
    if (currentPercentage >= alertLevelHigh) {
      if (!highAlertActive) {
        sendAlert("HIGH", currentPercentage);
        highAlertActive = true;
      }
    } else {
      highAlertActive = false;
    }
  } else {
    // Reset alert states when alerts are disabled
    lowAlertActive = false;
    highAlertActive = false;
  }
}