# Resumo das Alterações - Controle Bluetooth da Estufa

## 🎯 Mudanças Implementadas

### ✅ Código do Microcontrolador (`microcontroller/greenhouse/greenhouse.ino`)

**Removido:**
- Controle por botão físico (Pin 7)
- Variáveis de debounce do botão
- Lógica de leitura do botão no loop()

**Adicionado:**
- Biblioteca `SoftwareSerial` para comunicação Bluetooth
- Comunicação serial com módulo ZS-040 (pinos 9 e 10)
- Buffer de comandos Bluetooth (`btCommand`)
- Função `processBluetoothCommand()` para processar comandos
- Suporte para 3 tipos de comandos:
  - `PRESET:Nome` - Muda para preset específico
  - `NEXT` - Cicla para próximo preset
  - `STATUS` - Retorna configuração atual
- Feedback visual no LCD quando comando é recebido
- Respostas via Bluetooth confirmando ações

**Pinos Utilizados:**
- Pin 9: TX do Arduino → RX do Bluetooth (via divisor de tensão)
- Pin 10: RX do Arduino ← TX do Bluetooth

---

### ✅ Aplicação Flask (`remote_control/app.py`)

**Melhorias:**
- Código em inglês (mantendo comentários originais em português)
- Nova rota `/send_preset` - Envia comando de preset
- Nova rota `/send_command` - Envia comando genérico
- Nova rota `/get_status` - Solicita status atual
- Leitura de resposta do Arduino após cada comando
- Melhor tratamento de erros
- Tempo de espera após conexão para estabilizar

**Alterações de Variáveis:**
- `PORTA_BT` → `BT_PORT`
- `BAUD` → `BAUD_RATE`

---

### ✅ Interface Web (`remote_control/templates/index.html`)

**Completamente Redesenhada:**
- Interface moderna com gradiente e sombras
- Cards separados para cada funcionalidade
- Grid de botões para presets (Basil, Cilantro, Tomato)
- Botões de ação rápida (Next, Status)
- Feedback visual de comandos (cores: verde=sucesso, vermelho=erro)
- Design responsivo
- Informações sobre cada preset na interface
- Ícones para melhor usabilidade

---

## 📋 Ligações do Módulo Bluetooth ZS-040

### ⚠️ IMPORTANTE: Divisor de Tensão

O módulo Bluetooth opera em **3.3V**, mas o Arduino em **5V**. É **OBRIGATÓRIO** usar divisor de tensão no pino RX do Bluetooth!

### Conexões Básicas:

```
Bluetooth ZS-040          Arduino
─────────────────────────────────
VCC (5V)          →       5V
GND               →       GND
TX                →       Pin 10 (direto)
RX                →       Pin 9 (via divisor de tensão)
```

### Divisor de Tensão (Obrigatório!):

```
Arduino Pin 9 ──[Resistor 1KΩ]──┬── Bluetooth RX
                                 │
                          [Resistor 2KΩ]
                                 │
                                GND
```

**Cálculo:** 5V × (2K / (1K + 2K)) = 3.33V ✓

### Todas as Conexões:

**LCD 1602A:**
- VSS → GND
- VDD → 5V
- V0 → Potenciômetro (contraste)
- RS → Pin 12
- RW → GND
- E → Pin 11
- D4 → Pin 6
- D5 → Pin 5
- D6 → Pin 4
- D7 → Pin 3
- A → 5V (via resistor 220Ω)
- K → GND

**Bluetooth ZS-040:**
- VCC → 5V
- GND → GND
- TX → Arduino Pin 10
- RX → Arduino Pin 9 (via divisor 1K+2K)

**Potenciômetro 10K (para contraste do LCD):**
- Pino esquerdo → GND
- Pino central → LCD V0
- Pino direito → 5V

---

## 🚀 Como Usar

### 1. Montagem do Hardware
Siga o diagrama em `WIRING_DIAGRAM.txt` (versão ASCII art completa)

### 2. Upload do Código Arduino
```bash
Abra: microcontroller/greenhouse/greenhouse.ino
Configure: Board = Arduino Uno/Nano, Port = /dev/ttyUSB0
Clique: Upload
```

