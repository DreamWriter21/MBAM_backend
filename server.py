from flask import Flask, request
import os

app = Flask(__name__)

UPLOAD_FOLDER = "uploads"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route("/upload", methods=["POST"])
def upload_chunk():
    file_name = request.args.get("file", "image.jpg")
    part = int(request.args.get("part", 1))
    total = int(request.args.get("total", 1))
    filepath = os.path.join(UPLOAD_FOLDER, file_name)

    with open(filepath, "ab") as f:
        f.write(request.data)

    print(f"✅ Chunk {part}/{total} reçu ({len(request.data)} bytes)")
    if part == total:
        print(f"🎉 Fichier complet reconstitué : {filepath}")

    return "OK\n"

# ⚠️ Ne rien exécuter automatiquement ici
# Railway lancera le serveur depuis entrypoint.py
