/*
 * Automatic Greenhouse Controller with Bluetooth Control
 * 
 * LCD 1602A Wiring (Parallel - 4-bit mode):
 * - VSS -> GND
 * - VDD -> 5V
 * - V0 -> Potentiometer (10K) center pin (for contrast adjustment)
 * - RS -> Pin 12 (Digital)
 * - RW -> GND
 * - E -> Pin 8 (Digital)
 * - D4 -> Pin 7 (Digital)
 * - D5 -> Pin 4 (Digital)
 * - D6 -> Pin 2 (Digital)
 * - D7 -> Pin 13 (Digital)
 * - A (Backlight +) -> 5V (through 220Ω resistor)
 * - K (Backlight -) -> GND
 * 
 * Bluetooth Module ZS-040 (HC-05/HC-06) Wiring:
 * - VCC -> 5V (or 3.3V depending on module)
 * - GND -> GND
 * - TX -> Pin 10 (Arduino RX via SoftwareSerial) - Digital, no PWM needed
 * - RX -> Pin 9 (Arduino TX via SoftwareSerial) - Use voltage divider (1K + 2K resistors) to convert 5V to 3.3V
 * 
 * Voltage Divider for RX pin (to protect Bluetooth module):
 * Arduino Pin 9 -> 1K resistor -> Bluetooth RX
 *                                  |
 *                                2K resistor -> GND
 * 
 * Note: Most HC-05/HC-06 modules can handle 5V on VCC, but RX pin needs 3.3V
 * 
 * Pin Usage Summary:
 * - Pins 2, 4, 7, 8, 12, 13: LCD (Digital only, no PWM)
 * - Pins 9, 10: Bluetooth (Digital only, no PWM)
 * - PWM pins 3, 5, 6, 11: Temperature control (Fan and Heater)
 * 
 * Temperature Control (Baseboard):
 * - Pin 11 (PWM): Fan (Cooling - Channel A, Connector X10)
 * - Pin 6 (PWM): Resistor (Heating - Channel B, Connector X11, requires JP5 closed)
 * - Pin A0 (Analog): LM35 Temperature Sensor (Connector X1)
 */

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// LCD pins initialization (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 8, 7, 4, 2, 13);

// Bluetooth module pins (RX, TX)
SoftwareSerial btSerial(10, 9); // RX=10, TX=9

// =============================================================
// TEMPERATURE CONTROL - PI CONTROLLER (HEATING + COOLING)
// =============================================================

// VENTILADOR (Canal A - Resfriamento - Conector X10)
const int pinoFan_PWM = 11;

// RESISTOR (Canal B - Aquecimento - Conector X11)
const int pinoRes_PWM = 6;   // DOUT_5 -> Enable B (Requer Jumper JP5 FECHADO)

// SENSOR LM35 (Conector X1)
const int pinoSensor = A0;   

// PI Controller Parameters
float Kp = 15.0;  // Proportional Gain
float Ki = 0.5;   // Integral Gain

// Control Variables
double temperaturaAtual = 0;
double erro = 0;
double termoIntegral = 0;
double saidaControle = 0;

unsigned long ultimaLeitura = 0;
const int intervaloControle = 200; // Control every 200ms

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
  {"Basil", 666, 18, 10},
  {"Cilantro", 20, 12, 15},
  {"Tomato", 22, 16, 20}
};

// Active preset index
int activePreset = 0;

// Bluetooth command buffer
String btCommand = "";

// Display update tracking
int lastDisplayedPreset = -1;

// Display refresh for non-blocking updates
unsigned long ultimaAtualizacaoDisplay = 0;
const int intervaloDisplay = 1000; // Update display every 1 second

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  
  // Initialize Bluetooth serial communication
  btSerial.begin(9600);
  
  // Initialize LCD (16 columns, 2 rows)
  lcd.begin(16, 2);
  
  // Configure temperature control pins
  pinMode(pinoFan_PWM, OUTPUT);
  pinMode(pinoRes_PWM, OUTPUT);
  
  // Display welcome message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Greenhouse");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  
  // Display initial preset
  displayPreset();
  
  Serial.println("--- GREENHOUSE CONTROLLER WITH PI CONTROL ---");
  Serial.println("Greenhouse Controller Ready");
  Serial.println("Waiting for Bluetooth commands...");
  btSerial.println("Greenhouse Controller Ready");
}

