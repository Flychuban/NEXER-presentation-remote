import serial
import pyautogui
import time
import threading
from datetime import datetime

# Configuration
BAUD_RATE = 115200
RECONNECT_INTERVAL = 5  # seconds
KEEPALIVE_INTERVAL = 30  # seconds
PORT_SCAN_INTERVAL = 3  # seconds

# Global variables
esp32 = None
connected = False
last_command_time = time.time()
exit_flag = False

def log_event(message):
    """Log events with timestamp"""
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    log_message = f"[{timestamp}] {message}"
    print(log_message)
    
    # You can also write to a log file if needed
    # with open("presentation_remote.log", "a") as log_file:
    #     log_file.write(log_message + "\n")

def connect_to_esp32():
    """Try to connect to the ESP32 device"""
    global esp32, connected
    
    try:
        # port = "COM3"  # Default port for Windows
        port = "/dev/cu.DIY_Presentation_Remote" # Port for macOS
        if not port:
            log_event("No suitable port found. Please check your device connection.")
            return False
        
        log_event(f"Attempting to connect to {port}")
        esp32 = serial.Serial(port, BAUD_RATE, timeout=1)
        connected = True
        log_event(f"Connected to ESP32 via {port}")
        
        # Send initial message to check connection
        esp32.write(b"PING\n")
        time.sleep(0.5)
        
        if esp32.in_waiting:
            response = esp32.readline().decode('utf-8').strip()
            if response == "PONG":
                log_event("Communication verified with ESP32")
            else:
                log_event(f"Unexpected response: {response}")
        
        return True
    
    except Exception as e:
        log_event(f"Connection error: {str(e)}")
        connected = False
        return False

def keepalive_thread():
    """Thread to periodically check connection and send keepalive signals"""
    global esp32, connected, exit_flag
    
    while not exit_flag:
        if connected:
            # Check if we haven't received commands for a while
            if time.time() - last_command_time > KEEPALIVE_INTERVAL:
                try:
                    # Send a ping to check if the connection is still alive
                    esp32.write(b"PING\n")
                    log_event("Sent keepalive ping")
                    
                    # Check battery level periodically
                    esp32.write(b"BATTERY?\n")
                    time.sleep(0.5)
                    
                    if esp32.in_waiting:
                        response = esp32.readline().decode('utf-8').strip()
                        if response.startswith("BATTERY:"):
                            battery_level = response.split(":")[1]
                            log_event(f"ESP32 Battery Level: {battery_level}%")
                    
                except Exception as e:
                    log_event(f"Keepalive error: {str(e)}")
                    connected = False
        
        # Sleep for a while before next check
        time.sleep(KEEPALIVE_INTERVAL / 3)

def main():
    """Main function to handle connection and commands"""
    global esp32, connected, last_command_time, exit_flag
    
    log_event("DIY Presentation Remote Receiver Starting")
    
    # Start keepalive thread
    threading.Thread(target=keepalive_thread, daemon=True).start()
    
    try:
        while not exit_flag:
            # If not connected, try to connect
            if not connected:
                if connect_to_esp32():
                    # Successfully connected
                    pass
                else:
                    # Wait before trying again
                    log_event(f"Will try to reconnect in {RECONNECT_INTERVAL} seconds...")
                    time.sleep(RECONNECT_INTERVAL)
                    continue
            
            # Check for commands
            try:
                if esp32.in_waiting:
                    print("ESP32 in waiting")
                    command = esp32.readline().decode('utf-8').strip()
                    last_command_time = time.time()
                    print(command)
                    
                    if command:
                        log_event(f"Received command: '{command}'")  # Add quotes to see if there are any hidden characters
                        
                        if command == "NEXT":
                            log_event("Processing NEXT command")
                            pyautogui.press("right")  # Simulate pressing right arrow key
                            esp32.write(b"OK\n")  # Send acknowledgment
                            log_event("Sent OK acknowledgment for NEXT")
                        
                        elif command == "PREV":
                            log_event("Processing PREV command")
                            pyautogui.press("left")  # Simulate pressing left arrow key
                            esp32.write(b"OK\n")  # Send acknowledgment
                            log_event("Sent OK acknowledgment for PREV") # Send acknowledgment
                        
                        elif command == "SLEEP":
                            log_event("ESP32 entering sleep mode")
                        
                        else:
                            log_event(f"Unknown command: {command}")
                
                # A short delay to reduce CPU usage
                time.sleep(0.1)
                
            except Exception as e:
                log_event(f"Error reading from serial: {str(e)}")
                connected = False
                if esp32:
                    try:
                        esp32.close()
                    except:
                        pass
                time.sleep(RECONNECT_INTERVAL)
    
    except KeyboardInterrupt:
        log_event("Program terminated by user")
    finally:
        exit_flag = True
        if esp32 and esp32.is_open:
            esp32.close()
        log_event("Program exited")

main()

