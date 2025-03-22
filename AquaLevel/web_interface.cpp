#include <WebServer.h>
#include "config.h"
#include "web_interface.h"
#include "eeprom_manager.h"
#include <ESPmDNS.h>
#include "wifi_manager.h"
#include "tank_calculator.h"
#include "sensor_manager.h" 


WebServer server(WEB_SERVER_PORT);

// Forward declarations of handler functions
void handleRoot();
void handleSet();
void handleTankData();
void handleSettings();
void handleCalibrate();
void handleNetworkSettings();
void handleResetWifi();
void handleScanNetworks();
void handleSettingsPage();

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/tank-data", handleTankData);
  server.on("/settings", handleSettings);
  server.on("/calibrate", handleCalibrate);
  server.on("/network", handleNetworkSettings);
  server.on("/resetwifi", handleResetWifi);
  server.on("/scannetworks", handleScanNetworks);
  server.on("/settings.html", handleSettingsPage);
  

  
  server.begin();
  
  Serial.println("Web server started on port " + String(WEB_SERVER_PORT));
}

void handleWebServer() {
  server.handleClient();
}

// Handle scan networks API
void handleScanNetworks() {
  std::vector<WiFiNetwork> networks = wifiManager.scanNetworks();
  
  String json = "[";
  
  if (!networks.empty()) {
    // Limit to top 5 networks with strongest signal
    int count = min(5, (int)networks.size());
    
    for (int i = 0; i < count; i++) {
      if (i > 0) json += ",";
      
      // Encode the SSID for JSON (escaping quotes and backslashes)
      String escapedSSID = networks[i].ssid;
      escapedSSID.replace("\\", "\\\\");  // Escape backslashes first
      escapedSSID.replace("\"", "\\\"");  // Then escape quotes
      
      json += "{\"ssid\":\"" + escapedSSID + "\",";
      json += "\"rssi\":" + String(networks[i].rssi) + ",";
      json += "\"secure\":" + String(networks[i].encType != WIFI_AUTH_OPEN ? "true" : "false") + "}";
    }
  }
  
  json += "]";
  
  // Send response with appropriate headers
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "application/json", json);
}

// Handle tank data API (returns real-time tank data)
void handleTankData() {
  String json = "{";
  json += "\"distance\":" + String(currentDistance, 1) + ",";
  json += "\"waterLevel\":" + String(currentWaterLevel, 1) + ",";
  json += "\"percentage\":" + String(currentPercentage, 1) + ",";
  json += "\"volume\":" + String(currentVolume, 1) + ",";
  json += "\"tankHeight\":" + String(tankHeight, 1) + ",";
  json += "\"tankDiameter\":" + String(tankDiameter, 1) + ",";
  json += "\"tankVolume\":" + String(tankVolume, 1) + ",";
  json += "\"alertLevelLow\":" + String(alertLevelLow) + ",";
  json += "\"alertLevelHigh\":" + String(alertLevelHigh) + ",";
  json += "\"alertsEnabled\":" + String(alertsEnabled ? "true" : "false");
  json += "}";
  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "application/json", json);
}

