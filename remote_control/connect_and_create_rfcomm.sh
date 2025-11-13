#!/bin/bash
#
# Bluetooth Connection Script for ZS-040 (HC-05/HC-06) Module
# Automatically pairs, connects, and creates a virtual serial port
#
# Usage:
#   1. Edit DEVICE_MAC below with your Bluetooth module's MAC address
#   2. Make executable: chmod +x connect_and_create_rfcomm.sh
#   3. Run: sudo ./connect_and_create_rfcomm.sh
#
# Find your HC-05/HC-06 MAC address using:
#   hcitool scan
#   OR
#   bluetoothctl devices
#
# Example MAC: 98:D3:31:F6:42:AB
#

# === CONFIGURATION ===
DEVICE_MAC="98:D3:51:F9:A6:F1"  # Replace with your HC-05/HC-06 MAC address
DEVICE_NAME="HC-06"              # Device name (for reference only)
CHANNEL=1                        # Default channel for HC-05/HC-06
PORT="/dev/rfcomm0"              # Virtual serial port to create

# === COLOR CODES ===
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# === CHECK IF RUNNING AS ROOT ===
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Error: This script must be run as root (use sudo)${NC}"
    exit 1
fi

# === CHECK CONFIGURATION ===
if [ "$DEVICE_MAC" = "XX:XX:XX:XX:XX:XX" ]; then
    echo -e "${RED}Error: Please edit this script and set DEVICE_MAC to your device's MAC address${NC}"
    echo "Find it using: hcitool scan"
    exit 1
fi

# === MAIN SCRIPT ===
echo -e "${YELLOW}=== Bluetooth Connection Script ===${NC}"
echo "Device: $DEVICE_NAME ($DEVICE_MAC)"
echo "Port: $PORT"
echo ""

# Enable Bluetooth
echo -e "${YELLOW}[1/5]${NC} Enabling Bluetooth..."
rfkill unblock bluetooth
systemctl start bluetooth
sleep 2

# Scan for device
echo -e "${YELLOW}[2/5]${NC} Scanning for device..."
SCAN_RESULT=$(hcitool scan | grep -i "$DEVICE_MAC")
if [ -z "$SCAN_RESULT" ]; then
    echo -e "${RED}Warning: Device not found in scan. Make sure it's powered on.${NC}"
else
    echo -e "${GREEN}✓ Device found: $SCAN_RESULT${NC}"
fi

# Pair device
echo -e "${YELLOW}[3/5]${NC} Pairing device..."
bluetoothctl <<EOF
power on
agent on
default-agent
pair $DEVICE_MAC
trust $DEVICE_MAC
connect $DEVICE_MAC
exit
EOF

sleep 2

# Release any existing rfcomm binding
echo -e "${YELLOW}[4/5]${NC} Releasing old connections..."
rfcomm release 0 2>/dev/null

# Create virtual serial port
echo -e "${YELLOW}[5/5]${NC} Creating virtual serial port..."
rfcomm bind 0 "$DEVICE_MAC" $CHANNEL

# Fix permissions
if [ -e "$PORT" ]; then
    chmod 666 "$PORT"
fi

# Verify connection
echo ""
if [ -e "$PORT" ]; then
    echo -e "${GREEN}✓ SUCCESS! Bluetooth connection established!${NC}"
    echo -e "${GREEN}✓ Port available at: $PORT${NC}"
    echo ""
    echo "You can now run the Flask app:"
    echo "  cd /home/joao/projects/mini-smart-greenhouse/remote_control"
    echo "  python3 app.py"
    echo ""
    echo "To test the connection manually:"
    echo "  echo 'STATUS' > $PORT"
    echo "  cat $PORT"
else
    echo -e "${RED}✗ FAILED to create serial port${NC}"
    echo "Troubleshooting:"
    echo "  1. Check if device is powered on"
    echo "  2. Verify MAC address is correct"
    echo "  3. Try pairing manually: bluetoothctl"
    echo "  4. Check if module LED is blinking (unpaired) or solid (paired)"
    exit 1
fi
