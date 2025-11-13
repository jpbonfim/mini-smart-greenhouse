# Bluetooth Module ZS-040 (HC-05/HC-06) Wiring Guide

## Component List
- Arduino Uno/Nano
- LCD 1602A (Parallel mode)
- Bluetooth Module ZS-040 (HC-05 or HC-06)
- 10K Potentiometer (for LCD contrast)
- 1K Resistor (for voltage divider)
- 2K Resistor (for voltage divider)
- 220Ω Resistor (for LCD backlight)
- Breadboard and jumper wires

---

## LCD 1602A Wiring (Parallel - 4-bit mode)

| LCD Pin | Function | Arduino Pin |
|---------|----------|-------------|
| VSS     | Ground   | GND         |
| VDD     | Power    | 5V          |
| V0      | Contrast | Potentiometer center pin |
| RS      | Register Select | Pin 12 |
| RW      | Read/Write | GND      |
| E       | Enable   | Pin 11      |
| D4      | Data 4   | Pin 5       |
| D5      | Data 5   | Pin 4       |
| D6      | Data 6   | Pin 3       |
| D7      | Data 7   | Pin 2       |
| A       | Backlight + | 5V (through 220Ω resistor) |
| K       | Backlight - | GND      |

**Potentiometer Wiring (for LCD contrast):**
- Left pin → GND
- Center pin → LCD V0
- Right pin → 5V

---

## Bluetooth Module ZS-040 (HC-05/HC-06) Wiring

### ⚠️ IMPORTANT: Voltage Divider for RX Pin

The Bluetooth module operates at **3.3V logic**, but Arduino Uno/Nano uses **5V logic**. You **MUST** use a voltage divider to protect the Bluetooth RX pin.

### Basic Connections:

| Bluetooth Pin | Function | Connection |
|---------------|----------|------------|
| VCC (or 5V)   | Power    | Arduino 5V |
| GND           | Ground   | Arduino GND |
| TX            | Transmit | Arduino Pin 10 (SoftwareSerial RX) - Direct |
| RX            | Receive  | Arduino Pin 9 (SoftwareSerial TX) - **Through Voltage Divider** |

### Voltage Divider Circuit for Bluetooth RX:

```
Arduino Pin 9 ----[1K Resistor]----+---- Bluetooth RX
                                   |
                              [2K Resistor]
                                   |
                                  GND
```

This divider converts 5V from Arduino to approximately 3.3V for the Bluetooth module:
- Output voltage = 5V × (2K / (1K + 2K)) = 3.33V

---

## Complete Wiring Diagram (Text Format)

```
ARDUINO UNO/NANO
================

Digital Pins:
  Pin 2  → LCD D7
  Pin 3  → LCD D6
  Pin 4  → LCD D5
  Pin 5  → LCD D4
  Pin 9  → [1K Resistor] → BT RX (with 2K to GND)
  Pin 10 → BT TX (direct connection)
  Pin 11 → LCD E
  Pin 12 → LCD RS

Power:
  5V  → LCD VDD, LCD Backlight (through 220Ω), BT VCC, Potentiometer
  GND → LCD VSS, LCD RW, LCD K, BT GND, Potentiometer, Voltage Divider

Analog Pins:
  (Not used in this project)
```

---

## Assembly Steps

1. **Mount LCD on breadboard:**
   - Connect all LCD pins according to the table above
   - Connect potentiometer for contrast adjustment
   - Add 220Ω resistor for backlight

2. **Add Bluetooth Module:**
   - Connect VCC to 5V
   - Connect GND to GND
   - Connect TX directly to Arduino Pin 10

3. **Create Voltage Divider:**
   - Connect 1K resistor from Arduino Pin 9
   - Connect the other end to Bluetooth RX
   - Connect 2K resistor from the junction to GND

4. **Test Connections:**
   - Verify all power connections
   - Check voltage divider output (should be ~3.3V)
   - Ensure no short circuits

---

## Bluetooth Module Configuration

### Default Settings (HC-05/HC-06):
- Baud Rate: 9600 (default for most modules)
- Name: HC-05 or HC-06
- PIN: 1234 or 0000

### Pairing with Computer/Phone:
1. Power on the Bluetooth module
2. Search for Bluetooth devices on your computer
3. Connect to "HC-05" or "HC-06"
4. Enter PIN when prompted (usually 1234 or 0000)

### Linux: Creating RFCOMM Connection
```bash
# Find Bluetooth MAC address
hcitool scan

# Pair with device
sudo bluetoothctl
pair XX:XX:XX:XX:XX:XX
connect XX:XX:XX:XX:XX:XX

# Create serial port binding
sudo rfcomm bind /dev/rfcomm0 XX:XX:XX:XX:XX:XX 1

# Check if connection is established
ls -l /dev/rfcomm0
```

---

## Testing the Connection

1. **Upload Arduino code** to your board
2. **Open Serial Monitor** (9600 baud) to see debug messages
3. **Pair your computer** with the Bluetooth module
4. **Create RFCOMM binding** (Linux) or connect via COM port (Windows)
5. **Run the Flask app:**
   ```bash
   cd remote_control
   python3 app.py
   ```
6. **Open browser** to `http://localhost:5000`
7. **Click preset buttons** to test communication

---

## Troubleshooting

### LCD not displaying:
- Check contrast adjustment (turn potentiometer)
- Verify all data pin connections
- Ensure 5V and GND are connected

### Bluetooth not connecting:
- Verify voltage divider is correct
- Check if LED on BT module is blinking (not paired) or solid (paired)
- Ensure baud rate matches (9600)
- Try different RFCOMM channel (usually 1)

### Arduino not responding to commands:
- Open Serial Monitor to see if commands are received
- Check if Bluetooth module RX/TX are reversed
- Verify voltage divider connections

### "No serial connection" error in Flask app:
- Check RFCOMM binding: `ls -l /dev/rfcomm0`
- Update `BT_PORT` in `app.py` to match your port
- Ensure no other program is using the port

---

## Command Reference

The Arduino accepts the following Bluetooth commands:

| Command | Description | Example |
|---------|-------------|---------|
| `PRESET:Name` | Change to specific preset | `PRESET:Basil` |
| `NEXT` | Switch to next preset | `NEXT` |
| `STATUS` | Get current status | `STATUS` |

**Response Format:**
- Success: `OK: Changed to Basil`
- Error: `ERROR: Unknown preset - XYZ`
- Status: `Current: Basil | T:25C L:18h I:10m`

---

## Safety Notes

⚠️ **Always use the voltage divider** for Bluetooth RX - connecting 5V directly can damage the module!

⚠️ **Check polarity** before connecting power - reversed polarity can destroy components!

⚠️ **Use current-limiting resistor** for LCD backlight to prevent burnout!

---

## Additional Resources

- [HC-05 AT Commands Guide](https://www.instructables.com/AT-command-mode-of-HC-05-Bluetooth-module/)
- [Arduino SoftwareSerial Library](https://www.arduino.cc/en/Reference/SoftwareSerial)
- [Bluetooth Serial on Linux](https://www.linux.org/threads/bluetooth-serial-port.11643/)
