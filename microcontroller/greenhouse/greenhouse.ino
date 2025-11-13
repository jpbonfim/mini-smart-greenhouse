/*
 * Automatic Greenhouse Controller with Bluetooth Control
 * 
 * LCD 1602A Wiring (Parallel - 4-bit mode):
 * - VSS -> GND
 * - VDD -> 5V
 * - V0 -> Potentiometer (10K) center pin (for contrast adjustment)
 * - RS -> Pin 12
 * - RW -> GND
 * - E -> Pin 11
 * - D4 -> Pin 5
 * - D5 -> Pin 4
 * - D6 -> Pin 3
 * - D7 -> Pin 2
 * - A (Backlight +) -> 5V (through 220Ω resistor)
 * - K (Backlight -) -> GND
 * 
 * Bluetooth Module ZS-040 (HC-05/HC-06) Wiring:
 * - VCC -> 5V (or 3.3V depending on module)
 * - GND -> GND
 * - TX -> Pin 10 (Arduino RX via SoftwareSerial)
 * - RX -> Pin 9 (Arduino TX via SoftwareSerial) - Use voltage divider (1K + 2K resistors) to convert 5V to 3.3V
 * 
 * Voltage Divider for RX pin (to protect Bluetooth module):
 * Arduino Pin 9 -> 1K resistor -> Bluetooth RX
 *                                  |
 *                                2K resistor -> GND
 * 
 * Note: Most HC-05/HC-06 modules can handle 5V on VCC, but RX pin needs 3.3V
 */

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// LCD pins initialization (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Bluetooth module pins (RX, TX)
SoftwareSerial btSerial(10, 9); // RX=10, TX=9

// LM35 temperature sensor pin
const int LM35_PIN = A0;

// Temperature reading variables
float currentTemperature = 0.0;
unsigned long lastTempUpdate = 0;
const unsigned long TEMP_UPDATE_INTERVAL = 5000; // 5 seconds

// Preset structure
struct Preset {
  const char* name;
  int temperature;  // °C
  int lighting;     // hours per day
  int irrigation;   // minutes per day
};

// Define presets
const int NUM_PRESETS = 3;
Preset presets[NUM_PRESETS] = {
  {"Basil", 25, 18, 10},
  {"Cilantro", 20, 12, 15},
  {"Tomato", 22, 16, 20}
};

// Active preset index
int activePreset = 0;

// Bluetooth command buffer
String btCommand = "";

// Display update tracking
int lastDisplayedPreset = -1;

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  
  // Initialize Bluetooth serial communication
  btSerial.begin(9600);
  
  // Initialize LCD (16 columns, 2 rows)
  lcd.begin(16, 2);
  
  // Configure LM35 pin
  pinMode(LM35_PIN, INPUT);
  
  // Display welcome message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Greenhouse");
  lcd.setCursor(0, 1);
  lcd.print("Bluetooth Ready");
  delay(2000);
  
  // Read initial temperature
  readTemperature();
  
  // Display initial preset
  displayPreset();
  
  Serial.println("Greenhouse Controller Ready");
  Serial.println("Waiting for Bluetooth commands...");
  btSerial.println("Greenhouse Controller Ready");
}

void loop() {
  // Check for Bluetooth commands
  if (btSerial.available()) {
    char c = btSerial.read();
    
    if (c == '\n' || c == '\r') {
      // Process complete command
      if (btCommand.length() > 0) {
        processBluetoothCommand(btCommand);
        btCommand = "";
      }
    } else {
      // Build command string
      btCommand += c;
    }
  }
  
  // Update temperature reading every 5 seconds (non-blocking)
  unsigned long currentMillis = millis();
  if (currentMillis - lastTempUpdate >= TEMP_UPDATE_INTERVAL) {
    lastTempUpdate = currentMillis;
    readTemperature();
    displayPreset(); // Update display with new temperature
  }
  
  // Update display if preset changed
  if (activePreset != lastDisplayedPreset) {
    displayPreset();
    lastDisplayedPreset = activePreset;
  }
}

