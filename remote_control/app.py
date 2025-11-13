from flask import Flask, render_template, request, jsonify
import serial
import serial.tools.list_ports
import time

app = Flask(__name__)

# --- SERIAL / BLUETOOTH CONFIGURATION ---
# Discover available ports (shows in console)
ports = [p.device for p in serial.tools.list_ports.comports()]
print("🔌 Available ports:", ports)

# Change to your HC-05/ZS-040 device
# Example Linux: /dev/rfcomm0  |  Windows: COM5
BT_PORT = "/dev/rfcomm0"
BAUD_RATE = 9600

try:
    ser = serial.Serial(BT_PORT, BAUD_RATE, timeout=1)
    print(f"✅ Connected to Bluetooth module at {BT_PORT}")
    time.sleep(2)  # Wait for connection to stabilize
except Exception as e:
    print("⚠️ Could not open Bluetooth port:", e)
    ser = None


# --- WEB ROUTES ---

@app.route("/")
def index():
    return render_template("index.html")


@app.route("/send_preset", methods=["POST"])
def send_preset():
    """Send preset change command to Arduino via Bluetooth"""
    if not ser or not ser.is_open:
        return jsonify({"success": False, "message": "No serial connection"})

    data = request.json
    preset_name = data.get("preset", "").strip()
    
    if not preset_name:
        return jsonify({"success": False, "message": "Empty preset name"})

    # Send command in format: PRESET:PresetName
    command = f"PRESET:{preset_name}\n"
    ser.write(command.encode("utf-8"))
    print(f"➡️ Sent: {command.strip()}")
    
    # Wait for response
    time.sleep(0.5)
    response = ""
    if ser.in_waiting > 0:
        response = ser.readline().decode("utf-8", errors="ignore").strip()
        print(f"⬅️ Response: {response}")
    
    return jsonify({
        "success": True, 
        "message": f"Sent: {preset_name}",
        "response": response
    })


@app.route("/send_command", methods=["POST"])
def send_command():
    """Send custom command to Arduino via Bluetooth"""
    if not ser or not ser.is_open:
        return jsonify({"success": False, "message": "No serial connection"})

    data = request.json
    command = data.get("command", "").strip()
    
    if not command:
        return jsonify({"success": False, "message": "Empty command"})

    full_command = command + "\n"
    ser.write(full_command.encode("utf-8"))
    print(f"➡️ Sent: {full_command.strip()}")
    
    # Wait for response
    time.sleep(0.5)
    response = ""
    if ser.in_waiting > 0:
        response = ser.readline().decode("utf-8", errors="ignore").strip()
        print(f"⬅️ Response: {response}")
    
    return jsonify({
        "success": True, 
        "message": f"Sent: {command}",
        "response": response
    })


@app.route("/get_status", methods=["GET"])
def get_status():
    """Request current status from Arduino"""
    if not ser or not ser.is_open:
        return jsonify({"success": False, "message": "No serial connection"})

    # Send STATUS command
    ser.write(b"STATUS\n")
    print("➡️ Sent: STATUS")
    
    # Wait for response
    time.sleep(0.5)
    response = ""
    if ser.in_waiting > 0:
        response = ser.readline().decode("utf-8", errors="ignore").strip()
        print(f"⬅️ Response: {response}")
    
    return jsonify({
        "success": True,
        "status": response
    })


@app.route("/get_temperature", methods=["GET"])
def get_temperature():
    """Request current temperature from LM35 sensor via Arduino"""
    if not ser or not ser.is_open:
        return jsonify({"success": False, "message": "No serial connection"})

    # Send TEMP command
    ser.write(b"TEMP\n")
    print("➡️ Sent: TEMP")
    
    # Wait for response
    time.sleep(0.5)
    response = ""
    temperature = None
    
    if ser.in_waiting > 0:
        response = ser.readline().decode("utf-8", errors="ignore").strip()
        print(f"⬅️ Response: {response}")
        
        # Parse temperature from response (format: TEMP:25.3)
        if response.startswith("TEMP:"):
            try:
                temperature = float(response.split(":")[1])
            except (ValueError, IndexError):
                pass
    
    return jsonify({
        "success": True,
        "temperature": temperature,
        "raw_response": response
    })


@app.route("/disconnect", methods=["POST"])
def disconnect():
    """Close Bluetooth connection"""
    global ser
    if ser and ser.is_open:
        ser.close()
        return jsonify({"success": True, "message": "Connection closed"})
    return jsonify({"success": False, "message": "Already disconnected"})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
