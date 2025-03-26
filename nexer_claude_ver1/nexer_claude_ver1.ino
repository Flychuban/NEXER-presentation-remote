#include <BluetoothSerial.h> // ESP32 Bluetooth library
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Initialize BluetoothSerial
BluetoothSerial SerialBT;

// Define pin connections
#define BUTTON_NEXT 14       // Next slide button pin
#define BUTTON_PREV 27       // Previous slide button pin
#define BUTTON_LASER 26      // Laser button pin
#define LASER_PIN 4          // Laser module pin
#define VIBRATION_MOTOR_PIN 12 // Vibration motor pin
#define BATTERY_PIN 35       // ADC pin for battery monitoring

// Debounce configuration
#define DEBOUNCE_DELAY 50    // Debounce time in milliseconds
unsigned long lastDebounceTimeNext = 0;
unsigned long lastDebounceTimePrev = 0;
unsigned long lastDebounceTimeLaser = 0;
bool lastNextState = HIGH;
bool lastPrevState = HIGH;
bool lastLaserState = HIGH;

// Battery monitoring
#define BATTERY_CHECK_INTERVAL 60000  // Check battery every minute
unsigned long lastBatteryCheck = 0;
float batteryLevel = 0.0;

// Sleep configuration
#define INACTIVITY_TIMEOUT 300000  // 5 minutes of inactivity before sleep
unsigned long lastActivityTime = 0;
bool deepSleepEnabled = true;

void setup() {
  // Initialize Serial and Bluetooth
  Serial.begin(115200);
  if (!SerialBT.begin("DIY_Presentation_Remote")) {
    Serial.println("Bluetooth initialization failed!");
    while(1);
  }
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
  
  // Configure ADC for battery monitoring
  analogReadResolution(12);  // 12-bit resolution for ADC
  
  // Welcome vibration
  triggerVibration(200);
  
  Serial.println("DIY Presentation Remote is ready!");
  
  // Update last activity time
  lastActivityTime = millis();
}

void loop() {
  // Read button states with debouncing
  handleButtons();
  
  // Check for incoming messages
  checkBluetooth();
  
  // Check battery level periodically
  checkBattery();
  
  // Check for sleep condition
  checkSleep();
}

void handleButtons() {
  // Read current button states
  bool currentNextState = digitalRead(BUTTON_NEXT);
  bool currentPrevState = digitalRead(BUTTON_PREV);
  bool currentLaserState = digitalRead(BUTTON_LASER);
  
  // Handle next slide button with debouncing
  if (currentNextState != lastNextState) {
    lastDebounceTimeNext = millis();
  }
  
  if ((millis() - lastDebounceTimeNext) > DEBOUNCE_DELAY) {
    if (currentNextState == LOW) {  // Button pressed (active LOW with pull-up)
      sendBluetoothCommand("NEXT");
      waitForAcknowledgment();
      lastActivityTime = millis();  // Update activity time
    }
  }
  
  // Handle previous slide button with debouncing
  if (currentPrevState != lastPrevState) {
    lastDebounceTimePrev = millis();
  }
  
  if ((millis() - lastDebounceTimePrev) > DEBOUNCE_DELAY) {
    if (currentPrevState == LOW) {  // Button pressed (active LOW with pull-up)
      sendBluetoothCommand("PREV");
      waitForAcknowledgment();
      lastActivityTime = millis();  // Update activity time
    }
  }
  
  // Handle laser button with debouncing
  if (currentLaserState != lastLaserState) {
    lastDebounceTimeLaser = millis();
  }
  
  if ((millis() - lastDebounceTimeLaser) > DEBOUNCE_DELAY) {
    if (currentLaserState == LOW) {  // Button pressed (active LOW with pull-up)
      digitalWrite(LASER_PIN, HIGH);
      lastActivityTime = millis();  // Update activity time
    } else {
      digitalWrite(LASER_PIN, LOW);
    }
  }
  
  // Update last states
  lastNextState = currentNextState;
  lastPrevState = currentPrevState;
  lastLaserState = currentLaserState;
  
  delay(10);  // Small delay for stability
}

// Function to check for incoming Bluetooth messages
void checkBluetooth() {
  if (SerialBT.available()) {
    String message = SerialBT.readString();
    Serial.println("Received: " + message);
    
    // Handle commands from computer if any
    if (message.startsWith("BATTERY?")) {
      SerialBT.print("BATTERY:");
      SerialBT.println(batteryLevel);
    } else if (message.startsWith("PING")) {
      SerialBT.println("PONG");
    }
    
    lastActivityTime = millis();  // Update activity time
  }
}

// Function to check battery level
void checkBattery() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastBatteryCheck >= BATTERY_CHECK_INTERVAL) {
    lastBatteryCheck = currentTime;
    
    // Read battery voltage through voltage divider
    // Assuming a voltage divider that halves the battery voltage to fit in 3.3V range
    int rawValue = analogRead(BATTERY_PIN);
    float voltage = rawValue * (3.3 / 4095.0) * 2; // Convert to actual voltage
    
    // Convert voltage to percentage (assuming 4.2V is 100% and 3.3V is 0%)
    batteryLevel = ((voltage - 3.3) / 0.9) * 100.0;
    batteryLevel = constrain(batteryLevel, 0.0, 100.0);
    
    // Send battery level
    Serial.print("Battery: ");
    Serial.print(batteryLevel);
    Serial.println("%");
    
    // Low battery warning
    if (batteryLevel < 20.0) {
      // Triple vibration for low battery warning
      triggerVibration(100);
      delay(100);
      triggerVibration(100);
      delay(100);
      triggerVibration(100);
    }
  }
}

// Function to check if the device should go to sleep
void checkSleep() {
  if (deepSleepEnabled && millis() - lastActivityTime > INACTIVITY_TIMEOUT) {
    Serial.println("Entering deep sleep mode...");
    SerialBT.println("SLEEP");
    SerialBT.flush();
    delay(100);
    
    // Configure wake-up sources (any button press)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, LOW); // BUTTON_NEXT
    
    // Enter deep sleep
    esp_deep_sleep_start();
  }
}

// Function to send Bluetooth commands
void sendBluetoothCommand(const char* command) {
  Serial.println(String("Sending over Bluetooth: ") + command);  // Debug
  SerialBT.println(command);
}

// Function to wait for acknowledgment and trigger vibration
void waitForAcknowledgment() {
  long startTime = millis();
  while (millis() - startTime < 1000) { // Wait up to 1 second for response (reduced from 3s)
    if (SerialBT.available()) {
      String response = SerialBT.readString();
      Serial.println(String("Response received: ") + response);
      if (response.indexOf("OK") >= 0) {
        triggerVibration(200);
        return;
      }
    }
    yield(); // Allow other processes to run
  }
  
  // No acknowledgment received - vibrate twice to indicate failure
  Serial.println("No acknowledgment received.");
  triggerVibration(100);
  delay(100);
  triggerVibration(100);
}

// Function to trigger vibration motor with specified duration
void triggerVibration(int duration) {
  digitalWrite(VIBRATION_MOTOR_PIN, HIGH);
  delay(duration);
  digitalWrite(VIBRATION_MOTOR_PIN, LOW);
}