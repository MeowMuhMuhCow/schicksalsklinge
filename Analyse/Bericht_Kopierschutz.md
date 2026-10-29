# Der Kopierschutz der Schicksalsklinge (CD-Fassung 3.02)

Stand 20.10.2026. Der Kopierschutz ist wie alles andere lesbar rekonstruiert und arbeitet im Nachbau unverändert.
Einen Schalter zum Abschalten gibt es nicht: Bright-Eyes hatte einen (`M302de_FEATURE_MOD`, „Feature mod 5“), der ist
nach der Projektregel entfernt. Die Antworten werden hier nicht aufgelistet.

## Zwei Prüfungen beim Start

| Prüfung | Wo | Was passiert |
|---|---|---|
| **CD vorhanden** | seg001 `CD_init`, Aufruf in seg002 `main()` | Über MSCDEX (INT 2Fh) wird ein CD-Laufwerk gesucht und für CD-Audio eingerichtet. Schlägt das fehl, beendet sich das Spiel sofort (`cleanup_game(); exit(0);`). Die Anfrageblöcke stehen in seg013.asm |
| **Handbuch- und Kartenabfrage** | seg002 `copy_protection()`, nach der Wahl des Schwierigkeitsgrads | bis zu drei Fragen, eine richtige Antwort genügt. Sonst endet das Programm |

Anders als bei Sternenschweif (dort ist die Handbuchabfrage in der CD-Fassung toter Code) wird sie in der Schicksalsklinge
3.02 **tatsächlich gestellt**, zusätzlich zur CD-Prüfung.

## So läuft `copy_protection()`

| Schritt | Ablauf |
|---|---|
| 1 | FIGHTTXT.LTX laden (dort stehen die beiden Fragetexte 39 und 40), Textfarbe setzen |
| 2 | Je Versuch mit 50 % Wahrscheinlichkeit eine der beiden Fragearten |
| Handbuch | eine von 10 Fragen aus `g_questions_handbook` (ds:46fc): „Bitte schlagen Sie Ihr Handbuch auf Seite … auf und geben Sie das Wort, das in Spalte …, Zeile … an … Stelle steht, ein“. Die Antwort steht **bitweise invertiert** in der Tabelle, damit sie im EXE nicht als Klartext lesbar ist. Die Eingabe wird in Großbuchstaben umgewandelt, invertiert und verglichen |
| Karte | eine von 10 Fragen aus `g_questions_map` (ds:47ba): „Wie heißt die Stadt <Richtung> von <Stadt>?“ (aus der beiliegenden Farbkarte). Die Ortsnamen kommen aus TEXT.LTX 236–287, die Antwort ist ein Index in dieselbe Liste |
| 3 | Rechts unten steht dabei die Versionsnummer (`g_game_version`) |
| 4 | nach drei Fehlversuchen Rückgabe 0, das Spiel wird beendet |

## Beobachtungen

- Die Tabellen liegen im Datensegment des Hauptmoduls seg002 (`datseg.c`), mit Originaladresse.
- Die Invertierung ist keine Verschlüsselung, nur ein Schutz vor dem Blick in den Hex-Editor. Bei den Kartenfragen gibt es nicht einmal das, weil die Antwort ein Index in die normale Ortsnamenliste ist.
- Eine stille Strafe für Cracker wie bei Sternenschweif (Prüfsumme im Kampf-Zeitgeber) haben wir in DSA 1 nicht gefunden.
