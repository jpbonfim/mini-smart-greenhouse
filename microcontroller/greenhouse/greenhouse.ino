/*
 * Controlador de Estufa Automática - Versão 2
 * * Mapeamento de Hardware:
 * * DISPLAY LCD 1602A (Modo 4-bit):
 * - RS -> Pino 12
 * - E  -> Pino 8
 * - D4 -> Pino 7
 * - D5 -> Pino 4
 * - D6 -> Pino 2
 * - D7 -> Pino 13
 * * BLUETOOTH (HC-05/06):
 * - TX -> Pino 10 (Arduino RX)
 * - RX -> Pino 9  (Arduino TX)
 * * CONTROLE DE TEMPERATURA:
 * - Ventoinha (Resfriamento): Pino 11 (PWM - Ponte H Canal A)
 * - Resistor (Aquecimento):   Pino 6  (RELÉ - Agora Digital ON/OFF)
 * - Sensor Temp (LM35):       Pino A0
 * * CONTROLE DE UMIDADE:
 * - Válvula Solenoide:        Pino 5  (Digital - Ativa Relé/Driver)
 * - Sensor Solo (LM393):      Pino A1 (Saída Analógica AO do sensor)
 */

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// Inicialização do LCD
LiquidCrystal lcd(12, 8, 7, 4, 2, 13);

// Bluetooth
SoftwareSerial btSerial(10, 9); // RX=10, TX=9

// =============================================================
// PINOS DE CONTROLE (ATUALIZADO)
// =============================================================

// VENTILADOR (Resfriamento - PWM)
const int pinoFan_PWM = 11;

// RESISTOR (Aquecimento - RELÉ - Digital)
const int pinoRes_Rele = 6; 

// VÁLVULA SOLENOIDE (Irrigação - Digital)
const int pinoValvula = 5;

// SENSORES
const int pinoSensorTemp = A0;    // LM35
const int pinoSensorUmidade = A1; // LM393 (Usar pino AO do módulo)

// =============================================================
// PARÂMETROS DE CONTROLE
// =============================================================

// Parâmetros PI (Apenas para o Ventilador agora)
float Kp = 15.0; 
float Ki = 0.5;   

// Variáveis de Controle
double temperaturaAtual = 0;
int umidadeAtual = 0; // 0 a 100%
double erro = 0;
double termoIntegral = 0;
double saidaControle = 0;

unsigned long ultimaLeitura = 0;
const int intervaloControle = 200; 

// Estrutura de Presets (Plantas)
struct Preset {
  const char* name;
  float temperature;  // °C
  int soilMoisture;   // % (Alvo de umidade)
};

const int NUM_PRESETS = 5;
Preset presets[NUM_PRESETS] = {
  {"Manjericao", 29.0, 30}, // Gosta de solo úmido
  {"Coentro",    17.0, 40},
  {"Salsinha",   24.0, 40},
  {"Cebolinha",  18.5, 30},
  {"Oregano",    18.5, 10}  // Gosta de solo mais seco
};

int activePreset = 0;
String btCommand = "";
unsigned long ultimaAtualizacaoDisplay = 0;
const int intervaloDisplay = 1000;

void setup() {
  Serial.begin(9600);
  btSerial.begin(9600);
  lcd.begin(16, 2);
  
  // Configuração dos Pinos
  pinMode(pinoFan_PWM, OUTPUT);
  pinMode(pinoRes_Rele, OUTPUT); // Agora é saída digital p/ Relé
  pinMode(pinoValvula, OUTPUT);
  
  // Inicializa tudo desligado
  digitalWrite(pinoRes_Rele, LOW);
  digitalWrite(pinoValvula, LOW);
  analogWrite(pinoFan_PWM, 0);
  
  // Intro
  lcd.clear();
  lcd.print("Smart Greenhouse");
  lcd.setCursor(0, 1);
  lcd.print("V2.0 - Relay");
  delay(2000);
  
  displayPreset();
  
  Serial.println("--- GREENHOUSE CONTROLLER V2 ---");
  Serial.println("Aquecimento: Rele (Pino 6)");
  Serial.println("Irrigacao: Solenoide (Pino 5)");
}

void loop() {
  unsigned long agora = millis();
  
  // Executa ciclos de controle
  if (agora - ultimaLeitura >= intervaloControle) {
    executarControleTemperatura();
    executarControleUmidade(); // Nova função de irrigação
    ultimaLeitura = agora;
  }
  
  // Atualiza LCD
  if (agora - ultimaAtualizacaoDisplay >= intervaloDisplay) {
    displayPreset();
    ultimaAtualizacaoDisplay = agora;
  }
  
  // Leitura Bluetooth
  if (btSerial.available()) {
    char c = btSerial.read();
    if (c == '\n' || c == '\r') {
      if (btCommand.length() > 0) {
        processBluetoothCommand(btCommand);
        btCommand = "";
      }
    } else {
      btCommand += c;
    }
  }
}

// =============================================================
// FUNÇÕES DE CONTROLE
// =============================================================

void executarControleTemperatura() {
  // 1. Ler Temperatura
  temperaturaAtual = lerTemperaturaMedia();
  double setpoint = presets[activePreset].temperature;
  
  // 2. Calcular Erro e PI
  erro = setpoint - temperaturaAtual;
  termoIntegral += (erro * Ki);
  
  // Anti-Windup
  if (termoIntegral > 255) termoIntegral = 255;
  if (termoIntegral < -255) termoIntegral = -255;
  
  // Saída do PI
  saidaControle = (Kp * erro) + termoIntegral;
  
  // 3. Atuar no Sistema (Lógica Híbrida: Relé + PWM)
  atuarNoSistemaHibrido(saidaControle, temperaturaAtual, setpoint);
}

