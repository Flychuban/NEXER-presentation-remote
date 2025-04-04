#include <BluetoothSerial.h> // ESP32 Bluetooth library

// Initialize BluetoothSerial
BluetoothSerial SerialBT;

// Define pin connections
#define BUTTON_NEXT 14       // Next slide button pin
#define BUTTON_PREV 27       // Previous slide button pin
#define BUTTON_LASER 26      // Laser button pin
#define LASER_PIN 4          // Laser module pin
#define VIBRATION_MOTOR_PIN 12 // Vibration motor pin

void setup() {
  // Initialize Serial and Bluetooth
  Serial.begin(115200);
  SerialBT.begin("DIY_Presentation_Remote"); // Bluetooth device name
  
  // Configure button pins as input with pull-up resistors
  pinMode(BUTTON_NEXT, INPUT_PULLUP);
  pinMode(BUTTON_PREV, INPUT_PULLUP);
  pinMode(BUTTON_LASER, INPUT_PULLUP);

  // Configure output pins
  pinMode(LASER_PIN, OUTPUT);
  pinMode(VIBRATION_MOTOR_PIN, OUTPUT);

  // Turn off laser and vibration motor
  digitalWrite(LASER_PIN, LOW);
  digitalWrite(VIBRATION_MOTOR_PIN, LOW);
  
  Serial.println("DIY Presentation Remote is ready!");
}

void loop() {
  // Read button states
  bool nextButtonPressed = digitalRead(BUTTON_NEXT) == LOW;
  bool prevButtonPressed = digitalRead(BUTTON_PREV) == LOW;
  bool laserButtonPressed = digitalRead(BUTTON_LASER) == LOW;

  // Handle next slide button
  if (nextButtonPressed) {
    sendBluetoothCommand("NEXT");
    waitForAcknowledgment();
  }

  // Handle previous slide button
  if (prevButtonPressed) {
    sendBluetoothCommand("PREV");
    waitForAcknowledgment();
  }

  // Handle laser button
  if (laserButtonPressed) {
    digitalWrite(LASER_PIN, HIGH);
  } else {
    digitalWrite(LASER_PIN, LOW);
  }

  delay(100); // Small delay for debouncing
}

// Function to send Bluetooth commands
void sendBluetoothCommand(const char* command) {
  SerialBT.println(command);
  Serial.println(String("Command sent: ") + command);
}

// Function to wait for acknowledgment and trigger vibration
void waitForAcknowledgment() {
  long startTime = millis();
  while (millis() - startTime < 3000) { // Wait up to 3 seconds for response
    if (SerialBT.available()) {
      String response = SerialBT.readString();
      Serial.println(String("Response received: ") + response);
      if (response == "OK") {
        triggerVibration();
        return;
      }
    }
  }
  Serial.println("No acknowledgment received.");
}

// Function to trigger vibration motor
void triggerVibration() {
  digitalWrite(VIBRATION_MOTOR_PIN, HIGH);
  delay(500); // Vibration duration: 0.5 seconds
  digitalWrite(VIBRATION_MOTOR_PIN, LOW);
}