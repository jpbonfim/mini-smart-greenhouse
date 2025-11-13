# Quick Start Guide - Bluetooth Greenhouse Controller

## 🚀 Quick Setup

### 1. Hardware Assembly
- Follow `WIRING_DIAGRAM.txt` for complete connections
- **Critical:** Use voltage divider for Bluetooth RX pin (1K + 2K resistors)
- Adjust LCD contrast with potentiometer until text is visible

### 2. Upload Arduino Code
```bash
# Open Arduino IDE
# File → Open → microcontroller/greenhouse/greenhouse.ino
# Select correct board and port
# Click Upload
```

### 3. Pair Bluetooth Module
**Linux:**
```bash
# Scan for Bluetooth devices
hcitool scan

# Note the MAC address (XX:XX:XX:XX:XX:XX)
# Pair and connect
sudo bluetoothctl
pair XX:XX:XX:XX:XX:XX
connect XX:XX:XX:XX:XX:XX
exit

# Create serial port binding
sudo rfcomm bind /dev/rfcomm0 XX:XX:XX:XX:XX:XX 1

# Verify connection
ls -l /dev/rfcomm0
```

**Windows:**
- Settings → Bluetooth → Add Device
- Select HC-05 or HC-06
- Enter PIN (usually 1234 or 0000)
- Note the COM port assigned (e.g., COM5)

### 4. Configure Flask App
Edit `remote_control/app.py` line 13:
```python
# Linux
BT_PORT = "/dev/rfcomm0"

# Windows
BT_PORT = "COM5"  # Replace with your COM port
```

### 5. Install Python Dependencies
```bash
cd remote_control
pip install -r requirements.txt
```

### 6. Run the Web Interface
```bash
python3 app.py
```

Open browser: http://localhost:5000

---

## 📡 Bluetooth Commands

The Arduino accepts these commands via Bluetooth:

| Command | Description | Example | Response |
|---------|-------------|---------|----------|
| `PRESET:Name` | Change to specific preset | `PRESET:Basil` | `OK: Changed to Basil` |
| `NEXT` | Switch to next preset (cycles) | `NEXT` | `OK: Changed to Cilantro` |
| `STATUS` | Get current configuration | `STATUS` | `Current: Basil \| T:25C L:18h I:10m` |

### Available Presets:
- **Basil**: 25°C, 18 hours light, 10 minutes irrigation
- **Cilantro**: 20°C, 12 hours light, 15 minutes irrigation  
- **Tomato**: 22°C, 16 hours light, 20 minutes irrigation

---

## 🌐 Web Interface Usage

1. **Open** http://localhost:5000 in your browser
2. **Click** a preset button (Basil, Cilantro, or Tomato)
3. **Watch** the LCD update with new settings
4. **Use** "Next Preset" to cycle through presets
5. **Click** "Get Status" to query current configuration

---

## 🔧 Troubleshooting

### LCD shows nothing
- **Fix:** Adjust contrast potentiometer (turn slowly)
- **Check:** Power connections (VDD=5V, VSS=GND)

### LCD shows random characters
- **Check:** All data pins (D4-D7) are connected correctly
- **Verify:** RS and E pins match code (D12, D11)

### Bluetooth won't pair
- **Check:** LED on module is blinking (unpaired state)
- **Try:** Default PIN codes: 1234, 0000, or 1111
- **Verify:** Module has power (VCC=5V, GND)

### No response to Bluetooth commands
- **Verify:** Voltage divider is correct (1K + 2K)
- **Check:** TX/RX are not swapped
- **Test:** Open Serial Monitor (9600 baud) to see debug messages

### "No serial connection" error
- **Linux:** Verify RFCOMM binding: `ls -l /dev/rfcomm0`
- **Windows:** Check Device Manager for correct COM port
- **Both:** Ensure port is not used by another program

### Commands not working
- **Format:** Must include `\n` at end (Flask app does this)
- **Case:** Command names are case-insensitive
- **Spelling:** Preset names must match exactly

---

## 🧪 Testing Commands Manually

### Using Python:
```python
import serial
import time

ser = serial.Serial('/dev/rfcomm0', 9600, timeout=1)
time.sleep(2)

# Send command
ser.write(b'PRESET:Basil\n')

# Read response
response = ser.readline()
print(response.decode())

ser.close()
```

### Using Serial Monitor:
1. Open Arduino IDE Serial Monitor
2. Set baud rate to 9600
3. Watch for incoming Bluetooth commands
4. Debug messages will appear here

### Using Terminal (Linux):
```bash
# Send command
echo "PRESET:Tomato" > /dev/rfcomm0

# Read response
cat /dev/rfcomm0
```

---

## 📝 Adding New Presets

Edit `microcontroller/greenhouse/greenhouse.ino`:

```cpp
// Change NUM_PRESETS
const int NUM_PRESETS = 4;  // Add more presets

// Add new preset to array
Preset presets[NUM_PRESETS] = {
  {"Basil", 25, 18, 10},
  {"Cilantro", 20, 12, 15},
  {"Tomato", 22, 16, 20},
  {"Lettuce", 18, 14, 12}  // New preset
};
```

Update `remote_control/templates/index.html`:

```html
<button class="preset-btn" data-preset="Lettuce">
  🥬 Lettuce
</button>
```

---

## 🔒 Security Notes

- Default Bluetooth modules have **no security**
- Anyone nearby can connect and send commands
- For production, use Bluetooth pairing and authentication
- Consider changing default PIN in AT mode

### Changing Bluetooth PIN (HC-05):
```
1. Enter AT mode (connect KEY pin to VCC before power)
2. Send: AT+PSWD=9999
3. New PIN is 9999
```

---

## 📊 Serial Debug Output

When commands are received, you'll see:
```
Greenhouse Controller Ready
Waiting for Bluetooth commands...
Received command: PRESET:Basil
Changed to preset: Basil
=== Current Preset ===
Name: Basil
Temperature: 25°C
Lighting: 18 hours/day
Irrigation: 10 minutes/day
=====================
```

---

## 🎯 Next Steps

- Add actual sensors (temperature, humidity, soil moisture)
- Implement relay control for lights and irrigation
- Add data logging to SD card
- Create mobile app instead of web interface
- Add multiple plant zones with individual control

---

## 📞 Support

Check these files for more information:
- `BLUETOOTH_WIRING.md` - Detailed wiring guide
- `WIRING_DIAGRAM.txt` - ASCII wiring diagram
- `microcontroller/greenhouse/greenhouse.ino` - Arduino source code
- `remote_control/app.py` - Flask web server

Happy Growing! 🌱
