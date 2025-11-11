from flask import Flask, render_template, request, jsonify
import serial
import serial.tools.list_ports

app = Flask(__name__)

# --- CONFIGURAÇÃO SERIAL / BLUETOOTH ---
# Descobre portas disponíveis (mostra no console)
ports = [p.device for p in serial.tools.list_ports.comports()]
print("🔌 Portas disponíveis:", ports)

# Altere para o dispositivo do seu HC-05
# Exemplo Linux: /dev/rfcomm0  |  Windows: COM5
PORTA_BT = "/dev/rfcomm0"
BAUD = 9600

try:
    ser = serial.Serial(PORTA_BT, BAUD, timeout=1)
    print(f"✅ Conectado ao módulo Bluetooth em {PORTA_BT}")
except Exception as e:
    print("⚠️ Não foi possível abrir a porta Bluetooth:", e)
    ser = None


# --- ROTAS WEB ---

@app.route("/")
def index():
    return render_template("index.html")


@app.route("/send", methods=["POST"])
def send():
    if not ser or not ser.is_open:
        return jsonify({"success": False, "message": "Sem conexão serial"})

    data = request.json
    msg = data.get("message", "").strip()
    if not msg:
        return jsonify({"success": False, "message": "Mensagem vazia"})

    full_msg = msg + "\n"
    ser.write(full_msg.encode("utf-8"))
    print(f"➡️ Enviado: {full_msg.strip()}")
    return jsonify({"success": True, "message": f"Enviado: {msg}"})


@app.route("/disconnect", methods=["POST"])
def disconnect():
    global ser
    if ser and ser.is_open:
        ser.close()
        return jsonify({"success": True, "message": "Conexão encerrada"})
    return jsonify({"success": False, "message": "Já desconectado"})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
