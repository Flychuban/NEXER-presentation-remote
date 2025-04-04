# import serial
# import pyautogui
# import time
# import sys
# import glob
# import threading
# import os
# from datetime import datetime

# # Configuration
# BAUD_RATE = 115200
# RECONNECT_INTERVAL = 5  # seconds
# KEEPALIVE_INTERVAL = 30  # seconds
# PORT_SCAN_INTERVAL = 3  # seconds
# DEBUG_MODE = True  # Enable detailed debugging

# # Global variables
# esp32 = None
# connected = False
# last_command_time = time.time()
# exit_flag = False

# def log_event(message):
#     """Log events with timestamp"""
#     timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
#     log_message = f"[{timestamp}] {message}"
#     print(log_message)
    
#     # Write to log file
#     with open("presentation_remote.log", "a") as log_file:
#         log_file.write(log_message + "\n")

# def find_esp32_port():
#     """Find the ESP32 port by searching available ports"""
#     log_event("Scanning for ESP32 Bluetooth device...")
    
#     # Look for the ESP32 device in both /dev/cu.* and /dev/tty.* patterns
#     potential_ports = glob.glob('/dev/cu.*') + glob.glob('/dev/tty.*')
    
#     # Log all found ports for debugging
#     if DEBUG_MODE:
#         log_event(f"Available ports: {', '.join(potential_ports)}")
    
#     # First try the exact name we're looking for
#     for port in potential_ports:
#         if "DIY_Presentation_Remote" in port:
#             log_event(f"Found ESP32 at {port}")
#             return port
    
#     # If exact name not found, try any Bluetooth port as fallback
#     for port in potential_ports:
#         if "Bluetooth" in port or "bluetooth" in port:
#             log_event(f"Found potential Bluetooth device at {port}")
#             return port
    
#     log_event("No ESP32 or Bluetooth device found")
#     return None

# def test_port_communication(port):
#     """Test if we can communicate with the device on this port"""
#     try:
#         test_serial = serial.Serial(port, BAUD_RATE, timeout=2)
#         log_event(f"Port opened: {port}")
        
#         # Send ping
#         log_event("Sending test PING")
#         test_serial.write(b"PING\n")
#         time.sleep(1)
        
#         # Check for response
#         if test_serial.in_waiting:
#             response = test_serial.readline().decode('utf-8', errors='replace').strip()
#             log_event(f"Received test response: '{response}'")
#             test_serial.close()
#             return response == "PONG"
#         else:
#             log_event("No response received from test")
#             test_serial.close()
#             return False
            
#     except Exception as e:
#         log_event(f"Test connection error on {port}: {str(e)}")
#         return False

# def connect_to_esp32():
#     """Try to connect to the ESP32 device"""
#     global esp32, connected
    
#     try:
#         # Find ESP32 port
#         port = find_esp32_port()
#         if not port:
#             log_event("No suitable port found. Please check your device connection.")
#             return False
        
#         log_event(f"Attempting to connect to {port}")
        
#         # Test communication first
#         if not test_port_communication(port):
#             log_event("Port test failed. Device not responding correctly.")
#             # Try anyway in case test was flawed
        
#         # Open the serial connection
#         esp32 = serial.Serial(port, BAUD_RATE, timeout=1)
#         connected = True
#         log_event(f"Connected to ESP32 via {port}")
        
#         # Initialize communication
#         for _ in range(3):  # Try a few times to establish communication
#             esp32.write(b"PING\n")
#             log_event("Sent PING handshake")
#             time.sleep(0.5)
            
#             if esp32.in_waiting:
#                 response = esp32.readline().decode('utf-8', errors='replace').strip()
#                 log_event(f"Handshake response: '{response}'")
#                 if response == "PONG":
#                     log_event("Communication verified with ESP32")
#                     return True
#                 else:
#                     log_event(f"Unexpected handshake response: '{response}'")
#             else:
#                 log_event("No handshake response received")
        
#         # If we got here without a good response, we'll try anyway
#         log_event("Proceeding with connection despite handshake issues")
#         return True
    
#     except Exception as e:
#         log_event(f"Connection error: {str(e)}")
#         connected = False
#         return False

# def flush_input_buffer():
#     """Clear any pending data in the input buffer"""
#     global esp32
    
#     if not esp32 or not connected:
#         return
        
#     try:
#         # Read all available data to clear buffer
#         if esp32.in_waiting:
#             data = esp32.read(esp32.in_waiting)
#             log_event(f"Flushed input buffer: {data.hex()}")
#     except Exception as e:
#         log_event(f"Error flushing buffer: {str(e)}")

# def keepalive_thread():
#     """Thread to periodically check connection and send keepalive signals"""
#     global esp32, connected, exit_flag
    
#     while not exit_flag:
#         if connected:
#             # Check if we haven't received commands for a while
#             if time.time() - last_command_time > KEEPALIVE_INTERVAL:
#                 try:
#                     # Send a ping to check if the connection is still alive
#                     esp32.write(b"PING\n")
#                     log_event("Sent keepalive ping")
#                     time.sleep(0.5)
                    
#                     # Check for response
#                     if esp32.in_waiting:
#                         response = esp32.readline().decode('utf-8', errors='replace').strip()
#                         log_event(f"Ping response: '{response}'")
                        