void loop() {
  unsigned long agora = millis();
  
  // Execute temperature control cycle
  if (agora - ultimaLeitura >= intervaloControle) {
    executarControleTemperatura();
    ultimaLeitura = agora;
  }
  
  // Update display non-blocking
  if (agora - ultimaAtualizacaoDisplay >= intervaloDisplay) {
    displayPreset();
    ultimaAtualizacaoDisplay = agora;
  }
  
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
    btSerial.print("Preset:");
    btSerial.print(presets[activePreset].name);
    btSerial.print("|Temp:");
    btSerial.print(temperaturaAtual, 1);
    btSerial.print("C|Target:");
    btSerial.print(presets[activePreset].temperature);
    btSerial.println("C");
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
  
  // Line 1: Preset name
  lcd.setCursor(0, 0);
  lcd.print(presets[activePreset].name);
  
  // Line 2: Current temperature
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temperaturaAtual, 1);
  lcd.print("C");
}

// =============================================================
// TEMPERATURE CONTROL FUNCTIONS
// =============================================================

void executarControleTemperatura() {
  // 1. Read current temperature
  temperaturaAtual = lerTemperaturaMedia();
  
  // 2. Get setpoint from current preset
  double setpoint = presets[activePreset].temperature;
  
  // 3. Calculate error
  // Error > 0: Temperature Low (Need to Heat)
  // Error < 0: Temperature High (Need to Cool)
  erro = setpoint - temperaturaAtual;
  
  // 4. Calculate Integral (Accumulate error over time)
  termoIntegral += (erro * Ki);
  
  // Anti-Windup Protection (Prevent integral from growing infinitely)
  if (termoIntegral > 255) termoIntegral = 255;
  if (termoIntegral < -255) termoIntegral = -255;
  
  // 5. Calculate PI Output
  saidaControle = (Kp * erro) + termoIntegral;
  
  // 6. Actuate on devices (Split Range Logic)
  atuarNoSistema(saidaControle);
  
  // 7. Serial Monitor (for debugging/plotting)
  Serial.print("Temp:");
  Serial.print(temperaturaAtual);
  Serial.print(" Target:");
  Serial.print(setpoint);
  Serial.print(" Output:");
  Serial.println(saidaControle);
}

// Split Range Control Function
void atuarNoSistema(double output) {
  int pwmVal = 0;

  if (output > 0) { 
    // >>> HEATING MODE (Resistor) <<<
    // Positive output, turn on Channel B
    
    if (output > 255) output = 255; // Cap at maximum
    pwmVal = (int)output;

    // Apply to Resistor and turn off Fan
    analogWrite(pinoRes_PWM, pwmVal);
    analogWrite(pinoFan_PWM, 0);
  } 
  else {
    // >>> COOLING MODE (Fan) <<<
    // Negative output, convert to positive for PWM
    
    double outputFan = abs(output);
    
    if (outputFan > 255) outputFan = 255;
    pwmVal = (int)outputFan;

    // Fan Dead Zone Correction
    // 12V fans don't spin with very low PWM (e.g., < 60)
    // If control requests little, force minimum so it doesn't stall
    if (pwmVal > 0 && pwmVal < 60) {
      pwmVal = 60; 
    }

    // Turn off Resistor and apply to Fan
    analogWrite(pinoRes_PWM, 0);
    analogWrite(pinoFan_PWM, pwmVal);
  }
}

// LM35 Sensor Reading (Average for stability)
double lerTemperaturaMedia() {
  long soma = 0;
  int n_leituras = 20;
  for (int i = 0; i < n_leituras; i++) {
    soma += analogRead(pinoSensor);
    delay(2);
  }
  float media = soma / (float)n_leituras;
  
  // Conversion: (Value * 5V / 1023 steps) / 0.01V per degree
  return (media * 5.0 / 1023.0) / 0.01;
}