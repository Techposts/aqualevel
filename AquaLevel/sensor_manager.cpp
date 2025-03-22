
#include <Arduino.h>
#include "config.h"
#include "sensor_manager.h"

// Buffer for smoothing sensor readings - changed from fixed array to dynamic
float* distanceReadings = NULL;
int readingIndex = 0;
int currentSmoothingSize = 0;

void setupSensor() {
  Serial.println("Initializing ultrasonic sensor...");
  
  // Set pin modes for HC-SR04
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Initialize readings buffer with current smoothing value
  updateSmoothingBuffer();
  
  Serial.print("Ultrasonic sensor initialized on pins Trigger:");
  Serial.print(TRIGGER_PIN);
  Serial.print(", Echo:");
  Serial.println(ECHO_PIN);
  Serial.print("Smoothing level: ");
  Serial.println(readingSmoothing);
}

// New function to update smoothing buffer when setting changes
void updateSmoothingBuffer() {
  // Only reallocate if the size has changed
  if (currentSmoothingSize != readingSmoothing) {
    // Free previous buffer if it exists
    if (distanceReadings != NULL) {
      free(distanceReadings);
    }
    
    // Allocate new buffer of the correct size
    distanceReadings = (float*)malloc(readingSmoothing * sizeof(float));
    currentSmoothingSize = readingSmoothing;
    
    // Reset index and initialize buffer with zeros
    readingIndex = 0;
    for (int i = 0; i < readingSmoothing; i++) {
      distanceReadings[i] = 0;
    }
    
    Serial.print("Smoothing buffer updated to size: ");
    Serial.println(readingSmoothing);
  }
}

float getSingleReading() {
  // Clear trigger pin
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  
  // Send 10µs pulse to trigger
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  // Read echo pin (pulse duration in microseconds)
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout after 30ms
  
  // Calculate distance in centimeters
  // Speed of sound = 343 m/s = 0.0343 cm/µs
  // Distance = (duration x 0.0343) / 2 (divide by 2 for round trip)
  float distance = (duration * 0.0343) / 2.0;
  
  // Validate reading - HC-SR04 typically measures 2cm to 400cm
  if (distance <= 0 || distance > 400) {
    Serial.println("Invalid distance reading: " + String(distance) + " cm");
    return -1; // Invalid reading
  }
  
  return distance;
}

void readSensorDistance() {
  // Check if smoothing size has changed and update buffer if needed
  if (currentSmoothingSize != readingSmoothing) {
    updateSmoothingBuffer();
  }
  
  // Take multiple readings and average them
  float validReadings = 0;
  float totalDistance = 0;
  
  // Take 3 readings and use the median (to filter out anomalies)
  float readings[3];
  
  for (int i = 0; i < 3; i++) {
    readings[i] = getSingleReading();
    delay(10); // Small delay between readings
  }
  
  // Sort readings
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2 - i; j++) {
      if (readings[j] > readings[j + 1]) {
        float temp = readings[j];
        readings[j] = readings[j + 1];
        readings[j + 1] = temp;
      }
    }
  }
  
  // Use median reading (middle value)
  float medianReading = readings[1];
  
  // Check if median reading is valid
  if (medianReading > 0) {
    // Add to smoothing buffer
    distanceReadings[readingIndex] = medianReading;
    readingIndex = (readingIndex + 1) % readingSmoothing;
    
    // Calculate smoothed average
    totalDistance = 0;
    validReadings = 0;
    
    for (int i = 0; i < readingSmoothing; i++) {
      if (distanceReadings[i] > 0) {
        totalDistance += distanceReadings[i];
        validReadings++;
      }
    }
    
    if (validReadings > 0) {
      float smoothedDistance = totalDistance / validReadings;
      
      // Update global current distance
      currentDistance = smoothedDistance;
      
      // Debug output
      Serial.print("Distance: ");
      Serial.print(smoothedDistance);
      Serial.println(" cm");
    }
  } else {
    Serial.println("Failed to get valid reading");
  }
}