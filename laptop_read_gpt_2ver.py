import serial
import pyautogui

# Replace with the correct Bluetooth port for your system
BLUETOOTH_PORT = "/dev/cu.YOUR_ESP32_DEVICE"  # For macOS
BAUD_RATE = 115200

# Connect to ESP32 via Bluetooth
esp32 = serial.Serial(BLUETOOTH_PORT, BAUD_RATE, timeout=1)

print("Connected to ESP32. Listening for commands...")

while True:
    if esp32.in_waiting:
        command = esp32.readline().decode('utf-8').strip()
        print(f"Received command: {command}")

        if command == "NEXT":
            pyautogui.press("right")  # Simulate pressing right arrow key
            esp32.write(b"OK\n")  # Send acknowledgment

        elif command == "PREV":
            pyautogui.press("left")  # Simulate pressing left arrow key
            esp32.write(b"OK\n")  # Send acknowledgment