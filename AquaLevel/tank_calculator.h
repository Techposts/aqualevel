
// tank_calculator.h
#ifndef TANK_CALCULATOR_H
#define TANK_CALCULATOR_H

/**
 * Initialize the tank calculator
 */
void setupTankCalculator();

/**
 * Calculate water level based on current distance reading
 * Updates currentWaterLevel, currentPercentage, and currentVolume globals
 */
void calculateWaterLevel();

/**
 * Send an alert notification
 * @param alertType Type of alert ("LOW" or "HIGH")
 * @param level Current water level percentage
 */
void sendAlert(const char* alertType, float level);

#endif // TANK_CALCULATOR_H