#                         if response != "PONG":
#                             log_event("Invalid ping response, reconnecting...")
#                             connected = False
#                     else:
#                         # No response, connection might be lost
#                         log_event("No ping response, reconnecting...")
#                         connected = False
                    
#                     # Check battery level periodically
#                     if connected:
#                         esp32.write(b"BATTERY?\n")
#                         log_event("Sent battery check")
#                         time.sleep(0.5)
                        
#                         if esp32.in_waiting:
#                             response = esp32.readline().decode('utf-8', errors='replace').strip()
#                             log_event(f"Battery response: '{response}'")
                            
#                             if response.startswith("BATTERY:"):
#                                 battery_level = response.split(":")[1]
#                                 log_event(f"ESP32 Battery Level: {battery_level}%")
                    
#                 except Exception as e:
#                     log_event(f"Keepalive error: {str(e)}")
#                     connected = False
        
#         # Sleep for a while before next check
#         time.sleep(KEEPALIVE_INTERVAL / 3)

# def handle_command(command):
#     """Process commands from the ESP32"""
#     global last_command_time
    
#     last_command_time = time.time()
#     log_event(f"Processing command: '{command}'")
    
#     if command == "NEXT":
#         log_event("Executing NEXT command: right arrow")
#         pyautogui.press("right")
#         return True
#     elif command == "PREV":
#         log_event("Executing PREV command: left arrow")
#         pyautogui.press("left")
#         return True
#     elif command == "SLEEP":
#         log_event("ESP32 entering sleep mode")
#         return True
#     elif command == "PONG":
#         log_event("Received PONG response")
#         return True
#     else:
#         log_event(f"Unknown command: '{command}'")
#         return False

# def send_acknowledgment():
#     """Send acknowledgment to ESP32"""
#     global esp32
    
#     try:
#         esp32.write(b"OK\n")
#         log_event("Sent OK acknowledgment")
#     except Exception as e:
#         log_event(f"Error sending acknowledgment: {str(e)}")

# def main():
#     """Main function to handle connection and commands"""
#     global esp32, connected, last_command_time, exit_flag
    
#     log_event("DIY Presentation Remote Receiver Starting")
    
#     # Start keepalive thread
#     threading.Thread(target=keepalive_thread, daemon=True).start()
    
#     try:
#         while not exit_flag:
#             # If not connected, try to connect
#             if not connected:
#                 if connect_to_esp32():
#                     # Successfully connected
#                     last_command_time = time.time()
#                 else:
#                     # Wait before trying again
#                     log_event(f"Will try to reconnect in {RECONNECT_INTERVAL} seconds...")
#                     time.sleep(RECONNECT_INTERVAL)
#                     continue
            
#             # Check for commands
#             try:
#                 if esp32.in_waiting > 0:
#                     bytes_waiting = esp32.in_waiting
#                     log_event(f"ESP32 has {bytes_waiting} bytes waiting")
                    
#                     # Read a line and process it
#                     raw_data = esp32.readline()
                    
#                     # Check if we got any data
#                     if len(raw_data) > 0:
#                         # Log the raw data for debugging
#                         log_event(f"Raw data received: {raw_data.hex()}")
                        
#                         # Try to decode as UTF-8
#                         try:
#                             command = raw_data.decode('utf-8', errors='replace').strip()
#                             log_event(f"Decoded command: '{command}' (length: {len(command)})")
                            
#                             if command:
#                                 if handle_command(command):
#                                     send_acknowledgment()
#                         except Exception as e:
#                             log_event(f"Error decoding command: {str(e)}")
                
#                 # A short delay to reduce CPU usage
#                 time.sleep(0.1)
                
#             except Exception as e:
#                 log_event(f"Error reading from serial: {str(e)}")
#                 connected = False
#                 if esp32:
#                     try:
#                         esp32.close()
#                     except:
#                         pass
#                 time.sleep(RECONNECT_INTERVAL)
    
#     except KeyboardInterrupt:
#         log_event("Program terminated by user")
#     finally:
#         exit_flag = True
#         if esp32 and esp32.is_open:
#             esp32.close()
#         log_event("Program exited")

# if __name__ == "__main__":
#     main()


import serial
import glob
import time
import threading

BAUD_RATE = 115200
RECONNECT_INTERVAL = 5  # seconds

def find_esp32_port():
    potential_ports = glob.glob('/dev/cu.*') + glob.glob('/dev/tty.*')
    for port in potential_ports:
        if "DIY_Presentation_Remote" in port:
            return port
    return None

def connect():
    port = find_esp32_port()
    if port is None:
        print("No port found!")
        return None
    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=1)
        print(f"Connected to {port}")
        return ser
    except Exception as e:
        print("Error opening port:", e)
        return None

def read_from_port(ser):
    while True:
        if ser.in_waiting:
            try:
                line = ser.readline().decode('utf-8', errors='replace').strip()
                if line:
                    print("Received:", line)
            except Exception as e:
                print("Error decoding data:", e)
        time.sleep(0.1)

def main():
    ser = None
    while ser is None:
        ser = connect()
        if ser is None:
            print("Retrying in", RECONNECT_INTERVAL, "seconds...")
            time.sleep(RECONNECT_INTERVAL)
    
    threading.Thread(target=read_from_port, args=(ser,), daemon=True).start()
    
    while True:
        ser.write(b"PING\n")
        print("Sent: PING")
        time.sleep(2)

if __name__ == "__main__":
    main()