void atuarNoSistemaHibrido(double output, double tempAtual, double alvo) {
  // Histerese para o Relé do Resistor (Para não ficar clicando loucamente)
  // Se precisa aquecer (Output positivo)
  
  if (output > 0) {
    // --- MODO AQUECIMENTO (RESISTOR NO RELÉ) ---
    // Ventoinha desligada
    analogWrite(pinoFan_PWM, 0);
    
    // Lógica Bang-Bang com Histerese para o Relé
    // Só liga o relé se a temperatura cair 0.5 abaixo do alvo
    if (tempAtual < (alvo - 0.5)) {
      digitalWrite(pinoRes_Rele, HIGH); // Liga Relé
    }
    // Só desliga o relé se passar do alvo
    else if (tempAtual >= alvo) {
      digitalWrite(pinoRes_Rele, LOW);  // Desliga Relé
    }
  } 
  else {
    // --- MODO RESFRIAMENTO (VENTOINHA PWM) ---
    // Resistor Desligado (Segurança)
    digitalWrite(pinoRes_Rele, LOW);
    
    // Calcula PWM da Ventoinha
    double outputFan = abs(output);
    if (outputFan > 255) outputFan = 255;
    int pwmVal = (int)outputFan;

    // Correção Zona Morta Ventoinha
    if (pwmVal > 0 && pwmVal < 60) pwmVal = 60; 
    
    analogWrite(pinoFan_PWM, pwmVal);
  }
}

void executarControleUmidade() {
  // 1. Ler o Sensor de Umidade (LM393)
  // Mapeamento: O sensor geralmente retorna valor ALTO (1023) quando SECO
  // e valor BAIXO quando MOLHADO. Vamos inverter para porcentagem (0-100%).
  int leituraBruta = analogRead(pinoSensorUmidade);
  
  // CALIBRAÇÃO AJUSTADA:
  // - Sensor SECO (ar livre): ~1023 -> 0% de umidade
  // - Sensor MOLHADO (água): ~358 -> 100% de umidade
  // Ajuste esses valores conforme seu sensor específico
  const int VALOR_SECO = 1023;    // Leitura quando sensor está seco
  const int VALOR_MOLHADO = 358;  // Leitura quando sensor está molhado (calibrado)
  
  umidadeAtual = map(leituraBruta, VALOR_SECO, VALOR_MOLHADO, 0, 100);
  
  // Trava entre 0 e 100
  if (umidadeAtual < 0) umidadeAtual = 0;
  if (umidadeAtual > 100) umidadeAtual = 100;

  // 2. Controle On-Off da Válvula
  int alvoUmidade = presets[activePreset].soilMoisture;
  
  // Se a umidade estiver abaixo do alvo (ex: alvo 30%, atual 20%) -> LIGA ÁGUA
  if (umidadeAtual < alvoUmidade) {
    digitalWrite(pinoValvula, HIGH);
  } 
  // Se a umidade atingiu o alvo + margem (ex: 35%) -> DESLIGA ÁGUA
  else if (umidadeAtual > (alvoUmidade + 5)) {
    digitalWrite(pinoValvula, LOW);
  }
}

// =============================================================
// FUNÇÕES AUXILIARES E BLUETOOTH
// =============================================================

void processBluetoothCommand(String command) {
  command.trim();
  Serial.println("CMD: " + command);
  
  if (command.startsWith("PRESET:")) {
    String presetName = command.substring(7);
    presetName.trim();
    for (int i = 0; i < NUM_PRESETS; i++) {
      if (presetName.equalsIgnoreCase(presets[i].name)) {
        activePreset = i;
        btSerial.println("OK: " + String(presets[i].name));
        lcd.clear(); lcd.print("BT: Preset OK"); delay(500);
        return;
      }
    }
    btSerial.println("ERROR: Preset not found");
  }
  else if (command.equalsIgnoreCase("STATUS")) {
    // Formato compatível com o app Python:
    // Preset:Nome|Temp:25.3C|Target:29C|Moisture:30%|Valve:ON
    btSerial.print("Preset:"); 
    btSerial.print(presets[activePreset].name);
    btSerial.print("|Temp:"); 
    btSerial.print(temperaturaAtual, 1);
    btSerial.print("C|Target:");
    btSerial.print(presets[activePreset].temperature, 1);
    btSerial.print("C|Moisture:");
    btSerial.print(umidadeAtual);
    btSerial.print("%|Valve:");
    btSerial.println((digitalRead(pinoValvula) ? "ON" : "OFF"));
  }
}

void displayPreset() {
  lcd.clear();
  // Linha 1: Nome e Status Válvula
  lcd.setCursor(0, 0);
  lcd.print(presets[activePreset].name);
  lcd.setCursor(13, 0);
  if (digitalRead(pinoValvula)) lcd.print("H2O"); // Mostra se regando
  
  // Linha 2: Temp e Umidade
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperaturaAtual, 0);
  lcd.print("C ");
  
  lcd.print("U:");
  lcd.print(umidadeAtual);
  lcd.print("%");
}

double lerTemperaturaMedia() {
  long soma = 0;
  for (int i = 0; i < 10; i++) {
    soma += analogRead(pinoSensorTemp);
    delay(2);
  }
  return ((soma / 10.0) * 5.0 / 1023.0) / 0.01;
}