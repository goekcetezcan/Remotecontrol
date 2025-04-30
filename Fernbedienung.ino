#include "BluetoothSerial.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_main.h"

BluetoothSerial SerialBT;

// Joystick Pins (analog)
const int joyXPin = 34;
const int joyYPin = 35;
const int joyButtonPin = 32;

// Kalibrierung
const int DEADZONE = 400;
const int CENTER_X = 2048;
const int CENTER_Y = 2048;

String lastCommand = "";

void setup() {
  Serial.begin(115200);
  Serial.println("Starte ESP32 Bluetooth Master mit PIN...");

  if (!btStart()) {
    Serial.println("Fehler beim btStart.");
    while (true);
  }

  if (!SerialBT.begin("ESP32_Remote", true)) {
    Serial.println("Bluetooth Master Start fehlgeschlagen.");
    while (true);
  }

  // PIN setzen für Verbindung mit HC-05
  esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
  esp_bt_pin_code_t pin_code;
  strcpy((char *)pin_code, "1234");
  esp_bt_gap_set_pin(pin_type, 4, pin_code);

  // Zieladresse deines HC-05 (MAC-Adresse anpassen!)
  uint8_t address[] = {0x00, 0x00, 0x13, 0x04, 0xA8, 0x54};

  Serial.println("Verbindungsversuch zum Roboter...");

  if (SerialBT.connect(address)) {
    Serial.println("✅ Verbindung erfolgreich!");
  } else {
    Serial.println("❌ Verbindung fehlgeschlagen.");
  }

  pinMode(joyButtonPin, INPUT_PULLUP);
  analogReadResolution(12); // Für 0-4095 Bereich
}

void loop() {
  if (SerialBT.connected()) {
    int xValue = analogRead(joyXPin);
    int yValue = analogRead(joyYPin);
    bool buttonPressed = (digitalRead(joyButtonPin) == LOW);

    int xOffset = xValue - CENTER_X;
    int yOffset = yValue - CENTER_Y;

    String command = "STOP";

    if (abs(yOffset) > DEADZONE && abs(yOffset) >= abs(xOffset)) {
      command = (yOffset < 0) ? "FORWARD" : "BACKWARD";
    } else if (abs(xOffset) > DEADZONE) {
      command = (xOffset > 0) ? "RIGHT" : "LEFT";
    }

    if (command != lastCommand) {
      Serial.print("Gesendet: ");
      Serial.println(command);
      SerialBT.println(command);
      lastCommand = command;
    }

    if (buttonPressed) {
      SerialBT.println("MODE_SWITCH");
      Serial.println("🎛️ MODE_SWITCH gesendet.");
      delay(500); // Anti-Spam
    }
  } else {
    Serial.println("❌ Nicht verbunden. Warte...");
  }

  delay(100);
}