// Handle Settings API (returns current settings as JSON)
void handleSettings() {
  String json = "{";
  json += "\"tankHeight\":" + String(tankHeight, 1) + ",";
  json += "\"tankDiameter\":" + String(tankDiameter, 1) + ",";
  json += "\"tankVolume\":" + String(tankVolume, 1) + ",";
  json += "\"sensorOffset\":" + String(sensorOffset, 1) + ",";
  json += "\"emptyDistance\":" + String(emptyDistance, 1) + ",";
  json += "\"fullDistance\":" + String(fullDistance, 1) + ",";
  json += "\"measurementInterval\":" + String(measurementInterval) + ",";
  json += "\"readingSmoothing\":" + String(readingSmoothing) + ",";
  json += "\"alertLevelLow\":" + String(alertLevelLow) + ",";
  json += "\"alertLevelHigh\":" + String(alertLevelHigh) + ",";
  json += "\"alertsEnabled\":" + String(alertsEnabled ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

// Handle Calibration
void handleCalibrate() {
  if (server.hasArg("type")) {
    String calibrationType = server.arg("type");
    
    if (calibrationType == "empty") {
      // Store current distance as empty reading
      emptyDistance = currentDistance;
      saveSettings();
      server.send(200, "text/plain", "Empty calibration saved: " + String(emptyDistance, 1) + " cm");
    } 
    else if (calibrationType == "full") {
      // Store current distance as full reading
      fullDistance = currentDistance;
      saveSettings();
      server.send(200, "text/plain", "Full calibration saved: " + String(fullDistance, 1) + " cm");
    }
    else {
      server.send(400, "text/plain", "Invalid calibration type");
    }
  } else {
    server.send(400, "text/plain", "Missing calibration type");
  }
}

// Handle Settings Update
void handleSet() {
  bool settingsChanged = false;
  
  // Tank Height
  if (server.hasArg("tankHeight")) {
    float newTankHeight = server.arg("tankHeight").toFloat();
    if (newTankHeight > 0 && newTankHeight <= 1000) {
      tankHeight = newTankHeight;
      settingsChanged = true;
    }
  }
  
  // Tank Diameter
  if (server.hasArg("tankDiameter")) {
    float newTankDiameter = server.arg("tankDiameter").toFloat();
    if (newTankDiameter > 0 && newTankDiameter <= 1000) {
      tankDiameter = newTankDiameter;
      settingsChanged = true;
    }
  }
  
  // Tank Volume
  if (server.hasArg("tankVolume")) {
    float newTankVolume = server.arg("tankVolume").toFloat();
    if (newTankVolume > 0 && newTankVolume <= 100000) {
      tankVolume = newTankVolume;
      settingsChanged = true;
    }
  }
  
  // Sensor Offset
  if (server.hasArg("sensorOffset")) {
    float newSensorOffset = server.arg("sensorOffset").toFloat();
    if (newSensorOffset >= 0 && newSensorOffset <= 100) {
      sensorOffset = newSensorOffset;
      settingsChanged = true;
    }
  }
  
  // Empty Distance
  if (server.hasArg("emptyDistance")) {
    float newEmptyDistance = server.arg("emptyDistance").toFloat();
    if (newEmptyDistance > 0 && newEmptyDistance <= 500) {
      emptyDistance = newEmptyDistance;
      settingsChanged = true;
    }
  }
  
  // Full Distance
  if (server.hasArg("fullDistance")) {
    float newFullDistance = server.arg("fullDistance").toFloat();
    if (newFullDistance >= 0 && newFullDistance < emptyDistance) {
      fullDistance = newFullDistance;
      settingsChanged = true;
    }
  }
  
  // Measurement Interval
  if (server.hasArg("measurementInterval")) {
    int newMeasurementInterval = server.arg("measurementInterval").toInt();
    if (newMeasurementInterval >= 1 && newMeasurementInterval <= 3600) {
      measurementInterval = newMeasurementInterval;
      settingsChanged = true;
    }
  }
  
  // Reading Smoothing
  if (server.hasArg("readingSmoothing")) {
    int newReadingSmoothing = server.arg("readingSmoothing").toInt();
    if (newReadingSmoothing >= 1 && newReadingSmoothing <= 50) {
      readingSmoothing = newReadingSmoothing;
      settingsChanged = true;
    }
  }
  if (settingsChanged && server.hasArg("readingSmoothing")) {
    updateSmoothingBuffer();
  }
  // Alert Level Low
  if (server.hasArg("alertLevelLow")) {
    int newAlertLevelLow = server.arg("alertLevelLow").toInt();
    if (newAlertLevelLow >= 0 && newAlertLevelLow <= 100) {
      alertLevelLow = newAlertLevelLow;
      settingsChanged = true;
    }
  }
  
  // Alert Level High
  if (server.hasArg("alertLevelHigh")) {
    int newAlertLevelHigh = server.arg("alertLevelHigh").toInt();
    if (newAlertLevelHigh >= 0 && newAlertLevelHigh <= 100) {
      alertLevelHigh = newAlertLevelHigh;
      settingsChanged = true;
    }
  }
  
  // Alerts Enabled
  if (server.hasArg("alertsEnabled")) {
    String alertsEnabledStr = server.arg("alertsEnabled");
    alertsEnabled = (alertsEnabledStr == "true" || alertsEnabledStr == "1");
    settingsChanged = true;
  }
  
  // Save settings if any changed
  if (settingsChanged) {
    saveSettings();
    
    // Recalculate water level with new settings
    calculateWaterLevel();
    
    server.send(200, "text/html", "<h3>Settings Updated! <a href='/'>Back</a></h3>");
  } else {
    server.send(200, "text/html", "<h3>No settings were changed. <a href='/'>Back</a></h3>");
  }
}



// Handle Wifi Reset
void handleResetWifi() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Reset WiFi - AquaLevel</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial; margin: 0; padding: 20px; text-align: center; }
        .container { max-width: 500px; margin: 0 auto; }
        h1 { color: #ff4d4d; }
        .info { background-color: #f0f0f0; padding: 15px; border-radius: 5px; margin: 20px 0; }
        .countdown { font-size: 24px; font-weight: bold; margin: 20px 0; color: #ff4d4d; }
      </style>
      <script>
        var countdown = 5;
        function updateCountdown() {
          document.getElementById('timer').innerText = countdown;
          countdown--;
          if (countdown < 0) {
            window.location.href = '/';
          } else {
            setTimeout(updateCountdown, 1000);
          }
        }
        window.onload = function() {
          updateCountdown();
        };
      </script>
    </head>
    <body>
      <div class="container">
        <h1>Network Settings Reset</h1>
        <div class="info">
          <p>All network settings have been reset.</p>
          <p>The device will restart in AP mode.</p>
          <p>Restarting in <span id="timer">5</span> seconds...</p>
        </div>
      </div>
    </body>
    </html>
  )rawliteral";
  
  server.send(200, "text/html", html);
  
  // Reset WiFi settings
  delay(1000);
  wifiManager.resetWifiSettings();
  // resetWifiSettings() will restart the device
}


void handleNetworkSettings() {
  if (server.hasArg("ssid") && server.hasArg("password") && server.hasArg("deviceName")) {
    // Get form data
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String deviceName = server.arg("deviceName");
    
    // Validate inputs
    if (ssid.length() == 0 || deviceName.length() == 0) {
      server.send(400, "text/html", "<h1>Error</h1><p>SSID and Device Name cannot be empty</p><a href='/network'>Back</a>");
      return;
    }
    
    // Save credentials to EEPROM
    wifiManager.saveWifiCredentials(ssid.c_str(), password.c_str(), deviceName.c_str());
    
    // Format hostname from the device name
    String hostname = wifiManager.getSanitizedHostname(deviceName.c_str());

    String html = "<!DOCTYPE html><html><head><title>Network Settings - Aqualevel</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:0;padding:20px;text-align:center;}";
    html += ".container{max-width:500px;margin:0 auto;}h1{color:#4361ee;}";
    html += ".info{background-color:#f0f0f0;padding:15px;border-radius:5px;margin:20px 0;}";
    html += ".countdown{font-size:24px;font-weight:bold;margin:20px 0;color:#4361ee;}</style>";
    html += "<script>var countdown=10;function updateCountdown(){";
    html += "document.getElementById('timer').innerText=countdown;countdown--;";
    html += "if(countdown<0){window.location.href='http://" + hostname + ".local';}";
    html += "else{setTimeout(updateCountdown,1000);}}";
    html += "window.onload=function(){updateCountdown();};</script></head>";
    html += "<body><div class='container'><h1>Network Settings Updated</h1>";
    html += "<div class='info'><p>The device will now restart and attempt to connect to your WiFi network.</p>";
    html += "<p>If successful, you'll be able to access it at:<br><strong>http://" + hostname + ".local</strong></p>";
    html += "<p>Device restarting in <span id='timer'>10</span> seconds...</p></div>";
    html += "<div id='countdown-info' class='countdown'></div></div></body></html>";
          
    server.send(200, "text/html", html);
    
    // Schedule restart
    delay(1000);
    ESP.restart();  
  } else {
    // Display network settings form
    // Determine current mode text
    String modeText;
    if (wifiManager.getMode() == WIFI_MANAGER_MODE_AP) {
      modeText = "Access Point";
    } else if (wifiManager.getMode() == WIFI_MANAGER_MODE_STA) {
      modeText = "Connected to WiFi";
    } else {
      modeText = "Fallback AP Mode";
    }

    // Get the IP address from the WiFi manager
    String ipAddress = wifiManager.getIPAddress();

    // Create the network settings form HTML
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <title>Network Settings - Aqualevel</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    :root {
      --primary: #2196F3;
      --primary-light: #64B5F6;
      --primary-dark: #1976D2;
      --secondary: #03A9F4;
      --accent: #00BCD4;
      --success: #4CAF50;
      --warning: #FFC107;
      --danger: #F44336;
      --bg-dark: #121212;
      --bg-card: #1e1e1e;
      --text: #ffffff;
      --text-secondary: #b0b0b0;
      --border-radius: 12px;
      --shadow: 0 10px 20px rgba(0,0,0,0.3);
      --transition: all 0.3s ease;
    }
    
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: var(--bg-dark);
      color: var(--text);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      padding: 20px;
      background: linear-gradient(135deg, #121212 0%, #2a2a2a 100%);
    }
    
    .container {
      width: 100%;
      max-width: 600px;
      margin: 0 auto;
      display: flex;
      flex-direction: column;
      gap: 20px;
    }
    
    .card {
      background: var(--bg-card);
      border-radius: var(--border-radius);
      box-shadow: var(--shadow);
      overflow: hidden;
    }
    
    .card-header {
      background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 100%);
      padding: 20px;
      text-align: center;
    }
    
    .card-header h1 {
      font-size: 24px;
      font-weight: 600;
      margin-bottom: 8px;
      text-shadow: 0 2px 5px rgba(0,0,0,0.2);
    }
    
    .card-header p {
      opacity: 0.9;
      font-size: 14px;
    }
    
    .card-content {
      padding: 20px;
    }
    
    .status-container {
      background: #1a1a1a;
      border-radius: var(--border-radius);
      padding: 15px;
      margin-bottom: 20px;
    }
    
    .status-item {
      display: flex;
      justify-content: space-between;
      margin-bottom: 10px;
      font-size: 14px;
    }
    
    .status-item:last-child {
      margin-bottom: 0;
    }
    
    .status-label {
      color: var(--text-secondary);
    }
    
    .status-value {
      font-weight: 500;
    }
    
    .form-row {
      margin-bottom: 20px;
    }
    
    .form-label {
      display: block;
      margin-bottom: 8px;
      font-size: 14px;
      color: var(--text-secondary);
    }
    
    .form-input {
      width: 100%;
      padding: 12px;
      background: #2a2a2a;
      border: 1px solid #444;
      border-radius: var(--border-radius);
      color: var(--text);
      font-size: 16px;
    }
    
    .button {
      display: block;
      width: 100%;
      padding: 15px;
      background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 100%);
      color: white;
      border: none;
      border-radius: var(--border-radius);
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: var(--transition);
      text-transform: uppercase;
      letter-spacing: 1px;
      text-align: center;
      text-decoration: none;
      margin-top: 20px;
    }
    
    .button:hover {
      transform: translateY(-3px);
      box-shadow: 0 8px 20px rgba(33, 150, 243, 0.4);
    }
    
    .button-danger {
      background: linear-gradient(135deg, var(--danger) 0%, #FF7043 100%);
    }
    
    .footer {
      text-align: center;
      padding: 15px 0;
      margin-top: 30px;
      font-size: 12px;
      color: var(--text-secondary);
    }
    
    a.button-back {
      display: inline-block;
      padding: 10px 20px;
      margin-bottom: 20px;
      background: #333;
      color: white;
      text-decoration: none;
      border-radius: var(--border-radius);
      font-size: 14px;
    }
    
    a.button-back:hover {
      background: #444;
    }
    
    .network-list {
      margin-bottom: 20px;
    }
    
    .network-item {
      display: flex;
      align-items: center;
      padding: 12px;
      background: #1a1a1a;
      border-radius: var(--border-radius);
      margin-bottom: 8px;
      cursor: pointer;
      transition: var(--transition);
    }
    
    .network-item:hover {
      background: #252525;
    }
    
    .network-item.selected {
      background: #253a4f;
      border: 1px solid var(--primary);
    }
    
    .network-info {
      flex: 1;
    }
    
    .network-name {
      font-size: 15px;
      font-weight: 500;
    }
    
    .network-details {
      display: flex;
      align-items: center;
      font-size: 12px;
      color: var(--text-secondary);
      margin-top: 3px;
    }
    
    .signal-strength {
      display: flex;
      margin-right: 10px;
    }
    
    .signal-bar {
      width: 3px;
      margin-right: 2px;
      background: var(--primary-light);
      border-radius: 1px;
    }
    
    .signal-bar.active {
      opacity: 1;
    }
    
    .signal-bar.inactive {
      opacity: 0.3;
    }
    
    .security-icon {
      margin-left: auto;
      width: 16px;
      height: 16px;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    
    .spinner {
      width: 20px;
      height: 20px;
      border: 2px solid rgba(33, 150, 243, 0.3);
      border-radius: 50%;
      border-top-color: var(--primary);
      animation: spin 1s linear infinite;
      margin-right: 10px;
    }
    
    @keyframes spin {
      to { transform: rotate(360deg); }
    }
  </style>
</head>
<body>
  <div class="container">
    <a href="/" class="button-back">&larr;Back to Dashboard</a>
    
    <div class="card">
      <div class="card-header">
        <h1>Network Settings</h1>
        <p>Configure WiFi connection</p>
      </div>
      
      <div class="card-content">
        <div class="status-container">
          <div class="status-item">
            <span class="status-label">Current Mode:</span>
            <span class="status-value" id="currentMode">)rawliteral";
            
    html += modeText;
    
    html += R"rawliteral(</span>
          </div>
          <div class="status-item">
            <span class="status-label">IP Address:</span>
            <span class="status-value" id="ipAddress">)rawliteral";
            
    html += ipAddress;
    
    html += R"rawliteral(</span>
          </div>
        </div>
        
        <div class="network-section">
          <h3>Available Networks</h3>
          <div class="network-list" id="networkList">
            <div style="display:flex; align-items:center; justify-content:center; padding:20px;">
              <div class="spinner"></div>
              <span>Scanning for networks...</span>
            </div>
          </div>
        </div>
        
        <form action="/network" method="GET" id="wifiForm">
          <div class="form-row">
            <label for="ssid" class="form-label">WiFi Network Name (SSID)</label>
            <input type="text" id="ssid" name="ssid" class="form-input" required>
          </div>
          
          <div class="form-row">
            <label for="password" class="form-label">WiFi Password</label>
            <input type="password" id="password" name="password" class="form-input">
          </div>

          <div class="form-row">
            <label for="deviceName" class="form-label">Device Name</label>
            <input type="text" id="deviceName" name="deviceName" class="form-input" required>
          </div>
          
          <button type="submit" class="button">Save and Connect</button>
        </form>
        
        <a href="/resetwifi" class="button button-danger" onclick="return confirm('Are you sure you want to reset all network settings? The device will restart in AP mode.')">Reset Network Settings</a>
      </div>
    </div>
  </div>
  
    <div class="footer">
      <p>AquaLevel v1.0 | ESP32 + Ultrasonic Sensor | Copyright © 2025</p>
    </div>
  
  <script>
    // Scan for networks when page loads
    document.addEventListener('DOMContentLoaded', function() {
      scanNetworks();
    });
    
    function scanNetworks() {
      const networkList = document.getElementById('networkList');
      
      fetch('/scannetworks')
        .then(response => response.json())
        .then(networks => {
          if (networks.length === 0) {
            networkList.innerHTML = '<p>No networks found. <a href="#" onclick="scanNetworks(); return false;">Scan again</a></p>';
            return;
          }
          
          // Clear current list
          networkList.innerHTML = '';
          
          // Add each network to the list
          networks.forEach(network => {
            const networkItem = document.createElement('div');
            networkItem.className = 'network-item';
            networkItem.onclick = function() {
              selectNetwork(network.ssid);
            };
            
            // Calculate signal strength (0-4 bars)
            const signalStrength = calculateSignalStrength(network.rssi);
            
            networkItem.innerHTML = `
              <div class="network-info">
                <div class="network-name">${network.ssid}</div>
                <div class="network-details">
                  <div class="signal-strength">
                    ${generateSignalBars(signalStrength)}
                  </div>
                  <span>${network.secure ? 'Secure' : 'Open'}</span>
                </div>
              </div>
              <div class="security-icon">
                ${network.secure ? '<span>🔒</span>' : '<span>🔓</span>'}
              </div>
            `;
            
            networkList.appendChild(networkItem);
          });
        })
        .catch(error => {
          console.error('Error scanning networks:', error);
          networkList.innerHTML = '<p>Error scanning networks. <a href="#" onclick="scanNetworks(); return false;">Try again</a></p>';
        });
    }
    
    function calculateSignalStrength(rssi) {
      // RSSI to signal bars conversion (typically -30 to -90 dBm)
      if (rssi >= -50) return 4;      // Excellent
      else if (rssi >= -60) return 3; // Good
      else if (rssi >= -70) return 2; // Fair
      else if (rssi >= -80) return 1; // Poor
      else return 0;                  // Very poor
    }
    
    function generateSignalBars(strength) {
      let bars = '';
      
      for (let i = 0; i < 4; i++) {
        const barHeight = 5 + (i * 3);
        const barClass = i < strength ? 'active' : 'inactive';
        bars += `<div class="signal-bar ${barClass}" style="height: ${barHeight}px"></div>`;
      }
      
      return bars;
    }
    
    function selectNetwork(ssid) {
      // Set the SSID in the form
      document.getElementById('ssid').value = ssid;
      
      // Highlight the selected network
      const networkItems = document.querySelectorAll('.network-item');
      networkItems.forEach(item => {
        const networkName = item.querySelector('.network-name').textContent;
        if (networkName === ssid) {
          item.classList.add('selected');
        } else {
          item.classList.remove('selected');
        }
      });
    }
  </script>
</body>
</html>
    )rawliteral";
    
    server.send(200, "text/html", html);
  }
}

