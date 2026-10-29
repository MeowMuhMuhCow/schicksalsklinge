# Namensliste

`namen.tsv`: alle Funktionen und globalen Daten der Schicksalsklinge mit Modul und Adresse, erzeugt aus `BLADEM.MAP`
des bytegleichen Baus und `module.json`.

| Spalte | Inhalt |
|---|---|
| art | funktion / daten |
| name | Name im Quelltext (Bright-Eyes und unsere Korrekturen; Attics Bezeichner sind verloren) |
| adresse | Funktionen: Segment:Offset im Speicherabbild laut MAP (bei Overlays die Einsprungstelle). Daten: `ds:0x…` im Datensegment, so wie in den Kommentaren im Code |
| modul | Quelldatei |
| art_genauer | öffentlich / statisch (statische Funktionen stehen nicht in der MAP, daher ohne Adresse); vorbelegt (_DATA) / nicht vorbelegt (_BSS) / far_data |

Welche Namen wir geändert haben und warum: `Analyse/Beschriftung/`.
