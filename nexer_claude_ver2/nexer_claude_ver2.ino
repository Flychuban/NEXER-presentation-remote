#include <BluetoothSerial.h>

// Check if Bluetooth is properly supported
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

// Initialize BluetoothSerial
BluetoothSerial SerialBT;

// Define pin connections
#define BUTTON_NEXT 14       // Next slide button pin
#define BUTTON_PREV 27       // Previous slide button pin
#define BUTTON_LASER 26      // Laser button pin
#define LASER_PIN 4          // Laser module pin
#define VIBRATION_MOTOR_PIN 12 // Vibration motor pin
#define BATTERY_PIN 35       // ADC pin for battery monitoring
#define STATUS_LED_PIN 2     // Built-in LED for status indication

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

// Communication status
unsigned long lastCommandSentTime = 0;
bool waitingForAck = false;
#define ACK_TIMEOUT 3000     // Time to wait for acknowledgment (3 seconds)

// Debug mode
bool debugMode = true;

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  delay(1000); // Give time for serial to connect
  Serial.println("\n\n--- DIY Presentation Remote Starting ---");
  
  // Initialize Bluetooth Serial with unique name that matches Python code
  SerialBT.begin("DIY_Presentation_Remote");
  Serial.println("Bluetooth initialized as 'DIY_Presentation_Remote'");
  
  // Configure button pins as input with pull-up resistors
  pinMode(BUTTON_NEXT, INPUT_PULLUP);
  pinMode(BUTTON_PREV, INPUT_PULLUP);
  pinMode(BUTTON_LASER, INPUT_PULLUP);
  
  // Configure output pins
  pinMode(LASER_PIN, OUTPUT);
  pinMode(VIBRATION_MOTOR_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  
  // Turn off outputs
  digitalWrite(LASER_PIN, LOW);
  digitalWrite(VIBRATION_MOTOR_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  // Configure ADC for battery monitoring
  analogReadResolution(12);  // 12-bit resolution for ADC
  
  // Welcome indication
  blinkStatusLED(3, 200); // Blink 3 times
  triggerVibration(200);  // Short vibration
  
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
  
  // Check for acknowledgment timeout
  checkAckTimeout();
  
  // Small delay for stability
  delay(10);
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
    if (currentNextState == LOW && lastNextState == HIGH) {  // Button pressed (active LOW with pull-up)
      Serial.println("NEXT button pressed");
      if (!waitingForAck) { // Only send if not waiting for previous ACK
        sendBluetoothCommand("NEXT");
        waitingForAck = true;
        lastCommandSentTime = millis();
        digitalWrite(STATUS_LED_PIN, HIGH); // Turn on LED while waiting for ACK
      } else {
        Serial.println("Ignoring button - still waiting for previous ACK");
      }
      lastActivityTime = millis();  // Update activity time
    }
  }
  
  // Handle previous slide button with debouncing
  if (currentPrevState != lastPrevState) {
    lastDebounceTimePrev = millis();
  }
  
  if ((millis() - lastDebounceTimePrev) > DEBOUNCE_DELAY) {
    if (currentPrevState == LOW && lastPrevState == HIGH) {  // Button pressed (active LOW with pull-up)
      Serial.println("PREV button pressed");
      if (!waitingForAck) { // Only send if not waiting for previous ACK
        sendBluetoothCommand("PREV");
        waitingForAck = true;
        lastCommandSentTime = millis();
        digitalWrite(STATUS_LED_PIN, HIGH); // Turn on LED while waiting for ACK
      } else {
        Serial.println("Ignoring button - still waiting for previous ACK");
      }
      lastActivityTime = millis();  // Update activity time
    }
  }
  
  // Handle laser button with debouncing
  if (currentLaserState != lastLaserState) {
    lastDebounceTimeLaser = millis();
  }
  
  if ((millis() - lastDebounceTimeLaser) > DEBOUNCE_DELAY) {
    if (currentLaserState == LOW) {  // Button pressed (active LOW with pull-up)
      Serial.println("LASER button pressed");
      digitalWrite(LASER_PIN, HIGH);
      lastActivityTime = millis();  // Update activity time
    } else if (currentLaserState == HIGH && lastLaserState == LOW) {
      Serial.println("LASER button released");
      digitalWrite(LASER_PIN, LOW);
    }
  }
  
  // Update last states
  lastNextState = currentNextState;
  lastPrevState = currentPrevState;
  lastLaserState = currentLaserState;
}

// Function to check for acknowledgment timeout
void checkAckTimeout() {
  if (waitingForAck && (millis() - lastCommandSentTime > ACK_TIMEOUT)) {
    Serial.println("ACK timeout - no response received");
    waitingForAck = false;
    digitalWrite(STATUS_LED_PIN, LOW);
    
    // Double vibration to indicate failure
    triggerVibration(100);
    delay(100);
    triggerVibration(100);
  }
}

// Function to check for incoming Bluetooth messages
void checkBluetooth() {
  if (SerialBT.available()) {
    // Read the incoming message
    String message = SerialBT.readStringUntil('\n');
    message.trim(); // Remove any whitespace or newline characters
    
    Serial.print("Received from computer: '");
    Serial.print(message);
    Serial.println("'");
    
    // Process the message
    if (message.equals("PING")) {
      Serial.println("Responding to PING");
      SerialBT.println("PONG");
      SerialBT.flush();
    } 
    else if (message.equals("BATTERY?")) {
      String batteryMessage = "BATTERY:" + String(batteryLevel, 1);
      Serial.println("Sending: " + batteryMessage);
      SerialBT.println(batteryMessage);
      SerialBT.flush();
    }
    else if (message.equals("OK")) {
      Serial.println("Received ACK");
      waitingForAck = false;
      digitalWrite(STATUS_LED_PIN, LOW);
      triggerVibration(200); // Vibrate to indicate success
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
  Serial.print("Sending command: '");
  Serial.print(command);
  Serial.println("'");
  
  // Send the command with a newline
  SerialBT.println(command);
  
  // Flush to ensure data is sent
  SerialBT.flush();
  
  // Debug indicator
  blinkStatusLED(1, 100);
}

// Function to trigger vibration motor with specified duration
void triggerVibration(int duration) {
  digitalWrite(VIBRATION_MOTOR_PIN, HIGH);
  delay(duration);
  digitalWrite(VIBRATION_MOTOR_PIN, LOW);
}

// Function to blink the status LED
void blinkStatusLED(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(duration);
    digitalWrite(STATUS_LED_PIN, LOW);
    if (i < times - 1) {
      delay(duration);
    }
  }
}