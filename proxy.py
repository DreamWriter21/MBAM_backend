from flask import Flask, request, Response
import requests

app = Flask(__name__)

# URL de ton backend HTTPS
TARGET_URL = "https://mbam-backend.onrender.com/upload"

@app.route("/upload", methods=["POST"])
def proxy_upload():
    # Reprendre les paramètres de l’ESP32
    params = request.args.to_dict()
    data = request.get_data()

    # Transmettre la requête au vrai serveur HTTPS
    try:
        resp = requests.post(TARGET_URL, params=params, data=data, timeout=10)
        print(f"✅ Chunk relayé vers backend ({len(data)} bytes) -> {resp.status_code}")
        return Response("OK\n", status=200)
    except Exception as e:
        print("❌ Erreur proxy:", e)
        return Response("ERROR\n", status=500)

if __name__ == "__main__":
    import os
    port = int(os.environ.get("PORT", 8080))
    app.run(host="0.0.0.0", port=port)
