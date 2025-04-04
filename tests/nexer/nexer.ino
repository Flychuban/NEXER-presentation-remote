// #include <BluetoothSerial.h> // ESP32 Bluetooth library

// // Initialize BluetoothSerial
// BluetoothSerial SerialBT;

// // Define pin connections
// #define BUTTON_NEXT 12       // Next slide button pin
// #define BUTTON_PREV 13       // Previous slide button pin
// #define BUTTON_LASER 14      // Laser button pin
// #define LASER_PIN 25         // Laser module pin
// #define VIBRATION_MOTOR_PIN 26 // Vibration motor pin

// // Button states
// bool nextButtonPressed = false;
// bool prevButtonPressed = false;
// bool laserButtonPressed = false;

// void setup() {
//   // Initialize Serial and Bluetooth
//   Serial.begin(115200);
//   SerialBT.begin("DIY_Presentation_Remote"); // Bluetooth device name
  
//   // Configure pins
//   pinMode(BUTTON_NEXT, INPUT_PULLUP);
//   pinMode(BUTTON_PREV, INPUT_PULLUP);
//   pinMode(BUTTON_LASER, INPUT_PULLUP);
//   pinMode(LASER_PIN, OUTPUT);
//   pinMode(VIBRATION_MOTOR_PIN, OUTPUT);

//   // Turn off laser and vibration motor
//   digitalWrite(LASER_PIN, LOW);
//   digitalWrite(VIBRATION_MOTOR_PIN, LOW);
  
//   Serial.println("DIY Presentation Remote is ready!");
// }

// void loop() {
//   // Read button states
//   nextButtonPressed = digitalRead(BUTTON_NEXT) == LOW;
//   prevButtonPressed = digitalRead(BUTTON_PREV) == LOW;
//   laserButtonPressed = digitalRead(BUTTON_LASER) == LOW;

//   // Handle next slide button
//   if (nextButtonPressed) {
//     sendBluetoothCommand("NEXT");
//     waitForAcknowledgment();
//   }

//   // Handle previous slide button
//   if (prevButtonPressed) {
//     sendBluetoothCommand("PREV");
//     waitForAcknowledgment();
//   }

//   // Handle laser button
//   if (laserButtonPressed) {
//     digitalWrite(LASER_PIN, HIGH);
//   } else {
//     digitalWrite(LASER_PIN, LOW);
//   }

//   delay(100); // Small delay for debouncing
// }

// // Function to send Bluetooth commands
// void sendBluetoothCommand(const char* command) {
//   SerialBT.println(command);
//   Serial.println(String("Command sent: ") + command);
// }

// // Function to wait for acknowledgment and trigger vibration
// void waitForAcknowledgment() {
//   long startTime = millis();
//   while (millis() - startTime < 3000) { // Wait up to 3 seconds for response
//     if (SerialBT.available()) {
//       String response = SerialBT.readString();
//       Serial.println(String("Response received: ") + response);
//       if (response == "OK") {
//         triggerVibration();
//         return;
//       }
//     }
//   }
//   Serial.println("No acknowledgment received.");
// }

// // Function to trigger vibration motor
// void triggerVibration() {
//   digitalWrite(VIBRATION_MOTOR_PIN, HIGH);
//   delay(1000); // Vibration duration: 1 second
//   digitalWrite(VIBRATION_MOTOR_PIN, LOW);
// }


#define PIN 4  // Define the GPIO pin

void setup() {
    pinMode(PIN, OUTPUT); // Set GPIO 13 as an output
}

void loop() {
    digitalWrite(PIN, HIGH); // Set GPIO 13 HIGH
    delay(1000); // Wait for 1 second
    digitalWrite(PIN, LOW);  // Set GPIO 13 LOW
    delay(1000); // Wait for 1 second
}