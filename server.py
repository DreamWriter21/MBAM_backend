from flask import Flask, request, send_from_directory, render_template_string

import os
from datetime import datetime

app = Flask(__name__, static_folder="uploads")

UPLOAD_FOLDER = "uploads"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route("/upload", methods=["POST"])

def upload_chunk():
    part = int(request.args.get("part", 1))
    total = int(request.args.get("total", 1))
    #filename = datetime.now().strftime("%Y-%m-%d %H:%M:%S") + request.args.get("filename", "Photo_MBAM")
    filename = request.args.get("filename", "Photo_MBAM.jpeg")

    # Logs
    print(f"🔍 Receiving chunk {part}/{total} for file {filename}")
    print(f"🔍 Chunk size: {len(request.data)} bytes")
    print(f"🔍 Total size: {total} chunks")
    print(f"🔍 Part: {part}")
    print(f"🔍 Filepath: {filename}")
    print(f"🔍 Request method: {request.method}")
    print(f"🔍 Request url: {request.url}")

    filepath = os.path.join(UPLOAD_FOLDER, filename)
    with open(filepath, "ab") as f:
        f.write(request.data)

    print(f"✅ Chunk {part}/{total} reçu ({len(request.data)} bytes)")
    if part == total:
        timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        new_filename = f"{timestamp}_{filename}"
        new_filepath = os.path.join(UPLOAD_FOLDER, new_filename)
        os.rename(filepath, new_filepath)
        
        print(f"🎉 Fichier complet reconstitué : {new_filename}")
        

    return "OK\n"


@app.route("/files")
def list_files():
    files = [f for f in os.listdir(UPLOAD_FOLDER) if f.lower().endswith((".jpg", ".jpeg", ".png"))]
    
     # Trier par date de modification décroissante
    files.sort(key=lambda f: os.path.getmtime(os.path.join(UPLOAD_FOLDER, f)), reverse=True)
    
    html = """
    <!DOCTYPE html>
    <html lang="fr">
    <head>
        <meta charset="utf-8">
        <title>Galerie d’images</title>
        <style>
            body { font-family: sans-serif; background: #fafafa; margin: 40px; }
            h1 { text-align: center; }
            .grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(200px, 1fr)); gap: 15px; }
            .card { background: white; border-radius: 8px; box-shadow: 0 2px 6px rgba(0,0,0,0.15); padding: 10px; text-align: center; }
            img { width: 100%; border-radius: 6px; }
            a { text-decoration: none; color: #333; font-weight: bold; display: block; margin-top: 5px; }
            .title { font-size: 0.8em; font-weight: normal; color: #666; }
        </style>
        <!-- Auto-refresh toutes les 30 secondes -->
        <meta http-equiv="refresh" content="30">
    </head>
    <body>
        <h1>📸 Galerie d’images reçues</h1>
        <div class="grid">
            {% for f in files %}
            <div class="card">
                <a href="/uploads/{{ f }}" target="_blank">
                    <img src="{{ url_for('static', filename=f) }}" alt="{{ f }}">
                    <span class="title">{{ f }}</span>
                </a>
            </div>
            {% endfor %}
        </div>
    </body>
    </html>
    """
    return render_template_string(html, files=files)

@app.route("/uploads/<path:filename>")
def download_file(filename):
    return send_from_directory(UPLOAD_FOLDER, filename)


if __name__ == "__main__":
    port = int(os.environ.get("PORT", 8000))
    app.run(host="0.0.0.0", port=port)



