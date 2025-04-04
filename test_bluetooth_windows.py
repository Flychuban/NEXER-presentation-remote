import serial
import serial.tools.list_ports
import time
import threading
import sys

BAUD_RATE = 115200
RECONNECT_INTERVAL = 5  # seconds

def find_esp32_port():
    """
    Looks for a serial device with 'DIY_Presentation_Remote' in its description.
    Update this if your device shows a different identifier.
    """
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "DIY_Presentation_Remote" in port.description or "ESP32" in port.description:
            return port.device  # Returns something like 'COM3'
    return None

def connect():
    port = find_esp32_port()
    if port is None:
        print("No suitable COM port found!")
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
            print(f"Retrying in {RECONNECT_INTERVAL} seconds...")
            time.sleep(RECONNECT_INTERVAL)
    
    threading.Thread(target=read_from_port, args=(ser,), daemon=True).start()
    
    while True:
        try:
            ser.write(b"PING\n")
            print("Sent: PING")
        except Exception as e:
            print("Error writing to serial:", e)
            break
        time.sleep(2)

if __name__ == "__main__":
    main()