// Render the tank settings page
void handleSettingsPage() {
  const char* html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <title>Tank Settings - AquaLevel Monitor</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    :root {
      --primary: #2196F3;
      --primary-light: #64B5F6;
      --primary-dark: #1976D2;
      --secondary: #03A9F4;
      --accent: #00BCD4;
      --success: #4CAF50;
      --warning: #FFC107;
      --danger: #F44336;
      --bg-dark: #121212;
      --bg-card: #1e1e1e;
      --text: #ffffff;
      --text-secondary: #b0b0b0;
      --border-radius: 12px;
      --shadow: 0 10px 20px rgba(0,0,0,0.3);
      --transition: all 0.3s ease;
    }
    
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: var(--bg-dark);
      color: var(--text);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      padding: 20px;
      background: linear-gradient(135deg, #121212 0%, #2a2a2a 100%);
    }
    
    .container {
      width: 100%;
      max-width: 900px;
      margin: 0 auto;
      display: flex;
      flex-direction: column;
      gap: 20px;
    }
    
    .card {
      background: var(--bg-card);
      border-radius: var(--border-radius);
      box-shadow: var(--shadow);
      overflow: hidden;
    }
    
    .card-header {
      background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 100%);
      padding: 20px;
      text-align: center;
    }
    
    .card-header h1 {
      font-size: 24px;
      font-weight: 600;
      margin-bottom: 8px;
      text-shadow: 0 2px 5px rgba(0,0,0,0.2);
    }
    
    .card-header p {
      opacity: 0.9;
      font-size: 14px;
    }
    
    .card-content {
      padding: 20px;
    }
    
    .tab-container {
      display: flex;
      margin-bottom: 20px;
      border-bottom: 1px solid #333;
    }

    .tab {
      padding: 10px 15px;
      cursor: pointer;
      background-color: transparent;
      border: none;
      color: var(--text-secondary);
      font-size: 14px;
      position: relative;
    }

    .tab.active {
      color: var(--primary);
    }

    .tab.active::after {
      content: '';
      position: absolute;
      bottom: -1px;
      left: 0;
      width: 100%;
      height: 2px;
      background-color: var(--primary);
    }
    
    .form-row {
      margin-bottom: 20px;
    }
    
    .form-label {
      display: block;
      margin-bottom: 8px;
      font-size: 14px;
      color: var(--text-secondary);
    }
    
    .form-input {
      width: 100%;
      padding: 12px;
      background: #2a2a2a;
      border: 1px solid #444;
      border-radius: var(--border-radius);
      color: var(--text);
      font-size: 16px;
    }
    
    .input-group {
      display: flex;
      align-items: center;
    }
    
    .input-group .form-input {
      flex: 1;
      border-top-right-radius: 0;
      border-bottom-right-radius: 0;
    }
    
    .input-group-append {
      background: #444;
      padding: 12px 15px;
      border-top-right-radius: var(--border-radius);
      border-bottom-right-radius: var(--border-radius);
      color: var(--text-secondary);
    }
    
    .toggle-switch {
      position: relative;
      display: inline-block;
      width: 60px;
      height: 34px;
    }

    .toggle-switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    .toggle-slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #333;
      transition: .4s;
      border-radius: 34px;
    }

    .toggle-slider:before {
      position: absolute;
      content: "";
      height: 26px;
      width: 26px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: .4s;
      border-radius: 50%;
    }

    input:checked + .toggle-slider {
      background-color: var(--primary);
    }

    input:checked + .toggle-slider:before {
      transform: translateX(26px);
    }
    
    .toggle-container {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 20px;
    }
    
    .toggle-label {
      font-size: 14px;
      color: var(--text-secondary);
    }
    
    .tab-content {
      display: none;
    }
    
    .tab-content.active {
      display: block;
    }
    
    .footer {
      text-align: center;
      padding: 15px 0;
      margin-top: 30px;
      font-size: 12px;
      color: var(--text-secondary);
    }
    
    .button {
      display: block;
      width: 100%;
      padding: 15px;
      background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 100%);
      color: white;
      border: none;
      border-radius: var(--border-radius);
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: var(--transition);
      text-transform: uppercase;
      letter-spacing: 1px;
      text-align: center;
      text-decoration: none;
      margin-top: 20px;
    }
    
    .button:hover {
      transform: translateY(-3px);
      box-shadow: 0 8px 20px rgba(33, 150, 243, 0.4);
    }
    
    .button-secondary {
      background: linear-gradient(135deg, var(--secondary) 0%, var(--accent) 100%);
    }
    
    .button-success {
      background: linear-gradient(135deg, var(--success) 0%, #81C784 100%);
    }
    
    a.button-back {
      display: inline-block;
      padding: 10px 20px;
      margin-bottom: 20px;
      background: #333;
      color: white;
      text-decoration: none;
      border-radius: var(--border-radius);
      font-size: 14px;
    }
    
    a.button-back:hover {
      background: #444;
    }
  </style>
</head>
<body>
  <div class="container">
    <a href="/" class="button-back">&larr; Back to Dashboard</a>
    
    <div class="card">
      <div class="card-header">
        <h1>Tank Settings</h1>
        <p>Configure your water tank parameters</p>
      </div>
      
      <div class="card-content">
        <div class="tab-container">
          <button class="tab active" data-tab="tankSettings">Tank Dimensions</button>
          <button class="tab" data-tab="alertSettings">Alert Settings</button>
          <button class="tab" data-tab="sensorSettings">Sensor Calibration</button>
        </div>
        
        <!-- Tank Dimensions Settings -->
        <div id="tankSettings" class="tab-content active">
          <form action="/set" method="GET">
            <div class="form-row">
              <label for="tankHeight" class="form-label">Tank Height</label>
              <div class="input-group">
                <input type="number" id="tankHeight" name="tankHeight" class="form-input" step="0.1" min="10" max="1000">
                <div class="input-group-append">cm</div>
              </div>
            </div>
            
            <div class="form-row">
              <label for="tankDiameter" class="form-label">Tank Diameter</label>
              <div class="input-group">
                <input type="number" id="tankDiameter" name="tankDiameter" class="form-input" step="0.1" min="10" max="500">
                <div class="input-group-append">cm</div>
              </div>
            </div>
            
            <div class="form-row">
              <label for="tankVolume" class="form-label">Tank Volume</label>
              <div class="input-group">
                <input type="number" id="tankVolume" name="tankVolume" class="form-input" step="0.1" min="1" max="10000">
                <div class="input-group-append">liters</div>
              </div>
            </div>
            
            <button type="submit" class="button button-success">Save Tank Settings</button>
          </form>
        </div>
        
        <!-- Alert Settings -->
        <div id="alertSettings" class="tab-content">
          <form action="/set" method="GET">
            <div class="form-row">
              <label for="alertLevelLow" class="form-label">Low Water Alert Level</label>
              <div class="input-group">
                <input type="number" id="alertLevelLow" name="alertLevelLow" class="form-input" min="0" max="100">
                <div class="input-group-append">%</div>
              </div>
            </div>
            
            <div class="form-row">
              <label for="alertLevelHigh" class="form-label">High Water Alert Level</label>
              <div class="input-group">
                <input type="number" id="alertLevelHigh" name="alertLevelHigh" class="form-input" min="0" max="100">
                <div class="input-group-append">%</div>
              </div>
            </div>
            
            <div class="toggle-container">
              <span class="toggle-label">Enable Alerts</span>
              <label class="toggle-switch">
                <input type="checkbox" id="alertsEnabled" name="alertsEnabled" value="true">
                <span class="toggle-slider"></span>
              </label>
            </div>
            
            <button type="submit" class="button button-success">Save Alert Settings</button>
          </form>
        </div>
        
        <!-- Sensor Settings -->
        <div id="sensorSettings" class="tab-content">
          <form action="/set" method="GET">
            <div class="form-row">
              <label for="sensorOffset" class="form-label">Sensor Offset (distance from sensor to max water level)</label>
              <div class="input-group">
                <input type="number" id="sensorOffset" name="sensorOffset" class="form-input" step="0.1" min="0" max="100">
                <div class="input-group-append">cm</div>
              </div>
            </div>
            
            <div class="form-row">
              <label for="emptyDistance" class="form-label">Empty Tank Distance (from sensor to bottom)</label>
              <div class="input-group">
                <input type="number" id="emptyDistance" name="emptyDistance" class="form-input" step="0.1" min="10" max="500">
                <div class="input-group-append">cm</div>
              </div>
            </div>
            
            <div class="form-row">
              <label for="fullDistance" class="form-label">Full Tank Distance (from sensor to water when full)</label>
              <div class="input-group">
                <input type="number" id="fullDistance" name="fullDistance" class="form-input" step="0.1" min="0" max="100">
                <div class="input-group-append">cm</div>
              </div>
            </div>
            
            <div class="form-row">
              <label for="measurementInterval" class="form-label">Measurement Interval</label>
              <div class="input-group">
                <input type="number" id="measurementInterval" name="measurementInterval" class="form-input" min="1" max="60">
                <div class="input-group-append">seconds</div>
              </div>
            </div>
            
            <div class="form-row">
              <label for="readingSmoothing" class="form-label">Reading Smoothing (number of readings to average)</label>
              <div class="input-group">
                <input type="number" id="readingSmoothing" name="readingSmoothing" class="form-input" min="1" max="50">
                <div class="input-group-append">readings</div>
              </div>
            </div>
            
            <button type="submit" class="button button-success">Save Sensor Settings</button>
          </form>
        </div>
      </div>
    </div>
  </div>
  
  <div class="footer">
    <p>Water Tank Monitor | ESP32 + HC-SR04 Ultrasonic Sensor</p>
  </div>
  
  <script>
    // Fetch current settings when page loads
    document.addEventListener('DOMContentLoaded', function() {
      fetchSettings();
      setupTabHandlers();
    });
    
    function setupTabHandlers() {
      const tabs = document.querySelectorAll('.tab');
      
      tabs.forEach(tab => {
        tab.addEventListener('click', function() {
          // Remove active class from all tabs
          tabs.forEach(t => t.classList.remove('active'));
          
          // Add active class to clicked tab
          this.classList.add('active');
          
          // Hide all tab content
          document.querySelectorAll('.tab-content').forEach(content => {
            content.classList.remove('active');
          });
          
          // Show selected tab content
          const tabId = this.getAttribute('data-tab');
          document.getElementById(tabId).classList.add('active');
        });
      });
    }
    
    function fetchSettings() {
      fetch('/settings')
        .then(response => response.json())
        .then(settings => {
          // Populate form fields with current settings
          document.getElementById('tankHeight').value = settings.tankHeight;
          document.getElementById('tankDiameter').value = settings.tankDiameter;
          document.getElementById('tankVolume').value = settings.tankVolume;
          document.getElementById('sensorOffset').value = settings.sensorOffset;
          document.getElementById('emptyDistance').value = settings.emptyDistance;
          document.getElementById('fullDistance').value = settings.fullDistance;
          document.getElementById('measurementInterval').value = settings.measurementInterval;
          document.getElementById('readingSmoothing').value = settings.readingSmoothing;
          document.getElementById('alertLevelLow').value = settings.alertLevelLow;
          document.getElementById('alertLevelHigh').value = settings.alertLevelHigh;
          document.getElementById('alertsEnabled').checked = settings.alertsEnabled;
        })
        .catch(error => {
          console.error('Error fetching settings:', error);
        });
    }
  </script>
</body>
</html>
  )rawliteral";
  
  server.send(200, "text/html", html);
}
// Enhanced Web Interface - Root page with dashboard
void handleRoot() {
  const char* html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <title>AquaLevel v1.0</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    :root {
      --primary: #2196F3;
      --primary-light: #64B5F6;
      --primary-dark: #1976D2;
      --secondary: #03A9F4;
      --accent: #00BCD4;
      --success: #4CAF50;
      --warning: #FFC107;
      --danger: #F44336;
      --bg-dark: #121212;
      --bg-card: #1e1e1e;
      --text: #ffffff;
      --text-secondary: #b0b0b0;
      --border-radius: 12px;
      --shadow: 0 10px 20px rgba(0,0,0,0.3);
      --transition: all 0.3s ease;
    }
    
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: var(--bg-dark);
      color: var(--text);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      padding: 20px;
      background: linear-gradient(135deg, #121212 0%, #2a2a2a 100%);
    }
    
    .main-content {
      display: flex;
      justify-content: center;
      align-items: center;
      flex: 1;
    }
    
    .container {
      width: 100%;
      max-width: 900px;
      display: flex;
      flex-direction: column;
      gap: 20px;
    }
    
    .card {
      background: var(--bg-card);
      border-radius: var(--border-radius);
      box-shadow: var(--shadow);
      overflow: hidden;
      position: relative;
    }
    
    .card-header {
      background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 100%);
      padding: 20px;
      text-align: center;
      position: relative;
    }
    
    .card-header h1 {
      font-size: 24px;
      font-weight: 600;
      margin-bottom: 8px;
      text-shadow: 0 2px 5px rgba(0,0,0,0.2);
    }
    
    .card-header p {
      opacity: 0.9;
      font-size: 14px;
    }
    
    .grid {
      display: grid;
      grid-template-columns: 1fr;
      gap: 20px;
      padding: 20px;
    }
    
    @media (min-width: 768px) {
      .grid {
        grid-template-columns: 1fr 1fr;
      }
    }
    
    .tank-container {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      background: #1a1a1a;
      border-radius: var(--border-radius);
      padding: 20px;
      min-height: 400px;
    }
    
    .tank-visualization {
      width: 200px;
      height: 300px;
      position: relative;
      border: 8px solid #444;
      border-top: none;
      border-bottom-left-radius: 20px;
      border-bottom-right-radius: 20px;
      overflow: hidden;
      margin-bottom: 20px;
      background: rgba(0, 0, 0, 0.2);
    }
    
    /* Tank top lid */
    .tank-visualization::before {
      content: '';
      position: absolute;
      top: -10px;
      left: -8px;
      width: calc(100% + 16px);
      height: 10px;
      background: #444;
      border-top-left-radius: 10px;
      border-top-right-radius: 10px;
    }
    
    .water {
      position: absolute;
      bottom: 0;
      left: 0;
      width: 100%;
      background: linear-gradient(180deg, var(--primary-light) 0%, var(--primary-dark) 100%);
      transition: height 1s cubic-bezier(0.23, 1, 0.32, 1);
      border-top-left-radius: 6px;
      border-top-right-radius: 6px;
      overflow: hidden;
    }
    
    /* Water wave effect */
    .water::after {
      content: '';
      position: absolute;
      top: 0;
      left: 0;
      width: 200%;
      height: 10px;
      background: rgba(255, 255, 255, 0.3);
      border-radius: 50%;
      animation: wave 3s infinite linear;
    }
    
    @keyframes wave {
      0% {
        transform: translateX(-50%) scale(1);
      }
      50% {
        transform: translateX(0%) scale(1.2);
      }
      100% {
        transform: translateX(-50%) scale(1);
      }
    }
    
    .level-markers {
      position: absolute;
      left: -40px;
      top: 0;
      height: 100%;
      width: 30px;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      padding: 10px 0;
    }
    
    .level-marker {
      display: flex;
      align-items: center;
      font-size: 12px;
      color: var(--text-secondary);
    }
    
    .level-marker::before {
      content: '';
      display: block;
      width: 20px;
      height: 1px;
      background: var(--text-secondary);
      margin-right: 5px;
    }
    
    .alert-indicator {
      position: absolute;
      top: 5px;
      right: -35px;
      display: flex;
      flex-direction: column;
      gap: 10px;
    }
    
    .alert-marker {
      width: 30px;
      height: 4px;
      background: var(--danger);
      opacity: 0.4;
    }
    
    .alert-marker.high {
      background: var(--danger);
    }
    
    .alert-marker.low {
      background: var(--warning);
    }
    
    .alert-marker.active {
      opacity: 1;
      animation: blink 1s infinite;
    }
    
    @keyframes blink {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.5; }
    }
    
    .tank-info {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 10px;
    }
    
