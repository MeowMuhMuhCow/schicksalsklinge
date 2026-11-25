# Talentliste – Die Schicksalsklinge (aus dem Quelltext)

> **Hinweis:** Diese Liste wurde von einer KI aus dem rekonstruierten Quelltext erstellt. „Im Code“ ist das, was das Programm tatsächlich rechnet. „Deutung der KI“ ist die Auslegung der KI und kann falsch sein. Wer im Spiel etwas anderes beobachtet oder es genauer weiß: bitte im Crystals-DSA-Forum melden.
>
> Alle Angaben gelten für das Originalverhalten. Wo ein Fix des Patches etwas ändert, steht die Fix-Nummer dabei (Liste aller Fixes: `FIXES.md`).

## Inhaltsverzeichnis

1. [Allgemeine Talentmechanik im Quelltext](#1-allgemeine-talentmechanik-im-quelltext)
2. [Übersichtstabelle aller 52 Talente](#2-übersichtstabelle-aller-52-talente)
3. [Alle 52 Talente im Detail](#3-alle-52-talente-im-detail)

---

## 1. Allgemeine Talentmechanik im Quelltext

### Die Talentprobe (`Quelle/seg103.c:test_talent`)

- **Im Code:**
  - **Nahkampftalente** (Waffenlos bis Zweihänder) haben keine Probe: `test_talent()` gibt sofort `0` zurück. Ihr Wert wirkt nur über die Attacke- und Paradepunkte, die beim Steigern dazukommen (siehe unten).
  - **Schußwaffen und Wurfwaffen** benutzen eine eigene Fernkampfprobe: Zielwert = (KL + GE + KK) / 4 (ganzzahlig, jeweils aktueller Wert + Modifikator) + TaW − Zuschlag. Ein W20 muss kleiner oder gleich dem Zielwert sein. Eine 1 gelingt immer (Ergebnis 99), eine 20 misslingt immer (Ergebnis −1).
  - **Alle anderen Talente** würfeln eine **Summenprobe** auf die drei Eigenschaften des Talents (`Quelle/seg002.c:test_attrib3`): Die drei W20 werden addiert, dazu kommt Zuschlag − TaW. Ist das Ergebnis höchstens so groß wie die Summe der drei Eigenschaftswerte, gelingt die Probe. Es wird also nicht jeder Wurf einzeln mit seiner Eigenschaft verglichen. Zwei Zwanzigen sind immer ein Patzer; zwei Einsen haben keine Sonderwirkung.
  - **Beispiel:** Eigenschaften 13 / 13 / 13 (Summe 39), TaW 5, Zuschlag +4: Die drei W20 dürfen zusammen höchstens 39 − 4 + 5 = 40 ergeben. Der Durchschnitt von 3 W20 liegt bei 31,5, die Probe gelingt also meistens.
  - **Vorzeichen:** Ein positiver Zuschlag macht die Probe schwerer (in den Tabellen „+4“), ein negativer leichter („−4“).
- **Status:** im Code geprüft, im Spiel unbestätigt

### Wer würfelt?

- **Im Code:** Jede Abfragestelle legt selbst fest, wer die Probe macht. Es gibt fünf Muster:
  - **Erster einsatzfähiger Held** (`get_first_hero_available_in_group()`): der erste Held in der Reihenfolge der Gruppe, der handeln kann – **nicht** der beste. Das gilt für fast alle Reiseereignisse (Wildnisleben, Pflanzenkunde, Orientierung, Fährtensuchen, Kriegskunst) und für die meisten Geheimtüren, Schlösser und Fallen in Dungeons.
  - **Jeder Held einzeln:** alle Helden der Gruppe würfeln, oft entscheidet der schwächste (Klettern, Schleichen, Schwimmen, Gefahrensinn bei Fallen).
  - **Vom Spieler gewählt:** im Talentmenü, im Wildlager und beim Handel. Das Auswahlmenü markiert dabei den Helden mit der höchsten Summe aus den drei Probe-Eigenschaften und dem TaW vor (`Quelle/seg104.c:get_talented_hero_pos`).
  - **Zufälliger Held:** bei einigen Stadtereignissen und Gesprächen.
  - **Anfängermodus und Sinnesschärfe:** Jede Sinnesschärfe-Probe würfelt dort automatisch der beste Held der Gruppe, egal wen die Abfragestelle vorgesehen hat (`Quelle/seg103.c:get_best_hero_for_talent`, Fix 86: nur handlungsfähige Helden).
- **Deutung der KI:** Die Summenprobe ist deutlich leichter als die Papierregel, bei der jeder Wurf einzeln unter seiner Eigenschaft bleiben muss. Für Reisen und Dungeons lohnt es sich, den besten Wildnis-, Sinnes- und Schlösser-Helden an die **erste Stelle** der Gruppe zu setzen.
- **Status:** im Code geprüft, im Spiel unbestätigt

### Einmal gefunden, immer bekannt

- **Im Code:** Bei den meisten Fundorten auf Reisen (Rastplätze, Kräuterplätze, Spuren, Dungeon-Eingänge) ist die Abfrage so gebaut: Probe gelingt **oder** der Ort wurde schon einmal gefunden. Nach dem ersten Gelingen merkt sich das Spiel den Ort, und beim nächsten Mal gibt es keine Probe mehr. In den Tabellen steht dann „einmal gefunden, bleibt es bekannt“. Geheimtüren in Dungeons funktionieren genauso.
- **Status:** im Code geprüft, im Spiel unbestätigt

### Steigern (`Quelle/seg050.c:inc_talent_advanced`, `inc_talent_novice`)

- **Im Code:**
  - Pro Stufenaufstieg darf jedes Talent höchstens so oft steigen, wie die Spalte „Steig./Stufe“ angibt (Kampf und Intuition 1, Wissen 3, alle anderen 2). Nach **drei Fehlversuchen** in Folge ist das Talent für diesen Aufstieg gesperrt. Beide Zähler beginnen bei jedem Aufstieg von vorn.
  - Der Wurf ist **gleichverteilt**, nicht 2W6 oder 3W6: Bei TaW bis 10 eine Zahl von 2 bis 12, ab TaW 11 eine Zahl von 3 bis 18. Die Steigerung gelingt, wenn der TaW kleiner als die Zahl ist.
  - Beim Steigern eines Nahkampftalents kommt zusätzlich +1 Attacke oder +1 Parade für diese Waffenart dazu. Im Profimodus wählt der Spieler, im Anfängermodus geht der Punkt auf den kleineren der beiden Werte (bei Gleichstand auf die Attacke).
  - Im **Anfängermodus** verteilt das Spiel die Steigerungen selbst, nach einer festen Liste mit Zielwerten je Heldentyp aus der Datei `BSKILLS.DAT`. Dabei gibt es keine Grenze pro Stufe, nur die drei Fehlversuche und den Höchstwert 18.

| TaW vorher | Chance pro Versuch |
|---:|---:|
| 1 oder weniger | 100 % |
| 2 | 91 % |
| 4 | 73 % |
| 6 | 55 % |
| 8 | 36 % |
| 10 | 18 % |
| 11 | 44 % |
| 13 | 31 % |
| 15 | 19 % |
| 17 | 6 % |
| 18 | 0 % |

- **Deutung der KI:** Durch den gleichverteilten Wurf ist der Schritt von 10 auf 11 der schwerste (18 %), der von 11 auf 12 dagegen leichter (44 %). In der Papiervorlage mit 2W6 und 3W6 sieht die Kurve anders aus.
- **Status:** im Code geprüft, im Spiel unbestätigt

### Talent anwenden (`Quelle/seg103.c:select_talent`, `use_talent`)

- **Im Code:** Über das Talentmenü lassen sich nur wenige Talente direkt anwenden:

| Ort | Talente |
|---|---|
| überall | Heilen Gift, Heilen Krankheiten, Heilen Wunden |
| Taverne | zusätzlich Akrobatik, Falschspiel (nicht aus dem Statusmenü), Musizieren |
| Wildlager, Gasthaus | zusätzlich Alchimie |
| Markt | zusätzlich Akrobatik, Taschendiebstahl |
| Händler | zusätzlich Taschendiebstahl |

  Der Zuschlag ist dabei 0. In der **Taverne** gilt eine Ausnahme: Wurde vor weniger als 2 Minuten (echte Zeit) gespeichert, bekommt die Probe +50 und misslingt praktisch immer (`Quelle/seg059.c`). Das soll offenbar Speichern und Neuladen bis zum Erfolg verhindern.
- **Status:** im Code geprüft, im Spiel unbestätigt

### Wildlager: Vorräte und Kräuter (`Quelle/seg051.c:replenish_stocks`, `gather_herbs`)

- **Im Code:**
  - **Vorräte auffüllen:** Ein gewählter Held macht erst eine Wildnisleben-Probe (Wasser für alle), dann eine Fährtensuchen-Probe (Nahrung für alle), beide mit +5 Grundzuschlag. Ein aktives Efferd- bzw. Firun-Wunder lässt die jeweilige Probe automatisch gelingen.
  - **Platzzuschlag:** An Rastplätzen aus Reiseereignissen ±0, an manchen davon −3 auf Vorräte („gute Jagd“) oder −3 auf Kräuter; Nahrungs- und Kräuterplätze an Reisewegen geben ebenfalls −3. Wer dagegen unterwegs einfach über das Reisemenü „Kampieren“ wählt, bekommt +99 – Vorräte und Kräuter sind dann praktisch unmöglich (`Quelle/seg094.c`).
  - **Kräuter suchen:** Der gewählte Held sucht eine eingegebene Anzahl Stunden. Für jedes Kraut wird erst die Fundchance gewürfelt, dann eine Pflanzenkunde-Probe mit dem Krautzuschlag − (Stunden − 1) + Platzzuschlag. Jede zusätzliche Stunde erleichtert also um 1. An einem Kräuterplatz für ein bestimmtes Kraut steigt dessen Fundchance um 10 Prozentpunkte und die Höchstmenge um 1; bei Krautnummern über 127 klappte das im Original nicht (Fix 161). Die Erhöhung wird direkt in die Kräutertabelle geschrieben und nie zurückgesetzt, sie wächst also bei jedem weiteren Besuch eines solchen Platzes.

| Kraut | Fundchance | höchstens | Zuschlag |
|---|---:|---:|---:|
| Alraune | 5 % | 1 | +15 |
| Belmart | 12 % | 3 | +8 |
| Donfstengel | 10 % | 2 | +9 |
| Gulmond | 15 % | 3 | +8 |
| Jorugawurzel | 15 % | 1 | +10 |
| Kairanhalm | 1 % | 1 | +18 |
| Shurinknolle | 12 % | 2 | +8 |
| Tarnele | 45 % | 5 | +4 |
| Thonnysblüte | 3 % | 1 | +15 |
| Einbeere | 40 % | 4 | +8 |
| Wirselkraut | 30 % | 2 | +4 |
| Eitriger Krötenschemel | 20 % | 2 | +4 |

- **Status:** im Code geprüft, im Spiel unbestätigt

### Jagd auf Reisen (`Quelle/seg109.c:TRV_hunt_generic`)

- **Im Code:** Sechs Reiseereignisse benutzen denselben Jagdablauf: erst schleichen alle Helden an (Schleichen, Zuschlag 1), dann schleicht der gewählte Schütze näher heran (Schleichen, Zuschlag 2), dann zwei Schüsse (Schußwaffen, Zuschlag 3).

| Ereignis | Schleichen alle | Schleichen Schütze | Schüsse |
|---|---:|---:|---:|
| 4 (Auerochse) | −2 | +4 | +10 |
| 22 (Wildschwein) | −2 | +2 | +8 |
| 41 | −2 | +2 | +8 |
| 48 | −2 | +2 | +8 |
| 70 | −1 | +3 | +8 |
| 141 (Hirsch) | −1 | +5 | +10 |

- **Status:** im Code geprüft, im Spiel unbestätigt

### Feilschen (`Quelle/seg103.c:bargain`)

- **Im Code:** Der Zuschlag beim Feilschen im Laden wird zusammengesetzt aus:
  - Anzahl verschiedener Waren: 1 → +2, 2 → +1, 3–4 → 0, 5–8 → −1, ab 9 → −2
  - Gesamtpreis in Hellern: unter 100 → +2, unter 500 → +1, unter 1000 → 0, unter 2000 → −1, sonst −2
  - gewünschter Prozentsatz: Prozent / 5 + 1 (also 10 % → +3, 20 % → +5)
  - Harika in der Gruppe: −2 (im Original auch, wenn sie in einer anderen Gruppe oder handlungsunfähig ist; Fix 78)
  - beim Schmied (Reparatur) zusätzlich +2
- **Status:** im Code geprüft, im Spiel unbestätigt

### Was Talentwerte verändert

- **Im Code:**

| Quelle | Talent | Wirkung |
|---|---|---|
| Kristallkugel (ausgerüstet) | Gefahrensinn | +2 |
| Sichel, magisch (im Gepäck) | Pflanzenkunde | +3 |
| Atmonblüte (gegessen) | alle 10 Körpertalente | +2 für 5 Stunden |
| Zauber Adlerauge | Sinnesschärfe | +7 für 6 Minuten |
| Zauber Scharfes Auge | Schußwaffen, Wurfwaffen | +3 für 3 Kampfrunden |
| Zauber Silentium | Schleichen (ganze Gruppe) | +10 für 5 Minuten |
| Rondra-Wunder | Schwerter (+ Schwert-AT) | +1 für 3 Tage |
| Efferd-Wunder | Schwimmen | +2 für 4 Tage |
| Phex-Wunder | Taschendiebstahl und Schlösser, oder Feilschen | +1 für 3 Tage |
| Rahja-Wunder | Tanzen und Betören | +2 für 7 Tage, oder +1 dauerhaft |

- **Status:** im Code geprüft, im Spiel unbestätigt

### Talente ohne jede Abfrage

- **Im Code:** Waffenlos, Hiebwaffen, Stichwaffen, Schwerter, Äxte, Speere, Zweihänder (Nahkampf) sowie Reiten, Selbstbeherrschung, Tanzen, Bekehren, Betören, Gassenwissen, Lügen, Menschenkenntnis, Schätzen, Fesseln, Tierkunde, Alte Sprachen, Geographie, Geschichte, Magiekunde, Abrichten, Fahrzeuge werden nirgends geprüft. Die sieben Nahkampftalente wirken nur über die AT/PA-Punkte beim Steigern. Tanzen und Betören werden nur durch das Rahja-Wunder verändert, Reiten und Selbstbeherrschung nur durch die Atmonblüte – geprüft werden sie nie.
- **Deutung der KI:** Steigerungspunkte in diesen Talenten sind im Spiel verschenkt, mit Ausnahme der Nahkampftalente.
- **Status:** im Code geprüft, im Spiel unbestätigt

---

## 2. Übersichtstabelle aller 52 Talente

„Proben“ zählt die Stellen im Quelltext, an denen eine Talentprobe gewürfelt wird (eine Stelle kann im Spiel mehrfach vorkommen).

| ID | Talent | Gruppe | Probe | Steig./Stufe | Proben | Wo |
|---:|---|---|---|---:|---:|---|
| 0 | [Waffenlos](#talent-0) | Kampf | keine | 1 | 0 | – |
| 1 | [Hiebwaffen](#talent-1) | Kampf | keine | 1 | 0 | – |
| 2 | [Stichwaffen](#talent-2) | Kampf | keine | 1 | 0 | – |
| 3 | [Schwerter](#talent-3) | Kampf | keine | 1 | 0 | Götterwunder |
| 4 | [Äxte](#talent-4) | Kampf | keine | 1 | 0 | – |
| 5 | [Speere](#talent-5) | Kampf | keine | 1 | 0 | – |
| 6 | [Zweihänder](#talent-6) | Kampf | keine | 1 | 0 | – |
| 7 | [Schußwaffen](#talent-7) | Kampf | Fernkampf | 1 | 8 | Wildnis und Reise, Reiseereignis, Kampf, Zauber |
| 8 | [Wurfwaffen](#talent-8) | Kampf | Fernkampf | 1 | 1 | Kampf, Zauber |
| 9 | [Akrobatik](#talent-9) | Körper | MU / GE / KK | 2 | 3 | Talent anwenden, Reiseereignis |
| 10 | [Klettern](#talent-10) | Körper | MU / GE / KK | 2 | 17 | Dungeon, Reiseereignis |
| 11 | [Körperbeherrschung](#talent-11) | Körper | MU / IN / GE | 2 | 2 | Dungeon |
| 12 | [Reiten](#talent-12) | Körper | CH / GE / KK | 2 | 0 | – |
| 13 | [Schleichen](#talent-13) | Körper | MU / IN / GE | 2 | 14 | Dungeon, Wildnis und Reise, Reiseereignis, Zauber, Gruppe |
| 14 | [Schwimmen](#talent-14) | Körper | MU / GE / KK | 2 | 6 | Dungeon, Wildnis und Reise, Gespräch / Ort, Götterwunder |
| 15 | [Selbstbeherrschung](#talent-15) | Körper | MU / KK / KK | 2 | 0 | – |
| 16 | [Tanzen](#talent-16) | Körper | MU / IN / GE | 2 | 0 | Götterwunder |
| 17 | [Verstecken](#talent-17) | Körper | CH / GE / GE | 2 | 5 | Stadt, Reiseereignis |
| 18 | [Zechen](#talent-18) | Körper | KL / IN / KK | 2 | 2 | Gespräch / Ort |
| 19 | [Bekehren](#talent-19) | Gesellschaft | KL / IN / CH | 2 | 0 | – |
| 20 | [Betören](#talent-20) | Gesellschaft | IN / CH / CH | 2 | 0 | Götterwunder |
| 21 | [Feilschen](#talent-21) | Gesellschaft | MU / KL / CH | 2 | 2 | Gespräch / Ort, Handel, Götterwunder |
| 22 | [Gassenwissen](#talent-22) | Gesellschaft | KL / IN / CH | 2 | 0 | – |
| 23 | [Lügen](#talent-23) | Gesellschaft | MU / IN / CH | 2 | 0 | – |
| 24 | [Menschenkenntnis](#talent-24) | Gesellschaft | KL / IN / CH | 2 | 0 | – |
| 25 | [Schätzen](#talent-25) | Gesellschaft | KL / IN / IN | 2 | 0 | – |
| 26 | [Fährtensuchen](#talent-26) | Natur | KL / IN / GE | 2 | 13 | Wildnis und Reise, Reiseereignis |
| 27 | [Fesseln](#talent-27) | Natur | FF / GE / KK | 2 | 0 | – |
| 28 | [Orientierung](#talent-28) | Natur | KL / IN / FF | 2 | 23 | Dungeon, Reiseereignis, Gespräch / Ort |
| 29 | [Pflanzenkunde](#talent-29) | Natur | KL / IN / IN | 2 | 17 | Wildnis und Reise, Reiseereignis, Gegenstand, Wildlager |
| 30 | [Tierkunde](#talent-30) | Natur | MU / KL / IN | 2 | 0 | – |
| 31 | [Wildnisleben](#talent-31) | Natur | IN / FF / GE | 2 | 46 | Wildnis und Reise, Reiseereignis, Gespräch / Ort, Wildlager |
| 32 | [Alchimie](#talent-32) | Wissen | MU / KL / FF | 3 | 1 | Talent anwenden |
| 33 | [Alte Sprachen](#talent-33) | Wissen | KL / KL / IN | 3 | 0 | – |
| 34 | [Geographie](#talent-34) | Wissen | KL / KL / IN | 3 | 0 | – |
| 35 | [Geschichte](#talent-35) | Wissen | KL / KL / IN | 3 | 0 | – |
| 36 | [Götter/Kulte](#talent-36) | Wissen | KL / IN / CH | 3 | 2 | Dungeon |
| 37 | [Kriegskunst](#talent-37) | Wissen | MU / KL / CH | 3 | 7 | Reiseereignis |
| 38 | [Lesen](#talent-38) | Wissen | KL / KL / FF | 3 | 1 | Dungeon |
| 39 | [Magiekunde](#talent-39) | Wissen | KL / KL / FF | 3 | 0 | – |
| 40 | [Sprachen](#talent-40) | Wissen | KL / IN / CH | 3 | 2 | Dungeon, Gegenstand |
| 41 | [Abrichten](#talent-41) | Handwerk | MU / IN / CH | 2 | 0 | – |
| 42 | [Fahrzeuge](#talent-42) | Handwerk | IN / CH / FF | 2 | 0 | – |
| 43 | [Falschspiel](#talent-43) | Handwerk | MU / CH / FF | 2 | 2 | Gespräch / Ort, Talent anwenden |
| 44 | [Heilen Gift](#talent-44) | Handwerk | MU / KL / IN | 2 | 3 | Talent anwenden |
| 45 | [Heilen Krankheiten](#talent-45) | Handwerk | MU / KL / CH | 2 | 2 | Talent anwenden |
| 46 | [Heilen Wunden](#talent-46) | Handwerk | KL / CH / FF | 2 | 2 | Talent anwenden |
| 47 | [Musizieren](#talent-47) | Handwerk | KL / IN / FF | 2 | 1 | Talent anwenden |
| 48 | [Schlösser](#talent-48) | Handwerk | IN / FF / FF | 2 | 23 | Dungeon, Götterwunder |
| 49 | [Taschendiebstahl](#talent-49) | Handwerk | MU / IN / FF | 2 | 1 | Talent anwenden, Götterwunder |
| 50 | [Gefahrensinn](#talent-50) | Intuition | KL / IN / IN | 1 | 10 | Dungeon, Reiseereignis, Gegenstand |
| 51 | [Sinnesschärfe](#talent-51) | Intuition | KL / IN / IN | 1 | 36 | Stadt, Dungeon, Reiseereignis, Zauber |

---

## 3. Alle 52 Talente im Detail

### Gruppe Kampf

<a id="talent-0"></a>
#### 0. Waffenlos

- **Konstante:** `TA_WAFFENLOS` · **Probe:** keine (Nahkampftalent) · **Steigerungen je Stufe:** 1 · **Proben im Code:** 0
- **Im Code:** Keine Talentprobe: test_talent() gibt für Nahkampftalente sofort 0 zurück. Beim Steigern gibt jeder gelungene Punkt zusätzlich +1 Attacke oder +1 Parade für diese Waffenart (Profi-Modus: Spieler wählt; Anfängermodus: automatisch auf den kleineren Wert, bei Gleichstand Attacke). Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Kampftalent: Der TaW selbst wird nie geprüft. Wichtig ist nur, dass jede Steigerung einen Punkt auf Attacke oder Parade für diese Waffenart gibt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-1"></a>
#### 1. Hiebwaffen

- **Konstante:** `TA_HIEBWAFFEN` · **Probe:** keine (Nahkampftalent) · **Steigerungen je Stufe:** 1 · **Proben im Code:** 0
- **Im Code:** Keine Talentprobe: test_talent() gibt für Nahkampftalente sofort 0 zurück. Beim Steigern gibt jeder gelungene Punkt zusätzlich +1 Attacke oder +1 Parade für diese Waffenart (Profi-Modus: Spieler wählt; Anfängermodus: automatisch auf den kleineren Wert, bei Gleichstand Attacke). Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wie alle Nahkampftalente: zählt nur über den AT/PA-Punkt beim Steigern.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-2"></a>
#### 2. Stichwaffen

- **Konstante:** `TA_STICHWAFFEN` · **Probe:** keine (Nahkampftalent) · **Steigerungen je Stufe:** 1 · **Proben im Code:** 0
- **Im Code:** Keine Talentprobe: test_talent() gibt für Nahkampftalente sofort 0 zurück. Beim Steigern gibt jeder gelungene Punkt zusätzlich +1 Attacke oder +1 Parade für diese Waffenart (Profi-Modus: Spieler wählt; Anfängermodus: automatisch auf den kleineren Wert, bei Gleichstand Attacke). Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wie alle Nahkampftalente: zählt nur über den AT/PA-Punkt beim Steigern.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-3"></a>
#### 3. Schwerter

- **Konstante:** `TA_SCHWERTER` · **Probe:** keine (Nahkampftalent) · **Steigerungen je Stufe:** 1 · **Proben im Code:** 0
- **Im Code:** Keine Talentprobe: test_talent() gibt für Nahkampftalente sofort 0 zurück. Beim Steigern gibt jeder gelungene Punkt zusätzlich +1 Attacke oder +1 Parade für diese Waffenart (Profi-Modus: Spieler wählt; Anfängermodus: automatisch auf den kleineren Wert, bei Gleichstand Attacke).

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Götterwunder | Rondra |  |  | Schwerter +1 und Schwert-AT +1 für 3 Tage | `Quelle/seg062.c:ask_miracle` |

- **Deutung der KI:** Wie alle Nahkampftalente: zählt nur über den AT/PA-Punkt beim Steigern. Das Rondra-Wunder erhöht neben dem TaW auch die AT, und nur die AT wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-4"></a>
#### 4. Äxte

- **Konstante:** `TA_AEXTE` · **Probe:** keine (Nahkampftalent) · **Steigerungen je Stufe:** 1 · **Proben im Code:** 0
- **Im Code:** Keine Talentprobe: test_talent() gibt für Nahkampftalente sofort 0 zurück. Beim Steigern gibt jeder gelungene Punkt zusätzlich +1 Attacke oder +1 Parade für diese Waffenart (Profi-Modus: Spieler wählt; Anfängermodus: automatisch auf den kleineren Wert, bei Gleichstand Attacke). Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wie alle Nahkampftalente: zählt nur über den AT/PA-Punkt beim Steigern.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-5"></a>
#### 5. Speere

- **Konstante:** `TA_SPEERE` · **Probe:** keine (Nahkampftalent) · **Steigerungen je Stufe:** 1 · **Proben im Code:** 0
- **Im Code:** Keine Talentprobe: test_talent() gibt für Nahkampftalente sofort 0 zurück. Beim Steigern gibt jeder gelungene Punkt zusätzlich +1 Attacke oder +1 Parade für diese Waffenart (Profi-Modus: Spieler wählt; Anfängermodus: automatisch auf den kleineren Wert, bei Gleichstand Attacke). Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wie alle Nahkampftalente: zählt nur über den AT/PA-Punkt beim Steigern.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-6"></a>
#### 6. Zweihänder

- **Konstante:** `TA_ZWEIHAENDER` · **Probe:** keine (Nahkampftalent) · **Steigerungen je Stufe:** 1 · **Proben im Code:** 0
- **Im Code:** Keine Talentprobe: test_talent() gibt für Nahkampftalente sofort 0 zurück. Beim Steigern gibt jeder gelungene Punkt zusätzlich +1 Attacke oder +1 Parade für diese Waffenart (Profi-Modus: Spieler wählt; Anfängermodus: automatisch auf den kleineren Wert, bei Gleichstand Attacke). Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wie alle Nahkampftalente: zählt nur über den AT/PA-Punkt beim Steigern.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-7"></a>
#### 7. Schußwaffen

- **Konstante:** `TA_SCHUSSWAFFEN` · **Probe:** Fernkampfprobe: (KL + GE + KK) / 4 + TaW gegen W20 · **Steigerungen je Stufe:** 1 · **Proben im Code:** 8
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Wildnis und Reise | Jagd: erster Schuss | je Ereignis | vom Spieler gewählter Schütze | Treffer: Beute (Nahrung, AP) | `Quelle/seg109.c:TRV_hunt_generic` |
| Wildnis und Reise | Jagd: zweiter Schuss | je Ereignis | vom Spieler gewählter Schütze | Treffer: Beute (Nahrung, AP) | `Quelle/seg109.c:TRV_hunt_generic` |
| Reiseereignis | Ereignis 111: Skelellen – Phexcaer (Meile 63) | +12 | vom Spieler gewählter Held | Nashorn: drei Schüsse, jeder +12 | `Quelle/seg114.c:tevent_111` |
| Reiseereignis | Ereignis 111: Skelellen – Phexcaer (Meile 63) | +12 | vom Spieler gewählter Held | Nashorn: drei Schüsse, jeder +12 | `Quelle/seg114.c:tevent_111` |
| Reiseereignis | Ereignis 111: Skelellen – Phexcaer (Meile 63) | +12 | vom Spieler gewählter Held | Nashorn: drei Schüsse, jeder +12 | `Quelle/seg114.c:tevent_111` |
| Wildnis und Reise | Jagd auf Karene | ±0 | jeder Held einzeln | zählt die Treffer | `Quelle/seg117.c:hunt_karen` |
| Wildnis und Reise | Jagd auf Wildschweine | ±0 | jeder Held einzeln | zählt die Treffer | `Quelle/seg117.c:hunt_wildboar` |
| Kampf | Fernkampftreffer |  |  | jeder Schuss ist eine Fernkampfprobe; Zuschlag = Waffengrundwert + 2 × Entfernungsstufe − 2 × Zielgröße; Misslingen = kein Schaden | `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage` |
| Zauber | Scharfes Auge |  |  | +3 für 3 Kampfrunden | `Quelle/seg100.c:spell_scharfes_auge` |
| Anzeige | Statusseite |  |  | zeigt TaW + Fernkampf-Basiswert als „Probe“ | `Quelle/seg046.c:status_show` |

- **Deutung der KI:** Sehr wichtig für Fernkämpfer: Jeder Schuss ist eine Probe. Dazu kommen die Jagden auf Reisen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-8"></a>
#### 8. Wurfwaffen

- **Konstante:** `TA_WURFWAFFEN` · **Probe:** Fernkampfprobe: (KL + GE + KK) / 4 + TaW gegen W20 · **Steigerungen je Stufe:** 1 · **Proben im Code:** 1
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Kampf | Fernkampftreffer mit Wurfwaffen |  |  | wie Schußwaffen, nur mit dem Wurfwaffen-TaW | `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage` |
| Zauber | Scharfes Auge |  |  | +3 für 3 Kampfrunden | `Quelle/seg100.c:spell_scharfes_auge` |
| Anzeige | Statusseite |  |  | zeigt TaW + Fernkampf-Basiswert als „Probe“ | `Quelle/seg046.c:status_show` |

- **Deutung der KI:** Lohnt nur, wer wirklich mit Wurfwaffen kämpft. Außerhalb des Kampfes wird es nirgends abgefragt.
- **Status:** im Code geprüft, im Spiel unbestätigt

### Gruppe Körper

<a id="talent-9"></a>
#### 9. Akrobatik

- **Konstante:** `TA_AKROBATIK` · **Probe:** MU / GE / KK · **Steigerungen je Stufe:** 2 · **Proben im Code:** 3
- **Im Code:** Im Talentmenü in Tavernen und auf dem Markt: Gelingen bringt 10–200 Heller; danach ist Akrobatik 8 Stunden gesperrt, auch nach Misslingen.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Talent anwenden | Talent anwenden: Akrobatik vorführen | ±0 (Taverne: +50, wenn vor weniger als 2 Minuten gespeichert wurde) | vom Spieler gewählter Held | Gelingen: 10–200 Heller; danach 8 Stunden Pause | `Quelle/seg103.c:use_talent` |
| Reiseereignis | Ereignis 98: Rybon – Thoss (Meile 35) | ±0 | jeder Held einzeln | Spalt überspringen: erst ein Freiwilliger, dann alle anderen; Misslingen → der Held verschwindet und taucht im Efferd-Tempel von Liskor wieder auf | `Quelle/seg113.c:tevent_098` |
| Reiseereignis | Ereignis 98: Rybon – Thoss (Meile 35) | ±0 | vom Spieler gewählter Held | Spalt überspringen: erst ein Freiwilliger, dann alle anderen; Misslingen → der Held verschwindet und taucht im Efferd-Tempel von Liskor wieder auf | `Quelle/seg113.c:tevent_098` |
| Kraut | Atmonblüte |  |  | +2 für 5 Stunden (wirkt auf alle zehn Körpertalente) | `Quelle/seg108.c:consume` |

- **Deutung der KI:** Ein kleines Zubrot in der Stadt plus ein einziges Reiseereignis. Weil dort ein Misslingen den Helden verschwinden lässt, ist ein Punkt mehr nicht verkehrt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-10"></a>
#### 10. Klettern

- **Konstante:** `TA_KLETTERN` · **Probe:** MU / GE / KK · **Steigerungen je Stufe:** 2 · **Proben im Code:** 17
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Dungeon: Totenschiff | Klettern (Fix 93: der erste handlungsfähige Held) | ±0 | erster einsatzfähiger Held | Gelingen: Weg frei | `Quelle/seg077.c:DNG01_handler` |
| Dungeon: Totenschiff | Klettern (Originalzweig) | ±0 | Held auf Platz 1 | im Original klettert immer der Held auf Platz 1 | `Quelle/seg077.c:DNG01_handler` |
| Dungeon: Spinnenhöhle | Fallgrube mit Speeren umklettern | +2 | jeder Held einzeln | Misslingen: Sturz mit Schaden | `Quelle/seg079.c:DNG03_handler` |
| Dungeon: Orkbehausung | Kletterstelle (Fix 111: bei Platz 1 beginnen) | +2 | jeder Held einzeln | Misslingen: Sturz mit Schaden | `Quelle/seg083.c:DNG08_handler` |
| Dungeon: Zwingfeste | Abstieg in der Seilschaft (nur mit mindestens 5 Seilen/Strickleitern/Magierstäben) | ±0 | jeder Held einzeln | Misslingen: Sturz | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwingfeste | Abstieg zur Ebene 4 | +4 | jeder Held einzeln | Misslingen: Sturz | `Quelle/seg087.c:DNG14_handler` |
| Reiseereignis | Ereignis 57: Ottarje – Orvil (Meile 18) | +3 | jeder Held einzeln | Erdrutsch überklettern: Misslingen → Sturz; schwer nur ohne Seil und Strickleiter (Fix 158) | `Quelle/seg111.c:tevent_057` |
| Reiseereignis | Ereignis 60: Ottarje – Orvil (Meile 33) | ±0 | jeder Held einzeln | Abstieg in eine Klamm: Misslingen → Sturz, Zeitverlust | `Quelle/seg111.c:tevent_060` |
| Reiseereignis | Ereignis 107: Skelellen – Phexcaer (Meile 36) | +1 | jeder Held einzeln | steiler Hang: Misslingen → Sturz mit LE-Verlust | `Quelle/seg113.c:tevent_107` |
| Reiseereignis | Ereignis 110: Skelellen – Phexcaer (Meile 57) | ±0 | jeder Held einzeln | Kletterstelle: Misslingen → Sturz mit LE-Verlust | `Quelle/seg114.c:tevent_110` |
| Reiseereignis | Ereignis 123: Orvil – Rovik (Meile 14) | ±0 | jeder Held einzeln | Hängebrücke (zusammen mit Höhenangst-Probe); der Zweig „Held verschwindet“ ist im Original unerreichbar | `Quelle/seg114.c:tevent_123` |
| Reiseereignis | Ereignis 135: Rundweg bei Einsiedlersee (Meile 20) | −1 | vom Spieler gewählter Held | Monolith am Einsiedlersee: fünf Kletterproben nacheinander (−1, +1, ±0, +2, +1), jede weitere nur nach Gelingen der vorigen | `Quelle/seg116.c:tevent_135` |
| Reiseereignis | Ereignis 135: Rundweg bei Einsiedlersee (Meile 20) | +1 | vom Spieler gewählter Held | Monolith am Einsiedlersee: fünf Kletterproben nacheinander (−1, +1, ±0, +2, +1), jede weitere nur nach Gelingen der vorigen | `Quelle/seg116.c:tevent_135` |
| Reiseereignis | Ereignis 135: Rundweg bei Einsiedlersee (Meile 20) | ±0 | vom Spieler gewählter Held | Monolith am Einsiedlersee: fünf Kletterproben nacheinander (−1, +1, ±0, +2, +1), jede weitere nur nach Gelingen der vorigen | `Quelle/seg116.c:tevent_135` |
| Reiseereignis | Ereignis 135: Rundweg bei Einsiedlersee (Meile 20) | +2 | vom Spieler gewählter Held | Monolith am Einsiedlersee: fünf Kletterproben nacheinander (−1, +1, ±0, +2, +1), jede weitere nur nach Gelingen der vorigen | `Quelle/seg116.c:tevent_135` |
| Reiseereignis | Ereignis 135: Rundweg bei Einsiedlersee (Meile 20) | +1 | vom Spieler gewählter Held | Monolith am Einsiedlersee: fünf Kletterproben nacheinander (−1, +1, ±0, +2, +1), jede weitere nur nach Gelingen der vorigen | `Quelle/seg116.c:tevent_135` |
| Reiseereignis | Ereignis 124: Orvil – Rovik (Meile 17) | −2 | jeder Held einzeln | Weg an der Steilklippe: Misslingen → Sturz; ohne Kletterzeug ist ein Patzer tödlich (Fix 185: nur der echte Patzer) | `Quelle/seg118.c:tevent_124` |
| Kraut | Atmonblüte |  |  | +2 für 5 Stunden (wirkt auf alle zehn Körpertalente) | `Quelle/seg108.c:consume` |

- **Deutung der KI:** Häufig und mit echtem Schaden: Stürze auf Reisen und in mehreren Dungeons, dazu ein möglicher tödlicher Patzer an der Steilklippe. Für alle Helden wichtig, weil meist jeder einzeln würfelt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-11"></a>
#### 11. Körperbeherrschung

- **Konstante:** `TA_KOERPERBEHERRSCHUNG` · **Probe:** MU / IN / GE · **Steigerungen je Stufe:** 2 · **Proben im Code:** 2
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Dungeon: Daspota-Schatz | Sprung über eine Grube | RS-Behinderung | jeder Held einzeln | Misslingen: Sturz; Zuschlag = Rüstungsbehinderung (Fix 86: nur handlungsfähige Helden springen) | `Quelle/seg081.c:DNG06_handler` |
| Dungeon: Daspota-Schatz | herabfallende Steinscheibe | ±0 | erster einsatzfähiger Held | nur wer Körperbeherrschung UND Gefahrensinn besteht, weicht aus | `Quelle/seg081.c:DNG06_handler` |
| Kraut | Atmonblüte |  |  | +2 für 5 Stunden (wirkt auf alle zehn Körpertalente) | `Quelle/seg108.c:consume` |

- **Deutung der KI:** Nur zwei Stellen im Daspota-Schatz. Schwere Rüstung macht den Sprung dort schwerer.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-12"></a>
#### 12. Reiten

- **Konstante:** `TA_REITEN` · **Probe:** CH / GE / KK · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Keine einzige Probe im Spiel; nur die unten genannten Stellen verändern den Wert.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Kraut | Atmonblüte |  |  | +2 für 5 Stunden (wirkt auf alle zehn Körpertalente) | `Quelle/seg108.c:consume` |

- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-13"></a>
#### 13. Schleichen

- **Konstante:** `TA_SCHLEICHEN` · **Probe:** MU / IN / GE · **Steigerungen je Stufe:** 2 · **Proben im Code:** 14
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Dungeon: Wolfshöhle | einsturzgefährdete Decke | +2 | jeder Held einzeln | Misslingen: Steine treffen den Helden | `Quelle/seg080.c:DNG04_handler` |
| Dungeon: Wolfshöhle | einsturzgefährdete Decke | +4 | jeder Held einzeln | Misslingen: Steine treffen den Helden | `Quelle/seg080.c:DNG04_handler` |
| Dungeon: Drachenhort | Drachenhort plündern | RS-Behinderung + 3 | jeder Held einzeln | nur wenn KEIN Held misslingt, kann der Hort geplündert werden; Zuschlag = Rüstungsbehinderung + 3 | `Quelle/seg085.c:DNG10_handler` |
| Dungeon: Verlassene Mine | einstürzende Decke | −4 | jeder Held einzeln | Misslingen: Steine treffen den Helden (leicht, −4) | `Quelle/seg091.c:DNG13_collapsing_ceiling` |
| Dungeon: Verlassene Mine | einstürzende Decke (schwere Variante) | −1 | jeder Held einzeln | Misslingen: Steine treffen den Helden | `Quelle/seg091.c:DNG13_collapsing_ceiling_hard` |
| Wildnis und Reise | Jagd (gemeinsamer Baustein der Reiseereignisse) | je Ereignis | jeder Held einzeln | erst schleichen alle Helden an | `Quelle/seg109.c:TRV_hunt_generic` |
| Wildnis und Reise | Jagd | je Ereignis | vom Spieler gewählter Schütze | dann schleicht sich der Schütze näher heran | `Quelle/seg109.c:TRV_hunt_generic` |
| Reiseereignis | Ereignis 64: Ottarje – Orvil (Meile 58); Skjal – Orvil (Meile 61) | ±0 | jeder Held einzeln | nur mit Umbriks Auftrag: ab zwei misslungenen Proben gibt es einen Hinterhalt | `Quelle/seg111.c:tevent_064` |
| Reiseereignis | Ereignis 66: Orvil – Ala (Meile 19) | −2 | jeder Held einzeln | schon eine misslungene Probe verrät die Gruppe | `Quelle/seg111.c:tevent_066` |
| Reiseereignis | Ereignis 75: Felsteyn – Orkanger (Meile 25) | ±0 | jeder Held einzeln | Räuberlager: ab zwei misslungenen Proben wird die Gruppe entdeckt | `Quelle/seg112.c:tevent_075` |
| Reiseereignis | Ereignis 111: Skelellen – Phexcaer (Meile 63) | −5 | jeder Held einzeln | Nashorn anschleichen: erst alle (−5), dann der gewählte Held (±0) | `Quelle/seg114.c:tevent_111` |
| Reiseereignis | Ereignis 111: Skelellen – Phexcaer (Meile 63) | ±0 | vom Spieler gewählter Held | Nashorn anschleichen: erst alle (−5), dann der gewählte Held (±0) | `Quelle/seg114.c:tevent_111` |
| Wildnis und Reise | Jagd auf Karene | +2 | jeder Held einzeln | zählt die Helden mit gelungener Probe | `Quelle/seg117.c:hunt_karen` |
| Wildnis und Reise | Jagd auf Wildschweine | ±0 | jeder Held einzeln | zählt die Helden mit gelungener Probe | `Quelle/seg117.c:hunt_wildboar` |
| Zauber | Silentium |  |  | +10 für alle Helden der Gruppe, 5 Minuten | `Quelle/seg101.c:spell_silentium` |
| Gruppe | Abschied von Harika |  |  | alle Helden der Gruppe bekommen bis zu drei freie Steigerungsversuche (im Original auf Kosten der nächsten Stufe, Fix 255) | `Quelle/seg095.c:npc_farewell` |
| Kraut | Atmonblüte |  |  | +2 für 5 Stunden (wirkt auf alle zehn Körpertalente) | `Quelle/seg108.c:consume` |

- **Deutung der KI:** Wird oft für alle Helden einzeln geprüft (Jagd, Räuberlager, Drachenhort). Ein schwacher Schleicher verdirbt es der ganzen Gruppe. Silentium hilft enorm.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-14"></a>
#### 14. Schwimmen

- **Konstante:** `TA_SCHWIMMEN` · **Probe:** MU / GE / KK · **Steigerungen je Stufe:** 2 · **Proben im Code:** 6
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Dungeon: Zwingfeste | Tauchen (Ebenen 4 und 5): der Taucher | +8 | vom Spieler gewählter Held | Misslingen: Schaden im Wasser | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwingfeste | Tauchen: die übrigen Helden folgen | +2 / +8 | jeder Held einzeln | Misslingen: Schaden; Zuschlag +2, wenn der Taucher ein Seil hinübergebracht hat, ohne Tauchausrüstung +8 | `Quelle/seg088.c:DNG14_dive` |
| Wildnis und Reise | Fluss durchschwimmen (Furten) | ±0 | jeder Held einzeln | Gelingen: sicher am anderen Ufer | `Quelle/seg110.c:TRV_swim2` |
| Wildnis und Reise | Fluss durchschwimmen (Furten) | +2 (schwimmen) oder −1 (andere Wahl) | jeder Held einzeln | Misslingen: Krankheitsprobe Dumpfschädel, W5 LE, evtl. Gegenstand verloren | `Quelle/seg112.c:TRV_swimm` |
| Wildnis und Reise | Seereise: Krakenangriff | ±0 | betroffener Held | Misslingen: der über Bord gerissene Held hat Probleme im Wasser | `Quelle/seg117.c:octopus_attack` |
| Gespräch / Ort | Weg zu Hyggeliks Ruine (Gespräch) | +5 | zufällig bestimmter Held | Gelingen/Misslingen beim Schwimmen entscheidet den Verlauf | `Quelle/seg117.c:TLK_way_to_ruin` |
| Götterwunder | Efferd |  |  | +2 für 4 Tage | `Quelle/seg062.c:ask_miracle` |
| Kraut | Atmonblüte |  |  | +2 für 5 Stunden (wirkt auf alle zehn Körpertalente) | `Quelle/seg108.c:consume` |

- **Deutung der KI:** Bei Furten muss jeder durch, Misslingen kostet LE und manchmal Ausrüstung. Wichtig für die Zwingfeste.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-15"></a>
#### 15. Selbstbeherrschung

- **Konstante:** `TA_SELBSTBEHERRSCHUNG` · **Probe:** MU / KK / KK · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Keine einzige Probe im Spiel; nur die unten genannten Stellen verändern den Wert.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Kraut | Atmonblüte |  |  | +2 für 5 Stunden (wirkt auf alle zehn Körpertalente) | `Quelle/seg108.c:consume` |

- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-16"></a>
#### 16. Tanzen

- **Konstante:** `TA_TANZEN` · **Probe:** MU / IN / GE · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Keine einzige Probe im Spiel; nur die unten genannten Stellen verändern den Wert.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Götterwunder | Rahja |  |  | +2 für 7 Tage (zusammen mit Betören); ein anderes Rahja-Wunder gibt +1 dauerhaft | `Quelle/seg062.c:ask_miracle` |
| Kraut | Atmonblüte |  |  | +2 für 5 Stunden (wirkt auf alle zehn Körpertalente) | `Quelle/seg108.c:consume` |

- **Deutung der KI:** Wird nie geprüft. Nur das Rahja-Wunder verändert den Wert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-17"></a>
#### 17. Verstecken

- **Konstante:** `TA_VERSTECKEN` · **Probe:** CH / GE / GE · **Steigerungen je Stufe:** 2 · **Proben im Code:** 5
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Stadt | Einbruch in ein Haus | −2 | jeder Held einzeln | jeder Held muss bestehen; Misslingen in Thorwal, Prem, Phexcaer oder Oberorken: Wachen erwischen die Gruppe (Fix 86: nur handlungsfähige Helden) | `Quelle/seg025.c:do_house` |
| Reiseereignis | Ereignis 92: Felsteyn – Einsiedlersee, die alte Frau | −5 | jeder Held einzeln | Verstecken vor der Alten (Fix 153: nur lebende Helden der aktiven Gruppe) | `Quelle/seg116.c:TLK_old_woman` |
| Reiseereignis | Ereignis 92: Felsteyn – Einsiedlersee, die alte Frau | −5 | jeder Held einzeln | wie oben (zweite Abfrage derselben Stelle) | `Quelle/seg116.c:TLK_old_woman` |
| Reiseereignis | Ereignis 92: Felsteyn – Einsiedlersee, die alte Frau | −5 | jeder Held einzeln | Verstecken, zweite Situation | `Quelle/seg116.c:TLK_old_woman` |
| Reiseereignis | Ereignis 92: Felsteyn – Einsiedlersee, die alte Frau | −5 | jeder Held einzeln | wie oben (zweite Abfrage derselben Stelle) | `Quelle/seg116.c:TLK_old_woman` |
| Kraut | Atmonblüte |  |  | +2 für 5 Stunden (wirkt auf alle zehn Körpertalente) | `Quelle/seg108.c:consume` |

- **Deutung der KI:** Nur beim Einbruch in Häuser und bei der alten Frau. Beim Einbruch zählt der schwächste Held.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-18"></a>
#### 18. Zechen

- **Konstante:** `TA_ZECHEN` · **Probe:** KL / IN / KK · **Steigerungen je Stufe:** 2 · **Proben im Code:** 2
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Gespräch / Ort | Taverne: Wetttrinken | Runden − 8 | jeder Held der Reihe nach (Fix 60: nur aktive Gruppe) | Gelingen: Held bleibt nüchtern; Zuschlag = Anzahl getrunkener Runden − 8 | `Quelle/seg060.c:TLK_tavern` |
| Gespräch / Ort | Gespräch mit Tiomar (Informant): Trinken | ±0 | zufälliger Held | Ergebnis entscheidet den Gesprächsverlauf | `Quelle/seg072.c:INF_eliane_tiomar` |
| Kraut | Atmonblüte |  |  | +2 für 5 Stunden (wirkt auf alle zehn Körpertalente) | `Quelle/seg108.c:consume` |

- **Deutung der KI:** Nur fürs Wetttrinken in der Taverne und beim Informanten Tiomar.
- **Status:** im Code geprüft, im Spiel unbestätigt

### Gruppe Gesellschaft

<a id="talent-19"></a>
#### 19. Bekehren

- **Konstante:** `TA_BEKEHREN` · **Probe:** KL / IN / CH · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-20"></a>
#### 20. Betören

- **Konstante:** `TA_BETOEREN` · **Probe:** IN / CH / CH · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Keine einzige Probe im Spiel; nur die unten genannten Stellen verändern den Wert.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Götterwunder | Rahja |  |  | +2 für 7 Tage (zusammen mit Tanzen); ein anderes Rahja-Wunder gibt +1 dauerhaft | `Quelle/seg062.c:ask_miracle` |

- **Deutung der KI:** Wird nie geprüft. Nur das Rahja-Wunder verändert den Wert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-21"></a>
#### 21. Feilschen

- **Konstante:** `TA_FEILSCHEN` · **Probe:** MU / KL / CH · **Steigerungen je Stufe:** 2 · **Proben im Code:** 2
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Gespräch / Ort | Händlergespräch | ±0 | erster einsatzfähiger Held | Gelingen: Preisnachlass im Gespräch; im Original verschlechterte dasselbe Gelingen zugleich die Verkaufspreise (Fix 53 nimmt das heraus) | `Quelle/seg055.c:TLK_whandel` |
| Handel | Feilschen bei Kauf, Verkauf und Schmied | berechnet (siehe oben) | vom Spieler gewählter Held (Vorauswahl: bester Feilscher) | Gelingen: der gewünschte Preisnachlass bzw. -aufschlag wird akzeptiert | `Quelle/seg103.c:bargain` |
| Götterwunder | Phex |  |  | +1 für 3 Tage | `Quelle/seg062.c:ask_miracle` |
| Handel | Kaufen und Verkaufen |  |  | Vorauswahl im Heldenmenü: der Held mit der höchsten Summe aus den drei Eigenschaften und dem TaW | `Quelle/seg056.c:buy_screen` |
| Handel | Verkaufen |  |  | Vorauswahl wie beim Kaufen | `Quelle/seg057.c:sell_screen` |

- **Deutung der KI:** Spart bei jedem Einkauf Geld. Die Formel belohnt große Einkäufe mit mehreren Waren. Harika in der Gruppe gibt noch einmal −2.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-22"></a>
#### 22. Gassenwissen

- **Konstante:** `TA_GASSENWISSEN` · **Probe:** KL / IN / CH · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-23"></a>
#### 23. Lügen

- **Konstante:** `TA_LUEGEN` · **Probe:** MU / IN / CH · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-24"></a>
#### 24. Menschenkenntnis

- **Konstante:** `TA_MENSCHENKENNTNIS` · **Probe:** KL / IN / CH · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-25"></a>
#### 25. Schätzen

- **Konstante:** `TA_SCHAETZEN` · **Probe:** KL / IN / IN · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

### Gruppe Natur

<a id="talent-26"></a>
#### 26. Fährtensuchen

- **Konstante:** `TA_FAEHRTENSUCHEN` · **Probe:** KL / IN / GE · **Steigerungen je Stufe:** 2 · **Proben im Code:** 13
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Wildnis und Reise | Wildlager: Vorräte auffüllen, Nahrung | +5 + Platz | derselbe Held wie bei der Wassersuche | Gelingen: Hunger der Gruppe weg (Firun-Wunder: gelingt automatisch) | `Quelle/seg051.c:replenish_stocks` |
| Reiseereignis | Ereignis 4: Serske – Breida (Meile 13) | +2 | erster einsatzfähiger Held | Fährte von Waldrindern: Gelingen ermöglicht die Jagd auf einen Auerochsen (einmal gefunden, bleibt es bekannt) | `Quelle/seg109.c:tevent_004` |
| Reiseereignis | Ereignis 22: Rukian – Fähre Angbodirtal (Meile 15) | +5 | erster einsatzfähiger Held | windgeschützter Rastplatz: Gelingen ermöglicht danach eine Wildschweinjagd (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_022` |
| Reiseereignis | Ereignis 41: Varnheim – Auplog (Meile 25) | ±0 | erster einsatzfähiger Held | Bach mit zerstörter Hängebrücke: Gelingen ermöglicht eine Jagd | `Quelle/seg110.c:tevent_041` |
| Reiseereignis | Ereignis 48: Daspota – Ottarje (Meile 26) | +3 | erster einsatzfähiger Held | Rastplatz: Gelingen ermöglicht eine Jagd (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_048` |
| Reiseereignis | Ereignis 61: Ottarje – Orvil (Meile 41) | +3 | erster einsatzfähiger Held | Spur nahe dem Eingang der Wolfshöhle (zusammen mit Sinnesschärfe +6) | `Quelle/seg111.c:tevent_061` |
| Reiseereignis | Ereignis 66: Orvil – Ala (Meile 19) | +4 | erster einsatzfähiger Held | Spur wilder Hunde, danach evtl. Kampf | `Quelle/seg111.c:tevent_066` |
| Reiseereignis | Ereignis 70: Ala – Thoss (Meile 29); Tjanset – Thoss (Meile 34) | ±0 | erster einsatzfähiger Held | Fährte gefunden, danach Jagd möglich (einmal gefunden, bleibt es bekannt) | `Quelle/seg112.c:tevent_070` |
| Reiseereignis | Ereignis 71: Thoss – Liskor (Meile 11); Oberorken – Felsteyn (Meile 7) | +4 | erster einsatzfähiger Held | Monolith am anderen Ufer des Thossel: Gelingen bringt eine Spur | `Quelle/seg112.c:tevent_071` |
| Reiseereignis | Ereignis 80: Orkanger – Clanegh (Meile 21) | +5 | der Jäger aus „Vorräte auffüllen“ (Fix 163: ohne Jäger der erste Held) | Nahrungsplatz: der Jäger aus „Vorräte auffüllen“ findet evtl. eine Tatzelwurm-Spur (Fix 163) (einmal gefunden, bleibt es bekannt) | `Quelle/seg113.c:tevent_080` |
| Reiseereignis | Ereignis 111: Skelellen – Phexcaer (Meile 63) | +1 | erster einsatzfähiger Held | Fährte eines Nashorns, danach Jagd (einmal gefunden, bleibt es bekannt) | `Quelle/seg114.c:tevent_111` |
| Reiseereignis | Ereignis 141: Querverbindung (Meile 28) | ±0 | erster einsatzfähiger Held | Hirschfährte, danach Jagd (einmal gefunden, bleibt es bekannt) | `Quelle/seg116.c:tevent_141` |
| Reiseereignis | Ereignis 51: Ottarje – Skjal (Meile 27) | +4 | erster einsatzfähiger Held | Fährte gefunden, der Spur kann gefolgt werden | `Quelle/seg118.c:tevent_051` |

- **Deutung der KI:** Wichtig für Nahrung: im Wildlager und für viele Jagden auf Reisen. Fast immer würfelt der erste Held der Gruppe, also den besten Fährtensucher nach vorn.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-27"></a>
#### 27. Fesseln

- **Konstante:** `TA_FESSELN` · **Probe:** FF / GE / KK · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-28"></a>
#### 28. Orientierung

- **Konstante:** `TA_ORIENTIERUNG` · **Probe:** KL / IN / FF · **Steigerungen je Stufe:** 2 · **Proben im Code:** 23
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Dungeon: alle | Automap in Dungeons | +6 | erster einsatzfähiger Held | bei schwachem Licht zeigt die Karte nur bei gelungener Probe; ohne Licht gar nicht, bei vollem Licht immer | `Quelle/seg074.c:show_automap` |
| Reiseereignis | Ereignis 29: Vilnheim – Phexcaer (Meile 26) | +3 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg110.c:tevent_029` |
| Reiseereignis | Ereignis 57: Ottarje – Orvil (Meile 18) | +2 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg111.c:tevent_057` |
| Reiseereignis | Ereignis 60: Ottarje – Orvil (Meile 33) | +4 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg111.c:tevent_060` |
| Reiseereignis | Ereignis 60: Ottarje – Orvil (Meile 33) | +2 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg111.c:tevent_060` |
| Reiseereignis | Ereignis 103: Skjal – Orvil (Meile 29) | +2 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg113.c:tevent_103` |
| Reiseereignis | Ereignis 103: Skjal – Orvil (Meile 29) | +3 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg113.c:tevent_103` |
| Reiseereignis | Ereignis 103: Skjal – Orvil (Meile 29) | +4 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg113.c:tevent_103` |
| Reiseereignis | Ereignis 107: Skelellen – Phexcaer (Meile 36) | +1 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg113.c:tevent_107` |
| Reiseereignis | Ereignis 110: Skelellen – Phexcaer (Meile 57) | ±0 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg114.c:tevent_110` |
| Reiseereignis | Ereignis 96: Rybon – Thoss (Meile 21) | +2 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg115.c:tevent_096` |
| Reiseereignis | Ereignis 96: Rybon – Thoss (Meile 21) | +4 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg115.c:tevent_096` |
| Reiseereignis | Ereignis 96: Rybon – Thoss (Meile 21) | +3 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg115.c:tevent_096` |
| Reiseereignis | Ereignis 139: Querverbindung (Meile 10) | +3 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg116.c:tevent_139` |
| Reiseereignis | Ereignis 139: Querverbindung (Meile 10) | +5 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg116.c:tevent_139` |
| Reiseereignis | Ereignis 139: Querverbindung (Meile 10) | +3 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg116.c:tevent_139` |
| Reiseereignis | Ereignis 143: Querverbindung (Meile 43) | +2 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg116.c:tevent_143` |
| Reiseereignis | Ereignis 143: Querverbindung (Meile 43) | +4 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg116.c:tevent_143` |
| Reiseereignis | Ereignis 143: Querverbindung (Meile 43) | +3 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg116.c:tevent_143` |
| Gespräch / Ort | Weg zu Hyggeliks Ruine (Gespräch) | +5 | Held, den der Dialog bestimmt | Gelingen: Weg gefunden, sonst Umweg | `Quelle/seg117.c:TLK_way_to_ruin` |
| Gespräch / Ort | Weg zu Hyggeliks Ruine (Gespräch) | ±0 | jeder Held einzeln | zählt die Helden mit gelungener Probe | `Quelle/seg117.c:TLK_way_to_ruin` |
| Gespräch / Ort | Weg zu Hyggeliks Ruine (Gespräch) | ±0 | jeder Held einzeln | zählt die Helden mit gelungener Probe | `Quelle/seg117.c:TLK_way_to_ruin` |
| Reiseereignis | Ereignis 78: Orkanger – Clanegh (Meile 15) | +8 | erster einsatzfähiger Held | Gelingen: richtiger Weg; Misslingen: verirrt, kostet Zeit (teils auch LE) | `Quelle/seg118.c:tevent_078` |

- **Deutung der KI:** Auf Reisen spart sie Zeit und LE, im Halbdunkel der Dungeons öffnet sie die Karte. Es würfelt der erste Held.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-29"></a>
#### 29. Pflanzenkunde

- **Konstante:** `TA_PFLANZENKUNDE` · **Probe:** KL / IN / IN · **Steigerungen je Stufe:** 2 · **Proben im Code:** 17
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Wildnis und Reise | Wildlager: Kräuter suchen | je Kraut | vom Spieler gewählter Held | je Kraut: erst Fundchance in %, dann Probe mit Krautzuschlag − (Stunden − 1) + Platz | `Quelle/seg051.c:gather_herbs` |
| Reiseereignis | Ereignis 9: Breida – Peilinen (Meile 17) | +4 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg109.c:tevent_009` |
| Reiseereignis | Ereignis 13: Rovamund – Nordvest (Meile 8) | +3 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_013` |
| Reiseereignis | Ereignis 21: Kravik – Skelellen (Meile 22) | +8 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_021` |
| Reiseereignis | Ereignis 32: Vilnheim – Phexcaer (Meile 80) | +5 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_032` |
| Reiseereignis | Ereignis 36: Phexcaer – Einsiedlersee (Meile 22) | +6 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_036` |
| Reiseereignis | Ereignis 50: Ottarje – Skjal (Meile 23) | +4 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_050` |
| Reiseereignis | Ereignis 70: Ala – Thoss (Meile 29); Tjanset – Thoss (Meile 34) | +4 | erster einsatzfähiger Held | Kräuterplatz mit Jorugawurzel (Bonus wirkt im Original nicht, Fix 161) (einmal gefunden, bleibt es bekannt) | `Quelle/seg112.c:tevent_070` |
| Reiseereignis | Ereignis 83: Oberorken – Einsiedlersee (Meile 9) | +6 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg113.c:tevent_083` |
| Reiseereignis | Ereignis 85: Oberorken – Einsiedlersee (Meile 35) | +6 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg113.c:tevent_085` |
| Reiseereignis | Ereignis 112: Skelellen – Phexcaer (Meile 68) | +2 | erster einsatzfähiger Held | Rastplatz mit Thonnysblüte (Bonus wirkt im Original nicht, Fix 161) (einmal gefunden, bleibt es bekannt) | `Quelle/seg114.c:tevent_112` |
| Reiseereignis | Ereignis 118: Merske – Rovamund (Meile 18) | +3 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg114.c:tevent_118` |
| Reiseereignis | Ereignis 91: Felsteyn – Einsiedlersee (Meile 25) | +5 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg115.c:tevent_091` |
| Reiseereignis | Ereignis 128: Runinshaven – Leuchtturm Runin (Meile 15) | +4 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg115.c:tevent_128` |
| Reiseereignis | Ereignis 132: Brendhil – Manrin (Meile 19) | +1 | erster einsatzfähiger Held | Gelingen: Kräuterplatz entdeckt (dort Kräutersuche mit Erleichterung) (einmal gefunden, bleibt es bekannt) | `Quelle/seg116.c:tevent_132` |
| Reiseereignis | Ereignis 134: Rundweg bei Einsiedlersee (Meile 17) | +6 | erster einsatzfähiger Held | Kräuterplatz mit Kairanhalm (Bonus wirkt im Original nicht, Fix 161) (einmal gefunden, bleibt es bekannt) | `Quelle/seg116.c:tevent_134` |
| Reiseereignis | Ereignis 140: Querverbindung (Meile 23) | +4 | erster einsatzfähiger Held | Rastplatz mit Jorugawurzel (Bonus wirkt im Original nicht, Fix 161) (einmal gefunden, bleibt es bekannt) | `Quelle/seg116.c:tevent_140` |
| Gegenstand | Sichel (magisch) |  |  | +3, solange sie im Gepäck ist | `Quelle/seg105.c:give_new_item_to_hero` |
| Wildlager | Kräuter suchen |  |  | Vorauswahl im Heldenmenü: der beste Kräutersucher | `Quelle/seg051.c:do_wildcamp` |

- **Deutung der KI:** Kräuter sind Geld und Heilung. Die magische Sichel (+3) macht einen echten Unterschied.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-30"></a>
#### 30. Tierkunde

- **Konstante:** `TA_TIERKUNDE` · **Probe:** MU / KL / IN · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-31"></a>
#### 31. Wildnisleben

- **Konstante:** `TA_WILDNISLEBEN` · **Probe:** IN / FF / GE · **Steigerungen je Stufe:** 2 · **Proben im Code:** 46
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Wildnis und Reise | Wildlager: Vorräte auffüllen, Wasser | +5 + Platz | vom Spieler gewählter Held (Vorauswahl: bester in Wildnisleben) | Gelingen: alle Wasserschläuche voll, Durst weg (Efferd-Wunder: gelingt automatisch) | `Quelle/seg051.c:replenish_stocks` |
| Reiseereignis | Ereignis 5: Serske – Breida (Meile 19) | ±0 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg109.c:tevent_005` |
| Wildnis und Reise | Brückenbau an einem Bach (Reiseereignisse 7 und 41) | ±0 | jeder Held einzeln | zählt die Helden mit gelungener Probe; braucht außerdem Hammer, Axt und zwei Seile/Strickleitern (Fix 153: nur lebende Helden der aktiven Gruppe) | `Quelle/seg109.c:TRV_barrier` |
| Wildnis und Reise | Brückenbau | ±0 | jeder Held einzeln | wie oben (Originalzweig ohne Fix 153) | `Quelle/seg109.c:TRV_barrier` |
| Reiseereignis | Ereignis 8: Tjoila – Breida (Meile 12) | +2 | erster einsatzfähiger Held | Gelingen: Wasser-/Nahrungsplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg109.c:tevent_008` |
| Reiseereignis | Ereignis 17: Nordvest – Kravik (Meile 19) | +2 | erster einsatzfähiger Held | Gelingen: Wasser-/Nahrungsplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_017` |
| Reiseereignis | Ereignis 22: Rukian – Fähre Angbodirtal (Meile 15) | ±0 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_022` |
| Reiseereignis | Ereignis 24: Auplog – Vilnheim (Meile 10) | +2 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_024` |
| Reiseereignis | Ereignis 25: Auplog – Vilnheim (Meile 18) | +1 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_025` |
| Reiseereignis | Ereignis 28: Vilnheim – Phexcaer (Meile 20) | +2 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_028` |
| Reiseereignis | Ereignis 30: Vilnheim – Phexcaer (Meile 48) | ±0 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_030` |
| Reiseereignis | Ereignis 32: Vilnheim – Phexcaer (Meile 80) | +2 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_032` |
| Reiseereignis | Ereignis 34: Vilnheim – Phexcaer (Meile 95) | +2 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_034` |
| Reiseereignis | Ereignis 36: Phexcaer – Einsiedlersee (Meile 22) | +3 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_036` |
| Reiseereignis | Ereignis 39: Phexcaer – Einsiedlersee (Meile 40) | +4 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_039` |
| Reiseereignis | Ereignis 40: Varnheim – Auplog (Meile 18) | +4 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_040` |
| Reiseereignis | Ereignis 42: Varnheim – Auplog (Meile 38) | +2 | erster einsatzfähiger Held | Gelingen: Wasser-/Nahrungsplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_042` |
| Reiseereignis | Ereignis 43: Varnheim – Auplog (Meile 42) | +3 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_043` |
| Reiseereignis | Ereignis 48: Daspota – Ottarje (Meile 26) | +3 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_048` |
| Reiseereignis | Ereignis 49: Ottarje – Skjal (Meile 19) | +2 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_049` |
| Reiseereignis | Ereignis 50: Ottarje – Skjal (Meile 23) | +3 | erster einsatzfähiger Held | Gelingen: Wasser-/Nahrungsplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_050` |
| Reiseereignis | Ereignis 53: Ottarje – Skjal (Meile 38) | +1 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg111.c:tevent_053` |
| Reiseereignis | Ereignis 58: Ottarje – Orvil (Meile 20) | +2 | erster einsatzfähiger Held | Gelingen: Wasser-/Nahrungsplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg111.c:tevent_058` |
| Reiseereignis | Ereignis 66: Orvil – Ala (Meile 19) | +2 | erster einsatzfähiger Held | Gelingen: Wasser-/Nahrungsplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg111.c:tevent_066` |
| Reiseereignis | Ereignis 69: Ala – Thoss (Meile 17); Tjanset – Thoss (Meile 22) | +5 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg112.c:tevent_069` |
| Reiseereignis | Ereignis 70: Ala – Thoss (Meile 29); Tjanset – Thoss (Meile 34) | +3 | erster einsatzfähiger Held | Rastplatz, danach Kräutersuche (Pflanzenkunde +4) (einmal gefunden, bleibt es bekannt) | `Quelle/seg112.c:tevent_070` |
| Reiseereignis | Ereignis 72: Oberorken – Felsteyn (Meile 12) | +3 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg112.c:tevent_072` |
| Reiseereignis | Ereignis 80: Orkanger – Clanegh (Meile 21) | +4 | erster einsatzfähiger Held | Gelingen: Wasser-/Nahrungsplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg113.c:tevent_080` |
| Reiseereignis | Ereignis 81: Clanegh – Tyldon (Meile 18) | +2 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg113.c:tevent_081` |
| Reiseereignis | Ereignis 85: Oberorken – Einsiedlersee (Meile 35) | +4 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg113.c:tevent_085` |
| Reiseereignis | Ereignis 88: Oberorken – Einsiedlersee (Meile 64); Felsteyn – Einsiedlersee (Meile 50) | +3 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg113.c:tevent_088` |
| Reiseereignis | Ereignis 106: Skelellen – Phexcaer (Meile 20) | +4 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg113.c:tevent_106` |
| Reiseereignis | Ereignis 109: Skelellen – Phexcaer (Meile 48) | +6 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg113.c:tevent_109` |
| Reiseereignis | Ereignis 112: Skelellen – Phexcaer (Meile 68) | +2 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg114.c:tevent_112` |
| Reiseereignis | Ereignis 116: Skelellen – Phexcaer (Meile 122) | +6 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg114.c:tevent_116` |
| Reiseereignis | Ereignis 119: Merske – Rovamund (Meile 38) | +2 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg114.c:tevent_119` |
| Reiseereignis | Ereignis 93: Felsteyn – Einsiedlersee (Meile 34) | +4 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg115.c:tevent_093` |
| Reiseereignis | Ereignis 128: Runinshaven – Leuchtturm Runin (Meile 15) | +2 | erster einsatzfähiger Held | Gelingen: Wasser-/Nahrungsplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg115.c:tevent_128` |
| Reiseereignis | Ereignis 132: Brendhil – Manrin (Meile 19) | +1 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg116.c:tevent_132` |
| Reiseereignis | Ereignis 137: Rundweg bei Einsiedlersee (Meile 28) | +5 | erster einsatzfähiger Held | Seeufer: Vorräte auffüllen möglich (einmal gefunden, bleibt es bekannt) | `Quelle/seg116.c:tevent_137` |
| Reiseereignis | Ereignis 138: Leuchtturm Runin – Runinshaven (Meile 18) | +2 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg116.c:tevent_138` |
| Reiseereignis | Ereignis 140: Querverbindung (Meile 23) | +2 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg116.c:tevent_140` |
| Reiseereignis | Ereignis 142: Querverbindung (Meile 39) | +2 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg116.c:tevent_142` |
| Gespräch / Ort | Weg zu Hyggeliks Ruine (Gespräch) | +6 | Held, den der Dialog bestimmt | Ergebnis entscheidet den Weg | `Quelle/seg117.c:TLK_way_to_ruin` |
| Reiseereignis | Ereignis 79: Orkanger – Clanegh (Meile 18) | +4 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg118.c:tevent_079` |
| Reiseereignis | Ereignis 120: Merske – Rovamund (Meile 55) | +3 | erster einsatzfähiger Held | Gelingen: Rastplatz entdeckt (einmal gefunden, bleibt es bekannt) | `Quelle/seg118.c:tevent_120` |
| Wildlager | Vorräte auffüllen |  |  | Vorauswahl im Heldenmenü: der beste in Wildnisleben | `Quelle/seg051.c:replenish_stocks` |

- **Deutung der KI:** Das meistgeprüfte Talent auf Reisen: Rastplätze, Wasser, Brückenbau. Es würfelt fast immer der erste Held.
- **Status:** im Code geprüft, im Spiel unbestätigt

### Gruppe Wissen

<a id="talent-32"></a>
#### 32. Alchimie

- **Konstante:** `TA_ALCHIMIE` · **Probe:** MU / KL / FF · **Steigerungen je Stufe:** 3 · **Proben im Code:** 1
- **Im Code:** Im Talentmenü im Wildlager und im Gasthaus: öffnet das Brauen; die eigentliche Probe kommt beim Brauen mit dem Zuschlag des Rezepts.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Talent anwenden | Alchimie: ein Rezept brauen | Rezept | Held, der braut | Gelingen: der Trank entsteht; Zuschlag ist der Rezeptwert | `Quelle/seg104.c:do_alchemy` |

- **Deutung der KI:** Nur fürs Brauen. Der Zuschlag hängt am Rezept.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-33"></a>
#### 33. Alte Sprachen

- **Konstante:** `TA_ALTE_SPRACHEN` · **Probe:** KL / KL / IN · **Steigerungen je Stufe:** 3 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-34"></a>
#### 34. Geographie

- **Konstante:** `TA_GEOGRAPHIE` · **Probe:** KL / KL / IN · **Steigerungen je Stufe:** 3 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-35"></a>
#### 35. Geschichte

- **Konstante:** `TA_GESCHICHTE` · **Probe:** KL / KL / IN · **Steigerungen je Stufe:** 3 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-36"></a>
#### 36. Götter/Kulte

- **Konstante:** `TA_GOETTER_KULTE` · **Probe:** KL / IN / CH · **Steigerungen je Stufe:** 3 · **Proben im Code:** 2
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Dungeon: Orkbehausung | orkische Götterstatue | +4 | erster einsatzfähiger Held | Gelingen: die Helden erkennen den dargestellten Gott (Brazoragh); nur Information | `Quelle/seg083.c:DNG08_handler` |
| Dungeon: Orkbehausung | zweite orkische Götterstatue | +6 | erster einsatzfähiger Held | Gelingen: die Helden erkennen den dargestellten Gott (Tairach); nur Information | `Quelle/seg083.c:DNG08_handler` |

- **Deutung der KI:** Nur zwei Stellen in der Orkbehausung, und dort geht es nur um eine Information.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-37"></a>
#### 37. Kriegskunst

- **Konstante:** `TA_KRIEGSKUNST` · **Probe:** MU / KL / CH · **Steigerungen je Stufe:** 3 · **Proben im Code:** 7
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Reiseereignis | Ereignis 31: Vilnheim – Phexcaer (Meile 65) | +7 | erster einsatzfähiger Held | Überfall: wer die Flucht wählt und die Probe nicht schafft, kämpft, und die Gegner beginnen | `Quelle/seg110.c:tevent_031` |
| Reiseereignis | Ereignis 35: Phexcaer – Einsiedlersee (Meile 13) | +8 | erster einsatzfähiger Held | Kampf: Fluchtversuch, Misslingen → Kampf, Gegner beginnen | `Quelle/seg110.c:tevent_035` |
| Reiseereignis | Ereignis 74: Felsteyn – Orkanger (Meile 12) | +2 | erster einsatzfähiger Held | Wegelagerer im Hohlweg: Misslingen → Gegner beginnen | `Quelle/seg112.c:tevent_074` |
| Reiseereignis | Ereignis 77: Felsteyn – Orkanger (Meile 39) | +6 | erster einsatzfähiger Held | Räuber im Wald: Misslingen → Gegner beginnen | `Quelle/seg112.c:tevent_077` |
| Reiseereignis | Ereignis 84: Oberorken – Einsiedlersee (Meile 32) | +5 | erster einsatzfähiger Held | Kampf: Fluchtversuch, Misslingen → Kampf, Gegner beginnen | `Quelle/seg113.c:tevent_084` |
| Reiseereignis | Ereignis 99: Rybon – Thoss (Meile 44) | +4 | erster einsatzfähiger Held | Fluchtversuch, Misslingen → Kampf, Gegner beginnen | `Quelle/seg113.c:tevent_099` |
| Reiseereignis | Ereignis 101: Rybon – Thoss (Meile 52) | 3–6 (Anzahl der Harpyien) | erster einsatzfähiger Held | Harpyien: Fluchtversuch, Zuschlag = Anzahl der Harpyien (3–6); Misslingen → Kampf, Gegner beginnen | `Quelle/seg113.c:tevent_101` |

- **Deutung der KI:** Entscheidet bei mehreren Überfällen auf Reisen, wer den Kampf beginnt oder ob die Flucht klappt. Es würfelt der erste Held.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-38"></a>
#### 38. Lesen

- **Konstante:** `TA_LESEN` · **Probe:** KL / KL / FF · **Steigerungen je Stufe:** 3 · **Proben im Code:** 1
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Dungeon: Daspota-Schatz | Truhe mit Schriftstück | ±0 | jeder Held einzeln | Gelingen: Inhalt verstanden | `Quelle/seg081.c:DNG06_chest01_loot` |

- **Deutung der KI:** Eine einzige Stelle (Daspota-Schatz).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-39"></a>
#### 39. Magiekunde

- **Konstante:** `TA_MAGIEKUNDE` · **Probe:** KL / KL / FF · **Steigerungen je Stufe:** 3 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-40"></a>
#### 40. Sprachen

- **Konstante:** `TA_SPRACHEN` · **Probe:** KL / IN / CH · **Steigerungen je Stufe:** 3 · **Proben im Code:** 2
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Dungeon: Orkbehausung | Truhe 2: Schrift lesen | +2 | erster einsatzfähiger Held | Gelingen: Schrift verstanden | `Quelle/seg083.c:DNG08_chest02_open` |
| Gegenstand | Orkdokument lesen | +4 | Held, der das Dokument benutzt | Gelingen (oder schon einmal gelesen): der Inhalt wird verstanden | `Quelle/seg107.c:item_read_orkdokument` |

- **Deutung der KI:** Zwei Stellen: das Orkdokument und eine Truhe in der Orkbehausung.
- **Status:** im Code geprüft, im Spiel unbestätigt

### Gruppe Handwerk

<a id="talent-41"></a>
#### 41. Abrichten

- **Konstante:** `TA_ABRICHTEN` · **Probe:** MU / IN / CH · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-42"></a>
#### 42. Fahrzeuge

- **Konstante:** `TA_FAHRZEUGE` · **Probe:** IN / CH / FF · **Steigerungen je Stufe:** 2 · **Proben im Code:** 0
- **Im Code:** Wird im Quelltext nirgends abgefragt und von nichts verändert.
- **Deutung der KI:** Wird nirgends abgefragt. Steigern bringt nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-43"></a>
#### 43. Falschspiel

- **Konstante:** `TA_FALSCHSPIEL` · **Probe:** MU / CH / FF · **Steigerungen je Stufe:** 2 · **Proben im Code:** 2
- **Im Code:** Nur über „Talent anwenden“ in der Taverne (im Statusmenü gesperrt): Gelingen bringt 500–1000 Heller (5–10 Dukaten); Misslingen bringt dauerhaft Hausverbot in dieser Taverne.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Gespräch / Ort | Spielhaus in Phexcaer | +3 | jeder Held einzeln | Proben aller Helden werden gezählt; im Original verliert die Gruppe, wenn MEHR als die Hälfte besteht (Fix 72 dreht das um) | `Quelle/seg070.c:PHX_spielhaus` |
| Talent anwenden | Talent anwenden: Falschspiel (nur Taverne) | ±0 (Taverne: +50, wenn vor weniger als 2 Minuten gespeichert wurde) | vom Spieler gewählter Held | Gelingen: 500–1000 Heller; Misslingen: dauerhaft Hausverbot in dieser Taverne | `Quelle/seg103.c:use_talent` |

- **Deutung der KI:** Lukrativ, aber riskant: Misslingen bringt dauerhaft Hausverbot in der Taverne. Dazu das Spielhaus in Phexcaer.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-44"></a>
#### 44. Heilen Gift

- **Konstante:** `TA_HEILEN_GIFT` · **Probe:** MU / KL / IN · **Steigerungen je Stufe:** 2 · **Proben im Code:** 3
- **Im Code:** Im Talentmenü überall: erste Probe ±0 erkennt das Gift (20 Minuten), zweite Probe mit der Giftstärke stoppt es; danach kann man LE heilen lassen, die gewünschte Zahl LE ist der Zuschlag, Misslingen kostet den Patienten 3 LE (er behält mindestens 1). Pro Patient nur alle 4 Stunden.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Talent anwenden | Heilen Gift, 1. Probe | ±0 (Taverne: +50, wenn vor weniger als 2 Minuten gespeichert wurde) | vom Spieler gewählter Held | Gift erkennen (kostet 20 Minuten) | `Quelle/seg103.c:use_talent` |
| Talent anwenden | Heilen Gift, 2. Probe | Giftstärke | vom Spieler gewählter Held | Gelingen: Gift gestoppt; Zuschlag = Giftstärke | `Quelle/seg103.c:use_talent` |
| Talent anwenden | Heilen Gift, LE heilen (freiwillig) | gewünschte LE | vom Spieler gewählter Held | Gelingen: so viele LE wie gewählt; Misslingen: 3 LE Schaden (mindestens 1 LE bleibt) | `Quelle/seg103.c:use_talent` |

- **Deutung der KI:** Lebensrettend. Die LE-Heilung danach ist riskant, weil die gewünschte LE-Zahl der Zuschlag ist.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-45"></a>
#### 45. Heilen Krankheiten

- **Konstante:** `TA_HEILEN_KRANKHEITEN` · **Probe:** MU / KL / CH · **Steigerungen je Stufe:** 2 · **Proben im Code:** 2
- **Im Code:** Im Talentmenü überall: erste Probe erkennt die Krankheit, zweite Probe mit der Krankheitsstärke heilt sie. Auch Ereignisse rufen diese Heilung auf (z. B. Tollwut, dort +10 und ohne Erkennungsprobe).

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Talent anwenden | Krankheit heilen, 1. Probe | wie übergeben (Talentmenü ±0) | behandelnder Held | erkennen, welche Krankheit es ist (bei Ereignis-Heilungen entfällt diese Probe) | `Quelle/seg104.c:talent_cure_disease` |
| Talent anwenden | Krankheit heilen, 2. Probe | Krankheitsstärke | behandelnder Held | Gelingen: Krankheit wird geheilt; Zuschlag = Stärke der Krankheit | `Quelle/seg104.c:talent_cure_disease` |

- **Deutung der KI:** Lebensrettend. Die Krankheitsstärke steht als Zuschlag auf der zweiten Probe.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-46"></a>
#### 46. Heilen Wunden

- **Konstante:** `TA_HEILEN_WUNDEN` · **Probe:** KL / CH / FF · **Steigerungen je Stufe:** 2 · **Proben im Code:** 2
- **Im Code:** Im Talentmenü überall: zwei Proben ±0; beide gelungen heilen so viele LE wie der TaW (mindestens 1). Misslingt die erste, bekommt der Patient mit 35 % Wundfieber (Fix 258: nicht, wenn er es schon hat); misslingt die zweite, verliert er 3 LE (behält mindestens 1) und regeneriert einen Tag nicht. Pro Patient nur einmal am Tag.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Talent anwenden | Heilen Wunden, 1. Probe | ±0 (Taverne: +50, wenn vor weniger als 2 Minuten gespeichert wurde) | vom Spieler gewählter Held | Misslingen: mit 35 % Wundfieber (Fix 258: nicht, wenn schon krank) | `Quelle/seg103.c:use_talent` |
| Talent anwenden | Heilen Wunden, 2. Probe | ±0 (Taverne: +50, wenn vor weniger als 2 Minuten gespeichert wurde) | vom Spieler gewählter Held | Gelingen: LE in Höhe des TaW (mind. 1); Misslingen: 3 LE Schaden und einen Tag keine Regeneration | `Quelle/seg103.c:use_talent` |

- **Deutung der KI:** Einmal am Tag so viele LE wie der TaW. Bei hohem TaW eine sehr gute Heilquelle, aber Wundfieber bei Misslingen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-47"></a>
#### 47. Musizieren

- **Konstante:** `TA_MUSIZIEREN` · **Probe:** KL / IN / FF · **Steigerungen je Stufe:** 2 · **Proben im Code:** 1
- **Im Code:** Im Talentmenü in Tavernen: Gelingen bringt 100–300 Heller (1–3 Dukaten); danach 8 Stunden gesperrt.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Talent anwenden | Talent anwenden: Musizieren | ±0 (Taverne: +50, wenn vor weniger als 2 Minuten gespeichert wurde) | vom Spieler gewählter Held | Gelingen: 100–300 Heller; danach 8 Stunden Pause | `Quelle/seg103.c:use_talent` |

- **Deutung der KI:** Ein kleines Zubrot in der Taverne.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-48"></a>
#### 48. Schlösser

- **Konstante:** `TA_SCHLOESSER` · **Probe:** IN / FF / FF · **Steigerungen je Stufe:** 2 · **Proben im Code:** 23
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Dungeon: alle | verschlossene Tür in jedem Dungeon (Dietriche) | je Tür | erster einsatzfähiger Held (braucht Dietriche) | Gelingen: Tür offen; ein Patzer oder der 4. Versuch ohne Weitergehen zerbricht die Dietriche; Zuschlag = Schloss der Tür | `Quelle/seg076.c:DNG_door` |
| Dungeon: Verfallene Herberge | Geheimtür 1 öffnen | +4 | erster einsatzfähiger Held | Gelingen: Tür geht auf | `Quelle/seg078.c:DNG02_handler` |
| Dungeon: Verfallene Herberge | Geheimtür 2 öffnen | +2 | erster einsatzfähiger Held | Gelingen: Tür geht auf | `Quelle/seg078.c:DNG02_handler` |
| Dungeon: Verfallene Herberge | Geheimtür 3 öffnen | +4 | erster einsatzfähiger Held | Gelingen: Tür geht auf | `Quelle/seg078.c:DNG02_handler` |
| Dungeon: Ruine des Schwarzmagiers | verschlossene Tür | +7 | erster einsatzfähiger Held | Gelingen: Tür mit Dietrich geöffnet | `Quelle/seg082.c:DNG07_handler` |
| Dungeon: Orkbehausung | Geheimtür öffnen | +2 | erster einsatzfähiger Held | Gelingen: Tür geht auf | `Quelle/seg083.c:DNG08_handler` |
| Dungeon: Orkbehausung | Truhe 3 | +5 | erster einsatzfähiger Held | öffnet mit Bronzeschlüssel oder gelungener Probe | `Quelle/seg083.c:DNG08_chest03_open` |
| Dungeon: Orkbehausung | Truhe 4 | +5 | erster einsatzfähiger Held | öffnet mit Bronzeschlüssel oder gelungener Probe | `Quelle/seg083.c:DNG08_chest04_open` |
| Dungeon: Orkbehausung | Truhe 5 | +5 | erster einsatzfähiger Held | öffnet mit Bronzeschlüssel oder gelungener Probe | `Quelle/seg083.c:DNG08_chest05_open` |
| Dungeon: Kultstätte des Namenlosen | Geheimtür 1 öffnen | +4 | erster einsatzfähiger Held | Gelingen: Tür geht auf | `Quelle/seg084.c:DNG09_handler` |
| Dungeon: Kultstätte des Namenlosen | Geheimtür 2 öffnen | +6 | erster einsatzfähiger Held | Gelingen: Tür geht auf | `Quelle/seg084.c:DNG09_handler` |
| Dungeon: Drachenhort | Falle: Bodenplatte entschärfen | +7 | erster einsatzfähiger Held | Misslingen: Schaden (nur wenn der Anführer es versucht) | `Quelle/seg085.c:DNG10_handler` |
| Dungeon: Drachenhort | Falle: Bodenplatte entschärfen | +7 | erster einsatzfähiger Held | Gelingen: Falle entschärft | `Quelle/seg085.c:DNG10_handler` |
| Dungeon: Piratenhöhle | Geheimtür 1 öffnen (je nach Hebelstellung) | +5 | erster einsatzfähiger Held | Gelingen: Tür geht auf; bei falscher Hebelstellung gibt es keine Probe (Fix 119 räumt dort einen Zufallswert ab) | `Quelle/seg086.c:DNG11_handler` |
| Dungeon: Piratenhöhle | Geheimtür 2 öffnen (je nach Hebelstellung) | +3 | erster einsatzfähiger Held | Gelingen: Tür geht auf | `Quelle/seg086.c:DNG11_handler` |
| Dungeon: Piratenhöhle | Geheimtür 3 öffnen (je nach Hebelstellung) | +5 | erster einsatzfähiger Held | Gelingen: Tür geht auf | `Quelle/seg086.c:DNG11_handler` |
| Dungeon: Zwingfeste | Geheimtür 1 öffnen | −6 | erster einsatzfähiger Held | Gelingen: Tür geht auf (leicht, −6) | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwingfeste | Geheimtür 2 öffnen | +6 | erster einsatzfähiger Held | Gelingen: Tür geht auf | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwingfeste | Geheimtür 3 öffnen | −6 | erster einsatzfähiger Held | Gelingen: Tür geht auf (leicht, −6) | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwingfeste | Geheimtür 4 öffnen | +7 | erster einsatzfähiger Held | Gelingen: Tür geht auf | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwingfeste | Geheimtür 5 öffnen | +4 | erster einsatzfähiger Held | Gelingen: Tür geht auf | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwergenfeste | Speerfalle entschärfen | ±0 | erster einsatzfähiger Held | Misslingen: Schaden | `Quelle/seg090.c:DNG12_handler` |
| Dungeon: alle | verschlossene Truhe in jedem Dungeon (Dietriche) | je Truhe | erster einsatzfähiger Held (braucht Dietriche) | Gelingen: Truhe offen; Zuschlag = Schloss der Truhe | `Quelle/seg092.c:use_lockpicks_on_chest` |
| Götterwunder | Phex |  |  | +1 für 3 Tage (zusammen mit Taschendiebstahl) | `Quelle/seg062.c:ask_miracle` |

- **Deutung der KI:** Sehr häufig in Dungeons (Türen, Geheimtüren, Truhen). Bei Dietrich-Türen würfelt der erste Held.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-49"></a>
#### 49. Taschendiebstahl

- **Konstante:** `TA_TASCHENDIEBSTAHL` · **Probe:** MU / IN / FF · **Steigerungen je Stufe:** 2 · **Proben im Code:** 1
- **Im Code:** Im Talentmenü auf dem Markt und bei Händlern: Gelingen bringt 500–1000 Heller (5–10 Dukaten); Misslingen kostet die Gruppe ihr GESAMTES Geld.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Talent anwenden | Talent anwenden: Taschendiebstahl (Markt, Händler) | ±0 (Taverne: +50, wenn vor weniger als 2 Minuten gespeichert wurde) | vom Spieler gewählter Held | Gelingen: 500–1000 Heller; Misslingen: die Gruppe verliert ihr gesamtes Geld | `Quelle/seg103.c:use_talent` |
| Götterwunder | Phex |  |  | +1 für 3 Tage (zusammen mit Schlösser) | `Quelle/seg062.c:ask_miracle` |

- **Deutung der KI:** Hohes Risiko: Misslingen kostet ALLES Geld der Gruppe.
- **Status:** im Code geprüft, im Spiel unbestätigt

### Gruppe Intuition

<a id="talent-50"></a>
#### 50. Gefahrensinn

- **Konstante:** `TA_GEFAHRENSINN` · **Probe:** KL / IN / IN · **Steigerungen je Stufe:** 1 · **Proben im Code:** 10
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Dungeon: Spinnenhöhle | Säuregrube | +4 | erster einsatzfähiger Held | Misslingen: Schaden; geprüft werden nur die ersten zwei Helden | `Quelle/seg079.c:DNG03_handler` |
| Dungeon: Spinnenhöhle | Säuregrube | +4 | zweiter einsatzfähiger Held | wie oben | `Quelle/seg079.c:DNG03_handler` |
| Dungeon: Daspota-Schatz | Falltür | +5 | jeder Held einzeln | Gelingen: Falltür bemerkt | `Quelle/seg081.c:DNG06_handler` |
| Dungeon: Daspota-Schatz | Falltür | +5 | jeder Held einzeln | zweite gelungene Probe: Sicherung gefunden | `Quelle/seg081.c:DNG06_handler` |
| Dungeon: Daspota-Schatz | Falltür (Fix 354: Zustand als Bits) | +5 | jeder Held einzeln | Gelingen: Falltür bemerkt | `Quelle/seg081.c:DNG06_handler` |
| Dungeon: Daspota-Schatz | Falltür / umgelegter Stein | +5 | jeder Held einzeln | Gelingen: Falle bemerkt | `Quelle/seg081.c:DNG06_handler` |
| Dungeon: Daspota-Schatz | herabfallende Steinscheibe | ±0 | erster einsatzfähiger Held | nur wer Körperbeherrschung UND Gefahrensinn besteht, weicht aus | `Quelle/seg081.c:DNG06_handler` |
| Dungeon: Kultstätte des Namenlosen | Fallgrube (gemeinsamer Baustein, genutzt in der Kultstätte des Namenlosen) | +4 | jeder Held einzeln | Gelingen: Held fällt nicht hinein (Fix 86: nur handlungsfähige Helden) | `Quelle/seg081.c:DNG09_pitfall` |
| Dungeon: Kultstätte des Namenlosen | Bolzenfalle | +2 | jeder Held einzeln | Gelingen: Held entgeht der Falle | `Quelle/seg084.c:DNG09_handler` |
| Reiseereignis | Ereignis 90: Felsteyn – Einsiedlersee (Meile 19) | ±0 | jeder Held einzeln | Misslingen: W10 LE und zweimal 10 % Chance, einen Gegenstand zu verlieren | `Quelle/seg115.c:tevent_090` |
| Gegenstand | Kristallkugel |  |  | +2, solange sie ausgerüstet ist | `Quelle/seg105.c:add_equip_boni` |

- **Deutung der KI:** Fallen und Gruben in mehreren Dungeons, oft für jeden Helden einzeln. Die Kristallkugel gibt +2.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="talent-51"></a>
#### 51. Sinnesschärfe

- **Konstante:** `TA_SINNESSCHAERFE` · **Probe:** KL / IN / IN · **Steigerungen je Stufe:** 1 · **Proben im Code:** 36
- **Im Code:** siehe Tabelle.

| Bereich | Ort | Zuschlag | Wer | Wirkung | Codestelle |
|---|---|---|---|---|---|
| Stadt | Stadtereignis: Taschendieb | +2 | zufälliger Held | Misslingen: der Held verliert Geld (W20 × 10 Heller) | `Quelle/seg067.c:city_event_1` |
| Stadt | Stadtereignis: Taschendieb | +2 | zufälliger Held | Misslingen: der Held verliert sein gesamtes Geld | `Quelle/seg067.c:city_event_2` |
| Dungeon: Verfallene Herberge | ehemalige Schatzkammer | +6 | erster einsatzfähiger Held | Gelingen: etwas entdeckt | `Quelle/seg078.c:DNG02_handler` |
| Dungeon: Verfallene Herberge | Geheimtür 1 | +6 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg078.c:DNG02_handler` |
| Dungeon: Verfallene Herberge | Geheimtür 2 | +2 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg078.c:DNG02_handler` |
| Dungeon: Verfallene Herberge | Geheimtür 3 | +5 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg078.c:DNG02_handler` |
| Dungeon: Daspota-Schatz | versteckte Stelle | +10 | jeder Held einzeln | Gelingen: etwas entdeckt (sehr schwer, +10) | `Quelle/seg081.c:DNG06_handler` |
| Dungeon: Orkbehausung | Geheimtür (Fix 106: beim Betreten oder Drehen) | +1 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden | `Quelle/seg083.c:DNG08_handler` |
| Dungeon: Kultstätte des Namenlosen | Geheimtür 1 | +6 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg084.c:DNG09_handler` |
| Dungeon: Kultstätte des Namenlosen | Geheimtür 2 | +8 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg084.c:DNG09_handler` |
| Dungeon: Drachenhort | Mauerloch mit Hebel einer Falle | +7 | erster einsatzfähiger Held | Gelingen: Hebel entdeckt | `Quelle/seg085.c:DNG10_handler` |
| Dungeon: Drachenhort | Falle: loser Stein | +5 | erster einsatzfähiger Held | Gelingen: Falle bemerkt | `Quelle/seg085.c:DNG10_handler` |
| Dungeon: Drachenhort | Falle: Bodenplatte | +5 | erster einsatzfähiger Held | Gelingen: Platte bemerkt | `Quelle/seg085.c:DNG10_handler` |
| Dungeon: Drachenhort | Falle: Bodenplatte (Fix 116: Zustand als Bits) | +5 | erster einsatzfähiger Held | Gelingen: Platte bemerkt | `Quelle/seg085.c:DNG10_handler` |
| Dungeon: Piratenhöhle | Geheimtür 1 | +8 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg086.c:DNG11_handler` |
| Dungeon: Piratenhöhle | Geheimtür 2 | +4 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg086.c:DNG11_handler` |
| Dungeon: Piratenhöhle | Geheimtür 3 | +6 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg086.c:DNG11_handler` |
| Dungeon: Zwingfeste | Opferstock / Phex-Frevel | +6 | erster einsatzfähiger Held | Misslingen: das Ansehen bei Phex sinkt stark | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwingfeste | Geheimtür 1 | +4 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwingfeste | Geheimtür 2 | +4 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwingfeste | Geheimtür 3 | +4 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwingfeste | Geheimtür 4 | +8 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwingfeste | Geheimtür 5 | +6 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden (bleibt danach bekannt) | `Quelle/seg087.c:DNG14_handler` |
| Dungeon: Zwergenfeste | Geheimtür bei der Wasserfalle | +6 | erster einsatzfähiger Held | Gelingen: Geheimtür gefunden | `Quelle/seg090.c:DNG12_handler` |
| Dungeon: Zwergenfeste | Speerfalle | +2 | erster einsatzfähiger Held | Gelingen: Falle bemerkt | `Quelle/seg090.c:DNG12_handler` |
| Reiseereignis | Ereignis 11: Peilinen – Rovamund (Meile 10) | +8 | erster einsatzfähiger Held | Spuren am anderen Ufer des Roval: Pfad zur Querverbindung (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_011` |
| Reiseereignis | Ereignis 20: Kravik – Skelellen (Meile 8) | +7 | erster einsatzfähiger Held | alter Pfad nach Westen (einmal gefunden, bleibt es bekannt) | `Quelle/seg110.c:tevent_020` |
| Reiseereignis | Ereignis 46: Daspota – Ottarje (Meile 15) | ±0 | Held mit dem Lagervorfall, sonst erster einsatzfähiger Held | Verfallene Herberge: die Wache im Lager hört etwas | `Quelle/seg110.c:tevent_046` |
| Reiseereignis | Ereignis 61: Ottarje – Orvil (Meile 41) | +6 | erster einsatzfähiger Held | Eingang der Wolfshöhle | `Quelle/seg111.c:tevent_061` |
| Reiseereignis | Ereignis 67: Ala – Tjanset (Meile 7); Ala – Thoss (Meile 7) | +6 | erster einsatzfähiger Held | alter Pfad zu einer verfallenen Hütte (einmal gefunden, bleibt es bekannt) | `Quelle/seg112.c:tevent_067` |
| Reiseereignis | Ereignis 71: Thoss – Liskor (Meile 11); Oberorken – Felsteyn (Meile 7) | +8 | erster einsatzfähiger Held | Monolith am anderen Ufer des Thossel | `Quelle/seg112.c:tevent_071` |
| Reiseereignis | Ereignis unused_herb_swim (nie aufgerufen) | +8 | erster einsatzfähiger Held | Kräuterplatz am anderen Ufer des Hjaldorval; wird im Spiel nie aufgerufen (einmal gefunden, bleibt es bekannt) | `Quelle/seg112.c:tevent_unused_herb_swim` |
| Reiseereignis | Ereignis 76: Felsteyn – Orkanger (Meile 35) | +5 | erster einsatzfähiger Held | Eingang der Goblinhöhle | `Quelle/seg112.c:tevent_076` |
| Reiseereignis | Ereignis 108: Skelellen – Phexcaer (Meile 40) | +3 | erster einsatzfähiger Held | Eingang der Orkhöhle | `Quelle/seg113.c:tevent_108` |
| Reiseereignis | Ereignis 129: Runinshaven – Leuchtturm Runin (Meile 10) | +4 | erster einsatzfähiger Held | Eingang der Drachenhöhle auf Runin | `Quelle/seg115.c:tevent_129` |
| Reiseereignis | Ereignis 131: Brendhil – Manrin (Meile 17) | +8 | erster einsatzfähiger Held | etwas am Weg entdeckt | `Quelle/seg116.c:tevent_131` |
| Zauber | Adlerauge |  |  | +7 für 6 Minuten Spielzeit | `Quelle/seg099.c:spell_adleraug` |
| Regel | Anfängermodus |  |  | jede Sinnesschärfe-Probe würfelt automatisch der beste Held der Gruppe | `Quelle/seg103.c:test_talent` |

- **Deutung der KI:** Das wichtigste Entdeckungstalent: Geheimtüren, Fallen, Dungeon-Eingänge, Taschendiebe. Im Anfängermodus würfelt automatisch der beste Held, sonst meist der erste.
- **Status:** im Code geprüft, im Spiel unbestätigt
