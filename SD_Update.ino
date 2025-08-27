/*
 Name:      SD_Update.ino
 Created:   12.09.2017 15:07:17
 Modified:    @Trion
 Purpose:   Update firmware from SD card with version checking and LED status.

 ==============================================================================
 HOW TO USE VERSIONING:
   1.  Define the current firmware version below using #define FIRMWARE_VERSION.
       Start with 1.
   2.  When you compile a new version of your firmware, INCREMENT this number (e.g., to 2).
   3.  On your SD Card, create two files:
       - update.bin (your new compiled firmware)
       - version.txt (a simple text file)
   4.  Open version.txt and write the new version number inside it (e.g., "2").
       Ensure it's just the number, with no extra text.
   5.  The ESP32 will now only update if the number in version.txt is
       strictly greater than its own FIRMWARE_VERSION.
 ==============================================================================

 Steps:
   1. Flash this image (with FIRMWARE_VERSION 1) to the ESP32.
   2. Increment FIRMWARE_VERSION to 2, recompile, and copy the .bin to the SD card
      as "update.bin".
   3. Create "version.txt" on the SD card containing "2".
   4. Connect the SD Card and power on the ESP32. It should perform the update.
   5. If you reboot it again without changing the files, it will see that the
      versions match and skip the update.
*/

#include <Update.h>
#include <FS.h>
#include <SD.h>

// --- ADDED: Define the version of the current firmware ---
// !! INCREMENT THIS NUMBER FOR EACH NEW FIRMWARE RELEASE !!
#define FIRMWARE_VERSION 1

// --- Define GPIO pins for LEDs ---
#define LED_PIN_SD_OK   15 // Blue LED: Indicates SD card is mounted
#define LED_PIN_SUCCESS 13 // Green LED: Indicates successful update
#define LED_PIN_STATUS  14 // Yellow LED: Indicates the process is active
#define LED_PIN_ERROR   12 // Red LED: Indicates an error occurred

// --- Helper function to signal an error ---
void signalError() {
  digitalWrite(LED_PIN_STATUS, LOW);
  digitalWrite(LED_PIN_SD_OK, LOW);
  digitalWrite(LED_PIN_ERROR, HIGH); // Turn on the RED error LED
}

// perform the actual update from a given stream
void performUpdate(Stream &updateSource, size_t updateSize) {
  if (Update.begin(updateSize)) {
    size_t written = Update.writeStream(updateSource);
    if (written == updateSize) {
      Serial.println("Written : " + String(written) + " successfully");
    } else {
      Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      signalError(); // Signal error if write was incomplete
    }

    if (Update.end()) {
      Serial.println("OTA done!");
      if (Update.isFinished()) {
        Serial.println("Update successfully completed. Rebooting.");
        // --- Signal success before rebooting ---
        digitalWrite(LED_PIN_STATUS, LOW);
        digitalWrite(LED_PIN_SUCCESS, HIGH); // Turn on GREEN success LED
        delay(2000); // Keep it on for a moment
      } else {
        Serial.println("Update not finished? Something went wrong!");
        signalError();
      }
    } else {
      Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      signalError();
    }

  } else {
    Serial.println("Not enough space to begin OTA");
    signalError();
  }
}

// --- MODIFIED: check FS for update.bin and version.txt, and perform update if new ---
void updateFromFS(fs::FS &fs) {
  // First, check for the version file
  File versionFile = fs.open("/ver.txt");
  if (!versionFile || versionFile.isDirectory()) {
    Serial.println("Could not find version.txt file on SD card. Skipping update check.");
    return;
  }

  // Read the new version number from the file
  String newVersionStr = versionFile.readStringUntil('\n');
  versionFile.close();
  long newVersion = newVersionStr.toInt();

  if (newVersion == 0) {
    Serial.println("Invalid version number in version.txt. Skipping update.");
    signalError();
    return;
  }

  Serial.println("Current firmware version: " + String(FIRMWARE_VERSION));
  Serial.println("Available firmware version: " + String(newVersion));

  // Compare versions
  if (newVersion > FIRMWARE_VERSION) {
    Serial.println("New firmware available. Starting update...");
    digitalWrite(LED_PIN_STATUS, HIGH); // Yellow LED on for update process

    File updateBin = fs.open("/update.bin");
    if (updateBin) {
      if (updateBin.isDirectory()) {
        Serial.println("Error, update.bin is a directory");
        updateBin.close();
        signalError();
        return;
      }

      size_t updateSize = updateBin.size();
      if (updateSize > 0) {
        performUpdate(updateBin, updateSize);
      } else {
        Serial.println("Error, update.bin file is empty");
        signalError();
      }
      updateBin.close();

      // When finished, remove both files to prevent re-updating
      Serial.println("Removing update files from SD card...");
      fs.remove("/update.bin");
      fs.remove("/version.txt");

    } else {
      Serial.println("Could not load update.bin from SD root. Update failed.");
      signalError();
    }
  } else {
    Serial.println("Firmware is already up to date.");
  }
}

void rebootEspWithReason(String reason) {
  Serial.println(reason);
  // Blink error LED before restarting
  signalError();
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN_ERROR, !digitalRead(LED_PIN_ERROR));
    delay(200);
  }
  ESP.restart();
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nWelcome to the SD-Update example!");
  Serial.println("Current Firmware Version: " + String(FIRMWARE_VERSION));

  // Initialize LED pins
  pinMode(LED_PIN_SD_OK, OUTPUT);
  pinMode(LED_PIN_SUCCESS, OUTPUT);
  pinMode(LED_PIN_STATUS, OUTPUT);
  pinMode(LED_PIN_ERROR, OUTPUT);

  // Set initial state to OFF
  digitalWrite(LED_PIN_SD_OK, LOW);
  digitalWrite(LED_PIN_SUCCESS, LOW);
  digitalWrite(LED_PIN_STATUS, LOW);
  digitalWrite(LED_PIN_ERROR, LOW);
  
  // Turn on status LED to show the process has started
  digitalWrite(LED_PIN_STATUS, HIGH);
  delay(500); // Small delay to see the status LED

  // First, init and check SD card (Default CS pin is 5)
  if (!SD.begin(5)) {
    rebootEspWithReason("Card Mount Failed");
  }
  
  digitalWrite(LED_PIN_SD_OK, HIGH); // Blue LED on: SD card mounted

  if (SD.cardType() == CARD_NONE) {
    rebootEspWithReason("No SD card attached");
  } else {
    updateFromFS(SD);
  }

  // Turn off status LEDs when done
  digitalWrite(LED_PIN_STATUS, LOW);
  digitalWrite(LED_PIN_SD_OK, LOW);
  Serial.println("Update process finished. If no update was performed, device will idle.");
}

// will not be reached if an update happens
void loop() {
  // The device will idle here if no update file was found
  // or if an error occurred that did not trigger a reboot.
  delay(1000);
}
