# pip install pybluez pyautogui

import bluetooth
import pyautogui  # For controlling slides

# Set up Bluetooth server
server_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
server_sock.bind(("", bluetooth.PORT_ANY))
server_sock.listen(1)

# Get the port number
port = server_sock.getsockname()[1]

# Advertise the Bluetooth service
bluetooth.advertise_service(
    server_sock,
    "PresentationRemote",
    service_classes=[bluetooth.SERIAL_PORT_CLASS],
    profiles=[bluetooth.SERIAL_PORT_PROFILE],
)

print(f"Waiting for connection on RFCOMM channel {port}...")
client_sock, client_info = server_sock.accept()
print(f"Accepted connection from {client_info}")

# Function to handle slide changes
def change_slide(command):
    if command == "NEXT":
        pyautogui.press("right")  # Simulate "Next" key press
        return "OK"  # Acknowledge to ESP32
    elif command == "PREV":
        pyautogui.press("left")  # Simulate "Previous" key press
        return "OK"  # Acknowledge to ESP32
    else:
        return "UNKNOWN"

try:
    while True:
        # Wait for data from ESP32
        data = client_sock.recv(1024).decode("utf-8").strip()
        print(f"Received: {data}")

        # Change slide based on the command
        response = change_slide(data)
        
        # Send acknowledgment back to ESP32
        client_sock.send(response)
        print(f"Sent acknowledgment: {response}")

except OSError:
    print("Disconnected.")

# Close the sockets
client_sock.close()
server_sock.close()
