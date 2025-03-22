// web_interface.h
#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H
#include <ESPmDNS.h>

/**
 * Initialize the web server
 */
void setupWebServer();

/**
 * Process web server requests
 * Should be called regularly in the main loop
 */
void handleWebServer();

/**
 * Handle the root page
 */
void handleRoot();

/**
 * Handle setting updates
 */
void handleSet();

/**
 * Handle tank data API endpoint
 */
void handleTankData();

/**
 * Handle settings API endpoint
 */
void handleSettings();

/**
 * Handle settings page
 */
void handleSettingsPage();

/**
 * Handle calibration settings
 */
void handleCalibrate();

/**
 * Handle network settings page
 */
void handleNetworkSettings();

/**
 * Handle network scan API
 */
void handleScanNetworks();

/**
 * Handle WiFi reset
 */
void handleResetWifi();

#endif // WEB_INTERFACE_H