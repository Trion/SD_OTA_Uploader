# SD OTA Uploader

**SD OTA Uploader** is an ESP32 firmware update utility that allows you to update your device's firmware directly from an SD card. It includes automatic version checking and visual status indication using four LEDs (Blue, Green, Yellow, Red).

## Features

- **SD Card-Based Firmware Update**: Place your new firmware and a version file onto your SD card to trigger an update.
- **Version Checking**: Only updates if the SD card's version is newer than the device's current firmware.
- **LED Status Indication**: Four GPIO-controlled LEDs show SD card status, update progress, success, and errors.
- **Automatic Cleanup**: Removes update files from SD card after successful update to prevent repeated updates.

## How It Works

1. **Set the Version**:  
   In `SD_Update.ino`, set your current firmware version with:
   ```c++
   #define FIRMWARE_VERSION N  // Increment N for each release
   ```

2. **Prepare SD Card Files**:  
   - `update.bin`: Your new compiled ESP32 firmware.
   - `version.txt`: A text file containing the new version number (e.g., `2`).

3. **Insert SD Card and Power On**:  
   - If the version in `version.txt` is greater than `FIRMWARE_VERSION`, the device updates itself and signals success via LED.
   - If not, the update is skipped.

4. **LED Pin Usage**:
   - **Blue (GPIO 15)**: SD card is mounted.
   - **Green (GPIO 13)**: Update successful.
   - **Yellow (GPIO 14)**: Update process active.
   - **Red (GPIO 12)**: An error occurred.

## Getting Started

### 1. Flash Initial Firmware

Flash the ESP32 with the initial version (`FIRMWARE_VERSION 1`).

### 2. Prepare Update

Increment `FIRMWARE_VERSION` in the source, recompile, and copy the resulting `.bin` as `update.bin` to the SD card root.  
Create a `version.txt` file containing the new version number (e.g., `2`).

### 3. Insert SD Card & Boot

Insert the SD card and power on the ESP32. The device will check the version and perform the update if needed.

## Example Directory Structure on SD Card

```
/update.bin
/version.txt
```

## Requirements

- ESP32 board
- SD card module (CS pin default: GPIO 5)
- Four LEDs connected to GPIO pins:
  - 15 (Blue)
  - 13 (Green)
  - 14 (Yellow)
  - 12 (Red)

## Pinout

| LED     | Color  | GPIO Pin |
|---------|--------|----------|
| SD_OK   | Blue   | 15       |
| SUCCESS | Green  | 13       |
| STATUS  | Yellow | 14       |
| ERROR   | Red    | 12       |

## Troubleshooting

- If errors are signaled (Red LED), check:
  - SD card is inserted and working.
  - `update.bin` and `version.txt` exist in the SD card root.
  - `version.txt` contains only the version number (no extra characters).
  - `update.bin` is a valid ESP32 firmware binary.

## License

MIT

## Author

[@Trion](https://github.com/Trion)
