#!/bin/bash
#
# Simple script to create a Bluetooth virtual serial port (rfcomm)
# for serial communication via Python or another program.
#
# Usage:
# 1. Make executable: chmod +x create_rfcomm.sh
# 2. Run with sudo: sudo ./create_rfcomm.sh

# Note:
# Replace the MAC address below with that of your device.
# You can find the MAC address with the command: bluetoothctl devices

# === CONFIGURAÇÃO ===
DEVICE_MAC="XX:XX:XX:XX:XX:XX"   # Enter the actual MAC address of your device here
CHANNEL=1                        # normally 1
PORT="/dev/rfcomm0"              # virtual port name

# === EXECUÇÃO ===
echo "  Checking if there is an active rfcomm port...."
sudo rfcomm release 0 2>/dev/null

echo "  Establishing a connection with the Bluetooth device $DEVICE_MAC..."
sudo rfcomm bind 0 "$DEVICE_MAC" $CHANNEL
sudo chmod 666 /dev/rfcomm0

if [ $? -eq 0 ]; then
    echo "   Door successfully created: $PORT"
    echo "   Now you can use it in Python:"
    echo "   PORTA_BT = \"$PORT\""
else
    echo "Error creating the port. Check if the device is paired and turned on."
fi
