# AquaLevel - Smart Water Tank Monitor

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

A comprehensive ESP32-based water level monitoring system with WiFi connectivity, web dashboard, and customizable alerts.

![AquaLevel Dashboard](https://github.com/Techposts/aqualevel/blob/main/MainInterface.png)

## Overview

AquaLevel is an IoT water level monitoring solution built on the ESP32 platform, using an HC-SR04 ultrasonic sensor to track water levels in tanks with high precision. The system provides real-time monitoring through an elegant web interface, allowing users to visualize water levels, configure tank parameters, and receive alerts when water levels reach critical thresholds.

## Features

### Real-time Monitoring
- Live water level percentage and volume display
- Animated tank visualization
- Configurable measurement intervals
- Reading smoothing for stability

### Web Dashboard
- Modern, responsive interface works on all devices
- Real-time updates without page refreshes
- Dark mode UI for better visibility
- Easy-to-use settings controls

### Connectivity
- WiFi connectivity with fallback AP mode
- mDNS support for easy access (aqualevel.local)
- Simple network configuration interface
- Automatic reconnection if connection drops

![AquaLevel Network Settings](https://github.com/Techposts/aqualevel/blob/main/NetworkSettings.png)

### Calibration & Settings
- One-click calibration for empty and full states
- Configurable tank dimensions and parameters
- Intelligent volume calculation
- Persistent settings with EEPROM storage

![AquaLevel Tank Settings](https://github.com/Techposts/aqualevel/blob/main/TankSettings.png)


### Alert System
- Configurable high and low water level thresholds
- Visual alert indicators
- Expandable notification system

## Hardware Requirements

- ESP32 development board
- HC-SR04 ultrasonic sensor
- Jumper wires
- Power supply (5V)
- Optional: mounting hardware for sensor placement

## Pin Configuration

| Component | ESP32 Pin |
|-----------|-----------|
| HC-SR04 Trigger | GPIO 1 |
| HC-SR04 Echo | GPIO 3 |

![AquaLevel Connection Diagram](https://raw.githubusercontent.com/Techposts/aqualevel/refs/heads/main/DIY%20Solar%20Powered%20Water%20Level%20Sensor%20-%20Waterproof%2C%20Touchless%20and%20Truly%20Wireless%20-%20Works%20with%20home%20Assistant%20(1).jpg)

**NOTE: **Solar is Optional. You can provide direct power supply as well.

## Installation

### Hardware Setup
1. Connect the HC-SR04 ultrasonic sensor to the ESP32:
   - VCC to 5V
   - GND to GND
   - Trigger to GPIO 1
   - Echo to GPIO 3
2. Mount the sensor at the top of your tank, facing down toward the water surface

## Getting Started
You can install AquaLevel on your ESP32-C3 SuperMini in two ways:

### Option 1: Using Arduino IDE

1. Clone this repository or download the source code
2. Open the `WLSv1.0.ino` file in Arduino IDE
3. Install required libraries from Library Manager:
   - ESP32 board support
   - EEPROM
   - WiFi
   - WebServer
   - ESPmDNS
4. Select "ESP32C3 Dev Module" from the Board menu
5. Connect your ESP32-C3 SuperMini via USB
6. Click Upload to flash the code

### Option 2: Flashing Pre-compiled Binaries

You can flash the pre-compiled binaries directly to your ESP32-C3 using one of the following methods:

#### Using ESP Flash Download Tool (Recommended for beginners)

The ESP Flash Download Tool provides a user-friendly GUI for flashing ESP devices.

1. **Download ESP Flash Download Tool**:
   * Download from the [official Espressif website](https://www.espressif.com/en/support/download/other-tools)
   * Flash tool User guide: [Flash Download Tools Guide](https://www.espressif.com/sites/default/files/documentation/flash_download_tool_user_manual_en.pdf)

2. **Extract and run the tool**:
   * Extract the ZIP file
   * Run "flash_download_tool_x.x.x.exe" (where x.x.x is the version number)
   * Select "ESP32-C3 RISC-V" from the chip type dropdown

3. **Configure the tool as follows**:
   * Set SPI SPEED to "80MHz"
   * Set SPI MODE to "DIO"
   * Set FLASH SIZE to "4MB" (or match your ESP32-C3's flash size)

4. **Download and add the binary files with these specific addresses**:
   * Click [+] and add `AquaLevel-ESP32C3-bootloader.bin` at address `0x0`
   * Click [+] and add `AquaLevel-ESP32C3-partitions.bin` at address `0x8000`
   * Click [+] and add `AquaLevel-ESP32C3.bin` at address `0x10000`
   * Ensure all three checkboxes next to the file paths are checked

5. **Select the appropriate COM port** where your ESP32-C3 is connected
6. Set the baud rate to `921600` for faster flashing
7. Click the "START" button to begin flashing
8. Wait for the tool to display "FINISH" when the flashing is complete
9. Press the reset button on your ESP32-C3 board

#### Using esptool.py (Alternative method)

esptool is a command-line utility for flashing ESP devices. Here's how to use it:

1. **Install esptool** using pip (requires Python):
```
pip install esptool
```

2. **Open Command Prompt** (cmd) and navigate to your binary files:
```
cd C:\path\to\your\binaries
```

3. **Flash your ESP32-C3** with this command (replace COM3 with your actual port):
```
esptool.py --chip esp32c3 --port COM3 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 AquaLevel-ESP32C3-bootloader.bin 0x8000 AquaLevel-ESP32C3-partitions.bin 0x10000 AquaLevel-ESP32C3.bin
```

4. **Troubleshooting**:
   * If you have connection issues, try erasing the flash first:
   ```
   esptool.py --chip esp32c3 --port COM3 erase_flash
   ```
   
   * If you still have issues, try a lower baud rate:
   ```
   esptool.py --chip esp32c3 --port COM3 --baud 115200 write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 AquaLevel-ESP32C3-bootloader.bin 0x8000 AquaLevel-ESP32C3-partitions.bin 0x10000 AquaLevel-ESP32C3.bin
   ```
   
   * For detailed logs, add the `-v` flag:
   ```
   esptool.py -v --chip esp32c3 --port COM3 --baud 921600 write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 AquaLevel-ESP32C3-bootloader.bin 0x8000 AquaLevel-ESP32C3-partitions.bin 0x10000 AquaLevel-ESP32C3.bin
   ```

After flashing is complete, proceed with the below steps.
### Initial Setup
1. Power on your AquaLevel device
2. Connect to the WiFi network named "Aqualevel-XXXXX-Setup"
3. Open a web browser and navigate to `192.168.4.1`
4. Go to "Network Settings" and configure your home WiFi
5. After restarting, access your device at `http://aqualevel-[name].local`

### Tank Configuration
1. Navigate to "Tank Settings"
2. Enter your tank dimensions:
   - Height (cm)
   - Diameter (cm) 
   - Volume (liters) - will be calculated automatically but can be overridden

### Calibration
1. Ensure the tank is empty
2. Click "Calibrate Empty" on the dashboard
3. Fill the tank to your desired "full" level
4. Click "Calibrate Full"

### Alert Configuration
1. Go to "Alert Settings"
2. Set the low water percentage threshold (default: 10%)
3. Set the high water percentage threshold (default: 90%)
4. Toggle alerts on/off as needed

## Configuration Options

All settings are persistent and saved to EEPROM:

### Tank Parameters
- Tank height (cm)
- Tank diameter (cm)
- Tank volume (liters)

### Sensor Configuration
- Sensor offset (distance from sensor to max water level)
- Empty distance (reading when tank is empty)
- Full distance (reading when tank is full)
- Measurement interval (seconds between readings)
- Reading smoothing (number of readings to average)

### Alert Parameters
- Low alert level (percentage)
- High alert level (percentage)
- Alerts enabled/disabled

### Network Settings
- WiFi SSID
- WiFi password
- Device name (used for mDNS)

## How It Works

### Distance Measurement
The HC-SR04 sensor measures distance by sending ultrasonic pulses and timing how long they take to bounce back. The system calculates water level by comparing the current distance reading to the calibrated empty and full distances.

### Calibration Process
- **Empty calibration**: Records the distance reading when the tank is empty (maximum distance)
- **Full calibration**: Records the distance reading when the tank is full (minimum distance)
- The system calculates percentages based on the range between these two values

### Volume Calculation
Volume is calculated based on tank dimensions (for cylindrical tanks):
- For custom tank shapes, you can override the calculated volume with your own value

### Reading Smoothing
The system uses a dynamic buffer to average multiple readings, providing stable measurements even with choppy water surfaces or sensor noise.

## Advanced Configuration

### Code Customization
- Edit `config.h` to change default parameters
- Modify pin assignments for different hardware setups
- Adjust serial output for debugging

### Alert Extensions
The system includes hooks for expanding the alert system:
- Add relay controls for pumps or valves
- Implement MQTT for external notifications
- Connect additional indicators or buzzers

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Cannot connect to WiFi | Check credentials; reset WiFi settings with button on Network page |
| Inaccurate readings | Ensure sensor is properly mounted; recalibrate; increase smoothing |
| Cannot access web interface | Verify IP address; check WiFi connection; restart device |
| Erratic measurements | Adjust sensor position; increase smoothing; check for interference |

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgements

- Based on the original concept by Ravi Singh
- Thanks to all contributors who have helped improve this project

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request
