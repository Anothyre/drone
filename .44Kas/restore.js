import fs from "fs";        // Dateien lesen/schreiben
import path from "path";   // Pfade wie src/main.js korrekt behandeln

// 1. JSON-Datei laden
const data = JSON.parse(
  fs.readFileSync("base44-export.json", "utf8")
);

// 2. Alle Dateien aus dem JSON holen
const files = data.files;

// 3. Für jede Datei:
for (const [filePath, content] of Object.entries(files)) {

  // Ordnerpfad bestimmen (z.B. "src")
  const dir = path.dirname(filePath);

  // Ordner anlegen, falls er noch nicht existiert
  fs.mkdirSync(dir, { recursive: true });

  // Datei mit Inhalt schreiben
  fs.writeFileSync(filePath, content, "utf8");
}
