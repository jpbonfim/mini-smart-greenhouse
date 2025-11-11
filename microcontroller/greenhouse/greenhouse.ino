/*
 * Automatic Greenhouse Controller
 * 
 * LCD 1602A Wiring (I2C version - easier):
 * - VCC -> 5V
 * - GND -> GND
 * - SDA -> A4 (Arduino Uno/Nano)
 * - SCL -> A5 (Arduino Uno/Nano)
 * 
 * OR LCD 1602A Wiring (Parallel - 4-bit mode):
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
 * Button for preset selection:
 * - One side -> Pin 7
 * - Other side -> GND
 * - (Optional: 10K pull-up resistor between Pin 7 and 5V, or use internal pull-up)
 */

#include <LiquidCrystal.h>

// LCD pins initialization (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Button pin
const int BUTTON_PIN = 7;

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

// Button debounce variables
int lastButtonState = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Display update tracking
int lastDisplayedPreset = -1;

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  
  // Initialize LCD (16 columns, 2 rows)
  lcd.begin(16, 2);
  
  // Initialize button pin with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Display welcome message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Greenhouse");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  
  // Display initial preset
  displayPreset();
  
  Serial.println("Greenhouse Controller Ready");
  Serial.println("Press button to change preset");
}

void loop() {
  // Read button state
  int reading = digitalRead(BUTTON_PIN);
  
  // Check if button state changed
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  // Debounce button
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      // Button pressed (LOW because of pull-up)
      if (buttonState == LOW) {
        changePreset();
      }
    }
  }
  
  lastButtonState = reading;
  
  // Update display if preset changed
  if (activePreset != lastDisplayedPreset) {
    displayPreset();
    lastDisplayedPreset = activePreset;
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
  
  // Line 1: Preset name
  lcd.setCursor(0, 0);
  lcd.print("Plant: ");
  lcd.print(presets[activePreset].name);
  
  // Line 2: Quick info (Temperature, Lighting, Irrigation)
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(presets[activePreset].temperature);
  lcd.print("C ");
  
  lcd.print("L:");
  lcd.print(presets[activePreset].lighting);
  lcd.print("h ");
  
  lcd.print("I:");
  lcd.print(presets[activePreset].irrigation);
  lcd.print("m");
  
  // Debug output
  Serial.println("=== Current Preset ===");
  Serial.print("Name: ");
  Serial.println(presets[activePreset].name);
  Serial.print("Temperature: ");
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