"""
Smart Greenhouse Remote Control API
====================================

Flask web application for controlling an Arduino-based greenhouse system via Bluetooth.

API Endpoints:
--------------

1. GET /
   - Serves the main web interface

2. POST /send_preset
   - Change greenhouse preset (Basil, Cilantro, Tomato)
   - Body: {"preset": "PresetName"}

3. POST /send_command
   - Send custom command to Arduino
   - Body: {"command": "COMMAND_STRING"}

4. GET /get_status
   - Retrieve current greenhouse status (preset, temperature)

5. POST /disconnect
   - Close Bluetooth connection

Configuration:
--------------
- BT_PORT: Serial port for Bluetooth module (e.g., /dev/rfcomm0)
- BAUD_RATE: Communication speed (default: 9600)

Arduino Commands:
-----------------
- PRESET:PresetName - Change to specific preset
- NEXT - Cycle to next preset
- STATUS - Get current status

Setup:
------
1. Connect Bluetooth module to Arduino
2. Create RFCOMM connection: sudo rfcomm bind /dev/rfcomm0 <MAC_ADDRESS>
3. Update BT_PORT in this file
4. Run: python app.py

Requirements:
-------------
- Flask
- pyserial
"""

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
    """
    Render the main web interface.
    
    Returns:
        HTML: Main control panel page
    """
    return render_template("index.html")


@app.route("/send_preset", methods=["POST"])
def send_preset():
    """
    Send preset change command to Arduino via Bluetooth.
    
    Request Format:
        POST /send_preset
        Content-Type: application/json
        
        Body:
        {
            "preset": "Manjericao"  // Preset name (String): "Manjericao", "Coentro", "Salsinha", "Cebolinha", or "Oregano"
        }
    
    Arduino Command Sent:
        "PRESET:Basil\\n"
    
    Arduino Response Format:
        Success: "OK: Changed to Manjericao"
        Error: "ERROR: Unknown preset - InvalidName"
    
    Response Format:
        Success:
        {
            "success": true,
            "message": "Sent: Manjericao",
            "response": "OK: Changed to Manjericao"
        }
        
        Error (No connection):
        {
            "success": false,
            "message": "No serial connection"
        }
        
        Error (Empty preset):
        {
            "success": false,
            "message": "Empty preset name"
        }
    
    Example Usage:
        curl -X POST http://localhost:5000/send_preset \\
             -H "Content-Type: application/json" \\
             -d '{"preset": "Manjericao"}'
    """
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
    """
    Send custom command to Arduino via Bluetooth.
    
    Request Format:
        POST /send_command
        Content-Type: application/json
        
        Body:
        {
            "command": "STATUS"  // Any valid Arduino command (String)
        }
    
    Supported Arduino Commands:
        - "STATUS": Get current greenhouse status
        - "NEXT": Switch to next preset
        - "PRESET:PresetName": Change to specific preset
    
    Arduino Command Sent:
        The command string followed by newline (e.g., "STATUS\\n")
    
    Arduino Response Format:
        Varies by command. Examples:
        - STATUS: "Preset:Basil|Temp:25.3C|Target:25C"
        - NEXT: "OK: Changed to Cilantro"
        - Unknown: "ERROR: Unknown command"
    
    Response Format:
        Success:
        {
            "success": true,
            "message": "Sent: STATUS",
            "response": "Preset:Basil|Temp:25.3C|Target:25C"
        }
        
        Error (No connection):
        {
            "success": false,
            "message": "No serial connection"
        }
        
        Error (Empty command):
        {
            "success": false,
            "message": "Empty command"
        }
    
    Example Usage:
        curl -X POST http://localhost:5000/send_command \\
             -H "Content-Type: application/json" \\
             -d '{"command": "STATUS"}'
    """
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
    """
    Request current status from Arduino.
    
    Request Format:
        GET /get_status
        No body required
    
    Arduino Command Sent:
        "STATUS\\n"
    
    Arduino Response Format:
        Raw: "Preset:Manjericao|Temp:25.3C|Target:29C"
        
        Components:
        - Preset: Current preset name
        - Temp: Current measured temperature (°C)
        - Target: Target temperature from preset (°C)
    
    Response Format:
        Success:
        {
            "success": true,
            "status": "Preset:Manjericao|Temp:25.3C|Target:29C",
            "data": {
                "preset": "Manjericao",      // Current preset name (String)
                "current_temp": "25.3",      // Current temperature (String, numeric)
                "target_temp": "29"          // Target temperature (String, numeric)
            }
        }
        
        Error (No connection):
        {
            "success": false,
            "message": "No serial connection"
        }
    
    Example Usage:
        curl http://localhost:5000/get_status
        
    JavaScript Example:
        fetch('/get_status')
            .then(response => response.json())
            .then(data => {
                console.log('Preset:', data.data.preset);
                console.log('Current Temp:', data.data.current_temp);
                console.log('Target Temp:', data.data.target_temp);
            });
    """
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
    
    # Parse response format: "Preset:Basil|Temp:25.3C|Target:25C"
    status_data = {
        "preset": "",
        "current_temp": "",
        "target_temp": ""
    }
    
    if response:
        parts = response.split("|")
        for part in parts:
            if part.startswith("Preset:"):
                status_data["preset"] = part.replace("Preset:", "")
            elif part.startswith("Temp:"):
                status_data["current_temp"] = part.replace("Temp:", "").replace("C", "")
            elif part.startswith("Target:"):
                status_data["target_temp"] = part.replace("Target:", "").replace("C", "")
    
    return jsonify({
        "success": True,
        "status": response,
        "data": status_data
    })


@app.route("/disconnect", methods=["POST"])
def disconnect():
    """
    Close Bluetooth serial connection.
    
    Request Format:
        POST /disconnect
        No body required
    
    Response Format:
        Success (Connection closed):
        {
            "success": true,
            "message": "Connection closed"
        }
        
        Info (Already disconnected):
        {
            "success": false,
            "message": "Already disconnected"
        }
    
    Example Usage:
        curl -X POST http://localhost:5000/disconnect
        
    Note:
        After disconnecting, you'll need to restart the Flask app
        to reconnect to the Arduino.
    """
    global ser
    if ser and ser.is_open:
        ser.close()
        return jsonify({"success": True, "message": "Connection closed"})
    return jsonify({"success": False, "message": "Already disconnected"})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