void processBluetoothCommand(String command) {
  command.trim(); // Remove whitespace
  
  Serial.print("Received command: ");
  Serial.println(command);
  
  // Check for preset change command
  if (command.startsWith("PRESET:")) {
    String presetName = command.substring(7); // Get text after "PRESET:"
    presetName.trim();
    
    // Try to match preset name
    bool found = false;
    for (int i = 0; i < NUM_PRESETS; i++) {
      if (presetName.equalsIgnoreCase(presets[i].name)) {
        activePreset = i;
        found = true;
        
        Serial.print("Changed to preset: ");
        Serial.println(presets[activePreset].name);
        
        btSerial.print("OK: Changed to ");
        btSerial.println(presets[activePreset].name);
        
        // Brief feedback on LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("BT: Changing...");
        delay(500);
        break;
      }
    }
    
    if (!found) {
      Serial.print("Unknown preset: ");
      Serial.println(presetName);
      btSerial.print("ERROR: Unknown preset - ");
      btSerial.println(presetName);
      
      // Show error on LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("BT: Unknown");
      lcd.setCursor(0, 1);
      lcd.print("preset!");
      delay(1500);
    }
  }
  // Check for next preset command
  else if (command.equalsIgnoreCase("NEXT")) {
    activePreset = (activePreset + 1) % NUM_PRESETS;
    
    Serial.print("Changed to next preset: ");
    Serial.println(presets[activePreset].name);
    
    btSerial.print("OK: Changed to ");
    btSerial.println(presets[activePreset].name);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BT: Next preset");
    delay(500);
  }
  // Check for status request
  else if (command.equalsIgnoreCase("STATUS")) {
    btSerial.print("Current: ");
    btSerial.print(presets[activePreset].name);
    btSerial.print(" | T:");
    btSerial.print(presets[activePreset].temperature);
    btSerial.print("C L:");
    btSerial.print(presets[activePreset].lighting);
    btSerial.print("h I:");
    btSerial.print(presets[activePreset].irrigation);
    btSerial.println("m");
  }
  // Check for temperature request
  else if (command.equalsIgnoreCase("TEMP")) {
    btSerial.print("TEMP:");
    btSerial.println(currentTemperature, 1);
  }
  else {
    Serial.print("Unknown command: ");
    Serial.println(command);
    btSerial.println("ERROR: Unknown command");
  }
}

void changePreset() {
  // Cycle to next preset
  activePreset = (activePreset + 1) % NUM_PRESETS;
  
  // Debug output
  Serial.print("Changed to preset: ");
  Serial.println(presets[activePreset].name);
  
  // Brief feedback
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Changing preset");
  delay(300);
}

void displayPreset() {
  lcd.clear();
  
  // Line 1: Plant name
  lcd.setCursor(0, 0);
  lcd.print(presets[activePreset].name);
  
  // Line 2: Current temperature
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(currentTemperature, 1);
  lcd.print(" C");
  
  // Debug output
  Serial.println("=== Current Status ===");
  Serial.print("Plant: ");
  Serial.println(presets[activePreset].name);
  Serial.print("Current Temperature: ");
  Serial.print(currentTemperature, 1);
  Serial.println("°C");
  Serial.print("Target Temperature: ");
  Serial.print(presets[activePreset].temperature);
  Serial.println("°C");
  Serial.print("Lighting: ");
  Serial.print(presets[activePreset].lighting);
  Serial.println(" hours/day");
  Serial.print("Irrigation: ");
  Serial.print(presets[activePreset].irrigation);
  Serial.println(" minutes/day");
  Serial.println("=====================");
}

void readTemperature() {
  // Read analog value from LM35 (0-1023)
  int analogValue = analogRead(LM35_PIN);
  
  // Convert to voltage (0-5V)
  // Arduino ADC: 5V / 1024 = 0.0048828125V per step
  float voltage = analogValue * (5.0 / 1023.0);
  
  // LM35 outputs 10mV per degree Celsius
  // So voltage in V * 100 = temperature in °C
  currentTemperature = voltage * 100.0;
  
  // Debug output
  Serial.print("LM35 Reading - Analog: ");
  Serial.print(analogValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3);
  Serial.print("V | Temperature: ");
  Serial.print(currentTemperature, 1);
  Serial.println("°C");
}