from flask import Flask, request
import os
from datetime import datetime

app = Flask(__name__)

UPLOAD_FOLDER = "uploads"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route("/upload", methods=["POST"])
def upload_chunk():
    part = int(request.args.get("part", 1))
    total = int(request.args.get("total", 1))
    filename = request.args.get("filename", datetime.now().strftime("%Y-%m-%d %H:%M:%S") + ".jpg")
    
    print(f"🔍 Receiving chunk {part}/{total} for file {file_name}")
    print(f"🔍 Chunk size: {len(request.data)} bytes")
    print(f"🔍 Total size: {total} chunks")
    print(f"🔍 Part: {part}")
    print(f"🔍 Filepath: {filename}")
    print(f"🔍 Request method: {request.method}")
    print(f"🔍 Request url: {request.url}")

    with open(os.path.join(UPLOAD_FOLDER, filename), "ab") as f:
        f.write(request.data)

    print(f"✅ Chunk {part}/{total} reçu ({len(request.data)} bytes)")
    if part == total:
        print(f"🎉 Fichier complet reconstitué : {filename}")

    return "OK\n"

if __name__ == "__main__":
    port = int(os.environ.get("PORT", 8000))
    app.run(host="0.0.0.0", port=port)