### 3. Parear Bluetooth (Linux)
```bash
# Escanear dispositivos
hcitool scan

# Anotar o endereço MAC (XX:XX:XX:XX:XX:XX)
sudo bluetoothctl
pair XX:XX:XX:XX:XX:XX
connect XX:XX:XX:XX:XX:XX
exit

# Criar porta serial
sudo rfcomm bind /dev/rfcomm0 XX:XX:XX:XX:XX:XX 1
```

### 4. Configurar e Executar App Flask
```bash
cd remote_control

# Editar app.py linha 13 se necessário:
# BT_PORT = "/dev/rfcomm0"  # Linux
# BT_PORT = "COM5"          # Windows

# Instalar dependências
pip install -r requirements.txt

# Executar
python3 app.py
```

### 5. Acessar Interface Web
Abrir navegador: http://localhost:5000

---

## 📡 Comandos Bluetooth

O Arduino aceita os seguintes comandos:

| Comando | Descrição | Exemplo | Resposta |
|---------|-----------|---------|----------|
| `PRESET:Nome` | Muda para preset específico | `PRESET:Basil` | `OK: Changed to Basil` |
| `NEXT` | Próximo preset (cíclico) | `NEXT` | `OK: Changed to Cilantro` |
| `STATUS` | Consulta configuração atual | `STATUS` | `Current: Basil \| T:25C L:18h I:10m` |

### Presets Disponíveis:
- **Basil** (Manjericão): 25°C, 18h luz, 10min irrigação
- **Cilantro** (Coentro): 20°C, 12h luz, 15min irrigação
- **Tomato** (Tomate): 22°C, 16h luz, 20min irrigação

---

## 🔧 Solução de Problemas

### LCD não mostra nada
→ Ajuste o potenciômetro de contraste (girar lentamente)

### LCD mostra caracteres aleatórios
→ Verifique as conexões dos pinos de dados (D4-D7)

### Bluetooth não pareia
→ Tente PIN padrão: 1234, 0000 ou 1111

### Sem resposta aos comandos
→ Verifique o divisor de tensão (1K + 2K resistores)
→ Confirme que TX/RX não estão invertidos

### Erro "No serial connection"
→ Verifique: `ls -l /dev/rfcomm0`
→ Atualize BT_PORT em app.py

---

## 📁 Arquivos Criados/Modificados

### Modificados:
- ✏️ `microcontroller/greenhouse/greenhouse.ino` - Código Arduino com Bluetooth
- ✏️ `remote_control/app.py` - Servidor Flask melhorado
- ✏️ `remote_control/templates/index.html` - Interface web redesenhada

### Criados:
- 📄 `BLUETOOTH_WIRING.md` - Guia detalhado de ligações (inglês)
- 📄 `WIRING_DIAGRAM.txt` - Diagrama ASCII completo
- 📄 `QUICK_START.md` - Guia rápido de início
- 📄 `RESUMO_PT.md` - Este arquivo (resumo em português)

---

## 🎨 Interface Web - Preview

A nova interface possui:
- **Gradiente roxo** moderno
- **Cards separados** para organização
- **Botões grandes** para presets (Basil, Cilantro, Tomato)
- **Comandos rápidos** (Next, Status)
- **Área de status** com feedback em cores
- **Informações** sobre cada preset
- **Design responsivo**

---

## ⚡ Próximos Passos Sugeridos

1. Adicionar sensores reais (temperatura, umidade)
2. Implementar controle de relés para luzes e irrigação
3. Criar log de dados em cartão SD
4. Desenvolver app mobile nativo
5. Adicionar múltiplas zonas de plantio

---

## 📞 Documentação Adicional

- `BLUETOOTH_WIRING.md` - Guia completo de ligações
- `WIRING_DIAGRAM.txt` - Diagrama visual ASCII
- `QUICK_START.md` - Início rápido (inglês)

Bom cultivo! 🌱🌿

---

**Data das modificações:** 13 de Novembro de 2025
