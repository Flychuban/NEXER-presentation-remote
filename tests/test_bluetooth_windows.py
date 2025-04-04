import serial
import serial.tools.list_ports
import time
import threading
import sys

BAUD_RATE = 9600
RECONNECT_INTERVAL = 5  # seconds

def find_esp32_port():
    """
    Looks for a serial device with 'DIY_PRESENTATION_REMOTE' in its description.
    """
    ports = serial.tools.list_ports.comports()
    for port in ports:
        print(f"Port: {port.device}, Description: {port.description}")  # Debugging line
        if "DIY_PRESENTATION_REMOTE" in port.description:  # Update this line
            return port.device  # Returns something like 'COM3'
    return None



def connect():
    port = "/dev/tty.DIY_Presentation_Remote"  # Default port for macOS
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
    
    # threading.Thread(target=read_from_port, args=(ser,), daemon=True).start()
    # read_from_port(ser)
    i = 0
    while i < 5:
        try:
            print("Sent: PING")
            ser.write(b"PING\n")
            print("Sent: PING")
            i +=1
            
        except Exception as e:
            print("Error writing to serial:", e)
            break
        time.sleep(2)
    
    read_from_port(ser)
    

if __name__ == "__main__":
    main()