.info-value {
      font-size: 40px;
      font-weight: 700;
      color: var(--primary);
      text-shadow: 0 0 10px rgba(33, 150, 243, 0.4);
    }
    
    .info-label {
      font-size: 14px;
      color: var(--text-secondary);
    }
    
    .stats-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 15px;
    }
    
    .stat-card {
      background: #1a1a1a;
      border-radius: var(--border-radius);
      padding: 15px;
      display: flex;
      flex-direction: column;
      align-items: center;
      text-align: center;
    }
    
    .stat-value {
      font-size: 24px;
      font-weight: 600;
      margin-top: 5px;
      color: var(--primary-light);
    }
    
    .stat-label {
      font-size: 12px;
      color: var(--text-secondary);
    }
    
    .controls {
      display: flex;
      flex-direction: column;
      gap: 15px;
    }
    
    .button {
      display: block;
      width: 100%;
      padding: 15px;
      background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 100%);
      color: white;
      border: none;
      border-radius: var(--border-radius);
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: var(--transition);
      text-transform: uppercase;
      letter-spacing: 1px;
      text-align: center;
      text-decoration: none;
    }
    
    .button:hover {
      transform: translateY(-3px);
      box-shadow: 0 8px 20px rgba(33, 150, 243, 0.4);
    }
    
    .button-secondary {
      background: linear-gradient(135deg, var(--secondary) 0%, var(--accent) 100%);
    }
    
    .button-danger {
      background: linear-gradient(135deg, var(--danger) 0%, #FF7043 100%);
    }
    
    .button-success {
      background: linear-gradient(135deg, var(--success) 0%, #81C784 100%);
    }
    
    .button-warning {
      background: linear-gradient(135deg, var(--warning) 0%, #FFCA28 100%);
    }
    
    .calibration-card {
      margin-top: 20px;
    }
    
    .calibration-title {
      font-size: 16px;
      font-weight: 600;
      margin-bottom: 10px;
      text-align: center;
    }
    
    .calibration-buttons {
      display: flex;
      gap: 10px;
    }
    
    .loading-overlay {
      position: fixed;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background: rgba(0, 0, 0, 0.7);
      display: flex;
      justify-content: center;
      align-items: center;
      z-index: 1000;
    }
    
    .spinner {
      width: 50px;
      height: 50px;
      border: 5px solid rgba(255, 255, 255, 0.1);
      border-radius: 50%;
      border-top-color: var(--primary);
      animation: spin 1s linear infinite;
    }
    
    @keyframes spin {
      to { transform: rotate(360deg); }
    }
    
    .notification {
      position: fixed;
      top: 20px;
      right: 20px;
      padding: 15px 25px;
      background: linear-gradient(135deg, var(--success) 0%, #81C784 100%);
      color: white;
      border-radius: var(--border-radius);
      transform: translateX(calc(100% + 20px));
      transition: transform 0.3s ease;
      box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
      z-index: 1000;
    }
    
    .notification.error {
      background: linear-gradient(135deg, var(--danger) 0%, #FF7043 100%);
    }
    
    .notification.show {
      transform: translateX(0);
    }
    
    .footer {
      text-align: center;
      padding: 15px 0;
      margin-top: 30px;
      font-size: 12px;
      color: var(--text-secondary);
      width: 100%;
    }
    
    .tab-container {
      display: flex;
      margin-bottom: 20px;
      border-bottom: 1px solid #333;
    }

    .tab {
      padding: 10px 15px;
      cursor: pointer;
      background-color: transparent;
      border: none;
      color: var(--text-secondary);
      font-size: 14px;
      position: relative;
    }

    .tab.active {
      color: var(--primary);
    }

    .tab.active::after {
      content: '';
      position: absolute;
      bottom: -1px;
      left: 0;
      width: 100%;
      height: 2px;
      background-color: var(--primary);
    }
    
    /* Tank Settings Form */
    .form-row {
      margin-bottom: 20px;
    }
    
    .form-label {
      display: block;
      margin-bottom: 8px;
      font-size: 14px;
      color: var(--text-secondary);
    }
    
    .form-input {
      width: 100%;
      padding: 12px;
      background: #2a2a2a;
      border: 1px solid #444;
      border-radius: var(--border-radius);
      color: var(--text);
      font-size: 16px;
    }
    
    .input-group {
      display: flex;
      align-items: center;
    }
    
    .input-group .form-input {
      flex: 1;
      border-top-right-radius: 0;
      border-bottom-right-radius: 0;
    }
    
    .input-group-append {
      background: #444;
      padding: 12px 15px;
      border-top-right-radius: var(--border-radius);
      border-bottom-right-radius: var(--border-radius);
      color: var(--text-secondary);
    }
    
    .toggle-switch {
      position: relative;
      display: inline-block;
      width: 60px;
      height: 34px;
    }

    .toggle-switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    .toggle-slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #333;
      transition: .4s;
      border-radius: 34px;
    }

    .toggle-slider:before {
      position: absolute;
      content: "";
      height: 26px;
      width: 26px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: .4s;
      border-radius: 50%;
    }

    input:checked + .toggle-slider {
      background-color: var(--primary);
    }

    input:checked + .toggle-slider:before {
      transform: translateX(26px);
    }
    
    /* Settings page specific */
    .toggle-container {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 20px;
    }
    
    .toggle-label {
      font-size: 14px;
      color: var(--text-secondary);
    }
    
    #tankSettingsForm, #alertSettingsForm, #networkSettingsForm {
      display: none;
    }
    
    #tankSettingsForm.active, #alertSettingsForm.active, #networkSettingsForm.active {
      display: block;
    }
  </style>
</head>
<body>
  <div class="loading-overlay" id="loadingOverlay">
    <div class="spinner"></div>
  </div>

  <div class="notification" id="notification">
    Settings saved successfully!
  </div>

  <div class="main-content">
    <div class="container">
      <div class="card">
        <div class="card-header">
          <h1>AquaLevel Monitor</h1>
          <p>Real-time water level monitoring system</p>
        </div>
        
        <div class="grid">
          <div class="tank-container">
            <div class="tank-visualization">
              <div class="water" id="water" style="height: 0%;"></div>
              
              <div class="level-markers">
                <div class="level-marker">100%</div>
                <div class="level-marker">75%</div>
                <div class="level-marker">50%</div>
                <div class="level-marker">25%</div>
                <div class="level-marker">0%</div>
              </div>
              
              <div class="alert-indicator">
                <div class="alert-marker high" id="highAlert"></div>
                <div class="alert-marker low" id="lowAlert"></div>
              </div>
            </div>
            
            <div class="tank-info">
              <div class="info-value" id="percentageDisplay">--.-</div>
              <div class="info-label">Water Level</div>
            </div>
          </div>
          
          <div class="stats-container">
            <div class="stats-grid">
              <div class="stat-card">
                <div class="stat-label">Current Volume</div>
                <div class="stat-value" id="volumeDisplay">--.-</div>
                <div class="stat-label">liters</div>
              </div>
              <div class="stat-card">
                <div class="stat-label">Tank Capacity</div>
                <div class="stat-value" id="capacityDisplay">--.-</div>
                <div class="stat-label">liters</div>
              </div>
              <div class="stat-card">
                <div class="stat-label">Water Level</div>
                <div class="stat-value" id="levelDisplay">--.-</div>
                <div class="stat-label">cm</div>
              </div>
              <div class="stat-card">
                <div class="stat-label">Measured Distance</div>
                <div class="stat-value" id="distanceDisplay">--.-</div>
                <div class="stat-label">cm</div>
              </div>
            </div>
            
            <div class="controls">
              <a href="/settings.html" class="button">Tank Settings</a>
              <a href="/network" class="button button-secondary">Network Settings</a>
            </div>
            
            <div class="calibration-card">
              <div class="calibration-title">Quick Calibration</div>
              <div class="calibration-buttons">
                <button class="button button-warning" id="calibrateEmptyBtn">Calibrate Empty</button>
                <button class="button button-success" id="calibrateFullBtn">Calibrate Full</button>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
  
  <div class="footer">
    <p>Water Tank Monitor | ESP32 + HC-SR04 Ultrasonic Sensor</p>
  </div>
  
  <script>
    // DOM Elements
    const waterEl = document.getElementById('water');
    const percentageDisplay = document.getElementById('percentageDisplay');
    const volumeDisplay = document.getElementById('volumeDisplay');
    const capacityDisplay = document.getElementById('capacityDisplay');
    const levelDisplay = document.getElementById('levelDisplay');
    const distanceDisplay = document.getElementById('distanceDisplay');
    const highAlert = document.getElementById('highAlert');
    const lowAlert = document.getElementById('lowAlert');
    const loadingOverlay = document.getElementById('loadingOverlay');
    const notification = document.getElementById('notification');
    const calibrateEmptyBtn = document.getElementById('calibrateEmptyBtn');
    const calibrateFullBtn = document.getElementById('calibrateFullBtn');
    
    // Initialize data
    let tankData = {
      percentage: 0,
      volume: 0,
      waterLevel: 0,
      distance: 0,
      tankVolume: 0,
      alertLevelLow: 0,
      alertLevelHigh: 0,
      alertsEnabled: true
    };
    
    // Load initial data
    window.addEventListener('load', function() {
      fetchTankData();
      // Hide loading overlay after data is loaded
      setTimeout(function() {
        loadingOverlay.style.display = 'none';
      }, 1000);
    });
    
    // Setup periodic refresh
    setInterval(fetchTankData, 2000);
    
    // Fetch tank data from the API
    function fetchTankData() {
      fetch('/tank-data')
        .then(response => response.json())
        .then(data => {
          tankData = data;
          updateUI();
        })
        .catch(error => {
          console.error('Error fetching tank data:', error);
        });
    }
    
    // Update UI with current data
    function updateUI() {
      // Update water level visualization
      waterEl.style.height = tankData.percentage + '%';
      
      // Update displays
      percentageDisplay.textContent = tankData.percentage.toFixed(1) + '%';
      volumeDisplay.textContent = tankData.volume.toFixed(1);
      capacityDisplay.textContent = tankData.tankVolume.toFixed(1);
      levelDisplay.textContent = tankData.waterLevel.toFixed(1);
      distanceDisplay.textContent = tankData.distance.toFixed(1);
      
      // Update alerts
      if (tankData.alertsEnabled) {
        // High level alert
        if (tankData.percentage >= tankData.alertLevelHigh) {
          highAlert.classList.add('active');
        } else {
          highAlert.classList.remove('active');
        }
        
        // Low level alert
        if (tankData.percentage <= tankData.alertLevelLow) {
          lowAlert.classList.add('active');
        } else {
          lowAlert.classList.remove('active');
        }
      } else {
        highAlert.classList.remove('active');
        lowAlert.classList.remove('active');
      }
    }
    
    // Show notification
    function showNotification(message, isError = false) {
      notification.textContent = message;
      
      if (isError) {
        notification.classList.add('error');
      } else {
        notification.classList.remove('error');
      }
      
      notification.classList.add('show');
      
      setTimeout(function() {
        notification.classList.remove('show');
      }, 3000);
    }
    
    // Calibration handlers
    calibrateEmptyBtn.addEventListener('click', function() {
      if (confirm('Calibrate empty tank? Make sure the tank is empty.')) {
        fetch('/calibrate?type=empty')
          .then(response => response.text())
          .then(data => {
            showNotification(data);
            fetchTankData(); // Refresh data
          })
          .catch(error => {
            showNotification('Calibration failed: ' + error, true);
          });
      }
    });
    
    calibrateFullBtn.addEventListener('click', function() {
      if (confirm('Calibrate full tank? Make sure the tank is filled to your desired "full" level.')) {
        fetch('/calibrate?type=full')
          .then(response => response.text())
          .then(data => {
            showNotification(data);
            fetchTankData(); // Refresh data
          })
          .catch(error => {
            showNotification('Calibration failed: ' + error, true);
          });
      }
    });
  </script>
</body>
</html>
  )rawliteral";
  
  server.send(200, "text/html", html);
}