from flask import Flask, request, jsonify
import json
from datetime import datetime
import os

app = Flask(__name__)

# ALWAYS save file in same folder as receiver.py
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_FILE = os.path.join(BASE_DIR, "latest_input.json")

@app.route("/predict_from_device", methods=["POST"])
def receive_data():
    data = request.get_json(force=True)
    data["timestamp"] = datetime.now().isoformat()

    with open(DATA_FILE, "w") as f:
        json.dump(data, f, indent=2)

    print("✅ Data saved to:", DATA_FILE)
    print(data)

    return jsonify({"status": "received"}), 200

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
