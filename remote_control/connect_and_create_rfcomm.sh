#!/bin/bash
#
# Simple script to automatically connect to an HC-05 / HC-06 Bluetooth module
# and create a virtual serial port (/dev/rfcomm0) ready to use with Python.
#
# Usage:
#   chmod +x connect_hc05.sh
#   sudo ./connect_hc05.sh
#
# Find your HC-05 MAC address using:
#   bluetoothctl devices
#
# Example MAC: 98:D3:31:F6:42:AB
#

# === CONFIGURATION ===
DEVICE_MAC="XX:XX:XX:XX:XX:XX"  # Replace with your HC-05 MAC address
CHANNEL=1                        # Default channel for HC-05
PORT="/dev/rfcomm0"

# === EXECUTION ===
echo "- Enabling Bluetooth..."
sudo rfkill unblock bluetooth
sudo systemctl start bluetooth

echo "- Attempting to connect to device $DEVICE_MAC ..."
echo -e "connect $DEVICE_MAC\nquit" | bluetoothctl

# Release any existing rfcomm binding
sudo rfcomm release 0 2>/dev/null

echo "- Creating virtual serial port ($PORT)..."
sudo rfcomm bind 0 "$DEVICE_MAC" $CHANNEL

# Fix permissions so non-root users can access the port
sudo chmod 666 "$PORT" 2>/dev/null

# Check if the port was successfully created
if [ -e "$PORT" ]; then
    echo "   Bluetooth connection successfully established!"
    echo "   Port available at: $PORT"
    echo "   You can now run your Python script using:"
    echo "   PORT_BT = \"$PORT\""
else
    echo "   Failed to create the serial port. Make sure the device is powered on and paired."
fi
