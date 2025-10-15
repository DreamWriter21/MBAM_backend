from flask import Flask, request
import os

app = Flask(__name__)

# Dossier pour sauvegarder les images
UPLOAD_FOLDER = "uploads"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route("/upload", methods=["POST"])
def upload_chunk():
    """
    Recevoir un chunk d'image et l'ajouter au fichier final.
    Paramètres GET :
      - file : nom du fichier (ex: image.jpg)
      - part : numéro du chunk (1-based)
      - total : nombre total de chunks
    Corps : binaire JPEG
    """
    file_name = request.args.get("file", "image.jpg")
    part = int(request.args.get("part", 1))
    total = int(request.args.get("total", 1))

    filepath = os.path.join(UPLOAD_FOLDER, file_name)

    # Écrire le chunk à la suite
    with open(filepath, "ab") as f:
        f.write(request.data)

    print(f"✅ Chunk {part}/{total} reçu ({len(request.data)} bytes)")

    # Quand le dernier chunk arrive
    if part == total:
        print(f"🎉 Fichier complet reconstitué : {filepath}")

    return "OK\n"

if __name__ == "__main__":
    import os
    port = int(os.environ.get("PORT", 8080))  # Render fournit le port via env
    app.run(host="0.0.0.0", port=port)
