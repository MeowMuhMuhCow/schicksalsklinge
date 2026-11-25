# Zauberliste – Die Schicksalsklinge (aus dem Quelltext)

> **Hinweis:** Diese Liste wurde von einer KI aus dem rekonstruierten Quelltext erstellt. „Im Code“ ist das, was das Programm tatsächlich rechnet. „Deutung der KI“ ist die Auslegung der KI und kann falsch sein. Wer im Spiel etwas anderes beobachtet oder es genauer weiß: bitte im Crystals-DSA-Forum melden.

- **Grundlage:** Quelltext `Quelle/` der Fanpatch-Fassung 1.5i und die Tabellen im Datensegment. „Im Original“ heißt: ohne Fanpatch. „Mit Fix N“ heißt: im Fanpatch behoben, mit Angabe, ob der Fix in der Stufe „Wichtig“ oder nur in „Alle“ enthalten ist.
- **Prüfstand:** Jede Angabe unter „Im Code“ wurde gegen die genannte Codestelle nachgelesen; („Status: im Code geprüft“). „Im Spiel bestätigt“ setzen wir erst, wenn Spieler es im Spiel nachgeprüft haben.

## Inhaltsverzeichnis

1. [Wie Zaubern im Code funktioniert](#1-wie-zaubern-im-code-funktioniert)
2. [Übersichtstabelle aller 85 Heldenzauber](#2-übersichtstabelle-aller-85-heldenzauber)
3. [Alle 85 Heldenzauber im Detail](#3-alle-85-heldenzauber-im-detail)
4. [Die 7 Stabzauber der Magier](#4-die-7-stabzauber-der-magier)
5. [Die 14 Gegnerzauber](#5-die-14-gegnerzauber)

---

## 1. Wie Zaubern im Code funktioniert

- **Im Code:**
  - **Wer zaubern kann:** nur Hexe, Druide, Magier und die drei Elfen, und nur Sprüche mit einem Zauberfertigkeitswert (ZfW) von mindestens −5. Der Held braucht mindestens so viele AE, wie die Tabelle verlangt (bei variablen Kosten prüft das Spiel vorher nichts). (`Quelle/seg098.c:test_spell`)
  - **Die Probe:** Zuschlag = Zuschlag des Spruchs minus ZfW; Sprüche mit gesetzter MR-Spalte werden zusätzlich um die Magieresistenz des Ziels erschwert (Gegner oder Held). Gelungen ist die Probe, wenn die Summe der drei Eigenschaften minus (3W20 + Zuschlag) plus 1 größer als 0 ist. Zwei Zwanziger sind ein Patzer (misslungen). Zwei Einsen haben keine besondere Wirkung. Ist das Ziel in einen Pilz verwandelt (Salander), misslingt jeder Spruch mit MR-Spalte. (`Quelle/seg098.c:test_spell`, `Quelle/seg002.c:test_attrib3`)
  - **Einsatzort:** Sprüche „nur im Kampf“ gehen außerhalb nicht und umgekehrt (Meldung). Im Kampf lässt sich eine Zaubergruppe nur öffnen, wenn sie mindestens einen reinen Kampfspruch enthält, den der Held beherrscht. Außerhalb des Kampfes braucht sie einen Spruch, der nicht nur im Kampf geht. Darum ist die Gruppe Heilung im Original im Kampf gesperrt, obwohl Balsam Salabunde, Hexenspeichel, Klarum Purum und Tiere besprechen laut Tabelle auch im Kampf gehen; mit Fix 444 (Fanpatch-Stufe „nur Alle“) sind sie im Kampf wählbar. In anderen Gruppen (z. B. Verwandlung) lassen sich solche Sprüche im Kampf schon im Original wählen, sie stehen nur grau in der Liste. (`Quelle/seg098.c:use_spell`, `Quelle/seg098.c:can_use_spellclass`, `Quelle/seg098.c:select_spell`)
  - **Kosten:** Misslingt die Probe oder wirkt ein Rondra-Wunder, das Zauber verbietet, kostet der Spruch die halben Tabellenkosten (mindestens 1 AE; bei variablen Kosten zufällig 5–10 AE). Gelingt sie, legt der Zauber die Kosten selbst fest oder es gilt der Tabellenwert. Bricht der Zauber kostenlos ab, kostet er nichts. „Misslingt“ im Handler heißt dagegen: halbe Kosten wie bei misslungener Probe. Magier mit dem 4. Stabzauber zahlen jede AE-Ausgabe mit 2 AE weniger. (`Quelle/seg098.c:use_spell`, `Quelle/seg098.c:get_spell_cost`, `Quelle/seg002.c:sub_ae_splash`)
  - **Reichweite im Kampf:** Berührungszauber nur auf ein Nachbarfeld, Fernzauber auf jedes Feld. Sprüche mit Ziel „Held“ gehen nicht auf Gegner und umgekehrt (Meldung). (`Quelle/seg033.c:FIG_menu`)
  - **Schaden durch Zauber** (Fulminictus, Ignifaxius, Krähenruf, Radau) geht ohne Rüstungsschutz direkt von der LE ab. (`Quelle/seg098.c:FIG_do_spell_damage`)
  - **Skelette:** Bei Band und Fessel, Bannbaladin, Böser Blick, Große Verwirrung, Horriphobus, Somnigravis und Zwingtanz sind alle Gegner mit Skelett-Grafik immun (der Zauber misslingt). (`Quelle/seg099.c`)
- **Deutung der KI:** Sieht so aus, als ob die Probe im Computerspiel eine vereinfachte „Summenprobe“ ist und deutlich leichter fällt als nach dem Pen-&-Paper-Regelwerk. Viele Sprüche, die im Regelwerk außerhalb von Kämpfen wirken, sind im Code leer oder misslingen immer.
- **Status:** im Code geprüft, im Spiel unbestätigt

---

## 2. Übersichtstabelle aller 85 Heldenzauber

| Nr. | Zauber | Gruppe | Herkunft | Probe | AE (Tabelle) | Einsatz | Ziel | MR | Im Code |
|---:|---|---|---|---|---|---|---|---|---|
| 1 | [Beherrschung brechen](#zauber-1) | Antimagie | Druide | KL/IN/CH | variabel | im Kampf und außerhalb | Held | nein | umgesetzt |
| 2 | [Destructibo Arcanitas](#zauber-2) | Antimagie | Magier | KL/IN/FF | variabel | im Kampf und außerhalb | – | nein | misslingt immer |
| 3 | [Gardianum Paradei](#zauber-3) | Antimagie | Magier | IN/CH/KK | variabel | nur im Kampf | – | nein | umgesetzt |
| 4 | [Illusionen zerstören](#zauber-4) | Antimagie | Magier | KL/IN/CH | variabel | nur im Kampf | Gegner | nein | umgesetzt |
| 5 | [Verwandlung beenden](#zauber-5) | Antimagie | Hexe | MU/KL/CH | variabel | im Kampf und außerhalb | Held | nein | umgesetzt |
| 6 | [Band und Fessel](#zauber-6) | Beherrschung | Waldelf | KL/CH/KK | 12 | nur im Kampf | Gegner/Held | ja | umgesetzt |
| 7 | [Bannbaladin](#zauber-7) | Beherrschung | Auelf | IN/CH/CH | 8 | nur im Kampf | Gegner | ja | umgesetzt |
| 8 | [Böser Blick](#zauber-8) | Beherrschung | Druide | MU/CH/CH | 8 | nur im Kampf | Gegner | ja | umgesetzt |
| 9 | [Große Gier](#zauber-9) | Beherrschung | Hexe | KL/IN/CH | 8 | nur im Kampf | Gegner | ja | misslingt immer (mit Fix umgesetzt) |
| 10 | [Große Verwirrung](#zauber-10) | Beherrschung | Druide | KL/KL/CH | variabel | nur im Kampf | Gegner | ja | umgesetzt |
| 11 | [Herr über das Tierreich](#zauber-11) | Beherrschung | Druide | MU/MU/CH | variabel | nur im Kampf | Gegner | ja | umgesetzt |
| 12 | [Horriphobus](#zauber-12) | Beherrschung | Magier | MU/IN/CH | 7 | nur im Kampf | Gegner | ja | umgesetzt |
| 13 | [Magischer Raub](#zauber-13) | Beherrschung | Druide | MU/KL/KK | 6 | nur außerhalb des Kampfes | – | ja | leer: kostet AE, keine Wirkung |
| 14 | [Respondami Veritar](#zauber-14) | Beherrschung | Magier | KL/IN/CH | 5 | nur außerhalb des Kampfes | – | ja | leer: kostet AE, keine Wirkung |
| 15 | [Sanftmut](#zauber-15) | Beherrschung | Hexe | MU/CH/CH | variabel | nur im Kampf | Gegner | ja | umgesetzt |
| 16 | [Somnigravis Tausendschlaf](#zauber-16) | Beherrschung | Waldelf | KL/CH/CH | 8 | nur im Kampf | Gegner/Held | ja | umgesetzt |
| 17 | [Zwingtanz](#zauber-17) | Beherrschung | Druide | MU/KL/CH | 6 | nur im Kampf | Gegner | ja | umgesetzt |
| 18 | [Furor Blut und Sulphurdampf](#zauber-18) | Dämonologie | Magier | MU/MU/CH | 13 | nur außerhalb des Kampfes | – | nein | misslingt immer |
| 19 | [Geister austreiben](#zauber-19) | Dämonologie | Druide | MU/MU/CH | 13 | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 20 | [Geister beschwören](#zauber-20) | Dämonologie | Druide | MU/MU/CH | 11 | nur außerhalb des Kampfes | – | nein | misslingt immer |
| 21 | [Heptagon und Krötenei](#zauber-21) | Dämonologie | Magier | MU/MU/CH | 23 | nur außerhalb des Kampfes | – | nein | misslingt immer |
| 22 | [Krähenruf](#zauber-22) | Dämonologie | Hexe | MU/CH/CH | 13 | nur im Kampf | Gegner/Held | nein | umgesetzt |
| 23 | [Skelettarius Kryptaduft](#zauber-23) | Dämonologie | Magier | MU/MU/CH | 15 | nur im Kampf | Gegner | nein | umgesetzt |
| 24 | [Elementare beschwören](#zauber-24) | Elemente | Druide | MU/KL/CH | variabel | nur außerhalb des Kampfes | – | nein | misslingt immer |
| 25 | [Nihilatio Gravitas](#zauber-25) | Elemente | Magier | KL/KK/KK | 5 | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 26 | [Solidirid Farbenspiel](#zauber-26) | Elemente | Firnelf | IN/GE/KK | variabel | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 27 | [Axxeleratus Blitzgeschwind](#zauber-27) | Bewegung | Waldelf | KL/GE/KK | 7 | nur im Kampf | Held | nein | umgesetzt |
| 28 | [Foramen Foraminor](#zauber-28) | Bewegung | Magier | KL/KL/FF | 6 | nur außerhalb des Kampfes | – | nein | umgesetzt |
| 29 | [Motoricus Motilitich](#zauber-29) | Bewegung | Magier | KL/FF/KK | variabel | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 30 | [Spurlos trittlos fährtenlos](#zauber-30) | Bewegung | Waldelf | IN/GE/GE | 7 | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 31 | [Transversalis Teleport](#zauber-31) | Bewegung | Magier | KL/KL/KK | variabel | nur außerhalb des Kampfes | – | nein | umgesetzt |
| 32 | [Über Eis und über Schnee](#zauber-32) | Bewegung | Firnelf | KL/GE/KK | 5 | nur außerhalb des Kampfes | – | nein | misslingt immer |
| 33 | [Balsam Salabunde](#zauber-33) | Heilung | Auelf | KL/IN/CH | variabel | im Kampf und außerhalb (im Original im Kampf gesperrt, Fix 444) | Held | nein | umgesetzt |
| 34 | [Hexenspeichel](#zauber-34) | Heilung | Hexe | IN/CH/FF | variabel | im Kampf und außerhalb (im Original im Kampf gesperrt, Fix 444) | Held | nein | umgesetzt |
| 35 | [Klarum Purum Kräutersud](#zauber-35) | Heilung | Magier | KL/KL/CH | variabel | im Kampf und außerhalb (im Original im Kampf gesperrt, Fix 444) | Held | nein | umgesetzt |
| 36 | [Ruhe Körper, ruhe Geist](#zauber-36) | Heilung | Firnelf | KL/CH/KK | 7 | nur außerhalb des Kampfes | Held | nein | umgesetzt |
| 37 | [Tiere besprechen](#zauber-37) | Heilung | Hexe | MU/IN/CH | variabel | im Kampf und außerhalb (im Original im Kampf gesperrt, Fix 444) | – | nein | umgesetzt |
| 38 | [Adleraug und Luchsenohr](#zauber-38) | Wahrnehmung | Auelf | KL/IN/FF | 5 | im Kampf und außerhalb | – | nein | umgesetzt |
| 39 | [Analüs Arcanstruktur](#zauber-39) | Wahrnehmung | Magier | KL/KL/IN | 10 | nur außerhalb des Kampfes | – | nein | umgesetzt |
| 40 | [Eigenschaften seid gelesen](#zauber-40) | Wahrnehmung | Auelf | KL/IN/CH | 7 | nur im Kampf | Gegner | ja | umgesetzt |
| 41 | [Exposami Creatur](#zauber-41) | Wahrnehmung | Waldelf | KL/KL/IN | 5 | nur im Kampf | – | nein | umgesetzt |
| 42 | [Odem Arcanum](#zauber-42) | Wahrnehmung | Auelf | KL/IN/CH | 5 | nur außerhalb des Kampfes | – | nein | umgesetzt |
| 43 | [Penetrizzel Holz und Stein](#zauber-43) | Wahrnehmung | Magier | KL/KL/KK | 5 | nur außerhalb des Kampfes | – | nein | umgesetzt |
| 44 | [Sensibar wahr und klar](#zauber-44) | Wahrnehmung | Auelf | KL/IN/CH | 5 | nur außerhalb des Kampfes | – | ja | leer: kostet AE, keine Wirkung |
| 45 | [Chamaelioni Mimikry](#zauber-45) | Visionen | Waldelf | IN/CH/GE | 5 | nur im Kampf | – | nein | umgesetzt |
| 46 | [Duplicatus Doppelpein](#zauber-46) | Visionen | Magier | KL/CH/GE | 8 | nur im Kampf | – | nein | umgesetzt |
| 47 | [Harmlose Gestalt](#zauber-47) | Visionen | Hexe | KL/CH/GE | 6 | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 48 | [Hexenknoten](#zauber-48) | Visionen | Hexe | KL/IN/CH | 4 | nur im Kampf | – | nein | umgesetzt |
| 49 | [Blitz dich find](#zauber-49) | Kampf | Waldelf | KL/IN/GE | 5 | nur im Kampf | Gegner/Held | nein | umgesetzt |
| 50 | [Ecliptifactus](#zauber-50) | Kampf | Magier | MU/KL/KK | variabel | nur im Kampf | – | nein | umgesetzt |
| 51 | [Eisenrost und grüner Span](#zauber-51) | Kampf | Magier | KL/CH/GE | 5 | nur im Kampf | Gegner/Held | nein | umgesetzt |
| 52 | [Fulminictus Donnerkeil](#zauber-52) | Kampf | Auelf | KL/GE/KK | variabel | nur im Kampf | Gegner/Held | nein | umgesetzt |
| 53 | [Ignifaxius Flammenstrahl](#zauber-53) | Kampf | Magier | KL/GE/FF | variabel | nur im Kampf | Gegner/Held | nein | umgesetzt |
| 54 | [Plumbumbarum und Narretei](#zauber-54) | Kampf | Magier | CH/GE/KK | 5 | nur im Kampf | Gegner/Held | ja | umgesetzt |
| 55 | [Radau](#zauber-55) | Kampf | Hexe | MU/IN/CH | 4 | nur im Kampf | Gegner/Held | nein | umgesetzt |
| 56 | [Saft, Kraft, Monstermacht](#zauber-56) | Kampf | Magier | MU/IN/CH | variabel | nur im Kampf | Held | nein | umgesetzt |
| 57 | [Scharfes Auge und sichre Hand](#zauber-57) | Kampf | Waldelf | IN/GE/FF | 5 | nur im Kampf | Held | nein | umgesetzt |
| 58 | [Hexenblick](#zauber-58) | Verständigung | Hexe | IN/IN/CH | 2 | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 59 | [Nekropathia modernd Leich](#zauber-59) | Verständigung | Magier | MU/KL/CH | 11 | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 60 | [Adler, Wolf und Hammerhai](#zauber-60) | Verwandlung | Waldelf | MU/IN/GE | 7 | nur außerhalb des Kampfes | – | nein | misslingt immer |
| 61 | [Arcano Psychostabilis](#zauber-61) | Verwandlung | Magier | MU/KL/CH | 7 | nur im Kampf | Held | nein | umgesetzt |
| 62 | [Armatrutz](#zauber-62) | Verwandlung | Auelf | IN/GE/KK | variabel | nur im Kampf | – | nein | umgesetzt |
| 63 | [Charisma steigern](#zauber-63) | Verwandlung | Magier | KL/CH/CH | 7 | im Kampf und außerhalb | Held | nein | umgesetzt |
| 64 | [Feuerbann](#zauber-64) | Verwandlung | Hexe | MU/MU/KK | 7 | im Kampf und außerhalb | – | nein | umgesetzt |
| 65 | [Fingerfertigkeit steigern](#zauber-65) | Verwandlung | Waldelf | KL/CH/FF | 7 | im Kampf und außerhalb | Held | nein | umgesetzt |
| 66 | [Gewandtheit steigern](#zauber-66) | Verwandlung | Auelf | KL/CH/GE | 7 | im Kampf und außerhalb | Held | nein | umgesetzt |
| 67 | [Intuition steigern](#zauber-67) | Verwandlung | Firnelf | KL/IN/CH | 7 | im Kampf und außerhalb | Held | nein | umgesetzt |
| 68 | [Körperkraft steigern](#zauber-68) | Verwandlung | Firnelf | KL/CH/KK | 7 | im Kampf und außerhalb | Held | nein | umgesetzt |
| 69 | [Klugheit steigern](#zauber-69) | Verwandlung | Magier | KL/KL/CH | 7 | im Kampf und außerhalb | Held | nein | umgesetzt |
| 70 | [Mut steigern](#zauber-70) | Verwandlung | Firnelf | MU/KL/CH | 7 | im Kampf und außerhalb | Held | nein | umgesetzt |
| 71 | [Mutabili Hybridil](#zauber-71) | Verwandlung | Magier | KL/GE/KK | variabel | im Kampf und außerhalb | – | nein | misslingt immer |
| 72 | [Paralü Paralein](#zauber-72) | Verwandlung | Magier | IN/CH/KK | 13 | nur im Kampf | Gegner/Held | ja | umgesetzt |
| 73 | [Salander Mutanderer](#zauber-73) | Verwandlung | Magier | KL/IN/CH | variabel | nur im Kampf | Gegner | ja | umgesetzt |
| 74 | [In See und Fluss](#zauber-74) | Verwandlung | Auelf | MU/KL/KK | variabel | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 75 | [Visibili Vanitar](#zauber-75) | Verwandlung | Auelf | KL/KL/GE | variabel | im Kampf und außerhalb | – | nein | umgesetzt |
| 76 | [Abvenenum Pest und Galle](#zauber-76) | Veränderung | Waldelf | KL/KL/FF | 5 | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 77 | [Aeolitus Windgebraus](#zauber-77) | Veränderung | Firnelf | KL/CH/KK | 5 | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 78 | [Brenne toter Stoff!](#zauber-78) | Veränderung | Magier | MU/KL/KK | variabel | nur außerhalb des Kampfes | – | nein | umgesetzt |
| 79 | [Claudibus Clavistibor](#zauber-79) | Veränderung | Magier | KL/FF/KK | variabel | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 80 | [Dunkelheit](#zauber-80) | Veränderung | Druide | KL/KL/FF | 3 | nur im Kampf | – | nein | umgesetzt |
| 81 | [Weiches erstarre!](#zauber-81) | Veränderung | Magier | MU/KL/KK | variabel | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 82 | [Flim Flam Funkel](#zauber-82) | Veränderung | Firnelf | KL/KL/FF | 1 | nur außerhalb des Kampfes | – | nein | umgesetzt |
| 83 | [Hartes schmelze!](#zauber-83) | Veränderung | Magier | MU/KL/KK | variabel | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |
| 84 | [Silentium Silentille](#zauber-84) | Veränderung | Auelf | KL/KL/CH | variabel | nur außerhalb des Kampfes | – | nein | umgesetzt |
| 85 | [Sturmgebrüll besänft'ge dich](#zauber-85) | Veränderung | Firnelf | KL/CH/KK | 5 | nur außerhalb des Kampfes | – | nein | leer: kostet AE, keine Wirkung |

---

## 3. Alle 85 Heldenzauber im Detail

<a id="zauber-1"></a>
### 1. Beherrschung brechen (Heldenzauber) (`SP_BEHERRSCHUNG_BRECHEN`)

- **Im Code:** Zauber-ID 1 (`SP_BEHERRSCHUNG_BRECHEN`). Gruppe Antimagie, Herkunft Druide. Probe auf KL/IN/CH. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Held. Ist er nicht abtrünnig (nicht von einem Gegner beherrscht), misslingt der Zauber (halbe Kosten). Auf sich selbst: kostenlos abgebrochen. Sonst kostet er zufällig 8, 12, 16 oder 20 AE (2–5 × 4). Reicht die AE nicht, misslingt er; sonst ist der Held wieder frei.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_beherrschung`
- **Deutung der KI:** Vermutlich das Gegenmittel, wenn ein Gegner einen Helden mit Böser Blick übernommen hat. Die Kosten (8–20 AE) sind Glückssache.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-2"></a>
### 2. Destructibo Arcanitas (Heldenzauber) (`SP_DESTRUCTIBO_ARCANITAS`)

- **Im Code:** Zauber-ID 2 (`SP_DESTRUCTIBO_ARCANITAS`). Gruppe Antimagie, Herkunft Magier. Probe auf KL/IN/FF. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: im Kampf und außerhalb. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Nicht umgesetzt: Der Handler lässt den Zauber immer misslingen (halbe Kosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_destructibo`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Original nicht programmiert ist und immer misslingt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-3"></a>
### 3. Gardianum Paradei (Heldenzauber) (`SP_GARDIANUM_PARADEI`)

- **Im Code:** Zauber-ID 3 (`SP_GARDIANUM_PARADEI`). Gruppe Antimagie, Herkunft Magier. Probe auf IN/CH/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur im Kampf. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Zaubernde gibt ein, wie viele AE in den Schutzschirm fließen sollen. Diese Menge ist zugleich der Preis. Im Original wird der Wert nur aufaddiert und nirgends gelesen, der Schirm wirkt also nicht. Mit Fix 192 (Fanpatch-Stufe „Wichtig“) ist der Schirm so stark wie eingesetzte AE plus Stufe des Zaubernden und fängt Schaden aus feindlichen Fulminictus und Ignifaxius ab, bis er verbraucht ist (siehe Gegnerzauber).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_gardianum`, `Quelle/seg102.c:fix_hero_spell_protect`
- **Deutung der KI:** Sieht so aus, als ob der Schutzschirm im Original nur AE frisst. Erst mit dem Fanpatch schützt er vor feindlichen Kampfzaubern.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-4"></a>
### 4. Illusionen zerstören (Heldenzauber) (`SP_ILLUSIONEN_ZERSTOEREN`)

- **Im Code:** Zauber-ID 4 (`SP_ILLUSIONEN_ZERSTOEREN`). Gruppe Antimagie, Herkunft Magier. Probe auf KL/IN/CH. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ziel: ein Gegner. Ist er eine Illusion, kostet der Zauber 2 × (Stufe des Gegners − Stufe des Zaubernden) AE, mindestens 5. Bei genug AE verschwindet die Illusion (gilt als tot), sonst misslingt der Zauber. Ist der Gegner keine Illusion: Meldung, Kosten 2 AE.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_illusionen`
- **Deutung der KI:** Vermutlich nur gegen Illusionen nützlich. Gegen echte Gegner kostet er 2 AE für nichts.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-5"></a>
### 5. Verwandlung beenden (Heldenzauber) (`SP_VERWANDLUNG_BEENDEN`)

- **Im Code:** Zauber-ID 5 (`SP_VERWANDLUNG_BEENDEN`). Gruppe Antimagie, Herkunft Hexe. Probe auf MU/KL/CH. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Held. Ist er versteinert, kostet der Zauber 5–50 AE (1W10 × 5) und hebt die Versteinerung auf (bei zu wenig AE misslingt er). Ist er verwandelt (Fluch), wird der Fluch aufgehoben und alle sieben Eigenschaften steigen um 1. Sonst kostenlos abgebrochen. Im Original ist die Meldung für „kein Verwandlungszauber“ fehlerhaft formatiert, Fix 193 (Fanpatch-Stufe „nur Alle“) berichtigt sie.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_verwandlung`
- **Deutung der KI:** Vermutlich der Weg, eine Versteinerung oder einen Verwandlungsfluch loszuwerden.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-6"></a>
### 6. Band und Fessel (Heldenzauber) (`SP_BAND_UND_FESSEL`)

- **Im Code:** Zauber-ID 6 (`SP_BAND_UND_FESSEL`). Gruppe Beherrschung, Herkunft Waldelf. Probe auf KL/CH/KK, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 12 AE; bei misslungener Probe 6 AE. Einsatz: nur im Kampf. Ziel: ein Gegner oder ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel Gegner: Gegner mit Skelett-Grafik (alle Skelettarten) sind immun (misslingt). Alle anderen werden gefesselt (ohne Rundenzähler). Ein gefesselter Gegner kann sich nicht bewegen, aber aus dem Stand weiter angreifen und zaubern. Zu Beginn jedes seiner Züge befreit er sich mit 4 % Wahrscheinlichkeit. Er zählt für das Kampfende nicht mehr als aktiver Gegner, stehen nur noch Gefesselte, ist der Kampf gewonnen. Fix 312 (Fanpatch-Stufe „nur Alle“) sorgt dafür, dass so besiegte Gegner nicht in FIGHT.LST zurückgeschrieben werden und nicht erneut kämpfen. Ziel Held (nicht sich selbst): Er ist gefesselt. Will er sich im Kampf bewegen, braucht er eine MU-Probe +2, um sich zu befreien; misslingt sie, sinkt MU für 7 Stunden um 2 (nur bei MU über 4). Angreifen kann er weiter. Nach dem Kampf ist er wieder frei.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_band`, `Quelle/seg037.c:FIG_enemy_turn`, `Quelle/seg032.c:FIG_count_active_enemies`, `Quelle/seg032.c:do_fight`, `Quelle/seg033.c:FIG_menu`
- **Deutung der KI:** Sieht so aus, als ob Band und Fessel einen Gegner vor allem aus der Kampfwertung nimmt: Stehen nur noch Gefesselte, ist der Kampf gewonnen. Ein Gefesselter kann aber weiter zuschlagen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-7"></a>
### 7. Bannbaladin (Heldenzauber) (`SP_BANNBALADIN`)

- **Im Code:** Zauber-ID 7 (`SP_BANNBALADIN`). Gruppe Beherrschung, Herkunft Auelf. Probe auf IN/CH/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 8 AE; bei misslungener Probe 4 AE. Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Gegner. Tiere: Meldung „kein denkendes Wesen“, keine Kosten. Gegner mit Skelett-Grafik: misslingt. Sonst ist der Gegner besänftigt: Er kommt nicht mehr an die Reihe. Für das Kampfende zählt er aber weiter als aktiver Gegner.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_bannbaladin`, `Quelle/seg032.c:FIG_is_enemy_active`, `Quelle/seg032.c:FIG_count_active_enemies`
- **Deutung der KI:** Vermutlich ein starker Kampfzauber gegen denkende Gegner. Allerdings bleibt der Besänftigte für das Kampfende ein Gegner.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-8"></a>
### 8. Böser Blick (Heldenzauber) (`SP_BOESER_BLICK`)

- **Im Code:** Zauber-ID 8 (`SP_BOESER_BLICK`). Gruppe Beherrschung, Herkunft Druide. Probe auf MU/CH/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 8 AE; bei misslungener Probe 4 AE. Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Gegner. Gegner mit Skelett-Grafik: misslingt. Sonst kämpft der Gegner auf der Seite der Helden (abtrünnig) und bekommt 2 Angriffe pro Runde. Erleidet er Schaden durch einen Helden (Waffe oder Zauber), endet das wieder.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_boeser_blick`, `Quelle/seg041.c:FIG_damage_enemy`
- **Deutung der KI:** Vermutlich der stärkste Beherrschungszauber: Der Gegner kämpft für die Helden, bis ihm ein Held Schaden zufügt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-9"></a>
### 9. Große Gier (Heldenzauber) (`SP_GROSSE_GIER`)

- **Im Code:** Zauber-ID 9 (`SP_GROSSE_GIER`). Gruppe Beherrschung, Herkunft Hexe. Probe auf KL/IN/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 8 AE; bei misslungener Probe 4 AE. Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Im Original nicht umgesetzt: Der Handler lässt den Zauber immer misslingen (halbe Kosten). Mit Fix 382 (Fanpatch-Stufe „Wichtig“) flieht der Gegner wie beim Horriphobus (Tiere sind ausgenommen, dann misslingt er); Kosten aus der Tabelle.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_grosse_gier`
- **Deutung der KI:** Sieht so aus, als ob der Zauber im Original immer misslingt. Mit dem Fanpatch schlägt er Gegner in die Flucht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-10"></a>
### 10. Große Verwirrung (Heldenzauber) (`SP_GROSSE_VERWIRRUNG`)

- **Im Code:** Zauber-ID 10 (`SP_GROSSE_VERWIRRUNG`). Gruppe Beherrschung, Herkunft Druide. Probe auf KL/KL/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Gegner. Gegner mit Skelett-Grafik: misslingt. Sonst sinken AT und PA des Gegners dauerhaft (für diesen Kampf) um je 2, Kosten 8 AE.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_grosse_ver`
- **Deutung der KI:** Vermutlich ein billiger Schwächungszauber gegen einen Gegner.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-11"></a>
### 11. Herr über das Tierreich (Heldenzauber) (`SP_HERR_UEBER_DAS_TIERREICH`)

- **Im Code:** Zauber-ID 11 (`SP_HERR_UEBER_DAS_TIERREICH`). Gruppe Beherrschung, Herkunft Druide. Probe auf MU/MU/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Gegner. Wirkt nur auf Tiere (sonst Meldung, keine Kosten). Kostet so viele AE wie der Wert first_ap des Tiers (die AP, die es beim Sieg bringt). Bei genug AE ist das Tier besänftigt: Es kommt nicht mehr an die Reihe, zählt für das Kampfende aber weiter als aktiver Gegner.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_herrdertiere`
- **Deutung der KI:** Vermutlich nur gegen Tiere sinnvoll, und gegen starke Tiere teuer.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-12"></a>
### 12. Horriphobus (Heldenzauber) (`SP_HORRIPHOBUS`)

- **Im Code:** Zauber-ID 12 (`SP_HORRIPHOBUS`). Gruppe Beherrschung, Herkunft Magier. Probe auf MU/IN/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Gegner. Gegner mit Skelett-Grafik: misslingt. Sonst ist der Gegner verängstigt und flieht: Er sucht nur noch einen Fluchtweg und greift nicht mehr an. Ein abtrünniger Gegner (Böser Blick) verliert das wieder. Für das Kampfende zählt ein verängstigter Gegner weiter als aktiv (die Zählung prüft „verängstigt“ nicht).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_horriphobus`, `Quelle/seg037.c:FIG_enemy_turn`
- **Deutung der KI:** Vermutlich ein guter Zauber, um einen Gegner aus dem Kampf zu nehmen. Er bleibt aber Gegner, bis er geflohen ist.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-13"></a>
### 13. Magischer Raub (Heldenzauber) (`SP_MAGISCHER_RAUB`)

- **Im Code:** Zauber-ID 13 (`SP_MAGISCHER_RAUB`). Gruppe Beherrschung, Herkunft Druide. Probe auf MU/KL/KK, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 6 AE; bei misslungener Probe 3 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_magischerraub`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-14"></a>
### 14. Respondami Veritar (Heldenzauber) (`SP_RESPONDAMI_VERITAR`)

- **Im Code:** Zauber-ID 14 (`SP_RESPONDAMI_VERITAR`). Gruppe Beherrschung, Herkunft Magier. Probe auf KL/IN/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_respondami`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-15"></a>
### 15. Sanftmut (Heldenzauber) (`SP_SANFTMUT`)

- **Im Code:** Zauber-ID 15 (`SP_SANFTMUT`). Gruppe Beherrschung, Herkunft Hexe. Probe auf MU/CH/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ruft denselben Handler wie Herr über das Tierreich auf: wirkt nur auf Tiere, kostet so viele AE wie deren Wert first_ap, das Tier kommt danach nicht mehr an die Reihe.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_sanftmut`, `Quelle/seg099.c:spell_herrdertiere`
- **Deutung der KI:** Sieht so aus, als ob Sanftmut im Kampf genau wie Herr über das Tierreich wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-16"></a>
### 16. Somnigravis Tausendschlaf (Heldenzauber) (`SP_SOMNIGRAVIS_TAUSENDSCHLAF`)

- **Im Code:** Zauber-ID 16 (`SP_SOMNIGRAVIS_TAUSENDSCHLAF`). Gruppe Beherrschung, Herkunft Waldelf. Probe auf KL/CH/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 8 AE; bei misslungener Probe 4 AE. Einsatz: nur im Kampf. Ziel: ein Gegner oder ein Held, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ziel Gegner: Gegner mit Skelett-Grafik: misslingt. Sonst schläft der Gegner: Er kommt nicht an die Reihe, bis ihn ein Held angreift (der Angriff weckt ihn). Für das Kampfende zählt er weiter als aktiv. Ziel Held (nicht sich selbst): Er schläft. Zu Beginn jeder seiner Kampfphasen wacht er mit 74 % Wahrscheinlichkeit auf, ein Angriff auf ihn weckt ihn sofort.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_somnigravis`, `Quelle/seg032.c:FIG_do_round`, `Quelle/seg042.c:FIG_do_hero_action`
- **Deutung der KI:** Vermutlich ein guter Zauber gegen einzelne starke Gegner. Ein Angriff weckt sie aber.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-17"></a>
### 17. Zwingtanz (Heldenzauber) (`SP_ZWINGTANZ`)

- **Im Code:** Zauber-ID 17 (`SP_ZWINGTANZ`). Gruppe Beherrschung, Herkunft Druide. Probe auf MU/KL/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 6 AE; bei misslungener Probe 3 AE. Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ziel: ein Gegner. Gegner mit Skelett-Grafik: misslingt. Sonst tanzt der Gegner bis zum Kampfende: Er kommt nicht mehr an die Reihe, und seine Parade gegen Angriffe von Helden ist um 3 gesenkt. Für das Kampfende zählt er weiter als aktiv.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_zwingtanz`, `Quelle/seg032.c:FIG_is_enemy_active`, `Quelle/seg042.c:FIG_do_hero_action`
- **Deutung der KI:** Vermutlich nimmt der Zauber einen Gegner aus dem Geschehen und macht ihn leichter zu treffen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-18"></a>
### 18. Furor Blut und Sulphurdampf (Heldenzauber) (`SP_FUROR_BLUT_UND_SULPHURDAMPF`)

- **Im Code:** Zauber-ID 18 (`SP_FUROR_BLUT_UND_SULPHURDAMPF`). Gruppe Dämonologie, Herkunft Magier. Probe auf MU/MU/CH. AE-Kosten laut Tabelle: 13 AE; bei misslungener Probe 6 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Nicht umgesetzt: Der Handler lässt den Zauber immer misslingen (halbe Kosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_furor_blut`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Original nicht programmiert ist und immer misslingt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-19"></a>
### 19. Geister austreiben (Heldenzauber) (`SP_GEISTER_AUSTREIBEN`)

- **Im Code:** Zauber-ID 19 (`SP_GEISTER_AUSTREIBEN`). Gruppe Dämonologie, Herkunft Druide. Probe auf MU/MU/CH. AE-Kosten laut Tabelle: 13 AE; bei misslungener Probe 6 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_geister_bannen`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-20"></a>
### 20. Geister beschwören (Heldenzauber) (`SP_GEISTER_BESCHWOEREN`)

- **Im Code:** Zauber-ID 20 (`SP_GEISTER_BESCHWOEREN`). Gruppe Dämonologie, Herkunft Druide. Probe auf MU/MU/CH. AE-Kosten laut Tabelle: 11 AE; bei misslungener Probe 5 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Nicht umgesetzt: Der Handler lässt den Zauber immer misslingen (halbe Kosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_geister_rufen`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Original nicht programmiert ist und immer misslingt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-21"></a>
### 21. Heptagon und Krötenei (Heldenzauber) (`SP_HEPTAGON_UND_KROETENEI`)

- **Im Code:** Zauber-ID 21 (`SP_HEPTAGON_UND_KROETENEI`). Gruppe Dämonologie, Herkunft Magier. Probe auf MU/MU/CH. AE-Kosten laut Tabelle: 23 AE; bei misslungener Probe 11 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Nicht umgesetzt: Der Handler lässt den Zauber immer misslingen (halbe Kosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_heptagon`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Original nicht programmiert ist und immer misslingt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-22"></a>
### 22. Krähenruf (Heldenzauber) (`SP_KRAEHENRUF`)

- **Im Code:** Zauber-ID 22 (`SP_KRAEHENRUF`). Gruppe Dämonologie, Herkunft Hexe. Probe auf MU/CH/CH. AE-Kosten laut Tabelle: 13 AE; bei misslungener Probe 6 AE. Einsatz: nur im Kampf. Ziel: ein Gegner oder ein Held, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ziel: ein Gegner (oder ein Held). Für jede Stufe des Zaubernden plus 2 wird gewürfelt: Mit 40 % (W20 ≤ 8) greift ein Vogel an, und er trifft, wenn ein zweiter W20 über der Parade des Ziels liegt. Jeder Treffer macht 1 Schadenspunkt, Rüstungsschutz zählt nicht. Die Parade eines Helden ist dabei sein PA-Wert der Waffe minus Kampfmodifikator minus halbe Behinderung.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_kraehenruf`, `Quelle/seg098.c:get_attackee_parade`, `Quelle/seg098.c:FIG_do_spell_damage`
- **Deutung der KI:** Vermutlich ein schwacher Schadenszauber, dessen Schaden mit der Stufe des Zaubernden wächst.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-23"></a>
### 23. Skelettarius Kryptaduft (Heldenzauber) (`SP_SKELETTARIUS_KRYPTADUFT`)

- **Im Code:** Zauber-ID 23 (`SP_SKELETTARIUS_KRYPTADUFT`). Gruppe Dämonologie, Herkunft Magier. Probe auf MU/MU/CH. AE-Kosten laut Tabelle: 15 AE; bei misslungener Probe 7 AE. Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein toter Gegner. Ist er nicht tot: Meldung, keine Kosten. Sonst wird an seiner Stelle ein Zombie (Gegnertyp 16) aufgestellt, der auf der Seite der Helden kämpft (abtrünnig). Im Original bleibt eine darunterliegende Leiche sichtbar, aber nicht mehr wählbar (Fix 195 (Fanpatch-Stufe „nur Alle“)), und jeder Skelettarius belegt zusätzlichen Grafikspeicher, bis er überläuft (Fix 196 (Fanpatch-Stufe „Wichtig“)).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_skelettarius`
- **Deutung der KI:** Vermutlich ein starker Zauber im Kampf: Ein toter Gegner steht als Zombie auf der Seite der Helden wieder auf.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-24"></a>
### 24. Elementare beschwören (Heldenzauber) (`SP_ELEMENTARE_BESCHWOEREN`)

- **Im Code:** Zauber-ID 24 (`SP_ELEMENTARE_BESCHWOEREN`). Gruppe Elemente, Herkunft Druide. Probe auf MU/KL/CH. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Nicht umgesetzt: Der Handler lässt den Zauber immer misslingen (halbe Kosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_elementare`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Original nicht programmiert ist und immer misslingt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-25"></a>
### 25. Nihilatio Gravitas (Heldenzauber) (`SP_NIHILATIO_GRAVITAS`)

- **Im Code:** Zauber-ID 25 (`SP_NIHILATIO_GRAVITAS`). Gruppe Elemente, Herkunft Magier. Probe auf KL/KK/KK. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_nihilatio`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-26"></a>
### 26. Solidirid Farbenspiel (Heldenzauber) (`SP_SOLIDIRID_FARBENSPIEL`)

- **Im Code:** Zauber-ID 26 (`SP_SOLIDIRID_FARBENSPIEL`). Gruppe Elemente, Herkunft Firnelf. Probe auf IN/GE/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_solidirid`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-27"></a>
### 27. Axxeleratus Blitzgeschwind (Heldenzauber) (`SP_AXXELERATUS_BLITZGESCHWIND`)

- **Im Code:** Zauber-ID 27 (`SP_AXXELERATUS_BLITZGESCHWIND`). Gruppe Bewegung, Herkunft Waldelf. Probe auf KL/GE/KK. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: nur im Kampf. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Held. Wirkt der Zauber schon auf ihn, misslingt er. Sonst für 1 Spielminute: AT und PA seiner aktuellen Waffengattung +2, dazu in jeder Kampfrunde 4 Bewegungspunkte und eine Aktionsphase mehr. Weil ein Angriff die Bewegungspunkte auf 0 setzt, öffnet die zweite Aktionsphase kein Menü: Sie wiederholt denselben Angriff auf dasselbe Ziel, ohne Reichweite, Schusslinie oder Munition zu prüfen (Originalfehler). Mit Fix 379 (Fanpatch-Stufe „Wichtig“) nur noch, wenn das Ziel erreichbar ist, sonst wartet der Held.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_axxeleratus`, `Quelle/seg032.c:FIG_do_round`
- **Deutung der KI:** Sieht so aus, als ob Axxeleratus kurz, aber sehr stark ist: mehr Bewegung, eine zusätzliche Aktion und bessere AT/PA.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-28"></a>
### 28. Foramen Foraminor (Heldenzauber) (`SP_FORAMEN_FORAMINOR`)

- **Im Code:** Zauber-ID 28 (`SP_FORAMEN_FORAMINOR`). Gruppe Bewegung, Herkunft Magier. Probe auf KL/KL/FF. AE-Kosten laut Tabelle: 6 AE; bei misslungener Probe 3 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Wirkt nur vor einer verschlossenen Tür im Dungeon (Menü „Tür öffnen“): Die Tür geht auf, der Zaubernde bekommt 1 AP. Anderswo kostet der Zauber die vollen AE und bewirkt nichts. Im Original zählt dabei nur der Modus „Schloss knacken“. Mit Fix 197 (Fanpatch-Stufe „Wichtig“) auch „Tür öffnen“, außerhalb von Dungeons nie. Zuschlag der Tür: Beim Zaubern über das Zaubermenü fehlt im Original der türeigene Zuschlag, Fix 191 (Fanpatch-Stufe „Wichtig“) rechnet ihn ein.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_foramen`
- **Deutung der KI:** Vermutlich ein Ersatz für Dietriche an verschlossenen Türen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-29"></a>
### 29. Motoricus Motilitich (Heldenzauber) (`SP_MOTORICUS_MOTILITICH`)

- **Im Code:** Zauber-ID 29 (`SP_MOTORICUS_MOTILITICH`). Gruppe Bewegung, Herkunft Magier. Probe auf KL/FF/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_motoricus`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-30"></a>
### 30. Spurlos trittlos fährtenlos (Heldenzauber) (`SP_SPURLOS_TRITTLOS_FAEHRTENLOS`)

- **Im Code:** Zauber-ID 30 (`SP_SPURLOS_TRITTLOS_FAEHRTENLOS`). Gruppe Bewegung, Herkunft Waldelf. Probe auf IN/GE/GE. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_spurlos`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-31"></a>
### 31. Transversalis Teleport (Heldenzauber) (`SP_TRANSVERSALIS_TELEPORT`)

- **Im Code:** Zauber-ID 31 (`SP_TRANSVERSALIS_TELEPORT`). Gruppe Bewegung, Herkunft Magier. Probe auf KL/KL/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Nur in einem Dungeon oder einer Stadt. Auf der Karte wird ein Zielfeld gewählt: Teleport auf ein freies Feld kostet 15 AE. Wände, Häuser und Wasser sind verboten (Zauber misslingt), das eigene Feld kostet nichts. Im Original prüft der Code nur, ob die Gruppe in einem Dungeon oder einer Stadt ist. Mit Fix 383 (Fanpatch-Stufe „Wichtig“) ist er außerdem während einer Reise nicht erlaubt.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_transversalis`, `Quelle/seg074.c:select_teleport_dest`
- **Deutung der KI:** Vermutlich ein bequemer Weg, sich in Stadt oder Dungeon zu versetzen, für 15 AE.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-32"></a>
### 32. Über Eis und über Schnee (Heldenzauber) (`SP_UEBER_EIS_UND_UEBER_SCHNEE`)

- **Im Code:** Zauber-ID 32 (`SP_UEBER_EIS_UND_UEBER_SCHNEE`). Gruppe Bewegung, Herkunft Firnelf. Probe auf KL/GE/KK. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Nicht umgesetzt: Der Handler lässt den Zauber immer misslingen (halbe Kosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_ueber_eis`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Original nicht programmiert ist und immer misslingt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-33"></a>
### 33. Balsam Salabunde (Heldenzauber) (`SP_BALSAM_SALABUNDE`)

- **Im Code:** Zauber-ID 33 (`SP_BALSAM_SALABUNDE`). Gruppe Heilung, Herkunft Auelf. Probe auf KL/IN/CH. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Im Original im Kampf trotzdem nicht wählbar: Eine Zaubergruppe lässt sich im Kampf nur öffnen, wenn sie einen reinen Kampfspruch enthält, und die Gruppe Heilung hat keinen (Quelle/seg098.c:can_use_spellclass). Mit Fix 444 (Fanpatch-Stufe „nur Alle“) im Kampf wählbar. Wirkung im Handler: Ziel: ein Held. Der Zaubernde gibt ein, wie viele LE er heilen will. Unter 7 kostet es trotzdem 7 AE (geheilt wird die eingegebene Zahl). Sonst kostet jeder LE 1 AE, höchstens so viel, wie dem Ziel fehlt. Reicht die AE nicht, wird mit allen AE entsprechend weniger geheilt. Im Computerkampf (Autokampf oder ein NSC zaubert) heilt er die Hälfte der fehlenden LE ohne Nachfrage.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_balsam`
- **Deutung der KI:** Vermutlich der wichtigste Heilzauber: 1 AE pro LE, aber mindestens 7 AE.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-34"></a>
### 34. Hexenspeichel (Heldenzauber) (`SP_HEXENSPEICHEL`)

- **Im Code:** Zauber-ID 34 (`SP_HEXENSPEICHEL`). Gruppe Heilung, Herkunft Hexe. Probe auf IN/CH/FF. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Im Original im Kampf trotzdem nicht wählbar: Eine Zaubergruppe lässt sich im Kampf nur öffnen, wenn sie einen reinen Kampfspruch enthält, und die Gruppe Heilung hat keinen (Quelle/seg098.c:can_use_spellclass). Mit Fix 444 (Fanpatch-Stufe „nur Alle“) im Kampf wählbar. Wirkung im Handler: Ziel: ein anderer Held (nicht sich selbst). Eingabe der gewünschten LE. Mit Fix 199 (Fanpatch-Stufe „Wichtig“): Jeder geheilte LE kostet 2 AE, höchstens so viele LE, wie dem Ziel fehlen, gedeckelt durch die AE des Zaubernden. Im Original ist die Rechnung fehlerhaft: Fehlt dem Ziel weniger als das Doppelte der Eingabe, kostet es 2 × fehlende LE und heilt alle fehlenden LE. Sonst kostet es 2 × Eingabe. Die AE des Zaubernden werden dabei praktisch nicht geprüft.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_hexenspeichel`
- **Deutung der KI:** Sieht so aus, als ob Hexenspeichel teurer ist als Balsam (2 AE pro LE) und im Original falsch rechnet.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-35"></a>
### 35. Klarum Purum Kräutersud (Heldenzauber) (`SP_KLARUM_PURUM_KRAEUTERSUD`)

- **Im Code:** Zauber-ID 35 (`SP_KLARUM_PURUM_KRAEUTERSUD`). Gruppe Heilung, Herkunft Magier. Probe auf KL/KL/CH. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Im Original im Kampf trotzdem nicht wählbar: Eine Zaubergruppe lässt sich im Kampf nur öffnen, wenn sie einen reinen Kampfspruch enthält, und die Gruppe Heilung hat keinen (Quelle/seg098.c:can_use_spellclass). Mit Fix 444 (Fanpatch-Stufe „nur Alle“) im Kampf wählbar. Wirkung im Handler: Ziel: ein Held. Ist er nicht vergiftet: Meldung, keine Kosten. Sonst kostet der Zauber so viele AE wie die Stärke des ersten Gifts (4 bis 15) und heilt dieses Gift (bei zu wenig AE misslingt er).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_klarum_purum`, `Quelle/datseg.c:g_poison_strength`
- **Deutung der KI:** Vermutlich der Zauber gegen Vergiftungen; je stärker das Gift, desto teurer.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-36"></a>
### 36. Ruhe Körper, ruhe Geist (Heldenzauber) (`SP_RUHE_KOERPER_RUHE_GEIST`)

- **Im Code:** Zauber-ID 36 (`SP_RUHE_KOERPER_RUHE_GEIST`). Gruppe Heilung, Herkunft Firnelf. Probe auf KL/CH/KK. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: nur außerhalb des Kampfes. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Held. Er regeneriert in der nächsten Nacht doppelt.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_ruhe_koerper`
- **Deutung der KI:** Vermutlich ein günstiger Weg zu doppelter Regeneration in der nächsten Nacht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-37"></a>
### 37. Tiere besprechen (Heldenzauber) (`SP_TIERE_BESPRECHEN`)

- **Im Code:** Zauber-ID 37 (`SP_TIERE_BESPRECHEN`). Gruppe Heilung, Herkunft Hexe. Probe auf MU/IN/CH. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: im Kampf und außerhalb. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Im Original im Kampf trotzdem nicht wählbar: Eine Zaubergruppe lässt sich im Kampf nur öffnen, wenn sie einen reinen Kampfspruch enthält, und die Gruppe Heilung hat keinen (Quelle/seg098.c:can_use_spellclass). Mit Fix 444 (Fanpatch-Stufe „nur Alle“) im Kampf wählbar. Wirkung im Handler: Der Zaubernde gibt eine AE-Zahl ein, die abgezogen wird. Eine Heilwirkung gibt es im Code nicht.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_tiere_heilen`
- **Deutung der KI:** Sieht so aus, als ob man hier AE ausgibt, ohne dass etwas geschieht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-38"></a>
### 38. Adleraug und Luchsenohr (Heldenzauber) (`SP_ADLERAUG_UND_LUCHSENOHR`)

- **Im Code:** Zauber-ID 38 (`SP_ADLERAUG_UND_LUCHSENOHR`). Gruppe Wahrnehmung, Herkunft Auelf. Probe auf KL/IN/FF. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: im Kampf und außerhalb. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Wirkt nur auf den Zaubernden selbst: Sinnesschärfe +7 für 6 Spielminuten.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_adleraug`
- **Deutung der KI:** Vermutlich nur ein kurzer Schub für Sinnesschärfe des Zaubernden.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-39"></a>
### 39. Analüs Arcanstruktur (Heldenzauber) (`SP_ANALUES_ARCANSTRUKTUR`)

- **Im Code:** Zauber-ID 39 (`SP_ANALUES_ARCANSTRUKTUR`). Gruppe Wahrnehmung, Herkunft Magier. Probe auf KL/KL/IN. AE-Kosten laut Tabelle: 10 AE; bei misslungener Probe 5 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Zaubernde wählt einen Gegenstand aus seinem Gepäck. Hat der Gegenstand einen Analüs-Eintrag (siehe Gegenstandsliste), erscheint dessen Text, wenn das Probenergebnis mindestens den Wert des Eintrags erreicht, und der Gegenstand gilt als magisch erkannt. Sonst kommt die Meldung, dass sich der Zauber nicht bestimmen lässt. Im Original kostet dieser Fall die vollen AE, mit Fix 201 (Fanpatch-Stufe „nur Alle“) die halben. Gegenstände ohne Eintrag ergeben „nichts Besonderes“. Mit Fix 386 (Fanpatch-Stufe „nur Alle“) erscheint bei magischen Gegenständen ohne Eintrag „Zauber nicht bestimmbar“. Bricht man die Auswahl ab, liest das Original einen ungültigen Inventarplatz, Fix 200 (Fanpatch-Stufe „nur Alle“) fängt das ab. In der Akademie gilt das Probenergebnis als 99.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg099.c:spell_analues`, `Quelle/seg099.c:g_analues_items`
- **Deutung der KI:** Vermutlich der Weg, magische Gegenstände zu erkennen. Der Text verrät aber nicht immer, ob die Wirkung im Spiel auch existiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-40"></a>
### 40. Eigenschaften seid gelesen (Heldenzauber) (`SP_EIGENSCHAFTEN_SEID_GELESEN`)

- **Im Code:** Zauber-ID 40 (`SP_EIGENSCHAFTEN_SEID_GELESEN`). Gruppe Wahrnehmung, Herkunft Auelf. Probe auf KL/IN/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ziel: ein Gegner. Zeigt Name, Stufe, AT, PA, RS, Zahl der Angriffe, Trefferpunkte-Spanne, LE und AE des Gegners an. Im Original wird die Schadensspanne mit 0,8 malgenommen und ist damit zu niedrig angegeben. Fix 202 (Fanpatch-Stufe „nur Alle“) zeigt die echte Spanne.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_eigenschaften`
- **Deutung der KI:** Vermutlich nützlich, um einen Gegner einzuschätzen. Im Original ist die angezeigte Schadensspanne zu niedrig.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-41"></a>
### 41. Exposami Creatur (Heldenzauber) (`SP_EXPOSAMI_CREATUR`)

- **Im Code:** Zauber-ID 41 (`SP_EXPOSAMI_CREATUR`). Gruppe Wahrnehmung, Herkunft Waldelf. Probe auf KL/KL/IN. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur im Kampf. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Zählt die Gegner auf, die im laufenden Kampf erst noch erscheinen werden (nach Typ und Anzahl), sonst „keine weiteren Monster in Sicht“. Im Original wird beim Zählen ein nicht belegter Tabellenplatz mitgelesen, was falsche Zahlen ergeben kann. Fix 203 (Fanpatch-Stufe „nur Alle“) berichtigt das.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_exposami`
- **Deutung der KI:** Vermutlich eine Vorwarnung, welche Gegner im Kampf noch dazukommen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-42"></a>
### 42. Odem Arcanum (Heldenzauber) (`SP_ODEM_ARCANUM`)

- **Im Code:** Zauber-ID 42 (`SP_ODEM_ARCANUM`). Gruppe Wahrnehmung, Herkunft Auelf. Probe auf KL/IN/CH. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Zaubernde wählt einen Gegenstand aus seinem Gepäck und erfährt, ob er magisch ist; ein magischer gilt danach als erkannt. Bricht man die Auswahl ab, liest das Original einen ungültigen Inventarplatz, Fix 200 (Fanpatch-Stufe „nur Alle“) fängt das ab.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_odem_arcanum`
- **Deutung der KI:** Vermutlich die einfache Probe, ob ein Gegenstand magisch ist.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-43"></a>
### 43. Penetrizzel Holz und Stein (Heldenzauber) (`SP_PENETRIZZEL_HOLZ_UND_STEIN`)

- **Im Code:** Zauber-ID 43 (`SP_PENETRIZZEL_HOLZ_UND_STEIN`). Gruppe Wahrnehmung, Herkunft Magier. Probe auf KL/KL/KK. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Deckt auf der Automap ein 5 × 5 Felder großes Gebiet um die Gruppe auf.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_penetrizzel`
- **Deutung der KI:** Vermutlich eine kleine Kartenhilfe in Dungeons.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-44"></a>
### 44. Sensibar wahr und klar (Heldenzauber) (`SP_SENSIBAR_WAHR_UND_KLAR`)

- **Im Code:** Zauber-ID 44 (`SP_SENSIBAR_WAHR_UND_KLAR`). Gruppe Wahrnehmung, Herkunft Auelf. Probe auf KL/IN/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_sensibar`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-45"></a>
### 45. Chamaelioni Mimikry (Heldenzauber) (`SP_CHAMAELIONI_MIMIKRY`)

- **Im Code:** Zauber-ID 45 (`SP_CHAMAELIONI_MIMIKRY`). Gruppe Visionen, Herkunft Waldelf. Probe auf IN/CH/GE. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur im Kampf. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Wirkt auf den Zaubernden selbst: Gegner greifen ihn mit AT −5 an, bis er sich im Kampf bewegt oder der Kampf endet.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_chamaelioni`, `Quelle/seg043.c:FIG_do_enemy_action`, `Quelle/seg033.c:FIG_menu`
- **Deutung der KI:** Vermutlich ein Schutzzauber für den Zaubernden, solange er stehen bleibt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-46"></a>
### 46. Duplicatus Doppelpein (Heldenzauber) (`SP_DUPLICATUS_DOPPELPEIN`)

- **Im Code:** Zauber-ID 46 (`SP_DUPLICATUS_DOPPELPEIN`). Gruppe Visionen, Herkunft Magier. Probe auf KL/CH/GE. AE-Kosten laut Tabelle: 8 AE; bei misslungener Probe 4 AE. Einsatz: nur im Kampf. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Wirkt auf den Zaubernden selbst: Die AT von Gegnern gegen ihn wird bis zum Kampfende halbiert.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_duplicatus`, `Quelle/seg043.c:FIG_do_enemy_action`
- **Deutung der KI:** Vermutlich ein starker Schutzzauber für den Zaubernden für den ganzen Kampf.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-47"></a>
### 47. Harmlose Gestalt (Heldenzauber) (`SP_HARMLOSE_GESTALT`)

- **Im Code:** Zauber-ID 47 (`SP_HARMLOSE_GESTALT`). Gruppe Visionen, Herkunft Hexe. Probe auf KL/CH/GE. AE-Kosten laut Tabelle: 6 AE; bei misslungener Probe 3 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_harmlos`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-48"></a>
### 48. Hexenknoten (Heldenzauber) (`SP_HEXENKNOTEN`)

- **Im Code:** Zauber-ID 48 (`SP_HEXENKNOTEN`). Gruppe Visionen, Herkunft Hexe. Probe auf KL/IN/CH. AE-Kosten laut Tabelle: 4 AE; bei misslungener Probe 2 AE. Einsatz: nur im Kampf. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Stellt auf das Feld vor dem Zaubernden (in Blickrichtung) einen Hexenknoten, der das Feld blockiert. Ist das Feld besetzt oder der Grafikspeicher zu voll, misslingt der Zauber.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_hexenknoten`
- **Deutung der KI:** Vermutlich ein Hindernis, das einen Durchgang versperrt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-49"></a>
### 49. Blitz dich find (Heldenzauber) (`SP_BLITZ_DICH_FIND`)

- **Im Code:** Zauber-ID 49 (`SP_BLITZ_DICH_FIND`). Gruppe Kampf, Herkunft Waldelf. Probe auf KL/IN/GE. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur im Kampf. Ziel: ein Gegner oder ein Held, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ziel Held (nicht sich selbst) oder Gegner: geblendet für 3 Kampfphasen, in denen er nichts tun kann. Ein geblendeter Gegner pariert Angriffe von Helden mit PA −5.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_blitz`, `Quelle/seg032.c:FIG_do_round`, `Quelle/seg042.c:FIG_do_hero_action`
- **Deutung der KI:** Vermutlich ein günstiger Zauber, um einen Gegner für ein paar Phasen auszuschalten.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-50"></a>
### 50. Ecliptifactus (Heldenzauber) (`SP_ECLIPTIFACTUS`)

- **Im Code:** Zauber-ID 50 (`SP_ECLIPTIFACTUS`). Gruppe Kampf, Herkunft Magier. Probe auf MU/KL/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur im Kampf. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Zaubernde gibt die Zahl der Kampfrunden ein. Kosten: 2 × Runden + 5 AE (reicht die AE nicht: Meldung, keine Kosten). Solange der Zähler läuft, wird der Schaden seiner Angriffe verdoppelt.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_ecliptifactus`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob Ecliptifactus den Schaden des Zaubernden für mehrere Runden verdoppelt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-51"></a>
### 51. Eisenrost und grüner Span (Heldenzauber) (`SP_EISENROST_UND_GRUENER_SPAN`)

- **Im Code:** Zauber-ID 51 (`SP_EISENROST_UND_GRUENER_SPAN`). Gruppe Kampf, Herkunft Magier. Probe auf KL/CH/GE. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur im Kampf. Ziel: ein Gegner oder ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel Held (nicht sich selbst): Hat er keine Waffe in der rechten Hand, misslingt der Zauber. Ist sie schon zerbrochen: Meldung. Hat sie einen Bruchfaktor über 0, zerbricht sie, sonst misslingt der Zauber. Mit Fix 390 (Fanpatch-Stufe „nur Alle“) zerbrechen unzerbrechliche Waffen (Zauberstab, Hexenbesen, magische Orknase, magisches Kurzschwert, Grimring) nicht. Ziel Gegner: Gegner, die Tiere sind, haben keine Waffe (Meldung), im Original kostet das trotzdem die vollen AE, mit Fix 205 (Fanpatch-Stufe „nur Alle“) nichts. Sonst ist die Waffe des Gegners zerbrochen: Er richtet ab dann nur noch halben Schaden an.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_eisenrost`, `Quelle/seg100.c:fix_unbreakable_weapon`, `Quelle/seg041.c:FIG_get_enemy_attack_damage`
- **Deutung der KI:** Vermutlich ein guter Zauber gegen bewaffnete Gegner: Ihr Schaden halbiert sich.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-52"></a>
### 52. Fulminictus Donnerkeil (Heldenzauber) (`SP_FULMINICTUS_DONNERKEIL`)

- **Im Code:** Zauber-ID 52 (`SP_FULMINICTUS_DONNERKEIL`). Gruppe Kampf, Herkunft Auelf. Probe auf KL/GE/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur im Kampf. Ziel: ein Gegner oder ein Held, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ziel: Gegner oder anderer Held (nicht sich selbst). Schaden 3W6 + Stufe des Zaubernden, höchstens so viel, wie er AE hat. Der Schaden geht ohne Rüstungsschutz direkt von der LE ab, die AE-Kosten sind gleich dem Schaden.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_fulminictus`, `Quelle/seg098.c:FIG_do_spell_damage`
- **Deutung der KI:** Vermutlich der verlässliche Schadenszauber, der Rüstung ignoriert. Er kostet genau so viele AE wie er Schaden macht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-53"></a>
### 53. Ignifaxius Flammenstrahl (Heldenzauber) (`SP_IGNIFAXIUS_FLAMMENSTRAHL`)

- **Im Code:** Zauber-ID 53 (`SP_IGNIFAXIUS_FLAMMENSTRAHL`). Gruppe Kampf, Herkunft Magier. Probe auf KL/GE/FF. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur im Kampf. Ziel: ein Gegner oder ein Held, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ziel: Gegner oder anderer Held (nicht sich selbst). Der Spieler wählt die Stufe der Feuerlanze (höchstens Stufe des Zaubernden + 1; im Autokampf und bei NSC immer das Höchste). Schaden: Stufe × W6, höchstens so viel, wie der Zaubernde AE hat, direkt von der LE ohne Rüstungsschutz. Gegen Mumien doppelter Schaden (die AE-Kosten bleiben beim einfachen Wert). Danach: Schaden/10 Punkte Rüstungsschutz gehen verloren (beim Helden an der Körperrüstung, höchstens deren RS), und AT und PA sinken um Stufe/2 (beim Helden für 1 Stunde, beim Gegner für den Kampf). Kosten = Schaden.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_ignifaxius`, `Quelle/seg098.c:FIG_do_spell_damage`
- **Deutung der KI:** Vermutlich der stärkste Schadenszauber. Er schwächt das Ziel zusätzlich, besonders wirksam gegen Mumien.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-54"></a>
### 54. Plumbumbarum und Narretei (Heldenzauber) (`SP_PLUMBUMBARUM_UND_NARRETEI`)

- **Im Code:** Zauber-ID 54 (`SP_PLUMBUMBARUM_UND_NARRETEI`). Gruppe Kampf, Herkunft Magier. Probe auf CH/GE/KK, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur im Kampf. Ziel: ein Gegner oder ein Held, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ziel Held (nicht sich selbst): AT −3 für 5 Kampfrunden. Ziel Gegner: AT −3 für den Rest des Kampfes.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_plumbumbarum`
- **Deutung der KI:** Vermutlich ein günstiger Schwächungszauber.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-55"></a>
### 55. Radau (Heldenzauber) (`SP_RADAU`)

- **Im Code:** Zauber-ID 55 (`SP_RADAU`). Gruppe Kampf, Herkunft Hexe. Probe auf MU/IN/CH. AE-Kosten laut Tabelle: 4 AE; bei misslungener Probe 2 AE. Einsatz: nur im Kampf. Ziel: ein Gegner oder ein Held, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ruft denselben Handler wie Krähenruf auf (gleiche Wirkung, eigene Tabellenwerte).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_radau`, `Quelle/seg099.c:spell_kraehenruf`
- **Deutung der KI:** Sieht so aus, als ob Radau im Spiel dasselbe ist wie Krähenruf.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-56"></a>
### 56. Saft, Kraft, Monstermacht (Heldenzauber) (`SP_SAFT_KRAFT_MONSTERMACHT`)

- **Im Code:** Zauber-ID 56 (`SP_SAFT_KRAFT_MONSTERMACHT`). Gruppe Kampf, Herkunft Magier. Probe auf MU/IN/CH. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur im Kampf. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Held. Für W20 Kampfrunden: AT +5, PA −5 und 5 Schadenspunkte mehr pro Treffer. Die AE-Kosten sind gleich der gewürfelten Rundenzahl. Fix 206 (Fanpatch-Stufe „nur Alle“) berichtigt einen Buchstabendreher in der Meldung.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_saft_kraft`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob der Held mehr Schaden macht, sich aber schlechter verteidigt. Dauer und Kosten sind Glückssache.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-57"></a>
### 57. Scharfes Auge und sichre Hand (Heldenzauber) (`SP_SCHARFES_AUGE_UND_SICHRE_HAND`)

- **Im Code:** Zauber-ID 57 (`SP_SCHARFES_AUGE_UND_SICHRE_HAND`). Gruppe Kampf, Herkunft Waldelf. Probe auf IN/GE/FF. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur im Kampf. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Held. Talente Wurfwaffen und Schusswaffen je +3 für 3 Kampfrunden.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_scharfes_auge`
- **Deutung der KI:** Vermutlich ein kleiner Fernkampf-Bonus für drei Runden.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-58"></a>
### 58. Hexenblick (Heldenzauber) (`SP_HEXENBLICK`)

- **Im Code:** Zauber-ID 58 (`SP_HEXENBLICK`). Gruppe Verständigung, Herkunft Hexe. Probe auf IN/IN/CH. AE-Kosten laut Tabelle: 2 AE; bei misslungener Probe 1 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_hexenblick`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-59"></a>
### 59. Nekropathia modernd Leich (Heldenzauber) (`SP_NEKROPATHIA_MODERND_LEICH`)

- **Im Code:** Zauber-ID 59 (`SP_NEKROPATHIA_MODERND_LEICH`). Gruppe Verständigung, Herkunft Magier. Probe auf MU/KL/CH. AE-Kosten laut Tabelle: 11 AE; bei misslungener Probe 5 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg100.c:spell_necropathia`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-60"></a>
### 60. Adler, Wolf und Hammerhai (Heldenzauber) (`SP_ADLER_WOLF_UND_HAMMERHAI`)

- **Im Code:** Zauber-ID 60 (`SP_ADLER_WOLF_UND_HAMMERHAI`). Gruppe Verwandlung, Herkunft Waldelf. Probe auf MU/IN/GE. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Nicht umgesetzt: Der Handler lässt den Zauber immer misslingen (halbe Kosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_adler`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Original nicht programmiert ist und immer misslingt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-61"></a>
### 61. Arcano Psychostabilis (Heldenzauber) (`SP_ARCANO_PSYCHOSTABILIS`)

- **Im Code:** Zauber-ID 61 (`SP_ARCANO_PSYCHOSTABILIS`). Gruppe Verwandlung, Herkunft Magier. Probe auf MU/KL/CH. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: nur im Kampf. Ziel: ein Held, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ziel: ein Held. MR +2 für 1 Stunde.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_arcano`
- **Deutung der KI:** Vermutlich ein kleiner Schutz gegen feindliche Beherrschungszauber.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-62"></a>
### 62. Armatrutz (Heldenzauber) (`SP_ARMATRUTZ`)

- **Im Code:** Zauber-ID 62 (`SP_ARMATRUTZ`). Gruppe Verwandlung, Herkunft Auelf. Probe auf IN/GE/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur im Kampf. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Nur auf den Zaubernden selbst. Er wählt den RS-Bonus, höchstens 9. Kosten: Bonus × Bonus AE. RS +Bonus für 5 Spielminuten. Im Original wird das erlaubte Höchstmaß falsch berechnet: Es ist die kleinste Zahl, deren Quadrat die vorhandenen AE erreicht oder übersteigt, man kann also mehr wählen, als man bezahlen kann. Fix 207 (Fanpatch-Stufe „Wichtig“) nimmt die größte Zahl, deren Quadrat höchstens den AE entspricht.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_armatrutz`
- **Deutung der KI:** Vermutlich ein teurer Rüstungszauber nur für den Zaubernden.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-63"></a>
### 63. Charisma steigern (Heldenzauber) (`SP_CH_STEIGERN`)

- **Im Code:** Zauber-ID 63 (`SP_CH_STEIGERN`). Gruppe Verwandlung, Herkunft Magier. Probe auf KL/CH/CH. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein anderer Held (nicht sich selbst). CH +2 für 2 Stunden. Ist CH beim Ziel schon über den Normalwert erhöht, kommt nur eine Meldung, die AE werden trotzdem voll abgezogen (der Handler setzt keine Sonderkosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_inc_ch`
- **Deutung der KI:** Vermutlich ein kurzer Eigenschaftsbonus für einen Mitstreiter.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-64"></a>
### 64. Feuerbann (Heldenzauber) (`SP_FEUERBANN`)

- **Im Code:** Zauber-ID 64 (`SP_FEUERBANN`). Gruppe Verwandlung, Herkunft Hexe. Probe auf MU/MU/KK. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: im Kampf und außerhalb. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Nur auf den Zaubernden selbst (wirkt er schon, kostenlos abgebrochen). Setzt eine Feuerbann-Markierung, im Original für Stufe × 5 Spielminuten. Diese Markierung wird im Original nirgends ausgewertet, der Zauber schützt also vor nichts. Mit Fix 213 (Fanpatch-Stufe „Wichtig“) hält sie Stufe × 1 Stunde und halbiert den Schaden von feindlichem Ignifaxius (siehe Gegnerzauber).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_feuerbann`, `Quelle/seg102.c:fix_hero_spell_protect`
- **Deutung der KI:** Sieht so aus, als ob Feuerbann im Original wirkungslos ist. Mit dem Fanpatch halbiert er feindliches Feuer.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-65"></a>
### 65. Fingerfertigkeit steigern (Heldenzauber) (`SP_FF_STEIGERN`)

- **Im Code:** Zauber-ID 65 (`SP_FF_STEIGERN`). Gruppe Verwandlung, Herkunft Waldelf. Probe auf KL/CH/FF. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein anderer Held (nicht sich selbst). FF +2 für 2 Stunden. Ist FF beim Ziel schon über den Normalwert erhöht, kommt nur eine Meldung, die AE werden trotzdem voll abgezogen (der Handler setzt keine Sonderkosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_inc_ff`
- **Deutung der KI:** Vermutlich ein kurzer Eigenschaftsbonus für einen Mitstreiter.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-66"></a>
### 66. Gewandtheit steigern (Heldenzauber) (`SP_GE_STEIGERN`)

- **Im Code:** Zauber-ID 66 (`SP_GE_STEIGERN`). Gruppe Verwandlung, Herkunft Auelf. Probe auf KL/CH/GE. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein anderer Held (nicht sich selbst). GE +2 für 2 Stunden. Ist GE beim Ziel schon über den Normalwert erhöht, kommt nur eine Meldung, die AE werden trotzdem voll abgezogen (der Handler setzt keine Sonderkosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_inc_ge`
- **Deutung der KI:** Vermutlich ein kurzer Eigenschaftsbonus für einen Mitstreiter.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-67"></a>
### 67. Intuition steigern (Heldenzauber) (`SP_IN_STEIGERN`)

- **Im Code:** Zauber-ID 67 (`SP_IN_STEIGERN`). Gruppe Verwandlung, Herkunft Firnelf. Probe auf KL/IN/CH. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein anderer Held (nicht sich selbst). IN +2 für 2 Stunden. Ist IN beim Ziel schon über den Normalwert erhöht, kommt nur eine Meldung, die AE werden trotzdem voll abgezogen (der Handler setzt keine Sonderkosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_inc_in`
- **Deutung der KI:** Vermutlich ein kurzer Eigenschaftsbonus für einen Mitstreiter.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-68"></a>
### 68. Körperkraft steigern (Heldenzauber) (`SP_KK_STEIGERN`)

- **Im Code:** Zauber-ID 68 (`SP_KK_STEIGERN`). Gruppe Verwandlung, Herkunft Firnelf. Probe auf KL/CH/KK. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein anderer Held (nicht sich selbst). KK +2 für 2 Stunden. Ist KK beim Ziel schon über den Normalwert erhöht, kommt nur eine Meldung, die AE werden trotzdem voll abgezogen (der Handler setzt keine Sonderkosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_inc_kk`
- **Deutung der KI:** Vermutlich ein kurzer Eigenschaftsbonus für einen Mitstreiter.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-69"></a>
### 69. Klugheit steigern (Heldenzauber) (`SP_KL_STEIGERN`)

- **Im Code:** Zauber-ID 69 (`SP_KL_STEIGERN`). Gruppe Verwandlung, Herkunft Magier. Probe auf KL/KL/CH. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein anderer Held (nicht sich selbst). KL +2 für 2 Stunden. Ist KL beim Ziel schon über den Normalwert erhöht, kommt nur eine Meldung, die AE werden trotzdem voll abgezogen (der Handler setzt keine Sonderkosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_inc_kl`
- **Deutung der KI:** Vermutlich ein kurzer Eigenschaftsbonus für einen Mitstreiter.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-70"></a>
### 70. Mut steigern (Heldenzauber) (`SP_MU_STEIGERN`)

- **Im Code:** Zauber-ID 70 (`SP_MU_STEIGERN`). Gruppe Verwandlung, Herkunft Firnelf. Probe auf MU/KL/CH. AE-Kosten laut Tabelle: 7 AE; bei misslungener Probe 3 AE. Einsatz: im Kampf und außerhalb. Ziel: ein Held, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein anderer Held (nicht sich selbst). MU +2 für 2 Stunden. Ist MU beim Ziel schon über den Normalwert erhöht, kommt nur eine Meldung, die AE werden trotzdem voll abgezogen (der Handler setzt keine Sonderkosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_inc_mu`
- **Deutung der KI:** Vermutlich ein kurzer Eigenschaftsbonus für einen Mitstreiter.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-71"></a>
### 71. Mutabili Hybridil (Heldenzauber) (`SP_MUTABILI_HYBRIDIL`)

- **Im Code:** Zauber-ID 71 (`SP_MUTABILI_HYBRIDIL`). Gruppe Verwandlung, Herkunft Magier. Probe auf KL/GE/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: im Kampf und außerhalb. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Nicht umgesetzt: Der Handler lässt den Zauber immer misslingen (halbe Kosten).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_mutabili`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Original nicht programmiert ist und immer misslingt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-72"></a>
### 72. Paralü Paralein (Heldenzauber) (`SP_PARALUE_PARALEIN`)

- **Im Code:** Zauber-ID 72 (`SP_PARALUE_PARALEIN`). Gruppe Verwandlung, Herkunft Magier. Probe auf IN/CH/KK, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: 13 AE; bei misslungener Probe 6 AE. Einsatz: nur im Kampf. Ziel: ein Gegner oder ein Held, Reichweite Fernzauber im Kampf. Wirkung im Handler: Ziel Gegner: versteinert (ohne Ausnahme für Skelette). Ein versteinerter Gegner kommt nicht mehr an die Reihe und zählt nicht mehr als aktiver Gegner. Ziel Held (nicht sich selbst): versteinert. Im Original prüft der Handler zuerst das Ziel des vorigen Zaubers. War das der Zaubernde selbst, endet der Zauber kostenlos ohne Wirkung. Fix 208 (Fanpatch-Stufe „nur Alle“) setzt das Ziel vorher richtig.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_paralue`, `Quelle/seg032.c:FIG_count_active_enemies`
- **Deutung der KI:** Vermutlich einer der stärksten Zauber: Der Gegner ist versteinert und zählt nicht mehr.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-73"></a>
### 73. Salander Mutanderer (Heldenzauber) (`SP_SALANDER_MUTANDERER`)

- **Im Code:** Zauber-ID 73 (`SP_SALANDER_MUTANDERER`). Gruppe Verwandlung, Herkunft Magier. Probe auf KL/IN/CH, erschwert um die Magieresistenz des Ziels. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur im Kampf. Ziel: ein Gegner, Reichweite Berührung (Nachbarfeld) im Kampf. Wirkung im Handler: Ziel: ein Gegner. Kosten: 3 × MR des Gegners, mindestens 25 AE (reicht die AE nicht: Meldung, keine Kosten). Der Gegner wird in einen Pilz verwandelt: Er kommt nicht mehr an die Reihe und zählt nicht mehr als aktiver Gegner. Weitere Zauber mit MR-Zuschlag gegen ihn gelingen nicht mehr.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_salander`
- **Deutung der KI:** Vermutlich ein teurer, aber sicherer Weg, einen Gegner auszuschalten.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-74"></a>
### 74. In See und Fluss (Heldenzauber) (`SP_IN_SEE_UND_FLUSS`)

- **Im Code:** Zauber-ID 74 (`SP_IN_SEE_UND_FLUSS`). Gruppe Verwandlung, Herkunft Auelf. Probe auf MU/KL/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_see`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-75"></a>
### 75. Visibili Vanitar (Heldenzauber) (`SP_VISIBILI_VANITAR`)

- **Im Code:** Zauber-ID 75 (`SP_VISIBILI_VANITAR`). Gruppe Verwandlung, Herkunft Auelf. Probe auf KL/KL/GE. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: im Kampf und außerhalb. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Zaubernde gibt die Dauer in Spielrunden ein. Kosten 5 AE je Runde; bei 0 Runden oder wenn er schon unsichtbar ist, kostenlos abgebrochen. Er ist für Runden × 5 Spielminuten unsichtbar. Wirkung: Ein Kampf fällt aus, wenn die Gruppe nur aus diesem einen Helden besteht (Original). Mit Fix 186 (Fanpatch-Stufe „Wichtig“) macht der Zauber die ganze Gruppe unsichtbar, und der Kampf fällt aus, wenn alle lebenden Helden unsichtbar sind.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_visibili`, `Quelle/seg032.c:do_fight`
- **Deutung der KI:** Sieht so aus, als ob Unsichtbarkeit vor allem Kämpfe verhindert, im Original nur für einen allein reisenden Helden.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-76"></a>
### 76. Abvenenum Pest und Galle (Heldenzauber) (`SP_ABVENENUM_PEST_UND_GALLE`)

- **Im Code:** Zauber-ID 76 (`SP_ABVENENUM_PEST_UND_GALLE`). Gruppe Veränderung, Herkunft Waldelf. Probe auf KL/KL/FF. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_abvenenum`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-77"></a>
### 77. Aeolitus Windgebraus (Heldenzauber) (`SP_AEOLITUS_WINDGEBRAUS`)

- **Im Code:** Zauber-ID 77 (`SP_AEOLITUS_WINDGEBRAUS`). Gruppe Veränderung, Herkunft Firnelf. Probe auf KL/CH/KK. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_aeolitus`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-78"></a>
### 78. Brenne toter Stoff! (Heldenzauber) (`SP_BRENNE_TOTER_STOFF`)

- **Im Code:** Zauber-ID 78 (`SP_BRENNE_TOTER_STOFF`). Gruppe Veränderung, Herkunft Magier. Probe auf MU/KL/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Zündet eine Fackel oder Laterne im Gepäck des Zaubernden an (hat er beides, fragt das Spiel). Fackel: brennt danach mit Brennzähler 10 (2,5 Stunden). Laterne: braucht eine Flasche Öl (wird verbraucht, eine Bronzeflasche kommt zurück) und brennt mit Brennzähler 100 (25 Stunden). Kosten in beiden Fällen W20 AE statt des Tabellenwerts. Ohne Fackel und Laterne oder ohne Öl: Meldung, keine Kosten. Im Original wird immer die erste Fackel bzw. Laterne angezündet, und nach dem Anzünden über „Benutzen“ findet der Zauber nur noch dieselbe Art (Fix 210 (Fanpatch-Stufe „nur Alle“) und Fix 211 (Fanpatch-Stufe „Wichtig“)).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_brenne`, `Quelle/seg107.c:item_ignite`
- **Deutung der KI:** Vermutlich der magische Ersatz für das Zunderkästchen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-79"></a>
### 79. Claudibus Clavistibor (Heldenzauber) (`SP_CLAUDIBUS_CLAVISTIBOR`)

- **Im Code:** Zauber-ID 79 (`SP_CLAUDIBUS_CLAVISTIBOR`). Gruppe Veränderung, Herkunft Magier. Probe auf KL/FF/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_claudibus`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-80"></a>
### 80. Dunkelheit (Heldenzauber) (`SP_DUNKELHEIT`)

- **Im Code:** Zauber-ID 80 (`SP_DUNKELHEIT`). Gruppe Veränderung, Herkunft Druide. Probe auf KL/KL/FF. AE-Kosten laut Tabelle: 3 AE; bei misslungener Probe 1 AE. Einsatz: nur im Kampf. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Setzt für (Stufe + 3) Stunden den Dunkelheits-Zähler. In Dungeons ist es dann völlig dunkel, auch mit Fackeln, Flim Flam oder Zauberstab.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_dunkelheit`, `Quelle/seg075.c:DNG_lights`
- **Deutung der KI:** Sieht so aus, als ob Dunkelheit in Dungeons vor allem schadet, weil sie jede Lichtquelle auslöscht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-81"></a>
### 81. Weiches erstarre! (Heldenzauber) (`SP_WEICHES_ERSTARRE`)

- **Im Code:** Zauber-ID 81 (`SP_WEICHES_ERSTARRE`). Gruppe Veränderung, Herkunft Magier. Probe auf MU/KL/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_erstarre`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-82"></a>
### 82. Flim Flam Funkel (Heldenzauber) (`SP_FLIM_FLAM_FUNKEL`)

- **Im Code:** Zauber-ID 82 (`SP_FLIM_FLAM_FUNKEL`). Gruppe Veränderung, Herkunft Firnelf. Probe auf KL/KL/FF. AE-Kosten laut Tabelle: 1 AE; bei misslungener Probe 1 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Setzt für (Stufe + 3) Stunden den Flim-Flam-Zähler. In Dungeons ist es dann voll hell, ohne Fackel oder Laterne (außer wenn Dunkelheit wirkt).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_flim_flam`, `Quelle/seg075.c:DNG_lights`
- **Deutung der KI:** Vermutlich ein lang anhaltendes magisches Licht für Dungeons.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-83"></a>
### 83. Hartes schmelze! (Heldenzauber) (`SP_HARTES_SCHMELZE`)

- **Im Code:** Zauber-ID 83 (`SP_HARTES_SCHMELZE`). Gruppe Veränderung, Herkunft Magier. Probe auf MU/KL/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_schmelze`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-84"></a>
### 84. Silentium Silentille (Heldenzauber) (`SP_SILENTIUM_SILENTILLE`)

- **Im Code:** Zauber-ID 84 (`SP_SILENTIUM_SILENTILLE`). Gruppe Veränderung, Herkunft Auelf. Probe auf KL/KL/CH. AE-Kosten laut Tabelle: variabel (in der Tabelle −1, der Zauber legt die Kosten selbst fest); bei misslungener Probe 5–10 AE (zufällig). Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Alle lebenden Helden der aktiven Gruppe bekommen Schleichen +10 für 5 Spielminuten. Kosten 5 AE.
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_silentium`
- **Deutung der KI:** Vermutlich eine kurze Hilfe beim Schleichen für die ganze Gruppe.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="zauber-85"></a>
### 85. Sturmgebrüll besänft'ge dich (Heldenzauber) (`SP_STURMGEBRUELL_BESAENFTGE_DICH`)

- **Im Code:** Zauber-ID 85 (`SP_STURMGEBRUELL_BESAENFTGE_DICH`). Gruppe Veränderung, Herkunft Firnelf. Probe auf KL/CH/KK. AE-Kosten laut Tabelle: 5 AE; bei misslungener Probe 2 AE. Einsatz: nur außerhalb des Kampfes. Ziel: kein Ziel (wirkt auf den Zaubernden oder die Umgebung). Wirkung im Handler: Der Handler ist leer: Der Zauber kostet bei gelungener Probe die vollen AE laut Tabelle und bewirkt nichts (es erscheint keine eigene Meldung).
- **Codestellen:** `Quelle/datseg.c:g_spell_descriptions`, `Quelle/seg098.c:use_spell`, `Quelle/seg098.c:test_spell`, `Quelle/seg101.c:spell_sturmgebr`
- **Deutung der KI:** Sieht so aus, als ob dieser Zauber im Computerspiel nur AE kostet und nichts bewirkt. Vermutlich wurde er aus dem Regelwerk übernommen, aber nie programmiert.
- **Status:** im Code geprüft, im Spiel unbestätigt

---

## 4. Die 7 Stabzauber der Magier

<a id="stab-1"></a>
### 1. Stabzauber

- **Im Code:** Stabzauber 1 (`g_staffspell_descriptions[0]`). Probe beim Aufprägen auf KL/KL/CH mit Zuschlag 1, Kosten 21 AE, die maximale AE sinkt dauerhaft um 1. Aufprägen: Nur Magier, im Magie-Menü unter „Stabzauber“, immer der nächste der Reihe. Die AE müssen reichen. Bei gelungener Probe steigt die Stabzauber-Stufe um 1, die Kosten werden abgezogen, es vergehen 5 Stunden, und 12 Stunden lang regeneriert der Magier im Schlaf nicht. Bei misslungener Probe: halbe Kosten, 2 Stunden. Im Original prüft die Probe die zweite Eigenschaft doppelt und die dritte gar nicht, Fix 190 (Fanpatch-Stufe „Wichtig“) nimmt die dritte. Im Original endet die Reihe erst bei Stufe 7, man kann also weiter (vergeblich) Stabzauber 5–7 versuchen. Fix 189 (Fanpatch-Stufe „Wichtig“) beendet die Reihe nach dem 4. Wirkung: Der 1. Stabzauber selbst wird im Code nirgends abgefragt; er ist nur die Voraussetzung für die folgenden.
- **Codestellen:** `Quelle/datseg.c:g_staffspell_descriptions`, `Quelle/seg098.c:use_magic`, `Quelle/seg049.c:GRP_hero_sleep`
- **Deutung der KI:** Vermutlich nur der Einstieg in die Reihe der Stabzauber.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="stab-2"></a>
### 2. Stabzauber (Licht)

- **Im Code:** Stabzauber 2 (`g_staffspell_descriptions[1]`). Probe beim Aufprägen auf KL/KL/CH mit Zuschlag 0, Kosten 23 AE. Aufprägen: Nur Magier, im Magie-Menü unter „Stabzauber“, immer der nächste der Reihe. Die AE müssen reichen. Bei gelungener Probe steigt die Stabzauber-Stufe um 1, die Kosten werden abgezogen, es vergehen 5 Stunden, und 12 Stunden lang regeneriert der Magier im Schlaf nicht. Bei misslungener Probe: halbe Kosten, 2 Stunden. Im Original prüft die Probe die zweite Eigenschaft doppelt und die dritte gar nicht, Fix 190 (Fanpatch-Stufe „Wichtig“) nimmt die dritte. Im Original endet die Reihe erst bei Stufe 7, man kann also weiter (vergeblich) Stabzauber 5–7 versuchen. Fix 189 (Fanpatch-Stufe „Wichtig“) beendet die Reihe nach dem 4. Wirkung: Ist in der aktiven Gruppe ein handlungsfähiger Magier mit Stabzauber ab Stufe 2, ist es in Dungeons hell wie mit Flim Flam (außer bei Dunkelheit).
- **Codestellen:** `Quelle/datseg.c:g_staffspell_descriptions`, `Quelle/seg098.c:use_magic`, `Quelle/seg049.c:GRP_hero_sleep`, `Quelle/seg075.c:is_staff_lvl2_in_group`, `Quelle/seg075.c:DNG_lights`
- **Deutung der KI:** Vermutlich der praktische Ersatz für Fackeln in Dungeons.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="stab-3"></a>
### 3. Stabzauber (Seil)

- **Im Code:** Stabzauber 3 (`g_staffspell_descriptions[2]`). Probe beim Aufprägen auf KL/CH/CH mit Zuschlag 3, Kosten 19 AE. Aufprägen: Nur Magier, im Magie-Menü unter „Stabzauber“, immer der nächste der Reihe. Die AE müssen reichen. Bei gelungener Probe steigt die Stabzauber-Stufe um 1, die Kosten werden abgezogen, es vergehen 5 Stunden, und 12 Stunden lang regeneriert der Magier im Schlaf nicht. Bei misslungener Probe: halbe Kosten, 2 Stunden. Im Original prüft die Probe die zweite Eigenschaft doppelt und die dritte gar nicht, Fix 190 (Fanpatch-Stufe „Wichtig“) nimmt die dritte. Im Original endet die Reihe erst bei Stufe 7, man kann also weiter (vergeblich) Stabzauber 5–7 versuchen. Fix 189 (Fanpatch-Stufe „Wichtig“) beendet die Reihe nach dem 4. Wirkung: Ein Magier mit Stabzauber ab Stufe 3 ersetzt an Kletterstellen ein Seil (Fallgruben, Totenschiff mit Fix 92 (Fanpatch-Stufe „Wichtig“), Zwingfeste, Kultstätte des Namenlosen, Klamm auf der Strecke Ottarje–Orvil), siehe Seil in der Gegenstandsliste.
- **Codestellen:** `Quelle/datseg.c:g_staffspell_descriptions`, `Quelle/seg098.c:use_magic`, `Quelle/seg049.c:GRP_hero_sleep`, `Quelle/seg075.c:DNG_check_climb_tools`, `Quelle/seg087.c:DNG14_handler`, `Quelle/seg111.c:tevent_060`
- **Deutung der KI:** Vermutlich der Ersatz für ein Seil an Kletterstellen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="stab-4"></a>
### 4. Stabzauber (AE-Ersparnis)

- **Im Code:** Stabzauber 4 (`g_staffspell_descriptions[3]`). Probe beim Aufprägen auf KL/CH/CH mit Zuschlag 4, Kosten 27 AE. Aufprägen: Nur Magier, im Magie-Menü unter „Stabzauber“, immer der nächste der Reihe. Die AE müssen reichen. Bei gelungener Probe steigt die Stabzauber-Stufe um 1, die Kosten werden abgezogen, es vergehen 5 Stunden, und 12 Stunden lang regeneriert der Magier im Schlaf nicht. Bei misslungener Probe: halbe Kosten, 2 Stunden. Im Original prüft die Probe die zweite Eigenschaft doppelt und die dritte gar nicht, Fix 190 (Fanpatch-Stufe „Wichtig“) nimmt die dritte. Im Original endet die Reihe erst bei Stufe 7, man kann also weiter (vergeblich) Stabzauber 5–7 versuchen. Fix 189 (Fanpatch-Stufe „Wichtig“) beendet die Reihe nach dem 4. Wirkung: Ab Stufe 4 kostet jede AE-Ausgabe des Magiers 2 AE weniger (Zauber, auch weitere Stabzauber; nie unter 0). Beim Beruhigen eines Tiers mit Sanftmut im Reiseereignis reichen deshalb mit Fix 212 (Fanpatch-Stufe „Wichtig“) auch 13 statt 15 AE.
- **Codestellen:** `Quelle/datseg.c:g_staffspell_descriptions`, `Quelle/seg098.c:use_magic`, `Quelle/seg049.c:GRP_hero_sleep`, `Quelle/seg002.c:sub_ae_splash`, `Quelle/seg120.c`
- **Deutung der KI:** Sieht so aus, als ob der 4. Stabzauber auf Dauer viel AE spart.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="stab-5"></a>
### 5. Stabzauber (nicht erreichbar)

- **Im Code:** Stabzauber 5 (`g_staffspell_descriptions[4]`). Probe beim Aufprägen auf MU/KL/KL mit Zuschlag 99 (99 = praktisch unmöglich), Kosten 30 AE. Aufprägen: Nur Magier, im Magie-Menü unter „Stabzauber“, immer der nächste der Reihe. Die AE müssen reichen. Bei gelungener Probe steigt die Stabzauber-Stufe um 1, die Kosten werden abgezogen, es vergehen 5 Stunden, und 12 Stunden lang regeneriert der Magier im Schlaf nicht. Bei misslungener Probe: halbe Kosten, 2 Stunden. Im Original prüft die Probe die zweite Eigenschaft doppelt und die dritte gar nicht, Fix 190 (Fanpatch-Stufe „Wichtig“) nimmt die dritte. Im Original endet die Reihe erst bei Stufe 7, man kann also weiter (vergeblich) Stabzauber 5–7 versuchen. Fix 189 (Fanpatch-Stufe „Wichtig“) beendet die Reihe nach dem 4. Wirkung: Mit Zuschlag 99 gelingt die Probe praktisch nie, und eine Wirkung ist im Code nicht vorhanden.
- **Codestellen:** `Quelle/datseg.c:g_staffspell_descriptions`, `Quelle/seg098.c:use_magic`, `Quelle/seg049.c:GRP_hero_sleep`
- **Deutung der KI:** Sieht so aus, als ob die Stabzauber ab dem 5. im Spiel nicht vorgesehen sind.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="stab-6"></a>
### 6. Stabzauber (nicht erreichbar)

- **Im Code:** Stabzauber 6 (`g_staffspell_descriptions[5]`). Probe beim Aufprägen auf MU/CH/CH mit Zuschlag 99 (99 = praktisch unmöglich), Kosten 26 AE. Aufprägen: Nur Magier, im Magie-Menü unter „Stabzauber“, immer der nächste der Reihe. Die AE müssen reichen. Bei gelungener Probe steigt die Stabzauber-Stufe um 1, die Kosten werden abgezogen, es vergehen 5 Stunden, und 12 Stunden lang regeneriert der Magier im Schlaf nicht. Bei misslungener Probe: halbe Kosten, 2 Stunden. Im Original prüft die Probe die zweite Eigenschaft doppelt und die dritte gar nicht, Fix 190 (Fanpatch-Stufe „Wichtig“) nimmt die dritte. Im Original endet die Reihe erst bei Stufe 7, man kann also weiter (vergeblich) Stabzauber 5–7 versuchen. Fix 189 (Fanpatch-Stufe „Wichtig“) beendet die Reihe nach dem 4. Wirkung: Mit Zuschlag 99 gelingt die Probe praktisch nie, und eine Wirkung ist im Code nicht vorhanden.
- **Codestellen:** `Quelle/datseg.c:g_staffspell_descriptions`, `Quelle/seg098.c:use_magic`, `Quelle/seg049.c:GRP_hero_sleep`
- **Deutung der KI:** Sieht so aus, als ob die Stabzauber ab dem 5. im Spiel nicht vorgesehen sind.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="stab-7"></a>
### 7. Stabzauber (nicht erreichbar)

- **Im Code:** Stabzauber 7 (`g_staffspell_descriptions[6]`). Probe beim Aufprägen auf MU/KL/CH mit Zuschlag 99 (99 = praktisch unmöglich), Kosten 21 AE, die maximale AE sinkt dauerhaft um 2. Aufprägen: Nur Magier, im Magie-Menü unter „Stabzauber“, immer der nächste der Reihe. Die AE müssen reichen. Bei gelungener Probe steigt die Stabzauber-Stufe um 1, die Kosten werden abgezogen, es vergehen 5 Stunden, und 12 Stunden lang regeneriert der Magier im Schlaf nicht. Bei misslungener Probe: halbe Kosten, 2 Stunden. Im Original prüft die Probe die zweite Eigenschaft doppelt und die dritte gar nicht, Fix 190 (Fanpatch-Stufe „Wichtig“) nimmt die dritte. Im Original endet die Reihe erst bei Stufe 7, man kann also weiter (vergeblich) Stabzauber 5–7 versuchen. Fix 189 (Fanpatch-Stufe „Wichtig“) beendet die Reihe nach dem 4. Wirkung: Mit Zuschlag 99 gelingt die Probe praktisch nie, und eine Wirkung ist im Code nicht vorhanden.
- **Codestellen:** `Quelle/datseg.c:g_staffspell_descriptions`, `Quelle/seg098.c:use_magic`, `Quelle/seg049.c:GRP_hero_sleep`
- **Deutung der KI:** Sieht so aus, als ob die Stabzauber ab dem 5. im Spiel nicht vorgesehen sind.
- **Status:** im Code geprüft, im Spiel unbestätigt

---

## 5. Die 14 Gegnerzauber

### Wie Gegner zaubern

- **Im Code:**
  - Ein Gegner zaubert nur, wenn er ein Spruchbuch hat, nicht verängstigt ist und noch mindestens 5 AE hat. Ein Zauber kostet ihn 5 Bewegungspunkte. (`Quelle/seg037.c:FIG_enemy_turn`)
  - Auswahl: Das Spiel geht das Spruchbuch der Reihe nach durch und nimmt jeden Fernzauber mit 74 % Wahrscheinlichkeit. Trifft keiner, wird ein zufälliger Spruch des Buchs gewählt. Für Berührungszauber muss das Ziel angrenzen. (`Quelle/seg037.c:FIG_select_mspell`, `Quelle/seg037.c:FIG_get_mspell_target_mode`)
  - Ausführung: Reichen die AE nicht für die Tabellenkosten, unterbleibt der Zauber. Die Probe ist eine Summenprobe auf drei Eigenschaften des Gegners (3W20 + Zuschlag gegen die Summe, ohne Patzer), bei gesetzter MR-Spalte erschwert um die MR des Ziels. Misslingt sie oder wirkt das Rondra-Zauberverbot, kostet es die halben Tabellenkosten (bei variablen 5–10). Sonst setzt der Zauber die Kosten selbst oder es gilt der Tabellenwert. (`Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:MON_test_spell`, `Quelle/seg102.c:MON_test_attrib3`)
  - Ein übergelaufener Gegner (Böser Blick, Skelettarius) setzt nur Sprüche mit Zielart 0 (auf sich) oder 3 ein. Mit Fix 22 versucht er es gar nicht erst, wenn sein Buch keinen solchen Spruch hat (sonst Endlosschleife). (`Quelle/seg037.c:FIG_select_mspell`)
- **Deutung der KI:** Sieht so aus, als ob gegnerische Zauberer bevorzugt Fernzauber wie Fulminictus und Ignifaxius einsetzen.
- **Status:** im Code geprüft, im Spiel unbestätigt

### Spruchbücher und welche Gegner sie haben (`g_mon_spellbooks`, `MONSTER.DAT`)

| Buch | Sprüche | Gegner mit diesem Buch |
|---:|---|---|
| 0 | Verwandlung beenden; Blitz; Eisenrost | Druide (Gegnertyp 18, Stufe 2) |
| 1 | Verwandlung beenden; Blitz; Böser Blick; Horriphobus | Druide (Gegnertyp 19, Stufe 3) |
| 2 | Verwandlung beenden; Blitz; Horriphobus; Bannbaladin; Fulminictus | Gorah (Gegnertyp 70, Stufe 8) |
| 3 | Fulminictus; Paralü; Ignifaxius | Magier (Gegnertyp 46, Stufe 3) |
| 4 | Fulminictus; Ignifaxius; Saft, Kraft, Monstermacht; Armatrutz | Magier (Gegnertyp 63, Stufe 4) |
| 5 | Fulminictus; Ignifaxius; Saft, Kraft, Monstermacht; Plumbumbarum; Blitz | Magier (Gegnertyp 47, Stufe 6) |
| 6 | Fulminictus; Paralü; Blitz | Zauberer (Gegnertyp 48, Stufe 3) |
| 7 | Fulminictus; Paralü; Blitz; Saft, Kraft, Monstermacht; Horriphobus | Zauberer (Gegnertyp 49, Stufe 6) |
| 8 | Balsam; Verwandlung beenden; Eisenrost | Priester (Gegnertyp 57, Stufe 2) |
| 9 | Balsam; Verwandlung beenden; Axxeleratus; Blitz | Priester (Gegnertyp 58, Stufe 3) |
| 10 | Balsam; Paralü; Axxeleratus; Saft, Kraft, Monstermacht; Fulminictus | Hohepriester (Gegnertyp 59, Stufe 6) |

### Alle 14 Gegnerzauber im Detail

<a id="gegner-1"></a>
### Verwandlung beenden (Gegnerzauber Nr. 1) (`MONSTER_SPELL_VERWANDLUNG_BEENDEN`)

- **Im Code:** Gegnerzauber-ID 1 (`MONSTER_SPELL_VERWANDLUNG_BEENDEN`). Probe des Gegners auf MU/KL/CH. AE-Kosten laut Tabelle: 20 AE. Berührungszauber (Ziel muss angrenzen). Zielart (Spalte mode) 1: auf einen verbündeten Gegner (oder sich selbst). Steht in den Spruchbüchern 0, 1, 2, 8, 9. Wirkung im Handler: Ziel: ein verbündeter Gegner. Ist er versteinert oder in einen Pilz verwandelt, kostet der Zauber 5–50 AE (1W10 × 5) und hebt das auf. Reichen die AE nicht, zahlt der Zaubernde alle seine AE, ohne Wirkung. Sonst kostet er 2 AE ohne Wirkung.
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_verwandlung`
- **Deutung der KI:** Vermutlich der Weg, mit dem gegnerische Zauberer versteinerte Verbündete retten.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-2"></a>
### Bannbaladin (Gegnerzauber Nr. 2) (`MONSTER_SPELL_BANNBALADIN`)

- **Im Code:** Gegnerzauber-ID 2 (`MONSTER_SPELL_BANNBALADIN`). Probe des Gegners auf IN/CH/CH, erschwert um die MR des Ziels. AE-Kosten laut Tabelle: 8 AE. Berührungszauber (Ziel muss angrenzen). Zielart (Spalte mode) 2: auf einen Helden; ein übergelaufener Gegner setzt ihn nicht ein. Steht in den Spruchbüchern 2. Wirkung im Handler: Ziel: ein Held. Er gilt als besänftigt und greift nicht mehr an.
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_bannbaladin`
- **Deutung der KI:** Vermutlich nimmt der Zauber einen Helden aus dem Kampf.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-3"></a>
### Böser Blick (Gegnerzauber Nr. 3) (`MONSTER_SPELL_BOESER_BLICK`)

- **Im Code:** Gegnerzauber-ID 3 (`MONSTER_SPELL_BOESER_BLICK`). Probe des Gegners auf MU/CH/CH, erschwert um die MR des Ziels. AE-Kosten laut Tabelle: 8 AE. Berührungszauber (Ziel muss angrenzen). Zielart (Spalte mode) 2: auf einen Helden; ein übergelaufener Gegner setzt ihn nicht ein. Steht in den Spruchbüchern 1. Wirkung im Handler: Ziel: ein Held. Er wird abtrünnig und kämpft gegen die eigene Gruppe.
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_boeser_blick`
- **Deutung der KI:** Vermutlich der gefährlichste Gegnerzauber: Ein Held kämpft gegen die eigene Gruppe.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-4"></a>
### Horriphobus (Gegnerzauber Nr. 4) (`MONSTER_SPELL_HORRIPHOBUS`)

- **Im Code:** Gegnerzauber-ID 4 (`MONSTER_SPELL_HORRIPHOBUS`). Probe des Gegners auf MU/IN/CH, erschwert um die MR des Ziels. AE-Kosten laut Tabelle: 7 AE. Berührungszauber (Ziel muss angrenzen). Zielart (Spalte mode) 2: auf einen Helden; ein übergelaufener Gegner setzt ihn nicht ein. Steht in den Spruchbüchern 1, 2, 7. Wirkung im Handler: Ziel: ein Held. Er ist verängstigt und flieht (eine Beherrschung durch Böser Blick endet dabei).
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_horriphobus`
- **Deutung der KI:** Vermutlich nimmt der Zauber einen Helden aus dem Kampf.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-5"></a>
### Axxeleratus (Gegnerzauber Nr. 5) (`MONSTER_SPELL_AXXELERATUS`)

- **Im Code:** Gegnerzauber-ID 5 (`MONSTER_SPELL_AXXELERATUS`). Probe des Gegners auf KL/GE/KK. AE-Kosten laut Tabelle: 7 AE. Berührungszauber (Ziel muss angrenzen). Zielart (Spalte mode) 1: auf einen verbündeten Gegner (oder sich selbst). Steht in den Spruchbüchern 9, 10. Wirkung im Handler: Ziel: ein verbündeter Gegner. Er bekommt für den Rest des Kampfes einen Angriff mehr, AT +1, PA +1, und seine Bewegungspunkte werden verdoppelt.
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_axxeleratus`
- **Deutung der KI:** Vermutlich macht der Zauber einen Verbündeten deutlich gefährlicher.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-6"></a>
### Balsam (Gegnerzauber Nr. 6) (`MONSTER_SPELL_BALSAM`)

- **Im Code:** Gegnerzauber-ID 6 (`MONSTER_SPELL_BALSAM`). Probe des Gegners auf KL/IN/CH. AE-Kosten laut Tabelle: variabel (in der Tabelle −1). Berührungszauber (Ziel muss angrenzen). Zielart (Spalte mode) 1: auf einen verbündeten Gegner (oder sich selbst). Steht in den Spruchbüchern 8, 9, 10. Wirkung im Handler: Ziel: ein verbündeter Gegner. Im Original wird nur geheilt, wenn die Hälfte der fehlenden LE zwischen 1 und 6 liegt: Dann gibt es 7 LE für 7 AE (bei weniger AE entsprechend weniger). Fehlt mehr, heilt der Zauber nichts und kostet nichts. Mit Fix 36 (Fanpatch-Stufe „Wichtig“): Fehlen 16 oder mehr LE, wird die Hälfte geheilt, sonst bis zu 7 LE (höchstens so viel, wie fehlt). Gedeckelt durch die AE, Kosten gleich der Heilung.
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_balsam`
- **Deutung der KI:** Sieht so aus, als ob gegnerische Heiler im Original kaum heilen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-7"></a>
### Blitz (Gegnerzauber Nr. 7) (`MONSTER_SPELL_BLITZ`)

- **Im Code:** Gegnerzauber-ID 7 (`MONSTER_SPELL_BLITZ`). Probe des Gegners auf KL/IN/GE, erschwert um die MR des Ziels. AE-Kosten laut Tabelle: 5 AE. Fernzauber. Zielart (Spalte mode) 3: auf einen Helden; ein übergelaufener Gegner zielt damit auf seine früheren Verbündeten. Steht in den Spruchbüchern 0, 1, 2, 5, 6, 7, 9. Wirkung im Handler: Ziel: ein Held oder Gegner. Geblendet für 3 Kampfphasen (wie der Heldenzauber Blitz dich find).
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_blitz`
- **Deutung der KI:** Vermutlich setzt der Zauber einen Helden ein paar Phasen außer Gefecht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-8"></a>
### Eisenrost (Gegnerzauber Nr. 8) (`MONSTER_SPELL_EISENROST`)

- **Im Code:** Gegnerzauber-ID 8 (`MONSTER_SPELL_EISENROST`). Probe des Gegners auf KL/CH/GE. AE-Kosten laut Tabelle: 5 AE. Fernzauber. Zielart (Spalte mode) 2: auf einen Helden; ein übergelaufener Gegner setzt ihn nicht ein. Steht in den Spruchbüchern 0, 8. Wirkung im Handler: Ziel Held: Ohne Waffe in der rechten Hand kostet er 2 AE ohne Wirkung. Eine Waffe mit Bruchfaktor über 0 zerbricht, sonst misslingt der Zauber. Mit Fix 390 (Fanpatch-Stufe „nur Alle“) zerbrechen unzerbrechliche Waffen nicht. Ziel Gegner: dessen Waffe zerbricht (halber Schaden).
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_eisenrost`
- **Deutung der KI:** Vermutlich zerstört der Zauber Waffen der Helden.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-9"></a>
### Fulminictus (Gegnerzauber Nr. 9) (`MONSTER_SPELL_FULMINICTUS`)

- **Im Code:** Gegnerzauber-ID 9 (`MONSTER_SPELL_FULMINICTUS`). Probe des Gegners auf KL/GE/KK, erschwert um die MR des Ziels. AE-Kosten laut Tabelle: variabel (in der Tabelle −1). Fernzauber. Zielart (Spalte mode) 3: auf einen Helden; ein übergelaufener Gegner zielt damit auf seine früheren Verbündeten. Steht in den Spruchbüchern 2, 3, 4, 5, 6, 7, 10. Wirkung im Handler: Ziel: ein Held. Schaden 3W6 + Stufe des Gegners, höchstens so viel, wie er AE hat, direkt von der LE ohne Rüstungsschutz. Kosten gleich dem Schaden. Mit Fix 192 (Fanpatch-Stufe „Wichtig“) fängt ein Gardianum-Schutzschirm der Helden den Schaden ab, bis der Schirm verbraucht ist. Der Gegner zahlt die AE trotzdem.
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_fulminictus`, `Quelle/seg102.c:MON_do_spell_damage`, `Quelle/seg102.c:fix_hero_spell_protect`
- **Deutung der KI:** Vermutlich der häufigste Schadenszauber der Gegner.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-10"></a>
### Ignifaxius (Gegnerzauber Nr. 10) (`MONSTER_SPELL_IGNIFAXIUS`)

- **Im Code:** Gegnerzauber-ID 10 (`MONSTER_SPELL_IGNIFAXIUS`). Probe des Gegners auf KL/GE/FF, erschwert um die MR des Ziels. AE-Kosten laut Tabelle: variabel (in der Tabelle −1). Fernzauber. Zielart (Spalte mode) 3: auf einen Helden; ein übergelaufener Gegner zielt damit auf seine früheren Verbündeten. Steht in den Spruchbüchern 3, 4, 5. Wirkung im Handler: Ziel: ein Held. Schaden Stufe × W6 (Stufe des Gegners), höchstens so viel, wie er AE hat, direkt von der LE. Danach gehen Schaden/10 Punkte RS der Körperrüstung verloren, und AT und PA sinken für 1 Stunde um Stufe/2. Kosten gleich dem Schaden. Mit Fix 213 (Fanpatch-Stufe „Wichtig“) halbiert Feuerbann den Schaden. Mit Fix 192 (Fanpatch-Stufe „Wichtig“) fängt ein Gardianum-Schirm ab, was übrig bleibt. Wird so aller Schaden abgefangen, entfallen auch RS-Verlust und AT/PA-Malus.
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_ignifaxius`, `Quelle/seg102.c:fix_hero_spell_protect`
- **Deutung der KI:** Vermutlich der stärkste Schadenszauber der Gegner.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-11"></a>
### Plumbumbarum (Gegnerzauber Nr. 11) (`MONSTER_SPELL_PLUMBUMBARUM`)

- **Im Code:** Gegnerzauber-ID 11 (`MONSTER_SPELL_PLUMBUMBARUM`). Probe des Gegners auf CH/GE/KK, erschwert um die MR des Ziels. AE-Kosten laut Tabelle: 5 AE. Fernzauber. Zielart (Spalte mode) 3: auf einen Helden; ein übergelaufener Gegner zielt damit auf seine früheren Verbündeten. Steht in den Spruchbüchern 5. Wirkung im Handler: Ziel: ein Held. AT −3 für 1 Stunde. (Der Heldenzauber wirkt dagegen nur 5 Kampfrunden.)
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_plumbumbarum`
- **Deutung der KI:** Vermutlich ein lang wirkender AT-Malus für einen Helden.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-12"></a>
### Saft, Kraft, Monstermacht (Gegnerzauber Nr. 12) (`MONSTER_SPELL_SAFT_KRAFT_MONSTERMACHT`)

- **Im Code:** Gegnerzauber-ID 12 (`MONSTER_SPELL_SAFT_KRAFT_MONSTERMACHT`). Probe des Gegners auf MU/IN/CH. AE-Kosten laut Tabelle: variabel (in der Tabelle −1). Berührungszauber (Ziel muss angrenzen). Zielart (Spalte mode) 1: auf einen verbündeten Gegner (oder sich selbst). Steht in den Spruchbüchern 4, 5, 7, 10. Wirkung im Handler: Ziel: ein verbündeter Gegner. Für den Rest des Kampfes AT +5, PA −5, 5 Schadenspunkte mehr pro Treffer. Kosten W20 AE.
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_saft_kraft`
- **Deutung der KI:** Vermutlich macht der Zauber einen Verbündeten stärker im Angriff.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-13"></a>
### Armatrutz (Gegnerzauber Nr. 13) (`MONSTER_SPELL_ARMATRUTZ`)

- **Im Code:** Gegnerzauber-ID 13 (`MONSTER_SPELL_ARMATRUTZ`). Probe des Gegners auf IN/GE/KK. AE-Kosten laut Tabelle: variabel (in der Tabelle −1). Berührungszauber (Ziel muss angrenzen). Zielart (Spalte mode) 0: auf sich selbst. Steht in den Spruchbüchern 4. Wirkung im Handler: Nur auf sich selbst: RS steigt um einen zufälligen Wert zwischen 1 und dem Höchstmaß, Kosten Wert × Wert AE. Das Höchstmaß ist im Original die kleinste Zahl, deren Quadrat die AE erreicht (kann mehr kosten, als er hat). Mit Fix 207 (Fanpatch-Stufe „Wichtig“) ist es die größte Zahl, deren Quadrat höchstens den AE entspricht.
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_armatrutz`
- **Deutung der KI:** Vermutlich ein Selbstschutz gegnerischer Zauberer.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="gegner-14"></a>
### Paralü (Gegnerzauber Nr. 14) (`MONSTER_SPELL_PARALUE`)

- **Im Code:** Gegnerzauber-ID 14 (`MONSTER_SPELL_PARALUE`). Probe des Gegners auf IN/CH/KK, erschwert um die MR des Ziels. AE-Kosten laut Tabelle: 13 AE. Berührungszauber (Ziel muss angrenzen). Zielart (Spalte mode) 3: auf einen Helden; ein übergelaufener Gegner zielt damit auf seine früheren Verbündeten. Steht in den Spruchbüchern 3, 6, 7, 10. Wirkung im Handler: Ziel: ein Held (bei übergelaufenen Gegnern ein Gegner). Er ist versteinert.
- **Codestellen:** `Quelle/datseg.c:g_mon_spell_descriptions`, `Quelle/datseg.c:g_mon_spellbooks`, `Quelle/seg102.c:MON_cast_spell`, `Quelle/seg102.c:mspell_paralue`
- **Deutung der KI:** Vermutlich nimmt der Zauber einen Helden vollständig aus dem Kampf.
- **Status:** im Code geprüft, im Spiel unbestätigt

