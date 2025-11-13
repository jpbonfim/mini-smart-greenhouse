# 🌿 Smart Greenhouse Controller with Bluetooth

An Arduino-based greenhouse controller with Bluetooth connectivity, LCD display, and web interface for remote plant preset management.

## 📋 Features

- **Bluetooth Control**: Change plant presets remotely via ZS-040 (HC-05/HC-06) module
- **LCD Display**: Real-time display of current preset and parameters
- **Web Interface**: Modern, responsive web UI for easy control
- **Multiple Presets**: Pre-configured settings for different plants (Basil, Cilantro, Tomato)
- **Serial Debugging**: Monitor commands and responses via Serial Monitor
- **Extensible**: Easy to add new presets and sensors

## 🛠️ Hardware Requirements

- Arduino Uno or Nano
- LCD 1602A (16x2 character display)
- Bluetooth Module ZS-040 (HC-05 or HC-06)
- 10K Potentiometer (for LCD contrast)
- Resistors:
  - 1x 1KΩ (voltage divider)
  - 1x 2KΩ (voltage divider)
  - 1x 220Ω (LCD backlight)
- Breadboard and jumper wires
- USB cable for Arduino

## 💻 Software Requirements

- Arduino IDE (1.8.x or newer)
- Python 3.7+
- Flask
- pySerial

## 📦 Installation

### 1. Clone Repository
```bash
git clone https://github.com/jpbonfim/mini-smart-greenhouse.git
cd mini-smart-greenhouse
```

### 2. Hardware Setup
Follow the detailed wiring guide in:
- **Visual Diagram**: `WIRING_DIAGRAM.txt`
- **Detailed Guide**: `BLUETOOTH_WIRING.md`

**Critical:** Use voltage divider for Bluetooth RX pin!

### 3. Upload Arduino Code
```bash
# Open Arduino IDE
# File → Open → microcontroller/greenhouse/greenhouse.ino
# Select your board and port
# Click Upload
```

### 4. Pair Bluetooth Module

**Linux:**
```bash
# Find MAC address
hcitool scan

# Use the connection script (recommended)
cd remote_control
sudo ./connect_and_create_rfcomm.sh
```

**Windows:**
- Settings → Bluetooth → Add Device
- Pair with HC-05/HC-06 (PIN: usually 1234)
- Note the COM port (e.g., COM5)

### 5. Install Python Dependencies
```bash
cd remote_control
pip install -r requirements.txt
```

### 6. Configure Serial Port
Edit `remote_control/app.py` line 13:
```python
BT_PORT = "/dev/rfcomm0"  # Linux
# BT_PORT = "COM5"        # Windows - uncomment and update
```

### 7. Run Web Application
```bash
python3 app.py
```

Open browser: http://localhost:5000

## 🎮 Usage

### Web Interface
1. Open http://localhost:5000
2. Click preset buttons to change plant configuration
3. Use "Next Preset" to cycle through presets
4. Click "Get Status" to query current settings

### Bluetooth Commands
Send these commands directly via Bluetooth:

| Command | Description | Example |
|---------|-------------|---------|
| `PRESET:Name` | Change to specific preset | `PRESET:Basil` |
| `NEXT` | Cycle to next preset | `NEXT` |
| `STATUS` | Get current configuration | `STATUS` |

### Available Presets

| Plant | Temperature | Lighting | Irrigation |
|-------|------------|----------|------------|
| Basil | 25°C | 18h/day | 10min/day |
| Cilantro | 20°C | 12h/day | 15min/day |
| Tomato | 22°C | 16h/day | 20min/day |

## 📐 Wiring Diagram

### Bluetooth Module (Critical!)
```
Arduino Pin 9 ──[1KΩ]──┬── Bluetooth RX
                        │
                    [2KΩ]
                        │
                       GND
```

Arduino Pin 10 ── Bluetooth TX (direct)

### LCD Connections
- RS → Pin 12
- E → Pin 11
- D4-D7 → Pins 6, 5, 4, 3
- Power: VDD→5V, VSS→GND
- Contrast: V0→Potentiometer

See `WIRING_DIAGRAM.txt` for complete visual diagram.

## 🔧 Troubleshooting

### LCD Issues
- **Blank display**: Adjust contrast potentiometer
- **Random characters**: Check data pin connections

### Bluetooth Issues
- **Won't pair**: Try PIN 1234, 0000, or 1111
- **No response**: Verify voltage divider (1K + 2K)
- **Can't connect**: Ensure LED is blinking on module

### Connection Issues
- **Linux**: Verify RFCOMM: `ls -l /dev/rfcomm0`
- **Windows**: Check Device Manager for COM port
- **"No serial connection"**: Ensure port isn't used by another app

See `QUICK_START.md` for detailed troubleshooting.

## 📁 Project Structure

```
mini-smart-greenhouse/
├── microcontroller/
│   └── greenhouse/
│       └── greenhouse.ino          # Arduino code with Bluetooth
├── remote_control/
│   ├── app.py                      # Flask web server
│   ├── requirements.txt            # Python dependencies
│   ├── templates/
│   │   └── index.html             # Web interface
│   └── connect_and_create_rfcomm.sh  # Linux BT connection script
├── BLUETOOTH_WIRING.md            # Detailed wiring guide
├── WIRING_DIAGRAM.txt             # ASCII wiring diagram
├── QUICK_START.md                 # Quick start guide
├── RESUMO_PT.md                   # Portuguese summary
└── README.md                      # This file
```

## 🚀 Future Enhancements

- [ ] Add real sensors (DHT22, soil moisture)
- [ ] Implement relay control for lights and pumps
- [ ] Data logging to SD card
- [ ] Mobile app (React Native/Flutter)
- [ ] Multiple plant zones
- [ ] Automatic scheduling
- [ ] Email/SMS alerts

## 📝 Adding Custom Presets

Edit `microcontroller/greenhouse/greenhouse.ino`:

```cpp
const int NUM_PRESETS = 4;  // Increase count

Preset presets[NUM_PRESETS] = {
  {"Basil", 25, 18, 10},
  {"Cilantro", 20, 12, 15},
  {"Tomato", 22, 16, 20},
  {"Lettuce", 18, 14, 12}  // Add new preset
};
```

Update `remote_control/templates/index.html`:
```html
<button class="preset-btn" data-preset="Lettuce">
  🥬 Lettuce
</button>
```

## 📜 License

This project is open source and available under the MIT License.

## 👤 Author

João Bonfim ([@jpbonfim](https://github.com/jpbonfim))

## 🙏 Acknowledgments

- Arduino community for libraries and examples
- Flask documentation and tutorials
- HC-05/HC-06 module manufacturers

## 📞 Support

For issues and questions:
- Check documentation files in this repository
- Open an issue on GitHub
- Refer to troubleshooting sections in QUICK_START.md

---

**Happy Growing! 🌱**
