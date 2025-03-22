
// sensor_manager.h
#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

/**
 * Initialize the ultrasonic sensor
 */
void setupSensor();

/**
 * Updates the smoothing buffer when the reading smoothing value changes
 */
void updateSmoothingBuffer();

/**
 * Take a single distance reading
 * @return Distance in centimeters, or -1 if invalid reading
 */
float getSingleReading();

/**
 * Read sensor and update the global currentDistance variable
 * Uses smoothing and filtering to improve accuracy
 */
void readSensorDistance();

#endif // SENSOR_MANAGER_H