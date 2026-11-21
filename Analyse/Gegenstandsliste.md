# Gegenstandsliste – Die Schicksalsklinge (aus dem Quelltext)

> **Hinweis:** Diese Liste wurde von einer KI aus dem rekonstruierten Quelltext erstellt. „Im Code“ ist das, was das Programm tatsächlich rechnet. „Deutung der KI“ ist die Auslegung der KI und kann falsch sein. Wer im Spiel etwas anderes beobachtet oder es genauer weiß: bitte im Crystals-DSA-Forum melden.

- **Grundlage:** Quelltext `Quelle/` der Fanpatch-Fassung 1.5i, Werte aus `ITEMS.DAT` und den Tabellen im Datensegment. „Im Original“ heißt: ohne Fanpatch. „Mit Fix N“ heißt: im Fanpatch behoben, mit Angabe, ob der Fix in der Stufe „Wichtig“ oder nur in „Alle“ enthalten ist.
- **Nicht im Code:** Kampfbeute aus `FIGHT.LST` und die Orte der Händler stehen in den Spieldaten, nicht im Programm, und fehlen deshalb hier.
- **Prüfstand:** Jede Angabe unter „Im Code“ wurde gegen die genannte Codestelle nachgelesen („Status: im Code geprüft“). „Im Spiel bestätigt“ setzen wir erst, wenn Spieler es im Spiel nachgeprüft haben.

## Inhaltsverzeichnis

1. [Gegenstände ohne jede Spielwirkung im Code](#1-gegenstände-ohne-jede-spielwirkung-im-code)
2. [Übersichtstabelle aller 254 Einträge](#2-übersichtstabelle-aller-254-einträge-itemsdat)
3. [Alle Einträge im Detail](#3-alle-einträge-im-detail)

---

## 1. Gegenstände ohne jede Spielwirkung im Code

- **Im Code:** Bei diesen Gegenständen fragt der Quelltext weder die Gegenstandsnummer noch eine Eigenschaft ab, über die sie wirken könnten (keine Waffe, keine Rüstung, nicht benutzbar, keine Nahrung, kein Kraut). Sie haben nur Gewicht und Preis:
  24. Wurfhaken, 28. Angelhaken, 29. Schriftrolle, 34. Messingspiegel, 36. Schreibzeug, 37. Harfe, 38. Trinkhorn, 39. Silberschmuck, 46. Flöte, 72. Decke, 86. Schleifstein, 87. Essbesteck, 88. Essgeschirr, 94. Praiosamulett, 95. Laute, 162. Amulett (gegen Ignifaxius), 173. Amulett (Feuerbann), 177. Rote Mondscheibe, 188. Amulett (gewöhnlich), 191. Dokument (Fassung 2), 194. Schlüssel (gelb), 208. Schlüssel (silbern), 210. Eisenschlüssel (Fassung 2), 211. Roter Schlüssel.
- **Deutung der KI:** Sieht so aus, als ob viele Ausrüstungsgegenstände aus dem Pen-&-Paper-Regelwerk übernommen wurden, ohne dass das Computerspiel ihnen eine Aufgabe gibt. Wer im Spiel doch eine Wirkung beobachtet: bitte melden.
- **Status:** im Code geprüft, im Spiel unbestätigt

---

## 2. Übersichtstabelle aller 254 Einträge (`ITEMS.DAT`)

| ID | Name | Typ | Gewicht (Unzen) | Grundpreis (Heller) | Kampf-/Rüstungswerte | Wirkung im Code |
|---:|---|---|---:|---:|---|---|
| 1 | [Schwert](#item-1) | Waffe | 80 | 850 | TP 1W6+4, KK 14, BF 2, AT +0/PA +0 | nur Tabellenwerte |
| 2 | [Knüppel](#item-2) | Waffe | 60 | 50 | TP 1W6+1, KK 14, BF 6, AT -1/PA -3 | nur Tabellenwerte |
| 3 | [Säbel](#item-3) | Waffe | 60 | 600 | TP 1W6+3, KK 15, BF 2, AT +0/PA +0 | ja (siehe Eintrag) |
| 4 | [Messer](#item-4) | Waffe | 10 | 50 | TP 1W6, KK 16, BF 4, AT -3/PA -4 | nur Tabellenwerte |
| 5 | [Speer](#item-5) | Waffe | 80 | 300 | TP 1W6+3, KK 16, BF 5, AT +0/PA -3 | ja (siehe Eintrag) |
| 6 | [Kurzschwert](#item-6) | Waffe | 40 | 450 | TP 1W6+2, KK 15, BF 1, AT +0/PA -1 | nur Tabellenwerte |
| 7 | [Schild](#item-7) | Rüstung | 140 | 250 | RS 1, BE 0 (linke Hand (Schild)) | nur Tabellenwerte |
| 8 | [Kriegsbeil (Sonderfassung)](#item-8) | Waffe | 120 | 450 | TP 1W6+4, KK 14, BF 5, AT +0/PA -3 | nur Tabellenwerte |
| 9 | [Kurzbogen](#item-9) | Waffe | 20 | 450 | TP 1W6+3, BF 0, AT +0/PA +0 | ja (siehe Eintrag) |
| 10 | [Pfeil](#item-10) | Waffe, stapelbar | 4 | 5 | TP 0W6, BF 0, AT +0/PA +0 | ja (siehe Eintrag) |
| 11 | [Streitaxt](#item-11) | Waffe | 150 | 1100 | TP 2W6+4, KK 14, BF 3, AT -1/PA -4 | nur Tabellenwerte |
| 12 | [Armbrust](#item-12) | Waffe | 200 | 1230 | TP 2W6+2, BF 0, AT +0/PA +0 | ja (siehe Eintrag) |
| 13 | [Bolzen](#item-13) | Waffe, stapelbar | 5 | 8 | TP 0W6, BF 0, AT +0/PA +0 | ja (siehe Eintrag) |
| 14 | [Dolch](#item-14) | Waffe | 20 | 200 | TP 1W6+1, KK 15, BF 3, AT -2/PA -3 | ja (siehe Eintrag) |
| 15 | [Eisenschild](#item-15) | Rüstung | 200 | 450 | RS 2, BE 1 (linke Hand (Schild)) | nur Tabellenwerte |
| 16 | [Wurfbeil](#item-16) | Waffe | 120 | 350 | TP 1W6+3, BF 4, AT -1/PA -3 | ja (siehe Eintrag) |
| 17 | [Wurfstern](#item-17) | Waffe, stapelbar | 15 | 250 | TP 1W6+1, BF 0, AT +0/PA +0 | ja (siehe Eintrag) |
| 18 | [Zweihänder](#item-18) | Waffe | 160 | 1350 | TP 2W6+4, KK 14, BF 3, AT -2/PA -3 | nur Tabellenwerte |
| 19 | [Langbogen](#item-19) | Waffe | 30 | 500 | TP 1W6+4, BF 0, AT +0/PA +0 | ja (siehe Eintrag) |
| 20 | [Morgenstern](#item-20) | Waffe | 120 | 500 | TP 1W6+5, KK 15, BF 5, AT -1/PA -3 | nur Tabellenwerte |
| 21 | [Vulkanglasdolch](#item-21) | Waffe, magisch | 30 | 500 | TP 1W6, KK 16, BF 6, AT -2/PA -3 | nur Tabellenwerte |
| 22 | [Fackel (brennend)](#item-22) | – | 20 | 5 | – | ja (siehe Eintrag) |
| 23 | [Bier](#item-23) | Nahrung | 40 | 1 | – | ja (siehe Eintrag) |
| 24 | [Wurfhaken](#item-24) | – | 30 | 100 | – | **ohne Wirkung** |
| 25 | [Laterne (nicht brennend)](#item-25) | benutzbar | 15 | 30 | – | ja (siehe Eintrag) |
| 26 | [Brecheisen](#item-26) | – | 90 | 100 | – | ja (siehe Eintrag) |
| 27 | [Hammer](#item-27) | – | 20 | 60 | – | ja (siehe Eintrag) |
| 28 | [Angelhaken](#item-28) | – | 1 | 2 | – | **ohne Wirkung** |
| 29 | [Schriftrolle](#item-29) | – | 30 | 400 | – | **ohne Wirkung** |
| 30 | [Wasserschlauch](#item-30) | Nahrung | 40 | 20 | – | ja (siehe Eintrag) |
| 31 | [Glasflasche](#item-31) | – | 10 | 120 | – | ja (siehe Eintrag) |
| 32 | [Strickleiter](#item-32) | – | 70 | 120 | – | ja (siehe Eintrag) |
| 33 | [Wurfaxt](#item-33) | Waffe | 120 | 350 | TP 1W6+3, BF 4, AT -1/PA -4 | ja (siehe Eintrag) |
| 34 | [Messingspiegel](#item-34) | – | 20 | 120 | – | **ohne Wirkung** |
| 35 | [Dietriche](#item-35) | – | 15 | 1200 | – | ja (siehe Eintrag) |
| 36 | [Schreibzeug](#item-36) | – | 10 | 150 | – | **ohne Wirkung** |
| 37 | [Harfe](#item-37) | – | 40 | 1000 | – | **ohne Wirkung** |
| 38 | [Trinkhorn](#item-38) | – | 20 | 10 | – | **ohne Wirkung** |
| 39 | [Silberschmuck](#item-39) | – | 10 | 50 | – | **ohne Wirkung** |
| 40 | [Kletterhaken](#item-40) | stapelbar | 5 | 5 | – | ja (siehe Eintrag) |
| 41 | [Öl](#item-41) | – | 10 | 100 | – | ja (siehe Eintrag) |
| 42 | [Bronzeflasche](#item-42) | – | 50 | 80 | – | ja (siehe Eintrag) |
| 43 | [Eisenhelm](#item-43) | Rüstung | 180 | 800 | RS 3, BE 2 (Kopf) | nur Tabellenwerte |
| 44 | [Pike](#item-44) | Waffe | 150 | 500 | TP 1W6+3, KK 19, BF 7, AT -1/PA -4 | nur Tabellenwerte |
| 45 | [Proviantpaket](#item-45) | Nahrung, stapelbar | 20 | 30 | – | ja (siehe Eintrag) |
| 46 | [Flöte](#item-46) | – | 8 | 20 | – | **ohne Wirkung** |
| 47 | [Alchimieset](#item-47) | – | 100 | 2000 | – | ja (siehe Eintrag) |
| 48 | [Hemd](#item-48) | Rüstung | 30 | 20 | RS 1, BE 0 (Körper) | ja (siehe Eintrag) |
| 49 | [Hose](#item-49) | Rüstung | 30 | 30 | RS 0, BE 0 (Beine) | nur Tabellenwerte |
| 50 | [Schuhe](#item-50) | Rüstung | 40 | 10 | RS 0, BE 0 (Füße) | nur Tabellenwerte |
| 51 | [Stiefel](#item-51) | Rüstung | 75 | 90 | RS 0, BE 0 (Füße) | nur Tabellenwerte |
| 52 | [Schneeschuhe](#item-52) | Rüstung | 40 | 50 | RS 0, BE 0 (Füße) | nur Tabellenwerte |
| 53 | [Lederharnisch](#item-53) | Rüstung | 180 | 800 | RS 3, BE 3 (Körper) | ja (siehe Eintrag) |
| 54 | [Schuppenpanzer](#item-54) | Rüstung | 480 | 10000 | RS 5, BE 4 (Körper) | nur Tabellenwerte |
| 55 | [Shurinknollengift](#item-55) | benutzbar, Kraut/Trank | 10 | 7000 | – | ja (siehe Eintrag) |
| 56 | [Araxgift](#item-56) | benutzbar, Kraut/Trank | 10 | 3500 | – | ja (siehe Eintrag) |
| 57 | [Angstgift](#item-57) | benutzbar, Kraut/Trank | 10 | 2500 | – | ja (siehe Eintrag) |
| 58 | [Schlafgift](#item-58) | benutzbar, Kraut/Trank | 10 | 3500 | – | ja (siehe Eintrag) |
| 59 | [Goldleim](#item-59) | benutzbar, Kraut/Trank | 10 | 3000 | – | ja (siehe Eintrag) |
| 60 | [Vierblättrige Einbeere](#item-60) | benutzbar, Kraut/Trank, stapelbar | 1 | 70 | – | ja (siehe Eintrag) |
| 61 | [Wirselkraut](#item-61) | benutzbar, Kraut/Trank, stapelbar | 1 | 700 | – | ja (siehe Eintrag) |
| 62 | [Eitriger Krötenschemel](#item-62) | benutzbar, Kraut/Trank, stapelbar | 1 | 1500 | – | ja (siehe Eintrag) |
| 63 | [Gulmond Blatt](#item-63) | benutzbar, Kraut/Trank, stapelbar | 1 | 600 | – | ja (siehe Eintrag) |
| 64 | [Tarnele](#item-64) | benutzbar, Kraut/Trank, stapelbar | 1 | 50 | – | ja (siehe Eintrag) |
| 65 | [Fackel (nicht brennend)](#item-65) | benutzbar | 20 | 5 | – | ja (siehe Eintrag) |
| 66 | [Streitkolben](#item-66) | Waffe | 110 | 500 | TP 1W6+4, KK 13, BF 1, AT +0/PA -2 | nur Tabellenwerte |
| 67 | [Degen](#item-67) | Waffe | 35 | 550 | TP 1W6+3, KK 16, BF 3, AT +0/PA -1 | nur Tabellenwerte |
| 68 | [Florett](#item-68) | Waffe | 30 | 500 | TP 1W6+3, KK 16, BF 3, AT +0/PA -1 | nur Tabellenwerte |
| 69 | [Kampfstab](#item-69) | Waffe | 70 | 150 | TP 1W6+1, KK 15, BF 5, AT +0/PA -1 | ja (siehe Eintrag) |
| 70 | [Kristallkugel](#item-70) | – | 50 | 2000 | – | ja (siehe Eintrag) |
| 71 | [Peitsche](#item-71) | Waffe | 60 | 300 | TP 1W6, KK 19, BF 2, AT +0/PA -6 | nur Tabellenwerte |
| 72 | [Decke](#item-72) | – | 60 | 20 | – | **ohne Wirkung** |
| 73 | [Schaufel](#item-73) | – | 80 | 100 | – | ja (siehe Eintrag) |
| 74 | [Goldschmuck](#item-74) | – | 15 | 2000 | – | ja (siehe Eintrag) |
| 75 | [Robe (grün, Fassung 1)](#item-75) | Rüstung | 50 | 50 | RS 0, BE 0 (Körper) | ja (siehe Eintrag) |
| 76 | [Robe (rot)](#item-76) | Rüstung | 200 | 2000 | RS 0, BE 0 (Körper) | nur Tabellenwerte |
| 77 | [Topfhelm](#item-77) | Rüstung | 100 | 500 | RS 2, BE 1 (Kopf) | nur Tabellenwerte |
| 78 | [Lederhelm](#item-78) | Rüstung | 90 | 400 | RS 1, BE 0 (Kopf) | nur Tabellenwerte |
| 79 | [Waffenrock](#item-79) | Rüstung | 120 | 500 | RS 2, BE 2 (Körper) | nur Tabellenwerte |
| 80 | [Kettenhemd](#item-80) | Rüstung | 320 | 2000 | RS 4, BE 4 (Körper) | nur Tabellenwerte |
| 81 | [Kroetenhaut](#item-81) | Rüstung | 150 | 600 | RS 3, BE 3 (Körper) | nur Tabellenwerte |
| 82 | [Plattenzeug](#item-82) | Rüstung | 100 | 600 | RS 2, BE 1 (Arme) | nur Tabellenwerte |
| 83 | [Kettenzeug](#item-83) | Rüstung | 80 | 500 | RS 1, BE 1 (Arme) | nur Tabellenwerte |
| 84 | [Lederzeug](#item-84) | Rüstung | 70 | 400 | RS 1, BE 0 (Arme) | nur Tabellenwerte |
| 85 | [Zunderkästchen](#item-85) | benutzbar | 25 | 80 | – | ja (siehe Eintrag) |
| 86 | [Schleifstein](#item-86) | – | 25 | 90 | – | **ohne Wirkung** |
| 87 | [Essbesteck](#item-87) | – | 10 | 50 | – | **ohne Wirkung** |
| 88 | [Essgeschirr](#item-88) | – | 100 | 600 | – | **ohne Wirkung** |
| 89 | [Lakritze](#item-89) | Nahrung | 10 | 3 | – | ja (siehe Eintrag) |
| 90 | [Bonbons](#item-90) | Nahrung | 10 | 10 | – | ja (siehe Eintrag) |
| 91 | [Weinflasche](#item-91) | Nahrung | 40 | 5 | – | ja (siehe Eintrag) |
| 92 | [Schnapsflasche](#item-92) | Nahrung | 40 | 10 | – | ja (siehe Eintrag) |
| 93 | [Hacke](#item-93) | – | 200 | 200 | – | ja (siehe Eintrag) |
| 94 | [Praiosamulett](#item-94) | – | 10 | 1500 | – | **ohne Wirkung** |
| 95 | [Laute](#item-95) | – | 80 | 1500 | – | **ohne Wirkung** |
| 96 | [Wintermantel](#item-96) | Rüstung | 120 | 500 | RS 1, BE 1 (Körper) | nur Tabellenwerte |
| 97 | [Netz](#item-97) | – | 35 | 100 | – | ja (siehe Eintrag) |
| 98 | [Wurfmesser](#item-98) | Waffe | 10 | 100 | TP 1W6, BF 4, AT -3/PA -4 | ja (siehe Eintrag) |
| 99 | [Sichel](#item-99) | Waffe | 30 | 250 | TP 1W6+2, KK 17, BF 5, AT -3/PA -4 | nur Tabellenwerte |
| 100 | [Sense](#item-100) | Waffe | 100 | 350 | TP 1W6+3, KK 17, BF 6, AT -3/PA -4 | nur Tabellenwerte |
| 101 | [Kriegshammer](#item-101) | Waffe | 150 | 1000 | TP 2W6+3, KK 15, BF 2, AT -2/PA -4 | nur Tabellenwerte |
| 102 | [Dreizack](#item-102) | Waffe | 90 | 450 | TP 1W6+3, KK 15, BF 3, AT +0/PA -3 | ja (siehe Eintrag) |
| 103 | [Hellebarde](#item-103) | Waffe | 150 | 750 | TP 1W6+4, KK 15, BF 5, AT -1/PA -3 | nur Tabellenwerte |
| 104 | [Dreschflegel](#item-104) | Waffe | 100 | 150 | TP 1W6+2, KK 13, BF 6, AT -2/PA -3 | nur Tabellenwerte |
| 105 | [Zweililien](#item-105) | Waffe | 80 | 950 | TP 1W6+3, KK 18, BF 4, AT -1/PA -1 | nur Tabellenwerte |
| 106 | [Ochsenherde](#item-106) | Waffe | 240 | 1100 | TP 3W6+3, KK 17, BF 4, AT -3/PA -4 | nur Tabellenwerte |
| 107 | [Basilikenzunge](#item-107) | Waffe | 25 | 250 | TP 1W6+1, KK 16, BF 4, AT -2/PA -3 | nur Tabellenwerte |
| 108 | [Ogerfänger](#item-108) | Waffe | 30 | 950 | TP 1W6+2, KK 15, BF 4, AT -2/PA -3 | nur Tabellenwerte |
| 109 | [Mengbilar](#item-109) | Waffe | 20 | 1100 | TP 1W6+1, KK 16, BF 7, AT -3/PA -4 | ja (siehe Eintrag) |
| 110 | [Schwerer Dolch](#item-110) | Waffe | 30 | 300 | TP 1W6+2, KK 15, BF 2, AT -1/PA -2 | nur Tabellenwerte |
| 111 | [Rondrakamm](#item-111) | Waffe | 150 | 1400 | TP 2W6+2, KK 15, BF 3, AT -2/PA -2 | nur Tabellenwerte |
| 112 | [Entermesser](#item-112) | Waffe | 70 | 450 | TP 1W6+3, KK 15, BF 2, AT +0/PA -1 | nur Tabellenwerte |
| 113 | [Bastardschwert](#item-113) | Waffe | 140 | 1100 | TP 1W6+5, KK 14, BF 2, AT -1/PA -2 | nur Tabellenwerte |
| 114 | [Tuzakmesser](#item-114) | Waffe | 135 | 2000 | TP 1W6+6, KK 15, BF 1, AT -2/PA -2 | nur Tabellenwerte |
| 115 | [Rabenschnabel](#item-115) | Waffe | 90 | 1100 | TP 1W6+4, KK 16, BF 3, AT -1/PA -3 | nur Tabellenwerte |
| 116 | [Brabakbengel](#item-116) | Waffe | 120 | 800 | TP 1W6+5, KK 14, BF 1, AT -1/PA -2 | nur Tabellenwerte |
| 117 | [Rapier](#item-117) | Waffe | 35 | 600 | TP 1W6+3, KK 16, BF 4, AT +0/PA -1 | nur Tabellenwerte |
| 118 | [Khunchomer](#item-118) | Waffe | 70 | 700 | TP 1W6+4, KK 14, BF 2, AT +0/PA -1 | nur Tabellenwerte |
| 119 | [Doppelkhunchomer](#item-119) | Waffe | 150 | 1250 | TP 1W6+6, KK 15, BF 3, AT -1/PA -3 | nur Tabellenwerte |
| 120 | [Schleuder](#item-120) | Waffe | 10 | 150 | TP 1W6+2, BF 0, AT +0/PA +0 | ja (siehe Eintrag) |
| 121 | [Seil](#item-121) | – | 55 | 70 | – | ja (siehe Eintrag) |
| 122 | [Shurinknolle](#item-122) | benutzbar, Kraut/Trank, stapelbar | 1 | 800 | – | ja (siehe Eintrag) |
| 123 | [Belmartblatt](#item-123) | benutzbar, Kraut/Trank, stapelbar | 1 | 1200 | – | ja (siehe Eintrag) |
| 124 | [Donfstengel](#item-124) | benutzbar, Kraut/Trank, stapelbar | 1 | 2000 | – | ja (siehe Eintrag) |
| 125 | [Menchalkaktus](#item-125) | benutzbar, Kraut/Trank, stapelbar | 1 | 2000 | – | ja (siehe Eintrag) |
| 126 | [Alraune](#item-126) | benutzbar, Kraut/Trank, stapelbar | 1 | 80 | – | ja (siehe Eintrag) |
| 127 | [Atmonblüte](#item-127) | benutzbar, Kraut/Trank, stapelbar | 1 | 3000 | – | ja (siehe Eintrag) |
| 128 | [Ilmenblatt](#item-128) | benutzbar, Kraut/Trank, stapelbar | 1 | 1100 | – | ja (siehe Eintrag) |
| 129 | [Finagebäumchen](#item-129) | benutzbar, Kraut/Trank, stapelbar | 1 | 800 | – | ja (siehe Eintrag) |
| 130 | [Jorugawurzel](#item-130) | benutzbar, Kraut/Trank, stapelbar | 1 | 1800 | – | ja (siehe Eintrag) |
| 131 | [Thonnysblüte](#item-131) | benutzbar, Kraut/Trank, stapelbar | 1 | 3500 | – | ja (siehe Eintrag) |
| 132 | [Lotusblüte](#item-132) | benutzbar, Kraut/Trank, stapelbar | 1 | 2000 | – | ja (siehe Eintrag) |
| 133 | [Zauberstab](#item-133) | Waffe, persönlich, magisch | 70 | 10000 | TP 1W6+1, KK 15, BF unzerbr., AT +0/PA +0 | ja (siehe Eintrag) |
| 134 | [Skraja](#item-134) | Waffe | 90 | 500 | TP 1W6+3, KK 13, BF 4, AT +0/PA -2 | nur Tabellenwerte |
| 135 | [Kriegsbeil](#item-135) | Waffe | 120 | 500 | TP 1W6+4, KK 13, BF 2, AT -2/PA -4 | nur Tabellenwerte |
| 136 | [Orknase](#item-136) | Waffe | 120 | 600 | TP 1W6+5, KK 14, BF 2, AT -1/PA -3 | ja (siehe Eintrag) |
| 137 | [Schneidzahn](#item-137) | Waffe | 50 | 500 | TP 1W6+4, BF 0, AT +0/PA +0 | ja (siehe Eintrag) |
| 138 | [Robbentöter](#item-138) | Waffe | 70 | 600 | TP 1W6+3, KK 15, BF 2, AT +0/PA +0 | nur Tabellenwerte |
| 139 | [Wolfmesser](#item-139) | Waffe | 50 | 900 | TP 1W6+3, KK 15, BF 2, AT +0/PA -1 | nur Tabellenwerte |
| 140 | [Hexenbesen](#item-140) | Waffe, persönlich | 60 | 2000 | TP 1W6+1, KK 15, BF unzerbr., AT +0/PA +0 | ja (siehe Eintrag) |
| 141 | [Lotusgift](#item-141) | benutzbar, Kraut/Trank | 10 | 10000 | – | ja (siehe Eintrag) |
| 142 | [Kukris](#item-142) | benutzbar, Kraut/Trank | 10 | 9000 | – | ja (siehe Eintrag) |
| 143 | [Bannstaub](#item-143) | benutzbar, Kraut/Trank | 10 | 25000 | – | ja (siehe Eintrag) |
| 144 | [Kroetenschemelgift](#item-144) | benutzbar, Kraut/Trank | 10 | 1500 | – | ja (siehe Eintrag) |
| 145 | [Heiltrank](#item-145) | benutzbar, Kraut/Trank | 10 | 1000 | – | ja (siehe Eintrag) |
| 146 | [Starker Heiltrank](#item-146) | benutzbar, Kraut/Trank | 10 | 2400 | – | ja (siehe Eintrag) |
| 147 | [MU-Elixier](#item-147) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 148 | [KL-Elixier](#item-148) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 149 | [CH-Elixier](#item-149) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 150 | [FF-Elixier](#item-150) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 151 | [GE-Elixier](#item-151) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 152 | [IN-Elixier](#item-152) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 153 | [KK-Elixier](#item-153) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 154 | [Zaubertrank](#item-154) | benutzbar, Kraut/Trank | 10 | 1000 | – | ja (siehe Eintrag) |
| 155 | [Zaubertrank (stark)](#item-155) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 156 | [Olginwurzel](#item-156) | benutzbar, Kraut/Trank, stapelbar | 1 | 200 | – | ja (siehe Eintrag) |
| 157 | [Kairanhalm](#item-157) | benutzbar, Kraut/Trank, stapelbar | 1 | 150 | – | ja (siehe Eintrag) |
| 158 | [Bastardschwert (magisch)](#item-158) | Waffe, benutzbar, magisch | 50 | 1100 | TP 1W6+5, KK 14, BF -5, AT -2/PA -3 | ja (siehe Eintrag) |
| 159 | [Orknase (magisch)](#item-159) | Waffe, magisch | 120 | 600 | TP 1W6+10, KK 14, BF unzerbr., AT -2/PA -8 | ja (siehe Eintrag) |
| 160 | [Kurzschwert (magisch)](#item-160) | Waffe, magisch | 40 | 450 | TP 1W6+2, KK 15, BF unzerbr., AT +2/PA -1 | ja (siehe Eintrag) |
| 161 | [Sichel (magisch)](#item-161) | Waffe, magisch | 30 | 250 | TP 1W6+2, KK 17, BF 5, AT -3/PA -4 | ja (siehe Eintrag) |
| 162 | [Amulett (gegen Ignifaxius)](#item-162) | magisch | 10 | 1000 | – | **ohne Wirkung** |
| 163 | [Amulett (MR-Bonus)](#item-163) | magisch | 10 | 1000 | – | ja (siehe Eintrag) |
| 164 | [Knochen mit Rune](#item-164) | magisch | 10 | 100 | – | ja (siehe Eintrag) |
| 165 | [Ring (rot)](#item-165) | benutzbar, magisch | 10 | 1000 | – | ja (siehe Eintrag) |
| 166 | [Expurgicum](#item-166) | benutzbar | 10 | 1000 | – | ja (siehe Eintrag) |
| 167 | [Rezept für Expurgicum](#item-167) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 168 | [Vomicum](#item-168) | benutzbar | 10 | 1000 | – | ja (siehe Eintrag) |
| 169 | [Rezept für Vomicum](#item-169) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 170 | [Dokument (Einhorn)](#item-170) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 171 | [Silberner Stirnreif (Fassung 1)](#item-171) | benutzbar, magisch | 10 | 1000 | – | ja (siehe Eintrag) |
| 172 | [Säbel (magisch)](#item-172) | Waffe, magisch | 60 | 600 | TP 1W6+3, KK 15, BF 0, AT +0/PA +0 | ja (siehe Eintrag) |
| 173 | [Amulett (Feuerbann)](#item-173) | magisch | 10 | 1000 | – | **ohne Wirkung** |
| 174 | [Amulett (Flim Flam)](#item-174) | benutzbar, magisch | 10 | 1000 | – | ja (siehe Eintrag) |
| 175 | [Amulett (Travia)](#item-175) | magisch | 10 | 1000 | – | ja (siehe Eintrag) |
| 176 | [Schuldbuch](#item-176) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 177 | [Rote Mondscheibe](#item-177) | – | 10 | 200 | – | **ohne Wirkung** |
| 178 | [Zweihänder (magisch)](#item-178) | Waffe, magisch | 160 | 5000 | TP 2W6+4, KK 14, BF unzerbr., AT -3/PA -4 | ja (siehe Eintrag) |
| 179 | [Orkdokument](#item-179) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 180 | [Gegengift](#item-180) | benutzbar, Kraut/Trank | 10 | 1600 | – | ja (siehe Eintrag) |
| 181 | [Das Schwert Grimring](#item-181) | Waffe, magisch | 80 | 10000 | TP 1W6+4, KK 14, BF unzerbr., AT +2/PA +2 | ja (siehe Eintrag) |
| 182 | [Totenkopfgürtel](#item-182) | magisch | 10 | 200 | – | ja (siehe Eintrag) |
| 183 | [Kraftgürtel](#item-183) | magisch | 10 | 2000 | – | ja (siehe Eintrag) |
| 184 | [Magischer Brotbeutel](#item-184) | benutzbar, magisch | 10 | 1000 | – | ja (siehe Eintrag) |
| 185 | [Magischer Wasserschlauch](#item-185) | magisch | 10 | 1000 | – | ja (siehe Eintrag) |
| 186 | [Rezept für Heiltrank](#item-186) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 187 | [Schreiben von Jadra](#item-187) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 188 | [Amulett (gewöhnlich)](#item-188) | – | 10 | 200 | – | **ohne Wirkung** |
| 189 | [Lobpreisungen](#item-189) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 190 | [Mitgliederliste](#item-190) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 191 | [Dokument (Fassung 2)](#item-191) | – | 5 | 20 | – | **ohne Wirkung** |
| 192 | [Seekarte](#item-192) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 193 | [Kukrisdolch](#item-193) | Waffe | 20 | 1000 | TP 1W6+1, KK 15, BF 3, AT -2/PA -3 | ja (siehe Eintrag) |
| 194 | [Schlüssel (gelb)](#item-194) | – | 10 | 200 | – | **ohne Wirkung** |
| 195 | [Goldener Schlüssel](#item-195) | – | 10 | 200 | – | ja (siehe Eintrag) |
| 196 | [Helm (verflucht)](#item-196) | Rüstung | 90 | 20 | RS 1, BE 0 (Kopf) | ja (siehe Eintrag) |
| 197 | [Kettenhemd (verflucht)](#item-197) | Rüstung, magisch | 320 | 4000 | RS 2, BE 4 (Körper) | ja (siehe Eintrag) |
| 198 | [Kettenhemd (magisch)](#item-198) | Rüstung, magisch | 320 | 10000 | RS 5, BE 3 (Körper) | ja (siehe Eintrag) |
| 199 | [Schwert (magisch)](#item-199) | Waffe, magisch | 80 | 2000 | TP 1W6+4, KK 14, BF 0, AT +2/PA +0 | ja (siehe Eintrag) |
| 200 | [Kukrismenglibar](#item-200) | Waffe | 20 | 4000 | TP 1W6+1, KK 16, BF 7, AT -3/PA -4 | ja (siehe Eintrag) |
| 201 | [Platinschlüssel](#item-201) | – | 10 | 100 | – | ja (siehe Eintrag) |
| 202 | [Rezept für Gegengift](#item-202) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 203 | [Rezept für Hylailer Feuer](#item-203) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 204 | [Rezept für Kraftelixier](#item-204) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 205 | [Rezept für Mutelixier](#item-205) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 206 | [Rezept für Zaubertrank](#item-206) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 207 | [Ring (blau)](#item-207) | – | 10 | 500 | – | ja (siehe Eintrag) |
| 208 | [Schlüssel (silbern)](#item-208) | – | 10 | 100 | – | **ohne Wirkung** |
| 209 | [Eisenschlüssel (Fassung 1)](#item-209) | – | 10 | 100 | – | ja (siehe Eintrag) |
| 210 | [Eisenschlüssel (Fassung 2)](#item-210) | – | 10 | 100 | – | **ohne Wirkung** |
| 211 | [Roter Schlüssel](#item-211) | – | 10 | 100 | – | **ohne Wirkung** |
| 212 | [Bronzeschlüssel](#item-212) | – | 10 | 100 | – | ja (siehe Eintrag) |
| 213 | [Silberhelm](#item-213) | Rüstung | 180 | 1500 | RS 3, BE 2 (Kopf) | ja (siehe Eintrag) |
| 214 | [Silberstreitkolben](#item-214) | Waffe | 110 | 3000 | TP 1W6+4, KK 13, BF 1, AT +0/PA -2 | ja (siehe Eintrag) |
| 215 | [Silberschmuck (magisch)](#item-215) | magisch | 10 | 2000 | – | ja (siehe Eintrag) |
| 216 | [Speer (magisch)](#item-216) | Waffe, magisch | 80 | 1000 | TP 1W6+3, KK 16, BF 5, AT +1/PA +1 | ja (siehe Eintrag) |
| 217 | [Stirnreif (MR-Bonus)](#item-217) | magisch | 10 | 1000 | – | ja (siehe Eintrag) |
| 218 | [Wurfdolch (magisch)](#item-218) | Waffe, magisch | 10 | 1000 | TP 1W6, BF 4, AT -3/PA -4 | ja (siehe Eintrag) |
| 219 | [Goldschlüssel](#item-219) | – | 10 | 200 | – | ja (siehe Eintrag) |
| 220 | [Ring (grün)](#item-220) | magisch | 10 | 2000 | – | ja (siehe Eintrag) |
| 221 | [Beutel](#item-221) | benutzbar, magisch | 10 | 1000 | – | ja (siehe Eintrag) |
| 222 | [Buch (Piratenabrechnungen)](#item-222) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 223 | [Antikrankheitselixier](#item-223) | benutzbar, Kraut/Trank | 10 | 1000 | – | ja (siehe Eintrag) |
| 224 | [Buch (Kaisersprüche Hals)](#item-224) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 225 | [Kristall](#item-225) | – | 5 | 200 | – | ja (siehe Eintrag) |
| 226 | [MU-Elixier (schädliche Fassung)](#item-226) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 227 | [KL-Elixier (schädliche Fassung)](#item-227) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 228 | [CH-Elixier (schädliche Fassung)](#item-228) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 229 | [FF-Elixier (schädliche Fassung)](#item-229) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 230 | [GE-Elixier (schädliche Fassung)](#item-230) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 231 | [IN-Elixier (schädliche Fassung)](#item-231) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 232 | [KK-Elixier (schädliche Fassung)](#item-232) | benutzbar, Kraut/Trank | 10 | 4000 | – | ja (siehe Eintrag) |
| 233 | [Robe (grün, Fassung 2)](#item-233) | Rüstung | 50 | 50 | RS 0, BE 0 (Körper) | nur Tabellenwerte |
| 234 | [Goldener Schild](#item-234) | Rüstung | 200 | 2500 | RS 2, BE 1 (linke Hand (Schild)) | nur Tabellenwerte |
| 235 | [Empfehlungsschreiben (Hetmann)](#item-235) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 236 | [Wunderkur](#item-236) | benutzbar, Kraut/Trank | 10 | 6000 | – | ja (siehe Eintrag) |
| 237 | [Schlaftrunk](#item-237) | benutzbar, Kraut/Trank | 10 | 600 | – | ja (siehe Eintrag) |
| 238 | [Miasthmaticum](#item-238) | benutzbar | 10 | 1000 | – | ja (siehe Eintrag) |
| 239 | [Hylailer Feuer](#item-239) | benutzbar | 10 | 1000 | – | ja (siehe Eintrag) |
| 240 | [Rezept für Starken Heiltrank](#item-240) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 241 | [Rezept für Wunderkur](#item-241) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 242 | [Rezept für Schlaftrunk](#item-242) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 243 | [Rezept für Star. Zaubertrank](#item-243) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 244 | [Rezept für Miasthmaticum](#item-244) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 245 | [Silberner Stirnreif (Fassung 2)](#item-245) | benutzbar, magisch | 10 | 1000 | – | ja (siehe Eintrag) |
| 246 | [Buch (Heptagon)](#item-246) | benutzbar | 5 | 1000 | – | ja (siehe Eintrag) |
| 247 | [Empfehlungsschreiben (Siebenstein)](#item-247) | benutzbar | 5 | 20 | – | ja (siehe Eintrag) |
| 248 | [Schwarze Statuette](#item-248) | – | 10 | 2000 | – | ja (siehe Eintrag) |
| 249 | [Laterne (brennend)](#item-249) | benutzbar | 15 | 30 | – | ja (siehe Eintrag) |
| 250 | [Dukaten](#item-250) | – | 0 | 100 | – | ja (siehe Eintrag) |
| 251 | [200 Pfeile](#item-251) | – | 0 | 0 | – | ja (siehe Eintrag) |
| 252 | [50 Bolzen](#item-252) | – | 0 | 0 | – | ja (siehe Eintrag) |
| 253 | [20 Kletterhaken](#item-253) | – | 0 | 0 | – | ja (siehe Eintrag) |
| 254 | [(Endmarke, kein Gegenstand)](#item-254) | – | 0 | 0 | – | **ohne Wirkung** |

---

## 3. Alle Einträge im Detail

<a id="item-1"></a>
### 1. Schwert (`ITEM_ID_SCHWERT`)

- **Im Code:** Gegenstands-ID 1 (`ITEM_ID_SCHWERT`, Rohname in ITEMNAME `SCHWERT..ER`). ITEMS.DAT: Waffe; Gewicht 80 Unzen; Grundpreis 85 Silbertaler (= 850 Heller). Waffe, Waffenart Schwert (Untertyp 3), Waffentabelle `g_weapon_stats_table[0]`: TP 1W6+4, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), Bruchfaktor 2, AT +0, PA +0. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 19, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 19). Startausrüstung (`g_hero_startup_items`): Krieger. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Reiseereignis Rovamund–Nordvest, Verlassene Mine, Zwingfeste (2×), Wolfshöhle (2×), Daspotaschatz (2×), Kultstätte des Namenlosen (2×), Piratenhöhle (2×), Hyggeliks Ruine, Totenschiff. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-2"></a>
### 2. Knüppel (`ITEM_ID_KNUEPPEL`)

- **Im Code:** Gegenstands-ID 2 (`ITEM_ID_KNUEPPEL`, Rohname in ITEMNAME `KNÜPPEL..`). ITEMS.DAT: Waffe; Gewicht 60 Unzen; Grundpreis 5 Silbertaler (= 50 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[1]`: TP 1W6+1, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), Bruchfaktor 6, AT -1, PA -3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Magier. Handel: Sortimentswert 20, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 20). Liegt in Truhen/Beutetabellen im Code: Hyggeliks Ruine. Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit stumpfer Waffe dargestellt. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-3"></a>
### 3. Säbel (`ITEM_ID_SAEBEL`)

- **Im Code:** Gegenstands-ID 3 (`ITEM_ID_SAEBEL`, Rohname in ITEMNAME `SÄBEL..`). ITEMS.DAT: Waffe; Gewicht 60 Unzen; Grundpreis 60 Silbertaler (= 600 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[2]`: TP 1W6+3, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 2, AT +0, PA +0. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 18, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 18). Startausrüstung (`g_hero_startup_items`): Thorwaler. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Zwingfeste (4×), Verfallene Herberge, Wolfshöhle, Orkbehausung, Piratenhöhle (5×), Hyggeliks Ruine, Totenschiff (3×). Liegt auf dem Totenschiff auf Feld (1,13,5) zum Mitnehmen (einmalig). (`Quelle/seg077.c:DNG01_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg077.c:DNG01_handler`
- **Deutung der KI:** Vermutlich ein ganz normaler Säbel, den man auf dem Totenschiff zusätzlich einmal am Boden findet.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-4"></a>
### 4. Messer (`ITEM_ID_MESSER`)

- **Im Code:** Gegenstands-ID 4 (`ITEM_ID_MESSER`, Rohname in ITEMNAME `MESSER..`). ITEMS.DAT: Waffe; Gewicht 10 Unzen; Grundpreis 5 Silbertaler (= 50 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[3]`: TP 1W6, KK-Zuschlag ab KK 16 (je Punkt darüber +1 TP), Bruchfaktor 4, AT -3, PA -4. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 20, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 20). Startausrüstung (`g_hero_startup_items`): Waldelf. Liegt in Truhen/Beutetabellen im Code: Verfallene Herberge. Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit Messer/Dolch dargestellt. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-5"></a>
### 5. Speer (`ITEM_ID_SPEER`)

- **Im Code:** Gegenstands-ID 5 (`ITEM_ID_SPEER`, Rohname in ITEMNAME `SPEER..E`). ITEMS.DAT: Waffe; Gewicht 80 Unzen; Grundpreis 30 Silbertaler (= 300 Heller). Waffe, Waffenart Speer/Stab (Untertyp 5), Waffentabelle `g_weapon_stats_table[4]`: TP 1W6+3, KK-Zuschlag ab KK 16 (je Punkt darüber +1 TP), Bruchfaktor 5, AT +0, PA -3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 20, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 20). Startausrüstung (`g_hero_startup_items`): Firnelf. Liegt in Truhen/Beutetabellen im Code: Zwingfeste (4×), Orkbehausung, Piratenhöhle (5×). Wird als Wurfwaffe eingesetzt: Nach dem Wurf fliegt die Waffe aus der rechten Hand (liegt in der linken Hand eine gleiche, rückt sie nach). Bis zu 30 geworfene Waffen merkt sich der Kampf und gibt sie nach einem gewonnenen Kampf an die Gruppe zurück (nur die Gegenstandsnummer, nicht ihr Zustand). (Im Original ist der Speer damit eine Wurfwaffe.) (`Quelle/seg041.c:range_attack_check_ammo`) Zwingfeste: liegt in einer Nische auf Feld (2,1,2) (einmalig). (`Quelle/seg087.c:DNG14_handler`) Jagd-Reiseereignisse (Karene, Wildschweine): Jagen geht nur, wenn irgendein Held einen Kurzbogen, Langbogen, eine Armbrust oder einen Speer besitzt (Munition wird nicht geprüft und nicht verbraucht). (`Quelle/seg117.c:hunt_karen`, `Quelle/seg117.c:hunt_wildboar`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:range_attack_check_ammo`, `Quelle/seg087.c:DNG14_handler`, `Quelle/seg117.c:hunt_karen`, `Quelle/seg117.c:hunt_wildboar`
- **Deutung der KI:** Sieht so aus, als ob der Speer im Original eher als Wurfwaffe denn als Nahkampfwaffe behandelt wird. Der Code-Kommentar nennt das „Original-Bug 60“, weil der Speer nach den Regeln eine Nahkampfwaffe ist.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-6"></a>
### 6. Kurzschwert (`ITEM_ID_KURZSCHWERT`)

- **Im Code:** Gegenstands-ID 6 (`ITEM_ID_KURZSCHWERT`, Rohname in ITEMNAME `KURZSCHWERT..ER`). ITEMS.DAT: Waffe; Gewicht 40 Unzen; Grundpreis 45 Silbertaler (= 450 Heller). Waffe, Waffenart Schwert (Untertyp 3), Waffentabelle `g_weapon_stats_table[5]`: TP 1W6+2, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 1, AT +0, PA -1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 18, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 18). Liegt in Truhen/Beutetabellen im Code: Zwergenfeste (3×). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-7"></a>
### 7. Schild (`ITEM_ID_SCHILD`)

- **Im Code:** Gegenstands-ID 7 (`ITEM_ID_SCHILD`, Rohname in ITEMNAME `SCHILD..E`). ITEMS.DAT: Rüstung; Gewicht 140 Unzen; Grundpreis 25 Silbertaler (= 250 Heller). Rüstung, Platz linke Hand (Schild) (Untertyp 9), `g_armor_stats_table[0]`: RS 1, BE 0. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 18, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 18). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (5×), Zwingfeste (3×), Daspotaschatz (2×). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-8"></a>
### 8. Kriegsbeil (Sonderfassung) (`ITEM_ID_KRIEGSBEIL__SPECIAL`)

- **Im Code:** Gegenstands-ID 8 (`ITEM_ID_KRIEGSBEIL__SPECIAL`, Rohname in ITEMNAME `KRIEGSBEIL..E`). ITEMS.DAT: Waffe; Gewicht 120 Unzen; Grundpreis 45 Silbertaler (= 450 Heller). Waffe, Waffenart Axt (Untertyp 4), Waffentabelle `g_weapon_stats_table[6]`: TP 1W6+4, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), Bruchfaktor 5, AT +0, PA -3. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (3×), Totenschiff. Zählt als Axt für die Reiseereignisse Brückenbau (Breida–Tjoila, Auplog–Varnheim) und umgestürzter Baum (`g_travel_event_axes`). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg109.c:g_travel_event_axes`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-9"></a>
### 9. Kurzbogen (`ITEM_ID_KURZBOGEN`)

- **Im Code:** Gegenstands-ID 9 (`ITEM_ID_KURZBOGEN`, Rohname in ITEMNAME `KURZBOGEN..`). ITEMS.DAT: Waffe; Gewicht 20 Unzen; Grundpreis 45 Silbertaler (= 450 Heller). Waffe, Waffenart Schusswaffe (Untertyp 7), Waffentabelle `g_weapon_stats_table[7]`: TP 1W6+3, kein KK-Zuschlag, Bruchfaktor 0, AT +0, PA +0, Fernkampf-Entfernungstabelle Nr. 3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Magier. Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Liegt in Truhen/Beutetabellen im Code: Zwingfeste, Orkbehausung, Piratenhöhle (2×). Schießt nur, wenn in der linken Hand Pfeile liegen (sonst Meldung: keine Pfeile in der linken Hand). Jeder Schuss verbraucht einen Pfeil, verschossene Pfeile kommen nicht zurück. (`Quelle/seg041.c:range_attack_check_ammo`) Kann nicht vergiftet werden. (`Quelle/seg107.c:item_apply_weapon_poison`) Jagd-Reiseereignisse (Karene, Wildschweine): Jagen geht nur, wenn irgendein Held einen Kurzbogen, Langbogen, eine Armbrust oder einen Speer besitzt (Munition wird nicht geprüft und nicht verbraucht). (`Quelle/seg117.c:hunt_karen`, `Quelle/seg117.c:hunt_wildboar`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:range_attack_check_ammo`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg117.c:hunt_karen`, `Quelle/seg117.c:hunt_wildboar`
- **Deutung der KI:** Vermutlich die Standard-Schusswaffe: Ohne Pfeile in der linken Hand ist sie nutzlos, auf Reisen ermöglicht sie die Jagd.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-10"></a>
### 10. Pfeil (`ITEM_ID_PFEIL`)

- **Im Code:** Gegenstands-ID 10 (`ITEM_ID_PFEIL`, Rohname in ITEMNAME `PFEIL..E`). ITEMS.DAT: Waffe, stapelbar; Gewicht 4 Unzen; Grundpreis 5 Heller (= 5 Heller). Waffe, Waffenart waffenlos/Munition (Untertyp 0), Waffentabelle `g_weapon_stats_table[53]`: TP 0W6, kein KK-Zuschlag, Bruchfaktor 0, AT +0, PA +0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Liegt in Truhen/Beutetabellen im Code: Zwingfeste, Orkbehausung. Die Mitstreiter Nariell (in jeder ersten Kampfrunde) und Harika (bei LE ≤ 12) nehmen im Computerkampf automatisch Langbogen und Pfeile in die Hände, wenn sie beides im Gepäck haben. (`Quelle/seg036.c:AFIG_hero_turn`) Munition für Kurz- und Langbogen: muss in der linken Hand liegen, jeder Schuss verbraucht einen Pfeil. (`Quelle/seg041.c:range_attack_check_ammo`) Startausrüstung: Jäger, Auelfen und Waldelfen bekommen 20 Pfeile in die linke Hand. (`Quelle/seg106.c:startup_equipment`) Der Bündel-Gegenstand „200 Pfeile“ (ID 251) wird beim Erhalt (z. B. aus einer Truhe) in 200 Stück dieses Gegenstands umgewandelt. (`Quelle/seg105.c:give_new_item_to_group`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg036.c:AFIG_hero_turn`, `Quelle/seg041.c:range_attack_check_ammo`, `Quelle/seg106.c:startup_equipment`, `Quelle/seg105.c:give_new_item_to_group`
- **Deutung der KI:** Vermutlich reine Munition. Verschossene Pfeile sind verloren, anders als geworfene Waffen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-11"></a>
### 11. Streitaxt (`ITEM_ID_STREITAXT`)

- **Im Code:** Gegenstands-ID 11 (`ITEM_ID_STREITAXT`, Rohname in ITEMNAME `STREIT.AXT.ÄXTE`). ITEMS.DAT: Waffe; Gewicht 150 Unzen; Grundpreis 110 Silbertaler (= 1100 Heller). Waffe, Waffenart Zweihänder (Untertyp 6), Waffentabelle `g_weapon_stats_table[8]`: TP 2W6+4, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), Bruchfaktor 3, AT -1, PA -4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Zählt als Axt für die Reiseereignisse Brückenbau (Breida–Tjoila, Auplog–Varnheim) und umgestürzter Baum (`g_travel_event_axes`). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg109.c:g_travel_event_axes`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-12"></a>
### 12. Armbrust (`ITEM_ID_ARMBRUST`)

- **Im Code:** Gegenstands-ID 12 (`ITEM_ID_ARMBRUST`, Rohname in ITEMNAME `ARM.BRUST.BRÜSTE`). ITEMS.DAT: Waffe; Gewicht 200 Unzen; Grundpreis 123 Silbertaler (= 1230 Heller). Waffe, Waffenart Schusswaffe (Untertyp 7), Waffentabelle `g_weapon_stats_table[9]`: TP 2W6+2, kein KK-Zuschlag, Bruchfaktor 0, AT +0, PA +0, Fernkampf-Entfernungstabelle Nr. 5. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Hexe, Druide, Magier. Handel: Sortimentswert 14, damit bei 16 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (5×), Zwingfeste, Daspotaschatz (2×), Piratenhöhle. Schießt nur, wenn in der linken Hand Bolzen liegen (sonst Meldung: keine Bolzen in der linken Hand). Jeder Schuss verbraucht einen Bolzen. (`Quelle/seg041.c:range_attack_check_ammo`) Liegt auf dem Totenschiff auf Feld (3,2,9) zum Mitnehmen (einmalig). (`Quelle/seg077.c:DNG01_handler`) Kann nicht vergiftet werden. (`Quelle/seg107.c:item_apply_weapon_poison`) Jagd-Reiseereignisse (Karene, Wildschweine): Jagen geht nur, wenn irgendein Held einen Kurzbogen, Langbogen, eine Armbrust oder einen Speer besitzt (Munition wird nicht geprüft und nicht verbraucht). (`Quelle/seg117.c:hunt_karen`, `Quelle/seg117.c:hunt_wildboar`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:range_attack_check_ammo`, `Quelle/seg077.c:DNG01_handler`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg117.c:hunt_karen`, `Quelle/seg117.c:hunt_wildboar`
- **Deutung der KI:** Vermutlich die stärkste normale Schusswaffe. Sie braucht Bolzen in der linken Hand und ermöglicht auf Reisen die Jagd.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-13"></a>
### 13. Bolzen (`ITEM_ID_BOLZEN`)

- **Im Code:** Gegenstands-ID 13 (`ITEM_ID_BOLZEN`, Rohname in ITEMNAME `BOLZEN..`). ITEMS.DAT: Waffe, stapelbar; Gewicht 5 Unzen; Grundpreis 8 Heller (= 8 Heller). Waffe, Waffenart waffenlos/Munition (Untertyp 0), Waffentabelle `g_weapon_stats_table[53]`: TP 0W6, kein KK-Zuschlag, Bruchfaktor 0, AT +0, PA +0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 14, damit bei 16 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (3×), Zwingfeste, Reiseereignis Ala–Tjanset. Munition für die Armbrust: muss in der linken Hand liegen, jeder Schuss verbraucht einen Bolzen. (`Quelle/seg041.c:range_attack_check_ammo`) Der Bündel-Gegenstand „50 Bolzen“ (ID 252) wird beim Erhalt (z. B. aus einer Truhe) in 50 Stück dieses Gegenstands umgewandelt. (`Quelle/seg105.c:give_new_item_to_group`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:range_attack_check_ammo`, `Quelle/seg105.c:give_new_item_to_group`
- **Deutung der KI:** Vermutlich reine Munition für die Armbrust, jeder Schuss kostet einen Bolzen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-14"></a>
### 14. Dolch (`ITEM_ID_DOLCH`)

- **Im Code:** Gegenstands-ID 14 (`ITEM_ID_DOLCH`, Rohname in ITEMNAME `DOLCH..E`). ITEMS.DAT: Waffe; Gewicht 20 Unzen; Grundpreis 20 Silbertaler (= 200 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[10]`: TP 1W6+1, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 3, AT -2, PA -3. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 20, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 20). Startausrüstung (`g_hero_startup_items`): Gaukler, Jäger, Krieger, Streuner, Magier. Liegt in Truhen/Beutetabellen im Code: Verlassene Mine (2×), Zwingfeste (5×), Verfallene Herberge, Wolfshöhle (4×), Daspotaschatz (2×), Kultstätte des Namenlosen (4×), Piratenhöhle (3×), Totenschiff (2×). Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit Messer/Dolch dargestellt. Ersetzt nach dem ersten Treffer den vergifteten Kukrisdolch (ID 193) in der rechten Hand. (`Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`) Orkbehausung: Beim Durchsuchen eines Bettes findet ein Held mit 10 % Wahrscheinlichkeit einen Dolch. (`Quelle/seg083.c:DNG08_search_bed`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`, `Quelle/seg083.c:DNG08_search_bed`
- **Deutung der KI:** Vermutlich eine gewöhnliche Seitenwaffe, die im Code nur als Ersatz für den verbrauchten Kukrisdolch und als Zufallsfund auftaucht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-15"></a>
### 15. Eisenschild (`ITEM_ID_EISENSCHILD`)

- **Im Code:** Gegenstands-ID 15 (`ITEM_ID_EISENSCHILD`, Rohname in ITEMNAME `EISENSCHILD..E`). ITEMS.DAT: Rüstung; Gewicht 200 Unzen; Grundpreis 45 Silbertaler (= 450 Heller). Rüstung, Platz linke Hand (Schild) (Untertyp 9), `g_armor_stats_table[1]`: RS 2, BE 1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 14, damit bei 16 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-16"></a>
### 16. Wurfbeil (`ITEM_ID_WURFBEIL`)

- **Im Code:** Gegenstands-ID 16 (`ITEM_ID_WURFBEIL`, Rohname in ITEMNAME `WURFBEIL..E`). ITEMS.DAT: Waffe; Gewicht 120 Unzen; Grundpreis 35 Silbertaler (= 350 Heller). Waffe, Waffenart Wurfwaffe (Untertyp 8), Waffentabelle `g_weapon_stats_table[11]`: TP 1W6+3, kein KK-Zuschlag, Bruchfaktor 4, AT -1, PA -3, Fernkampf-Entfernungstabelle Nr. 2. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 12, damit bei 15 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 12). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (2×), Totenschiff. Zählt als Axt für die Reiseereignisse Brückenbau (Breida–Tjoila, Auplog–Varnheim) und umgestürzter Baum (`g_travel_event_axes`). Wird als Wurfwaffe eingesetzt: Nach dem Wurf fliegt die Waffe aus der rechten Hand (liegt in der linken Hand eine gleiche, rückt sie nach). Bis zu 30 geworfene Waffen merkt sich der Kampf und gibt sie nach einem gewonnenen Kampf an die Gruppe zurück (nur die Gegenstandsnummer, nicht ihr Zustand). (`Quelle/seg041.c:range_attack_check_ammo`) Gehört zum Werkzeugsatz Schaufel, Hacke, Brecheisen und Wurfbeil. Nur wenn die Gruppe alle vier dabei hat, gilt sie als ausgerüstet. Verlassene Mine: Gang freiräumen 2 statt 6 Stunden. Einsturzstelle (schwer): mit Werkzeug 12, ohne 18 Stunden. Einsturzstelle (leicht): im Original dauert es MIT Werkzeug einen Tag und ohne 6 Stunden (vertauscht), mit Fix 122 (Fanpatch-Stufe „Wichtig“) richtig herum. Hyggeliks Ruine, Gang freiräumen: im Original mit Werkzeug 1 Stunde, ohne 30 Minuten (vertauscht), mit Fix 122 (Fanpatch-Stufe „Wichtig“) richtig herum. (`Quelle/seg091.c:DNG13_unblock_passage`, `Quelle/seg091.c:DNG13_collapsing_ceiling`, `Quelle/seg091.c:DNG13_collapsing_ceiling_hard`, `Quelle/seg089.c:DNG15_clear_way`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg109.c:g_travel_event_axes`, `Quelle/seg041.c:range_attack_check_ammo`, `Quelle/seg091.c:DNG13_unblock_passage`, `Quelle/seg091.c:DNG13_collapsing_ceiling`, `Quelle/seg091.c:DNG13_collapsing_ceiling_hard`, `Quelle/seg089.c:DNG15_clear_way`
- **Deutung der KI:** Sieht so aus, als ob das Wurfbeil gleich dreifach nützlich ist: als Wurfwaffe, als Axt bei Reisehindernissen und als Teil des vierteiligen Werkzeugsatzes zum Freiräumen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-17"></a>
### 17. Wurfstern (`ITEM_ID_WURFSTERN`)

- **Im Code:** Gegenstands-ID 17 (`ITEM_ID_WURFSTERN`, Rohname in ITEMNAME `WURFSTERN..E`). ITEMS.DAT: Waffe, stapelbar; Gewicht 15 Unzen; Grundpreis 25 Silbertaler (= 250 Heller). Waffe, Waffenart Wurfwaffe (Untertyp 8), Waffentabelle `g_weapon_stats_table[12]`: TP 1W6+1, kein KK-Zuschlag, Bruchfaktor 0, AT +0, PA +0, Fernkampf-Entfernungstabelle Nr. 0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 12, damit bei 15 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 12). Wird als Wurfwaffe eingesetzt: Nach dem Wurf fliegt die Waffe aus der rechten Hand (liegt in der linken Hand eine gleiche, rückt sie nach). Bis zu 30 geworfene Waffen merkt sich der Kampf und gibt sie nach einem gewonnenen Kampf an die Gruppe zurück (nur die Gegenstandsnummer, nicht ihr Zustand). (`Quelle/seg041.c:range_attack_check_ammo`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg041.c:range_attack_check_ammo`
- **Deutung der KI:** Vermutlich eine stapelbare Wurfwaffe, die nach gewonnenen Kämpfen wieder eingesammelt wird.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-18"></a>
### 18. Zweihänder (`ITEM_ID_ZWEIHAENDER`)

- **Im Code:** Gegenstands-ID 18 (`ITEM_ID_ZWEIHAENDER`, Rohname in ITEMNAME `ZWEIHÄNDER..`). ITEMS.DAT: Waffe; Gewicht 160 Unzen; Grundpreis 135 Silbertaler (= 1350 Heller). Waffe, Waffenart Zweihänder (Untertyp 6), Waffentabelle `g_weapon_stats_table[13]`: TP 2W6+4, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), Bruchfaktor 3, AT -2, PA -3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Thorwaler, Zwerg, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 10, damit bei 13 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 10). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-19"></a>
### 19. Langbogen (`ITEM_ID_LANGBOGEN`)

- **Im Code:** Gegenstands-ID 19 (`ITEM_ID_LANGBOGEN`, Rohname in ITEMNAME `LANGBOGEN..`). ITEMS.DAT: Waffe; Gewicht 30 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Waffe, Waffenart Schusswaffe (Untertyp 7), Waffentabelle `g_weapon_stats_table[14]`: TP 1W6+4, kein KK-Zuschlag, Bruchfaktor 0, AT +0, PA +0, Fernkampf-Entfernungstabelle Nr. 4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Magier. Handel: Sortimentswert 10, damit bei 13 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 10). Startausrüstung (`g_hero_startup_items`): Jäger, Auelf, Waldelf. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Wolfshöhle. Die Mitstreiter Nariell (in jeder ersten Kampfrunde) und Harika (bei LE ≤ 12) nehmen im Computerkampf automatisch Langbogen und Pfeile in die Hände, wenn sie beides im Gepäck haben. (`Quelle/seg036.c:AFIG_hero_turn`) Schießt nur, wenn in der linken Hand Pfeile liegen (sonst Meldung: keine Pfeile in der linken Hand). Jeder Schuss verbraucht einen Pfeil, verschossene Pfeile kommen nicht zurück. (`Quelle/seg041.c:range_attack_check_ammo`) Kann nicht vergiftet werden. (`Quelle/seg107.c:item_apply_weapon_poison`) Jagd-Reiseereignisse (Karene, Wildschweine): Jagen geht nur, wenn irgendein Held einen Kurzbogen, Langbogen, eine Armbrust oder einen Speer besitzt (Munition wird nicht geprüft und nicht verbraucht). (`Quelle/seg117.c:hunt_karen`, `Quelle/seg117.c:hunt_wildboar`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg036.c:AFIG_hero_turn`, `Quelle/seg041.c:range_attack_check_ammo`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg117.c:hunt_karen`, `Quelle/seg117.c:hunt_wildboar`
- **Deutung der KI:** Vermutlich der bessere Bogen. Die Mitstreiter Nariell und Harika greifen im Computerkampf von sich aus zu ihm.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-20"></a>
### 20. Morgenstern (`ITEM_ID_MORGENSTERN`)

- **Im Code:** Gegenstands-ID 20 (`ITEM_ID_MORGENSTERN`, Rohname in ITEMNAME `MORGENSTERN..E`). ITEMS.DAT: Waffe; Gewicht 120 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[15]`: TP 1W6+5, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 5, AT -1, PA -3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Thorwaler, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 8, damit bei 10 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 8). Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit stumpfer Waffe dargestellt. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-21"></a>
### 21. Vulkanglasdolch (`ITEM_ID_VULKANGLASDOLCH`)

- **Im Code:** Gegenstands-ID 21 (`ITEM_ID_VULKANGLASDOLCH`, Rohname in ITEMNAME `VULKANGLASDOLCH..E`). ITEMS.DAT: Waffe, magisch; Gewicht 30 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[16]`: TP 1W6, KK-Zuschlag ab KK 16 (je Punkt darüber +1 TP), Bruchfaktor 6, AT -2, PA -3. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 1, damit bei 1 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 1). Startausrüstung (`g_hero_startup_items`): Druide. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit Messer/Dolch dargestellt. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-22"></a>
### 22. Fackel (brennend) (`ITEM_ID_FACKEL__LIT`)

- **Im Code:** Gegenstands-ID 22 (`ITEM_ID_FACKEL__LIT`, Rohname in ITEMNAME `FACKEL..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 20 Unzen; Grundpreis 5 Heller (= 5 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Alle 15 Spielminuten wird der Brennzähler (lighting_timer) jeder brennenden Fackel um 1 verringert; erreicht er 0, verschwindet die Fackel aus dem Inventar. (`Quelle/seg002.c:sub_light_timers`) Entsteht beim Anzünden einer Fackel (ID 65) mit Brennzähler 10 (2,5 Stunden). (`Quelle/seg101.c:spell_brenne`) Licht in Dungeons (ohne Dunkelheit-Zauber, ohne Flim Flam und ohne Magier mit Stabzauber ab Stufe 2): Die Helligkeit richtet sich nach der Lichtquelle mit dem meisten Restbrennwert. Eine brennende Fackel zählt mit ihrem Brennzähler (10 = volles Licht, dann immer dunkler), eine brennende Laterne mit Brennzähler/10 (bleibt also viel länger hell). Im Original zählen auch Lichtquellen von Helden anderer Gruppen. Mit Fix 347 (Fanpatch-Stufe „nur Alle“) nur die der aktiven Gruppe. (`Quelle/seg106.c:get_max_light_time`, `Quelle/seg075.c:DNG_lights`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg002.c:sub_light_timers`, `Quelle/seg101.c:spell_brenne`, `Quelle/seg106.c:get_max_light_time`, `Quelle/seg075.c:DNG_lights`
- **Deutung der KI:** Sieht so aus, als ob eine brennende Fackel 2,5 Stunden hält und dabei langsam dunkler wird.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-23"></a>
### 23. Bier (`ITEM_ID_BIER`)

- **Im Code:** Gegenstands-ID 23 (`ITEM_ID_BIER`, Rohname in ITEMNAME `BIER..`). ITEMS.DAT: Nahrung; Gewicht 40 Unzen; Grundpreis 1 Heller (= 1 Heller). Nahrung (Getränk: senkt den Durst um 40 Punkte beim Einnehmen). Handel: Sortimentswert 18, damit bei 55 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 18). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (7×), Zwingfeste (10×), Kultstätte des Namenlosen. Steigt der Durst eines Helden über 90, trinkt er in der stündlichen Versorgung automatisch zuerst Bier und erst danach einen vollen Wasserschlauch. (`Quelle/seg002.c:herokeeping`) Einnehmen: verbraucht, keine Flasche zurück. Ein Code-Zweig für eine Glasflasche nach anderen Getränken wird laut Kommentar nie erreicht (nur Bier landet dort). (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg108.c:consume`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg002.c:herokeeping`
- **Deutung der KI:** Vermutlich das bevorzugte Getränk der automatischen Versorgung. Anders als Wein und Schnaps macht es im Code nicht betrunken.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-24"></a>
### 24. Wurfhaken (`ITEM_ID_WURFHAKEN`)

- **Im Code:** Gegenstands-ID 24 (`ITEM_ID_WURFHAKEN`, Rohname in ITEMNAME `WURFHAKEN..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 30 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: Sortimentswert 14, damit bei 46 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 14). Liegt in Truhen/Beutetabellen im Code: Totenschiff. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-25"></a>
### 25. Laterne (nicht brennend) (`ITEM_ID_LATERNE__UNLIT`)

- **Im Code:** Gegenstands-ID 25 (`ITEM_ID_LATERNE__UNLIT`, Rohname in ITEMNAME `LATERNE..N`). ITEMS.DAT: benutzbar; Gewicht 15 Unzen; Grundpreis 3 Silbertaler (= 30 Heller). Benutzbar-Eintrag 12 (`g_usable_items_table`): Benutzen ruft `item_ignite()` auf. Handel: Sortimentswert 14, damit bei 46 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 14). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (2×), Wolfshöhle, Kultstätte des Namenlosen (3×), Totenschiff. Eine ausgebrannte Laterne (ID 249) wird nach Ablauf ihres Brennzählers wieder zu diesem Gegenstand. (`Quelle/seg002.c:sub_light_timers`) Wolfshöhle: liegt in einer Nische auf Feld (0,10,6) zum Mitnehmen (einmalig). (`Quelle/seg080.c:DNG04_handler`) Verlassene Mine: liegt auf Feld (0,9,13) am Boden (einmalig). (`Quelle/seg091.c:DNG13_handler`) Anzünden (Zauber „Brenne toter Stoff“, Zunderkästchen oder Benutzen der Laterne): braucht eine Flasche Öl im Inventar des Anzündenden. Das Öl wird verbraucht (zurück kommt eine Bronzeflasche), die Laterne wird zur brennenden Laterne (ID 249) mit Brennzähler 100 (= 25 Stunden). Im Original wird beim Benutzen immer die erste Laterne im Inventar angezündet, nicht die gewählte. Fix 210 (Fanpatch-Stufe „nur Alle“) zündet die gewählte an. (`Quelle/seg101.c:spell_brenne`) Benutzen: braucht ein Zunderkästchen und Öl im Inventar. Im Original findet der Zauber „Brenne toter Stoff“ danach nur noch Laternen, Fix 211 (Fanpatch-Stufe „Wichtig“) behebt das. (`Quelle/seg107.c:item_ignite`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_ignite`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg002.c:sub_light_timers`, `Quelle/seg080.c:DNG04_handler`, `Quelle/seg091.c:DNG13_handler`, `Quelle/seg101.c:spell_brenne`
- **Deutung der KI:** Sieht so aus, als ob die Laterne die deutlich bessere Lichtquelle ist (25 Stunden, lange volle Helligkeit), aber Öl und zum Anzünden ein Zunderkästchen braucht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-26"></a>
### 26. Brecheisen (`ITEM_ID_BRECHEISEN`)

- **Im Code:** Gegenstands-ID 26 (`ITEM_ID_BRECHEISEN`, Rohname in ITEMNAME `BRECHEISEN..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 90 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: Sortimentswert 18, damit bei 55 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 18). Startausrüstung (`g_hero_startup_items`): Zwerg. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Zwingfeste, Spinnenhöhle, Piratenhöhle. Gehört zum Werkzeugsatz Schaufel, Hacke, Brecheisen und Wurfbeil. Nur wenn die Gruppe alle vier dabei hat, gilt sie als ausgerüstet. Verlassene Mine: Gang freiräumen 2 statt 6 Stunden. Einsturzstelle (schwer): mit Werkzeug 12, ohne 18 Stunden. Einsturzstelle (leicht): im Original dauert es MIT Werkzeug einen Tag und ohne 6 Stunden (vertauscht), mit Fix 122 (Fanpatch-Stufe „Wichtig“) richtig herum. Hyggeliks Ruine, Gang freiräumen: im Original mit Werkzeug 1 Stunde, ohne 30 Minuten (vertauscht), mit Fix 122 (Fanpatch-Stufe „Wichtig“) richtig herum. (`Quelle/seg091.c:DNG13_unblock_passage`, `Quelle/seg091.c:DNG13_collapsing_ceiling`, `Quelle/seg091.c:DNG13_collapsing_ceiling_hard`, `Quelle/seg089.c:DNG15_clear_way`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg091.c:DNG13_unblock_passage`, `Quelle/seg091.c:DNG13_collapsing_ceiling`, `Quelle/seg091.c:DNG13_collapsing_ceiling_hard`, `Quelle/seg089.c:DNG15_clear_way`
- **Deutung der KI:** Sieht so aus, als ob das Brecheisen im Code nur als Teil des vierteiligen Werkzeugsatzes zählt. Türen oder Truhen öffnet es nicht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-27"></a>
### 27. Hammer (`ITEM_ID_HAMMER`)

- **Im Code:** Gegenstands-ID 27 (`ITEM_ID_HAMMER`, Rohname in ITEMNAME `H.AMMER.ÄMMER.`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 20 Unzen; Grundpreis 6 Silbertaler (= 60 Heller). Handel: Sortimentswert 17, damit bei 52 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 17). Startausrüstung (`g_hero_startup_items`): Zwerg. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Verlassene Mine, Spinnenhöhle, Drachenhort, Piratenhöhle (4×), Hyggeliks Ruine, Totenschiff. Reiseereignis Bach ohne Brücke (Breida–Tjoila und Auplog–Varnheim): Ohne Hammer (zusätzlich zu 2 Seilen und einer Axt) kann die Gruppe keine Brücke bauen. Der Hammer wird nicht verbraucht. (`Quelle/seg109.c:TRV_barrier`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg109.c:TRV_barrier`
- **Deutung der KI:** Vermutlich nur beim Brückenbau auf Reisen nötig, sonst ohne Wirkung.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-28"></a>
### 28. Angelhaken (`ITEM_ID_ANGELHAKEN`)

- **Im Code:** Gegenstands-ID 28 (`ITEM_ID_ANGELHAKEN`, Rohname in ITEMNAME `ANGELHAKEN..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 1 Unzen; Grundpreis 2 Heller (= 2 Heller). Handel: Sortimentswert 13, damit bei 43 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 13). Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-29"></a>
### 29. Schriftrolle (`ITEM_ID_SCHRIFTROLLE`)

- **Im Code:** Gegenstands-ID 29 (`ITEM_ID_SCHRIFTROLLE`, Rohname in ITEMNAME `SCHRIFTROLLE..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 30 Unzen; Grundpreis 40 Silbertaler (= 400 Heller). Handel: Sortimentswert 10, damit bei 36 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 10). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Reiseereignis Ottarje–Orvil, Piratenhöhle, Totenschiff. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-30"></a>
### 30. Wasserschlauch (`ITEM_ID_WASSERSCHLAUCH`)

- **Im Code:** Gegenstands-ID 30 (`ITEM_ID_WASSERSCHLAUCH`, Rohname in ITEMNAME `WASSERSCHL.AUCH.ÄUCHE`). ITEMS.DAT: Nahrung; Gewicht 40 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Nahrung (Getränk: senkt den Durst um 40 Punkte beim Einnehmen). Handel: Sortimentswert 18, damit bei 55 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 18). Startausrüstung (`g_hero_startup_items`): alle Helden. Liegt in Truhen/Beutetabellen im Code: Verfallene Herberge, Wolfshöhle. Findet ein Held beim Lagern in der Wildnis Wasser (Wildnisleben-Probe oder Efferd-Wunder), werden alle Wasserschläuche der Helden der aktiven Gruppe wieder voll. (`Quelle/seg051.c:replenish_stocks`) An Wasserfässern in Dungeons kann man die Wasserschläuche der aktiven Gruppe füllen: ein halbleerer Schlauch braucht 1 Einheit, ein leerer 2. Im Original wird auch aus einem leeren Fass gefüllt. Mit Fix 91 (Fanpatch-Stufe „Wichtig“) füllt ein leeres Fass nichts mehr, und mit der letzten Einheit wird ein leerer Schlauch nur halb voll. (`Quelle/seg076.c:DNG_waterbarrel`) Für das automatische Trinken sucht das Spiel einen nicht leeren Wasserschlauch nur im Rucksack. Ein Schlauch in der Hand wird im Original übersehen. Mit Fix 223 (Fanpatch-Stufe „nur Alle“) zählt auch der in der Hand. (`Quelle/seg106.c:get_full_waterskin_inv_slot`) Einnehmen (Essen/Trinken-Menü): Ein voller Schlauch wird halb leer, ein halb leerer leer (zweimal trinken pro Füllung). (`Quelle/seg108.c:consume`) Reiseereignis Schilfmeer am Bodir (Vilnheim–Phexcaer): Verirrt sich die Gruppe (Orientierung +3 misslungen), verliert jeder Held mit Wasserschlauch einen davon. (`Quelle/seg110.c:tevent_029`) Reiseereignis Seeufer am Einsiedlersee (Rundweg): Bei jedem Helden der Gruppe wird der erste Wasserschlauch gefüllt (weitere nicht). (`Quelle/seg116.c:tevent_137`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg108.c:consume`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg051.c:replenish_stocks`, `Quelle/seg076.c:DNG_waterbarrel`, `Quelle/seg106.c:get_full_waterskin_inv_slot`, `Quelle/seg110.c:tevent_029`, `Quelle/seg116.c:tevent_137`
- **Deutung der KI:** Vermutlich unverzichtbar, weil die automatische Versorgung daraus trinkt. Ein voller Schlauch reicht für zwei Schlucke.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-31"></a>
### 31. Glasflasche (`ITEM_ID_GLASFLASCHE`)

- **Im Code:** Gegenstands-ID 31 (`ITEM_ID_GLASFLASCHE`, Rohname in ITEMNAME `GLASFLASCHE..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 12 Silbertaler (= 120 Heller). Handel: Sortimentswert 5, damit bei 19 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 5). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (3×). Alchimie-Zutat (1×) für Heiltrank (Rezept ID 186). Alchimie-Zutat (1×) für Wunderkur (Rezept ID 241). Alchimie-Zutat (1×) für Gegengift (Rezept ID 202). Alchimie-Zutat (1×) für Schlaftrunk (Rezept ID 242). Alchimie-Zutat (1×) für Zaubertrank (Rezept ID 206). Alchimie-Zutat (1×) für Zaubertrank (stark) (Rezept ID 243). Alchimie-Zutat (1×) für Miasthmaticum (Rezept ID 244). Alchimie-Zutat (1×) für MU-Elixier (Rezept ID 205). Alchimie-Zutat (1×) für KK-Elixier (Rezept ID 204). Alchimie-Zutat (1×) für Hylailer Feuer (Rezept ID 203). Alchimie: Zurückgegeben, wenn eine Wein- oder Schnapsflasche als Zutat verbraucht wird. (`Quelle/seg104.c:hero_use_ingredients`) Zurückgegeben, wenn ein Waffengift (außer Expurgicum und Vomicum) aufgetragen wird. (`Quelle/seg107.c:item_apply_weapon_poison`) Zurückgegeben beim Einnehmen von Heiltrank, Wunderkur, Schlaftrunk, Zaubertrank (auch stark), Gegengift, Antikrankheitselixier und allen Elixieren. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg104.c:hero_use_ingredients`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich vor allem Leergut für die Alchimie: Die meisten Tränke werden in Glasflaschen gebraut.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-32"></a>
### 32. Strickleiter (`ITEM_ID_STRICKLEITER`)

- **Im Code:** Gegenstands-ID 32 (`ITEM_ID_STRICKLEITER`, Rohname in ITEMNAME `STRICKLEITER..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 70 Unzen; Grundpreis 12 Silbertaler (= 120 Heller). Handel: Sortimentswert 14, damit bei 46 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 14). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Für Abstiege in Dungeons (Fallgruben, Schächte, auch einige Reiseereignisse) reicht ein Seil oder eine Strickleiter bei irgendeinem Helden. Ein Magier der Gruppe mit Stabzauber-Stufe über 2 ersetzt beides. (`Quelle/seg075.c:DNG_check_climb_tools`) Auf dem Totenschiff kann man durch einen Spalt in der Decke nur mit Seil oder Strickleiter nach oben klettern. Mit Fix 92 (Fanpatch-Stufe „Wichtig“) hilft dort auch ein Magier mit Stabzauber ab Stufe 3, wie an den Gruben. (`Quelle/seg077.c:DNG01_handler`) Kultstätte des Namenlosen: Um einen in die Fallgrube gestürzten Helden heraufzuholen, braucht die Gruppe 2 Seile/Strickleitern, oder 1 Seil/Strickleiter plus einen Magier mit Stabzauber über Stufe 2. Mit nur einem Seil: Meldung, dass ein Seil für die Tiefe nicht reicht (`Quelle/seg081.c:DNG09_pitfall`) Zwingfeste: zählt wie ein Seil (Loch auf Feld (2,4,13) und eingestürzter Gang auf (3,10,10)/(3,14,10)). (`Quelle/seg087.c:DNG14_handler`) Reiseereignis Bach ohne Brücke: zählt wie ein Seil (2 nötig, beide werden verbraucht). (`Quelle/seg109.c:TRV_barrier`) Reiseereignis Erdrutsch (Ottarje–Orvil): Im Original macht eine Strickleiter in der Gruppe einen Sturz beim Überklettern SCHWERER (1W9+3 statt 1W8 LE), sinnwidrige Bedingung. Mit Fix 158 (Fanpatch-Stufe „Wichtig“) schützt sie wie ein Seil. (`Quelle/seg111.c:tevent_057`) Reiseereignis Pass mit Abstieg in eine Klamm (Ottarje–Orvil): zählt wie ein Seil (3 nötig, werden verbraucht). (`Quelle/seg111.c:tevent_060`) Reiseereignis Klamm (Rybon–Thoss): Nur mit Seil oder Strickleiter in der Gruppe gibt es die Antwort, sich hinüberzuhangeln. (`Quelle/seg113.c:tevent_098`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg075.c:DNG_check_climb_tools`, `Quelle/seg077.c:DNG01_handler`, `Quelle/seg081.c:DNG09_pitfall`, `Quelle/seg087.c:DNG14_handler`, `Quelle/seg109.c:TRV_barrier`, `Quelle/seg111.c:tevent_057`, `Quelle/seg111.c:tevent_060`, `Quelle/seg113.c:tevent_098`
- **Deutung der KI:** Sieht so aus, als ob die Strickleiter meist wie ein Seil zählt, aber nicht überall. Beim Erdrutsch schadet sie im Original sogar (vermutlich ein Programmierfehler).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-33"></a>
### 33. Wurfaxt (`ITEM_ID_WURFAXT`)

- **Im Code:** Gegenstands-ID 33 (`ITEM_ID_WURFAXT`, Rohname in ITEMNAME `WURF.AXT.ÄXTE`). ITEMS.DAT: Waffe; Gewicht 120 Unzen; Grundpreis 35 Silbertaler (= 350 Heller). Waffe, Waffenart Wurfwaffe (Untertyp 8), Waffentabelle `g_weapon_stats_table[17]`: TP 1W6+3, kein KK-Zuschlag, Bruchfaktor 4, AT -1, PA -4, Fernkampf-Entfernungstabelle Nr. 2. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 12, damit bei 15 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 12). Zählt als Axt für die Reiseereignisse Brückenbau (Breida–Tjoila, Auplog–Varnheim) und umgestürzter Baum (`g_travel_event_axes`). Wird als Wurfwaffe eingesetzt: Nach dem Wurf fliegt die Waffe aus der rechten Hand (liegt in der linken Hand eine gleiche, rückt sie nach). Bis zu 30 geworfene Waffen merkt sich der Kampf und gibt sie nach einem gewonnenen Kampf an die Gruppe zurück (nur die Gegenstandsnummer, nicht ihr Zustand). (`Quelle/seg041.c:range_attack_check_ammo`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg109.c:g_travel_event_axes`, `Quelle/seg041.c:range_attack_check_ammo`
- **Deutung der KI:** Vermutlich eine Wurfwaffe, die auf Reisen zugleich als Axt gilt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-34"></a>
### 34. Messingspiegel (`ITEM_ID_MESSINGSPIEGEL`)

- **Im Code:** Gegenstands-ID 34 (`ITEM_ID_MESSINGSPIEGEL`, Rohname in ITEMNAME `MESSINGSPIEGEL..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 20 Unzen; Grundpreis 12 Silbertaler (= 120 Heller). Handel: Sortimentswert 9, damit bei 28 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 9). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (5×), Zwingfeste. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-35"></a>
### 35. Dietriche (`ITEM_ID_DIETRICHE`)

- **Im Code:** Gegenstands-ID 35 (`ITEM_ID_DIETRICHE`, Rohname in ITEMNAME `DIETRICHE..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 15 Unzen; Grundpreis 120 Silbertaler (= 1200 Heller). Handel: Sortimentswert 3, damit bei 13 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 3). Startausrüstung (`g_hero_startup_items`): Streuner. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Kultstätte des Namenlosen, Totenschiff. Zum Schlösserknacken an Türen und Truhen braucht der erste handlungsfähige Held der Gruppe unzerbrochene Dietriche (Schlösser-Probe mit dem Zuschlag des Schlosses). Bei einem Patzer (zwei Zwanzigen) zerbrechen sie, an Türen auch beim 4. Versuch ohne weiterzugehen. Zerbrochene Dietriche zählen nicht. An Truhen löst ein Misserfolg die Falle aus. (`Quelle/seg092.c:hero_has_lockpicks`, `Quelle/seg092.c:use_lockpicks_on_chest`, `Quelle/seg076.c:DNG_door`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg092.c:hero_has_lockpicks`, `Quelle/seg092.c:use_lockpicks_on_chest`, `Quelle/seg076.c:DNG_door`
- **Deutung der KI:** Vermutlich Pflicht für den Gruppenführer, weil immer der erste handlungsfähige Held das Schloss knackt. Ersatz-Dietriche lohnen sich.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-36"></a>
### 36. Schreibzeug (`ITEM_ID_SCHREIBZEUG`)

- **Im Code:** Gegenstands-ID 36 (`ITEM_ID_SCHREIBZEUG`, Rohname in ITEMNAME `SCHREIBZEUG..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 15 Silbertaler (= 150 Heller). Handel: Sortimentswert 10, damit bei 36 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 10). Startausrüstung (`g_hero_startup_items`): Magier. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Piratenhöhle, Totenschiff. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-37"></a>
### 37. Harfe (`ITEM_ID_HARFE`)

- **Im Code:** Gegenstands-ID 37 (`ITEM_ID_HARFE`, Rohname in ITEMNAME `HARFE..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 40 Unzen; Grundpreis 100 Silbertaler (= 1000 Heller). Handel: Sortimentswert 6, damit bei 20 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 6). Startausrüstung (`g_hero_startup_items`): Auelf. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-38"></a>
### 38. Trinkhorn (`ITEM_ID_TRINKHORN`)

- **Im Code:** Gegenstands-ID 38 (`ITEM_ID_TRINKHORN`, Rohname in ITEMNAME `TRINKH.ORN.ÖRNER`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 20 Unzen; Grundpreis 1 Silbertaler (= 10 Heller). Handel: Sortimentswert 7, damit bei 24 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 7). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-39"></a>
### 39. Silberschmuck (`ITEM_ID_SILBERSCHMUCK`)

- **Im Code:** Gegenstands-ID 39 (`ITEM_ID_SILBERSCHMUCK`, Rohname in ITEMNAME `SILBERSCHMUCK..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 5 Silbertaler (= 50 Heller). Handel: Sortimentswert 10, damit bei 36 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 10). Liegt in Truhen/Beutetabellen im Code: Zwingfeste (2×), Spinnenhöhle (2×), Orkbehausung (2×), Kultstätte des Namenlosen (6×), Piratenhöhle (8×), Totenschiff. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-40"></a>
### 40. Kletterhaken (`ITEM_ID_KLETTERHAKEN`)

- **Im Code:** Gegenstands-ID 40 (`ITEM_ID_KLETTERHAKEN`, Rohname in ITEMNAME `KLETTERHAKEN..`). ITEMS.DAT: stapelbar; Gewicht 5 Unzen; Grundpreis 5 Heller (= 5 Heller). Handel: Sortimentswert 15, damit bei 47 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 15). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Spinnenhöhle. Der Bündel-Gegenstand „20 Kletterhaken“ (ID 253) wird beim Erhalt (z. B. aus einer Truhe) in 20 Stück dieses Gegenstands umgewandelt. (`Quelle/seg105.c:give_new_item_to_group`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg105.c:give_new_item_to_group`
- **Deutung der KI:** Sieht so aus, als ob Kletterhaken im Code nirgends abgefragt werden. Sie sind dann reiner Ballast.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-41"></a>
### 41. Öl (`ITEM_ID_OEL`)

- **Im Code:** Gegenstands-ID 41 (`ITEM_ID_OEL`, Rohname in ITEMNAME `ÖL..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: Sortimentswert 14, damit bei 46 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 14). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (2×), Zwingfeste, Kultstätte des Namenlosen (3×), Totenschiff. Alchimie-Zutat (1×) für Vomicum (Rezept ID 169). Alchimie-Zutat (2×) für Hylailer Feuer (Rezept ID 203). Alchimie: Wird Öl als Zutat verbraucht, bekommt der Brauer eine Bronzeflasche zurück. (`Quelle/seg104.c:hero_use_ingredients`) Wird beim Anzünden einer Laterne verbraucht (eine Bronzeflasche kommt zurück). Ohne Öl lässt sich keine Laterne anzünden. (`Quelle/seg101.c:spell_brenne`) Benutzen einer brennenden Laterne füllt sie mit einer Flasche Öl wieder auf (Brennzähler 100). (`Quelle/seg107.c:item_ignite`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg104.c:hero_use_ingredients`, `Quelle/seg101.c:spell_brenne`, `Quelle/seg107.c:item_ignite`
- **Deutung der KI:** Vermutlich der Brennstoff für Laternen und eine Alchimie-Zutat. Die Flasche bekommt man zurück.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-42"></a>
### 42. Bronzeflasche (`ITEM_ID_BRONZEFLASCHE`)

- **Im Code:** Gegenstands-ID 42 (`ITEM_ID_BRONZEFLASCHE`, Rohname in ITEMNAME `BRONZEFLASCHE..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 50 Unzen; Grundpreis 8 Silbertaler (= 80 Heller). Handel: Sortimentswert 8, damit bei 27 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 8). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Alchimie-Zutat (1×) für Starker Heiltrank (Rezept ID 240). Alchimie-Zutat (1×) für Vomicum (Rezept ID 169). Alchimie-Zutat (1×) für Expurgicum (Rezept ID 167). Zurückgegeben, wenn Öl verbraucht wird (Alchimie-Zutat oder Laterne anzünden). (`Quelle/seg104.c:hero_use_ingredients`, `Quelle/seg101.c:spell_brenne`) Zurückgegeben, wenn Expurgicum oder Vomicum aufgetragen wird. (`Quelle/seg107.c:item_apply_weapon_poison`) Zurückgegeben, wenn eine brennende Laterne mit Öl nachgefüllt wird. (`Quelle/seg107.c:item_ignite`) Zurückgegeben beim Einnehmen eines starken Heiltranks. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg104.c:hero_use_ingredients`, `Quelle/seg101.c:spell_brenne`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg107.c:item_ignite`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich Leergut, das vor allem beim Brauen starker Tränke gebraucht wird.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-43"></a>
### 43. Eisenhelm (`ITEM_ID_EISENHELM`)

- **Im Code:** Gegenstands-ID 43 (`ITEM_ID_EISENHELM`, Rohname in ITEMNAME `EISENHELM..E`). ITEMS.DAT: Rüstung; Gewicht 180 Unzen; Grundpreis 80 Silbertaler (= 800 Heller). Rüstung, Platz Kopf (Untertyp 0), `g_armor_stats_table[2]`: RS 3, BE 2. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-44"></a>
### 44. Pike (`ITEM_ID_PIKE`)

- **Im Code:** Gegenstands-ID 44 (`ITEM_ID_PIKE`, Rohname in ITEMNAME `PIKE..N`). ITEMS.DAT: Waffe; Gewicht 150 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[18]`: TP 1W6+3, KK-Zuschlag ab KK 19 (je Punkt darüber +1 TP), Bruchfaktor 7, AT -1, PA -4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Zwerg, Hexe, Druide, Magier. Handel: Sortimentswert 12, damit bei 15 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 12). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-45"></a>
### 45. Proviantpaket (`ITEM_ID_PROVIANTPAKET`)

- **Im Code:** Gegenstands-ID 45 (`ITEM_ID_PROVIANTPAKET`, Rohname in ITEMNAME `PROVIANTPAKET..E`). ITEMS.DAT: Nahrung, stapelbar; Gewicht 20 Unzen; Grundpreis 3 Silbertaler (= 30 Heller). Nahrung (Essen: senkt den Hunger um 50 Punkte beim Einnehmen). Handel: Sortimentswert 19, damit bei 56 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 19). Startausrüstung (`g_hero_startup_items`): alle Helden. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Zwingfeste, Spinnenhöhle (6×), Kultstätte des Namenlosen (8×). Steigt der Hunger eines Helden über 90, isst er in der stündlichen Versorgung automatisch ein Proviantpaket aus seinem Inventar; war es das letzte, kommt eine Warnmeldung. (`Quelle/seg002.c:herokeeping`) Gelingt beim Lagern in der Wildnis die Jagd (Fährtensuchen-Probe oder Firun-Wunder), bekommt die Gruppe 3 Proviantpakete (wenn Platz ist). (`Quelle/seg051.c:replenish_stocks`) Goblinhöhle: aus einem toten Wildschwein auf Feld (0,5,12) bekommt die Gruppe einmalig 10 Proviantpakete. (`Quelle/seg080.c:DNG05_handler`) Daspotaschatz: In einem Vorratsraum auf Feld (0,5,3) bekommt die Gruppe einmalig 40 Proviantpakete. (`Quelle/seg081.c:DNG06_handler`) Orkbehausung: In der Truhe DNG08_chest00 bekommt die Gruppe einmalig 40 Proviantpakete. (`Quelle/seg083.c:DNG08_chest00_loot`) Truhen mit Nahrungsangabe in der Truhentabelle geben beim ersten Öffnen entsprechend viele Proviantpakete. (`Quelle/seg092.c:loot_special_chest`) Jagd-Reiseereignisse (Wildschwein, Karen u. a.): bei Erfolg bekommt die Gruppe Proviantpakete. (`Quelle/seg109.c:TRV_hunt_generic`) Reiseereignis Schilfmeer am Bodir (Vilnheim–Phexcaer): Verirrt sich die Gruppe (Orientierung +3 misslungen), verliert jeder Held 2 LE und seinen ersten Stapel Proviantpakete. (`Quelle/seg110.c:tevent_029`) Reiseereignis Nashornjagd (Skelellen–Phexcaer): Bei erlegtem Nashorn bekommt die Gruppe 200 oder 180 Proviantpakete (je nach Verlauf). (`Quelle/seg114.c:tevent_111`) Reiseereignis Seeufer am Einsiedlersee (Rundweg): Jeder Held der Gruppe bekommt 5 Proviantpakete. (`Quelle/seg116.c:tevent_137`) Jagd-Reiseereignisse: Karene geben 80 Proviantpakete (mindestens die halbe Gruppe trifft) oder 40 (wenigstens einer trifft), Wildschweine 30. (`Quelle/seg117.c:hunt_karen`, `Quelle/seg117.c:hunt_wildboar`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg108.c:consume`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg002.c:herokeeping`, `Quelle/seg051.c:replenish_stocks`, `Quelle/seg080.c:DNG05_handler`, `Quelle/seg081.c:DNG06_handler`, `Quelle/seg083.c:DNG08_chest00_loot`, `Quelle/seg092.c:loot_special_chest`, `Quelle/seg109.c:TRV_hunt_generic`, `Quelle/seg110.c:tevent_029`, `Quelle/seg114.c:tevent_111`, `Quelle/seg116.c:tevent_137`, `Quelle/seg117.c:hunt_karen`, `Quelle/seg117.c:hunt_wildboar`
- **Deutung der KI:** Vermutlich das wichtigste Nahrungsmittel. Die automatische Versorgung isst es, sobald der Hunger über 90 steigt, und es gibt viele Quellen (Jagd, Truhen, Reiseereignisse).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-46"></a>
### 46. Flöte (`ITEM_ID_FLOETE`)

- **Im Code:** Gegenstands-ID 46 (`ITEM_ID_FLOETE`, Rohname in ITEMNAME `FLÖTE..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 8 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Handel: Sortimentswert 9, damit bei 28 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 9). Startausrüstung (`g_hero_startup_items`): Firnelf, Waldelf. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (2×), Wolfshöhle. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-47"></a>
### 47. Alchimieset (`ITEM_ID_ALCHIMIESET`)

- **Im Code:** Gegenstands-ID 47 (`ITEM_ID_ALCHIMIESET`, Rohname in ITEMNAME `ALCHIMIESET..S`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 100 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Handel: Sortimentswert 2, damit bei 11 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 2). Liegt in Truhen/Beutetabellen im Code: Verfallene Herberge (3×), Piratenhöhle. Ohne Alchimieset im Inventar kann ein Held keine Alchimie betreiben (Meldung: keine Alchimie-Ausrüstung dabei). Das Set wird nicht verbraucht. (`Quelle/seg104.c:plan_alchemy`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:plan_alchemy`
- **Deutung der KI:** Vermutlich die Grundvoraussetzung fürs Brauen. Es wird nicht verbraucht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-48"></a>
### 48. Hemd (`ITEM_ID_HEMD`)

- **Im Code:** Gegenstands-ID 48 (`ITEM_ID_HEMD`, Rohname in ITEMNAME `HEMD..EN`). ITEMS.DAT: Rüstung; Gewicht 30 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[3]`: RS 1, BE 0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 20, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 20). Liegt in Truhen/Beutetabellen im Code: Kultstätte des Namenlosen (3×), Piratenhöhle. Startausrüstung: Weibliche Helden außer Kriegerinnen und Magierinnen bekommen zusätzlich ein Hemd, am Körper angelegt. (`Quelle/seg106.c:startup_equipment`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg106.c:startup_equipment`
- **Deutung der KI:** Vermutlich ein einfaches Kleidungsstück. Im Code taucht es nur in der Startausrüstung weiblicher Helden auf.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-49"></a>
### 49. Hose (`ITEM_ID_HOSE`)

- **Im Code:** Gegenstands-ID 49 (`ITEM_ID_HOSE`, Rohname in ITEMNAME `HOSE..N`). ITEMS.DAT: Rüstung; Gewicht 30 Unzen; Grundpreis 3 Silbertaler (= 30 Heller). Rüstung, Platz Beine (Untertyp 5), `g_armor_stats_table[4]`: RS 0, BE 0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 20, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 20). Startausrüstung (`g_hero_startup_items`): alle Helden. Liegt in Truhen/Beutetabellen im Code: Kultstätte des Namenlosen (3×), Piratenhöhle. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-50"></a>
### 50. Schuhe (`ITEM_ID_SCHUHE`)

- **Im Code:** Gegenstands-ID 50 (`ITEM_ID_SCHUHE`, Rohname in ITEMNAME `SCHUHE..`). ITEMS.DAT: Rüstung; Gewicht 40 Unzen; Grundpreis 1 Silbertaler (= 10 Heller). Rüstung, Platz Füße (Untertyp 6), `g_armor_stats_table[5]`: RS 0, BE 0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 20, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 20). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-51"></a>
### 51. Stiefel (`ITEM_ID_STIEFEL`)

- **Im Code:** Gegenstands-ID 51 (`ITEM_ID_STIEFEL`, Rohname in ITEMNAME `STIEFEL..`). ITEMS.DAT: Rüstung; Gewicht 75 Unzen; Grundpreis 9 Silbertaler (= 90 Heller). Rüstung, Platz Füße (Untertyp 6), `g_armor_stats_table[6]`: RS 0, BE 0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 18, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 18). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-52"></a>
### 52. Schneeschuhe (`ITEM_ID_SCHNEESCHUHE`)

- **Im Code:** Gegenstands-ID 52 (`ITEM_ID_SCHNEESCHUHE`, Rohname in ITEMNAME `SCHNEESCHUHE..`). ITEMS.DAT: Rüstung; Gewicht 40 Unzen; Grundpreis 5 Silbertaler (= 50 Heller). Rüstung, Platz Füße (Untertyp 6), `g_armor_stats_table[7]`: RS 0, BE 0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 9, damit bei 10 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 9). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-53"></a>
### 53. Lederharnisch (`ITEM_ID_LEDERHARNISCH`)

- **Im Code:** Gegenstands-ID 53 (`ITEM_ID_LEDERHARNISCH`, Rohname in ITEMNAME `LEDERHARNISCH..E`). ITEMS.DAT: Rüstung; Gewicht 180 Unzen; Grundpreis 80 Silbertaler (= 800 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[8]`: RS 3, BE 3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Magier. Handel: Sortimentswert 18, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 18). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Startausrüstung (`g_hero_startup_items`): Krieger. Liegt in Truhen/Beutetabellen im Code: Daspotaschatz (2×). Startausrüstung: Der Krieger bekommt den Lederharnisch direkt am Körper angelegt. (`Quelle/seg106.c:startup_equipment`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich die Startrüstung des Kriegers, sonst eine normale Rüstung.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-54"></a>
### 54. Schuppenpanzer (`ITEM_ID_SCHUPPENRUESTUNG`)

- **Im Code:** Gegenstands-ID 54 (`ITEM_ID_SCHUPPENRUESTUNG`, Rohname in ITEMNAME `SCHUPPENPANZER..`). ITEMS.DAT: Rüstung; Gewicht 480 Unzen; Grundpreis 100 Dukaten (= 10000 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[9]`: RS 5, BE 4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 12, damit bei 15 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 12). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (2×). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-55"></a>
### 55. Shurinknollengift (`ITEM_ID_SHURINKNOLLEKNOLLENGIFT`)

- **Im Code:** Gegenstands-ID 55 (`ITEM_ID_SHURINKNOLLEKNOLLENGIFT`, Rohname in ITEMNAME `SHURINKNOLLENGIFT..`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 70 Dukaten (= 7000 Heller). Benutzbar-Eintrag 8 (`g_usable_items_table`): Benutzen ruft `item_apply_weapon_poison()` auf. Gruppe Kraut/Gift (Untertyp 0). Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Krieger. Handel: Sortimentswert 15, damit bei 16 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 15). Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt Benutzen: wird auf den Gegenstand in der rechten Hand aufgetragen. Das geht mit jedem Gegenstand dort (das Spiel prüft nicht, ob es eine Waffe ist), nur nicht mit Kurzbogen, Langbogen und Armbrust (mit Fix 224 (Fanpatch-Stufe „nur Alle“) auch nicht mit der Schleuder). Mit leerer rechter Hand: Meldung: nichts zum Vergiften in der rechten Hand. Mit Fix 400 (Fanpatch-Stufe „nur Alle“) gilt auch hier die Klassensperre (z. B. für Krieger). Die Waffe bekommt 5 Ladungen: Jeder Treffer mit mindestens 1 Schadenspunkt richtet zusätzlich 10 × Giftstärke 6 = 60 Schadenspunkte an und verbraucht eine Ladung. Das Gift ist verbraucht, zurück kommt eine Glasflasche. (`Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg108.c:consume`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob dieses Gift wegen „10 × Giftstärke“ extrem viel Zusatzschaden macht (Code-Kommentar: „totally excessive and unbalanced“).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-56"></a>
### 56. Araxgift (`ITEM_ID_ARAXGIFT`)

- **Im Code:** Gegenstands-ID 56 (`ITEM_ID_ARAXGIFT`, Rohname in ITEMNAME `ARAXGIFT..`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 35 Dukaten (= 3500 Heller). Benutzbar-Eintrag 8 (`g_usable_items_table`): Benutzen ruft `item_apply_weapon_poison()` auf. Gruppe Kraut/Gift (Untertyp 0). Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Krieger. Handel: Sortimentswert 14, damit bei 14 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Liegt in Truhen/Beutetabellen im Code: Spinnenhöhle (10×). Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt Benutzen: wird auf den Gegenstand in der rechten Hand aufgetragen. Das geht mit jedem Gegenstand dort (das Spiel prüft nicht, ob es eine Waffe ist), nur nicht mit Kurzbogen, Langbogen und Armbrust (mit Fix 224 (Fanpatch-Stufe „nur Alle“) auch nicht mit der Schleuder). Mit leerer rechter Hand: Meldung: nichts zum Vergiften in der rechten Hand. Mit Fix 400 (Fanpatch-Stufe „nur Alle“) gilt auch hier die Klassensperre (z. B. für Krieger). Die Waffe bekommt 5 Ladungen: Jeder Treffer mit mindestens 1 Schadenspunkt richtet zusätzlich 10 × Giftstärke 5 = 50 Schadenspunkte an und verbraucht eine Ladung. Das Gift ist verbraucht, zurück kommt eine Glasflasche. (`Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg108.c:consume`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob Arax im Code nur Zusatzschaden macht (10 × Giftstärke) und keine besondere Wirkung hat.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-57"></a>
### 57. Angstgift (`ITEM_ID_ANGSTGIFT`)

- **Im Code:** Gegenstands-ID 57 (`ITEM_ID_ANGSTGIFT`, Rohname in ITEMNAME `ANGSTGIFT..`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 25 Dukaten (= 2500 Heller). Benutzbar-Eintrag 8 (`g_usable_items_table`): Benutzen ruft `item_apply_weapon_poison()` auf. Gruppe Kraut/Gift (Untertyp 0). Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Krieger. Handel: Sortimentswert 9, damit bei 9 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 9). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt Benutzen: wird auf den Gegenstand in der rechten Hand aufgetragen. Das geht mit jedem Gegenstand dort (das Spiel prüft nicht, ob es eine Waffe ist), nur nicht mit Kurzbogen, Langbogen und Armbrust (mit Fix 224 (Fanpatch-Stufe „nur Alle“) auch nicht mit der Schleuder). Mit leerer rechter Hand: Meldung: nichts zum Vergiften in der rechten Hand. Mit Fix 400 (Fanpatch-Stufe „nur Alle“) gilt auch hier die Klassensperre (z. B. für Krieger). Die Waffe bekommt 5 Ladungen: Jeder Treffer mit mindestens 1 Schadenspunkt macht den getroffenen Gegner ängstlich (er flieht) und hebt eine Beherrschung auf, statt Zusatzschaden anzurichten. Jeder solche Treffer verbraucht eine Ladung. Im Original wird bei Held gegen Held dabei über einen nicht gesetzten Zeiger geschrieben, Fix 30 (Fanpatch-Stufe „Wichtig“) lässt die Wirkung dort weg. Zurück kommt eine Glasflasche. (`Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg108.c:consume`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob Angstgift keinen Schaden macht, sondern Gegner in die Flucht schlägt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-58"></a>
### 58. Schlafgift (`ITEM_ID_SCHLAFGIFT`)

- **Im Code:** Gegenstands-ID 58 (`ITEM_ID_SCHLAFGIFT`, Rohname in ITEMNAME `SCHLAFGIFT..`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 35 Dukaten (= 3500 Heller). Benutzbar-Eintrag 8 (`g_usable_items_table`): Benutzen ruft `item_apply_weapon_poison()` auf. Gruppe Kraut/Gift (Untertyp 0). Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Krieger. Handel: Sortimentswert 7, damit bei 5 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 7). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt Benutzen: wird auf den Gegenstand in der rechten Hand aufgetragen. Das geht mit jedem Gegenstand dort (das Spiel prüft nicht, ob es eine Waffe ist), nur nicht mit Kurzbogen, Langbogen und Armbrust (mit Fix 224 (Fanpatch-Stufe „nur Alle“) auch nicht mit der Schleuder). Mit leerer rechter Hand: Meldung: nichts zum Vergiften in der rechten Hand. Mit Fix 400 (Fanpatch-Stufe „nur Alle“) gilt auch hier die Klassensperre (z. B. für Krieger). Die Waffe bekommt 5 Ladungen: Jeder Treffer mit mindestens 1 Schadenspunkt richtet zusätzlich 10 × Giftstärke 5 = 50 Schadenspunkte an und verbraucht eine Ladung. Das Gift ist verbraucht, zurück kommt eine Glasflasche. (`Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg108.c:consume`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob Schlafgift im Code nicht einschläfert, sondern nur Zusatzschaden macht (10 × Giftstärke).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-59"></a>
### 59. Goldleim (`ITEM_ID_GOLDLEIM`)

- **Im Code:** Gegenstands-ID 59 (`ITEM_ID_GOLDLEIM`, Rohname in ITEMNAME `GOLDLEIM..`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 30 Dukaten (= 3000 Heller). Benutzbar-Eintrag 8 (`g_usable_items_table`): Benutzen ruft `item_apply_weapon_poison()` auf. Gruppe Kraut/Gift (Untertyp 0). Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Krieger. Handel: Sortimentswert 3, damit bei 1 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 3). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt Benutzen: wird auf den Gegenstand in der rechten Hand aufgetragen. Das geht mit jedem Gegenstand dort (das Spiel prüft nicht, ob es eine Waffe ist), nur nicht mit Kurzbogen, Langbogen und Armbrust (mit Fix 224 (Fanpatch-Stufe „nur Alle“) auch nicht mit der Schleuder). Mit leerer rechter Hand: Meldung: nichts zum Vergiften in der rechten Hand. Mit Fix 400 (Fanpatch-Stufe „nur Alle“) gilt auch hier die Klassensperre (z. B. für Krieger). Die Waffe bekommt 5 Ladungen: Jeder Treffer mit mindestens 1 Schadenspunkt richtet zusätzlich 10 × Giftstärke 5 = 50 Schadenspunkte an und verbraucht eine Ladung. Das Gift ist verbraucht, zurück kommt eine Glasflasche. (`Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg108.c:consume`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob Goldleim im Code nur Zusatzschaden macht (10 × Giftstärke) und keine besondere Wirkung hat.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-60"></a>
### 60. Vierblättrige Einbeere (`ITEM_ID_EINBEERE`)

- **Im Code:** Gegenstands-ID 60 (`ITEM_ID_EINBEERE`, Rohname in ITEMNAME `VIERBLÄTTRIGE EINBEERE..N`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 7 Silbertaler (= 70 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 20, damit bei 17 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 20). Startausrüstung (`g_hero_startup_items`): Hexe. Alchimie-Zutat (2×) für Heiltrank (Rezept ID 186). Kräutersuche beim Lagern (`g_gather_herbs_table`): 40 % Chance, dann Pflanzenkunde-Probe +8 (leichter je Suchstunde), bei Erfolg 1 bis 4 Stück. Kräuterplatz im Reiseereignis Nr. 9 (Breida–Peilinen): Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Kräuterplatz im Reiseereignis Nr. 118 (Merske–Rovamund): Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Heilkunde Krankheiten: Der Heiler braucht das passende Kraut im eigenen Inventar, sonst Meldung: die nötigen Kräuter fehlen. Das Kraut wird nur bei gelungener Heilung verbraucht. Einbeere hilft gegen Dumpfschädel. (`Quelle/seg104.c:has_herb_for_disease`) Einnehmen (Essen/Trinken-Menü): Heilt 1W6 LE (höchstens bis zum Maximum). Wird verbraucht. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg109.c:tevent_009`, `Quelle/seg114.c:tevent_118`, `Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein günstiges Heilkraut: ein paar LE beim Essen, Zutat für Heiltränke, hilft gegen Dumpfschädel.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-61"></a>
### 61. Wirselkraut (`ITEM_ID_WIRSELKRAUT`)

- **Im Code:** Gegenstands-ID 61 (`ITEM_ID_WIRSELKRAUT`, Rohname in ITEMNAME `WIRSELKR.AUT.ÄUTER`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 7 Dukaten (= 700 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 19, damit bei 17 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 19). Startausrüstung (`g_hero_startup_items`): Druide. Alchimie-Zutat (2×) für Starker Heiltrank (Rezept ID 240). Alchimie-Zutat (2×) für Wunderkur (Rezept ID 241). Kräutersuche beim Lagern (`g_gather_herbs_table`): 30 % Chance, dann Pflanzenkunde-Probe +4 (leichter je Suchstunde), bei Erfolg 1 bis 2 Stück. Kräuterplatz im Reiseereignis Nr. 83: Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Kräuterplatz im Reiseereignis Nr. 132 (Brendhil–Manrin): Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). In der Apotheke von Phexcaer bekommt man auf die Frage nach Heilkräutern für 10 D ein Wirselkraut. (`Quelle/seg070.c:PHX_apotheke`) Heilkunde Krankheiten: Der Heiler braucht das passende Kraut im eigenen Inventar, sonst Meldung: die nötigen Kräuter fehlen. Das Kraut wird nur bei gelungener Heilung verbraucht. Wirselkraut hilft gegen Frostschäden (einziges Mittel) und gegen Dumpfschädel. (`Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg104.c:talent_cure_disease`) Einnehmen (Essen/Trinken-Menü): Heilt 10 LE (höchstens bis zum Maximum). Wird verbraucht. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg113.c:tevent_083`, `Quelle/seg116.c:tevent_132`, `Quelle/seg070.c:PHX_apotheke`, `Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg104.c:talent_cure_disease`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich eines der nützlichsten Kräuter: 10 LE beim Essen, das einzige Mittel gegen Frostschäden, Zutat für starke Heiltränke.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-62"></a>
### 62. Eitriger Krötenschemel (`ITEM_ID_EITRIGER_KROETENSCHEMEL`)

- **Im Code:** Gegenstands-ID 62 (`ITEM_ID_EITRIGER_KROETENSCHEMEL`, Rohname in ITEMNAME `EITRIGE.R KRÖTENSCHEMEL. KRÖTENSCHEMEL`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 15 Dukaten (= 1500 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 18, damit bei 17 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 18). Kräutersuche beim Lagern (`g_gather_herbs_table`): 20 % Chance, dann Pflanzenkunde-Probe +4 (leichter je Suchstunde), bei Erfolg 1 bis 2 Stück. Kräuterplatz im Reiseereignis Nr. 32: Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg110.c:tevent_032`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich vor allem Handelsware oder Alchimie-Zutat; eine eigene Wirkung ist im Code nicht zu sehen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-63"></a>
### 63. Gulmond Blatt (`ITEM_ID_GULMOND_BLATT`)

- **Im Code:** Gegenstands-ID 63 (`ITEM_ID_GULMOND_BLATT`, Rohname in ITEMNAME `GULMOND BL.ATT.ÄTTER`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 6 Dukaten (= 600 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 16, damit bei 17 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Alchimie-Zutat (1×) für Schlaftrunk (Rezept ID 242). Alchimie-Zutat (2×) für KK-Elixier (Rezept ID 204). Kräutersuche beim Lagern (`g_gather_herbs_table`): 15 % Chance, dann Pflanzenkunde-Probe +8 (leichter je Suchstunde), bei Erfolg 1 bis 3 Stück. Kräuterplatz im Reiseereignis Nr. 36: Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Ein Kräuterplatz mit besseren Chancen für dieses Kraut ist im Code vorhanden (`tevent_unused_herb_swim`), wird laut Code-Kommentar aber nie aufgerufen. Heilkunde Krankheiten: Der Heiler braucht das passende Kraut im eigenen Inventar, sonst Meldung: die nötigen Kräuter fehlen. Das Kraut wird nur bei gelungener Heilung verbraucht. Gulmondblatt hilft nur zusammen mit einer Jorugawurzel gegen Schlachtenfieber (dann werden beide verbraucht). (`Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg104.c:talent_cure_disease`) Einnehmen (Essen/Trinken-Menü): KK +2 für 12 Stunden und sofort LE +2 (Meldung über die stärkende Wirkung). Wird verbraucht. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg110.c:tevent_036`, `Quelle/seg112.c:tevent_unused_herb_swim`, `Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg104.c:talent_cure_disease`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein Stärkungskraut (KK +2 für 12 Stunden) und zusammen mit Jorugawurzel das Mittel gegen Schlachtenfieber.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-64"></a>
### 64. Tarnele (`ITEM_ID_TARNELE`)

- **Im Code:** Gegenstands-ID 64 (`ITEM_ID_TARNELE`, Rohname in ITEMNAME `TARNELE..N`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 5 Silbertaler (= 50 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 19, damit bei 17 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 19). Alchimie-Zutat (1×) für Heiltrank (Rezept ID 186). Alchimie-Zutat (1×) für Starker Heiltrank (Rezept ID 240). Alchimie-Zutat (1×) für Schlaftrunk (Rezept ID 242). Kräutersuche beim Lagern (`g_gather_herbs_table`): 45 % Chance, dann Pflanzenkunde-Probe +4 (leichter je Suchstunde), bei Erfolg 1 bis 5 Stück. Heilkunde Krankheiten: Der Heiler braucht das passende Kraut im eigenen Inventar, sonst Meldung: die nötigen Kräuter fehlen. Das Kraut wird nur bei gelungener Heilung verbraucht. Tarnele hilft gegen Dumpfschädel. (`Quelle/seg104.c:has_herb_for_disease`) Einnehmen (Essen/Trinken-Menü): Doppelte Regeneration in der nächsten Nacht. Es kommt keine Meldung (im Original kann ein alter Text erscheinen, Fix 226 (Fanpatch-Stufe „nur Alle“) zeigt dann nichts). Wird verbraucht. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein Schlafkraut (doppelte Regeneration in der nächsten Nacht) und häufige Alchimie-Zutat.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-65"></a>
### 65. Fackel (nicht brennend) (`ITEM_ID_FACKEL__UNLIT`)

- **Im Code:** Gegenstands-ID 65 (`ITEM_ID_FACKEL__UNLIT`, Rohname in ITEMNAME `FACKEL..N`). ITEMS.DAT: benutzbar; Gewicht 20 Unzen; Grundpreis 5 Heller (= 5 Heller). Benutzbar-Eintrag 12 (`g_usable_items_table`): Benutzen ruft `item_ignite()` auf. Handel: Sortimentswert 19, damit bei 56 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 19). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Verlassene Mine (4×), Zwingfeste. Zwingfeste: in einer Nische auf Feld (2,2,14) liegen 4 Fackeln (einmalig). (`Quelle/seg087.c:DNG14_handler`) Anzünden (Zauber „Brenne toter Stoff“, Zunderkästchen oder Benutzen der Fackel): wird zur brennenden Fackel (ID 22) mit Brennzähler 10 (= 10 × 15 Minuten = 2,5 Stunden). Im Original wird beim Benutzen immer die erste Fackel im Inventar angezündet, nicht die gewählte. Fix 210 (Fanpatch-Stufe „nur Alle“) zündet die gewählte an. (`Quelle/seg101.c:spell_brenne`) Benutzen: braucht ein Zunderkästchen im Inventar und zündet die Fackel dann an. Im Original findet der Zauber „Brenne toter Stoff“ danach nur noch Fackeln (der Anzündmodus wird nicht zurückgesetzt), Fix 211 (Fanpatch-Stufe „Wichtig“) behebt das. (`Quelle/seg107.c:item_ignite`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_ignite`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg087.c:DNG14_handler`, `Quelle/seg101.c:spell_brenne`
- **Deutung der KI:** Vermutlich die einfache Lichtquelle: brennt 2,5 Stunden und wird dabei dunkler.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-66"></a>
### 66. Streitkolben (`ITEM_ID_STREITKOLBEN`)

- **Im Code:** Gegenstands-ID 66 (`ITEM_ID_STREITKOLBEN`, Rohname in ITEMNAME `STREITKOLBEN..`). ITEMS.DAT: Waffe; Gewicht 110 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[19]`: TP 1W6+4, KK-Zuschlag ab KK 13 (je Punkt darüber +1 TP), Bruchfaktor 1, AT +0, PA -2. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Startausrüstung (`g_hero_startup_items`): Zwerg. Liegt in Truhen/Beutetabellen im Code: Orkbehausung. Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit stumpfer Waffe dargestellt. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-67"></a>
### 67. Degen (`ITEM_ID_DEGEN`)

- **Im Code:** Gegenstands-ID 67 (`ITEM_ID_DEGEN`, Rohname in ITEMNAME `DEGEN..`). ITEMS.DAT: Waffe; Gewicht 35 Unzen; Grundpreis 55 Silbertaler (= 550 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[20]`: TP 1W6+3, KK-Zuschlag ab KK 16 (je Punkt darüber +1 TP), Bruchfaktor 3, AT +0, PA -1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 14, damit bei 16 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Liegt in Truhen/Beutetabellen im Code: Totenschiff. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-68"></a>
### 68. Florett (`ITEM_ID_FLORETT`)

- **Im Code:** Gegenstands-ID 68 (`ITEM_ID_FLORETT`, Rohname in ITEMNAME `FLORETT..E`). ITEMS.DAT: Waffe; Gewicht 30 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[21]`: TP 1W6+3, KK-Zuschlag ab KK 16 (je Punkt darüber +1 TP), Bruchfaktor 3, AT +0, PA -1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 14, damit bei 16 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-69"></a>
### 69. Kampfstab (`ITEM_ID_KAMPFSTAB`)

- **Im Code:** Gegenstands-ID 69 (`ITEM_ID_KAMPFSTAB`, Rohname in ITEMNAME `KAMPFST.AB.ÄBE`). ITEMS.DAT: Waffe; Gewicht 70 Unzen; Grundpreis 15 Silbertaler (= 150 Heller). Waffe, Waffenart Speer/Stab (Untertyp 5), Waffentabelle `g_weapon_stats_table[22]`: TP 1W6+1, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 5, AT +0, PA -1. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 10, damit bei 13 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 10). Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit stumpfer Waffe dargestellt. Hat den Waffen-Untertyp Speer, wird aber ausdrücklich nicht wie ein Speer als Wurf-/Fernkampfwaffe behandelt. (`Quelle/seg039.c:FIG_weapon_gfx_id_ranged`) Zählt im Nahkampf trotz Untertyp Speer als normale Nahkampfwaffe. (`Quelle/seg041.c:FIG_weapon_gfx_id_melee`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`, `Quelle/seg039.c:FIG_weapon_gfx_id_ranged`
- **Deutung der KI:** Vermutlich eine reine Nahkampfwaffe, auch wenn sie in den Daten unter Speer steht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-70"></a>
### 70. Kristallkugel (`ITEM_ID_KRISTALLKUGEL`)

- **Im Code:** Gegenstands-ID 70 (`ITEM_ID_KRISTALLKUGEL`, Rohname in ITEMNAME `KRISTALLKUGEL..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 50 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Handel: Sortimentswert 4, damit bei 16 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 4). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ottarje–Orvil. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 2 und gibt einen eigenen Text aus (SPELLTXT Nr. 79, sinngemäß: hilft, drohende Gefahren rechtzeitig zu bemerken). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Drachenhort: Lohn des Drachen für den Platinschlüssel. (`Quelle/seg085.c:DNG10_handler`) Angelegt (an einem Körperplatz oder in einer Hand): Talent Gefahrensinn +2, beim Ablegen wieder -2. (`Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg085.c:DNG10_handler`, `Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`
- **Deutung der KI:** Vermutlich ein Lohn des Drachen. Angelegt verbessert sie den Gefahrensinn.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-71"></a>
### 71. Peitsche (`ITEM_ID_PEITSCHE`)

- **Im Code:** Gegenstands-ID 71 (`ITEM_ID_PEITSCHE`, Rohname in ITEMNAME `PEITSCHE..N`). ITEMS.DAT: Waffe; Gewicht 60 Unzen; Grundpreis 30 Silbertaler (= 300 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[23]`: TP 1W6, KK-Zuschlag ab KK 19 (je Punkt darüber +1 TP), Bruchfaktor 2, AT +0, PA -6. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 8, damit bei 10 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 8). Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit stumpfer Waffe dargestellt. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-72"></a>
### 72. Decke (`ITEM_ID_DECKE`)

- **Im Code:** Gegenstands-ID 72 (`ITEM_ID_DECKE`, Rohname in ITEMNAME `DECKE..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 60 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Handel: Sortimentswert 20, damit bei 56 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 20). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Verfallene Herberge, Wolfshöhle. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-73"></a>
### 73. Schaufel (`ITEM_ID_SCHAUFEL`)

- **Im Code:** Gegenstands-ID 73 (`ITEM_ID_SCHAUFEL`, Rohname in ITEMNAME `SCHAUFEL..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 80 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: Sortimentswert 16, damit bei 50 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 16). Liegt in Truhen/Beutetabellen im Code: Verlassene Mine, Zwingfeste, Spinnenhöhle, Wolfshöhle, Piratenhöhle. Gehört zum Werkzeugsatz Schaufel, Hacke, Brecheisen und Wurfbeil. Nur wenn die Gruppe alle vier dabei hat, gilt sie als ausgerüstet. Verlassene Mine: Gang freiräumen 2 statt 6 Stunden. Einsturzstelle (schwer): mit Werkzeug 12, ohne 18 Stunden. Einsturzstelle (leicht): im Original dauert es MIT Werkzeug einen Tag und ohne 6 Stunden (vertauscht), mit Fix 122 (Fanpatch-Stufe „Wichtig“) richtig herum. Hyggeliks Ruine, Gang freiräumen: im Original mit Werkzeug 1 Stunde, ohne 30 Minuten (vertauscht), mit Fix 122 (Fanpatch-Stufe „Wichtig“) richtig herum. (`Quelle/seg091.c:DNG13_unblock_passage`, `Quelle/seg091.c:DNG13_collapsing_ceiling`, `Quelle/seg091.c:DNG13_collapsing_ceiling_hard`, `Quelle/seg089.c:DNG15_clear_way`) Reiseereignis Schneewächte (Orkanger–Clanegh, nur im Winter): Den Weg freischaufeln dauert mit Schaufel 5 Stunden, ohne 8 Stunden. (`Quelle/seg118.c:tevent_078`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg091.c:DNG13_unblock_passage`, `Quelle/seg091.c:DNG13_collapsing_ceiling`, `Quelle/seg091.c:DNG13_collapsing_ceiling_hard`, `Quelle/seg089.c:DNG15_clear_way`, `Quelle/seg118.c:tevent_078`
- **Deutung der KI:** Sieht so aus, als ob die Schaufel nur zusammen mit Hacke, Brecheisen und Wurfbeil und bei der Schneewächte hilft.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-74"></a>
### 74. Goldschmuck (`ITEM_ID_GOLDSCHMUCK`)

- **Im Code:** Gegenstands-ID 74 (`ITEM_ID_GOLDSCHMUCK`, Rohname in ITEMNAME `GOLDSCHMUCK..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 15 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Handel: Sortimentswert 7, damit bei 24 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 7). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (4×), Zwingfeste (3×), Daspotaschatz (2×), Orkbehausung, Kultstätte des Namenlosen (5×), Piratenhöhle (4×), Totenschiff. Hyggeliks Ruine: In der verfluchten Schatztruhe bekommt die Gruppe 50 Goldschmuck und 189,28 D, ist danach aber verflucht (gs_dng15_took_cursed_money). (`Quelle/seg089.c:DNG15_cursed_money_chest`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg089.c:DNG15_cursed_money_chest`
- **Deutung der KI:** Vermutlich vor allem Verkaufsware. Der Fund in Hyggeliks Ruine bringt aber einen Fluch.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-75"></a>
### 75. Robe (grün, Fassung 1) (`ITEM_ID_ROBE__GREEN_1`)

- **Im Code:** Gegenstands-ID 75 (`ITEM_ID_ROBE__GREEN_1`, Rohname in ITEMNAME `ROBE..N`). ITEMS.DAT: Rüstung; Gewicht 50 Unzen; Grundpreis 5 Silbertaler (= 50 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[10]`: RS 0, BE 0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 17, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 17). Startausrüstung (`g_hero_startup_items`): Magier. Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ottarje–Orvil, Spinnenhöhle (6×), Kultstätte des Namenlosen (3×). Startausrüstung: Der Magier bekommt die grüne Robe direkt am Körper angelegt. (`Quelle/seg106.c:startup_equipment`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich die Startkleidung des Magiers.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-76"></a>
### 76. Robe (rot) (`ITEM_ID_ROBE__RED`)

- **Im Code:** Gegenstands-ID 76 (`ITEM_ID_ROBE__RED`, Rohname in ITEMNAME `ROBE..N`). ITEMS.DAT: Rüstung; Gewicht 200 Unzen; Grundpreis 200 Silbertaler (= 2000 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[11]`: RS 0, BE 0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 15, damit bei 17 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 15). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (3×), Zwingfeste, Spinnenhöhle (6×), Kultstätte des Namenlosen (4×), Piratenhöhle (2×). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-77"></a>
### 77. Topfhelm (`ITEM_ID_TOPFHELM`)

- **Im Code:** Gegenstands-ID 77 (`ITEM_ID_TOPFHELM`, Rohname in ITEMNAME `TOPFHELM..E`). ITEMS.DAT: Rüstung; Gewicht 100 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Rüstung, Platz Kopf (Untertyp 0), `g_armor_stats_table[12]`: RS 2, BE 1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 14, damit bei 16 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Verlassene Mine. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-78"></a>
### 78. Lederhelm (`ITEM_ID_LEDERHELM`)

- **Im Code:** Gegenstands-ID 78 (`ITEM_ID_LEDERHELM`, Rohname in ITEMNAME `LEDERHELM..E`). ITEMS.DAT: Rüstung; Gewicht 90 Unzen; Grundpreis 40 Silbertaler (= 400 Heller). Rüstung, Platz Kopf (Untertyp 0), `g_armor_stats_table[13]`: RS 1, BE 0. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Magier. Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Zwergenfeste (2×), Daspotaschatz (2×). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-79"></a>
### 79. Waffenrock (`ITEM_ID_WAFFENROCK`)

- **Im Code:** Gegenstands-ID 79 (`ITEM_ID_WAFFENROCK`, Rohname in ITEMNAME `WAFFENR.OCK.ÖCKE`). ITEMS.DAT: Rüstung; Gewicht 120 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[14]`: RS 2, BE 2. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-80"></a>
### 80. Kettenhemd (`ITEM_ID_KETTENHEMD`)

- **Im Code:** Gegenstands-ID 80 (`ITEM_ID_KETTENHEMD`, Rohname in ITEMNAME `KETTENHEMD..E`). ITEMS.DAT: Rüstung; Gewicht 320 Unzen; Grundpreis 200 Silbertaler (= 2000 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[15]`: RS 4, BE 4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Hexe, Druide, Magier, Firnelf, Waldelf. Handel: Sortimentswert 12, damit bei 15 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 12). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Zwergenfeste (2×), Wolfshöhle, Daspotaschatz, Drachenhort. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-81"></a>
### 81. Kroetenhaut (`ITEM_ID_KROETENHAUT`)

- **Im Code:** Gegenstands-ID 81 (`ITEM_ID_KROETENHAUT`, Rohname in ITEMNAME `KROETENH.AUT.ÄUTE`). ITEMS.DAT: Rüstung; Gewicht 150 Unzen; Grundpreis 60 Silbertaler (= 600 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[16]`: RS 3, BE 3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 12, damit bei 15 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 12). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-82"></a>
### 82. Plattenzeug (`ITEM_ID_PLATTENZEUG`)

- **Im Code:** Gegenstands-ID 82 (`ITEM_ID_PLATTENZEUG`, Rohname in ITEMNAME `PLATTENZEUG..`). ITEMS.DAT: Rüstung; Gewicht 100 Unzen; Grundpreis 60 Silbertaler (= 600 Heller). Rüstung, Platz Arme (Untertyp 1), `g_armor_stats_table[17]`: RS 2, BE 1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 12, damit bei 15 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 12). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-83"></a>
### 83. Kettenzeug (`ITEM_ID_KETTENZEUG`)

- **Im Code:** Gegenstands-ID 83 (`ITEM_ID_KETTENZEUG`, Rohname in ITEMNAME `KETTENZEUG..`). ITEMS.DAT: Rüstung; Gewicht 80 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Rüstung, Platz Arme (Untertyp 1), `g_armor_stats_table[18]`: RS 1, BE 1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier, Firnelf, Waldelf. Handel: Sortimentswert 14, damit bei 16 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-84"></a>
### 84. Lederzeug (`ITEM_ID_LEDERZEUG`)

- **Im Code:** Gegenstands-ID 84 (`ITEM_ID_LEDERZEUG`, Rohname in ITEMNAME `LEDERZEUG..`). ITEMS.DAT: Rüstung; Gewicht 70 Unzen; Grundpreis 40 Silbertaler (= 400 Heller). Rüstung, Platz Arme (Untertyp 1), `g_armor_stats_table[19]`: RS 1, BE 0. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Magier. Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Wolfshöhle. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-85"></a>
### 85. Zunderkästchen (`ITEM_ID_ZUNDERKAESTCHEN`)

- **Im Code:** Gegenstands-ID 85 (`ITEM_ID_ZUNDERKAESTCHEN`, Rohname in ITEMNAME `ZUNDERKÄSTCHEN..`). ITEMS.DAT: benutzbar; Gewicht 25 Unzen; Grundpreis 8 Silbertaler (= 80 Heller). Benutzbar-Eintrag 12 (`g_usable_items_table`): Benutzen ruft `item_ignite()` auf. Handel: Sortimentswert 17, damit bei 52 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 17). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Verlassene Mine (2×), Wolfshöhle. Zum Anzünden einer Fackel oder einer nicht brennenden Laterne über „Benutzen“ braucht der Held ein Zunderkästchen (Meldung: kein Zunder dabei). Das Kästchen wird nicht verbraucht. Benutzt man das Zunderkästchen selbst, wird wie beim Zauber „Brenne toter Stoff“ eine Fackel oder Laterne gesucht. (`Quelle/seg107.c:item_ignite`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_ignite`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich Pflicht für alle, die ohne Zauber Fackeln oder Laternen anzünden wollen. Das Kästchen wird nicht verbraucht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-86"></a>
### 86. Schleifstein (`ITEM_ID_SCHLEIFSTEIN`)

- **Im Code:** Gegenstands-ID 86 (`ITEM_ID_SCHLEIFSTEIN`, Rohname in ITEMNAME `SCHLEIFSTEIN..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 25 Unzen; Grundpreis 9 Silbertaler (= 90 Heller). Handel: Sortimentswert 18, damit bei 55 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 18). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-87"></a>
### 87. Essbesteck (`ITEM_ID_ESSBESTECK`)

- **Im Code:** Gegenstands-ID 87 (`ITEM_ID_ESSBESTECK`, Rohname in ITEMNAME `ESSBESTECK..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 5 Silbertaler (= 50 Heller). Handel: Sortimentswert 16, damit bei 50 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 16). Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-88"></a>
### 88. Essgeschirr (`ITEM_ID_ESSGESCHIRR`)

- **Im Code:** Gegenstands-ID 88 (`ITEM_ID_ESSGESCHIRR`, Rohname in ITEMNAME `ESSGESCHIRR..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 100 Unzen; Grundpreis 60 Silbertaler (= 600 Heller). Handel: Sortimentswert 16, damit bei 50 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 16). Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-89"></a>
### 89. Lakritze (`ITEM_ID_LAKRITZE`)

- **Im Code:** Gegenstands-ID 89 (`ITEM_ID_LAKRITZE`, Rohname in ITEMNAME `LAKRITZE..`). ITEMS.DAT: Nahrung; Gewicht 10 Unzen; Grundpreis 3 Heller (= 3 Heller). Nahrung (Essen: senkt den Hunger um 2 Punkte beim Einnehmen). Handel: Sortimentswert 14, damit bei 46 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 14). Alchimie-Zutat (1×) für Expurgicum (Rezept ID 167). Alchimie-Zutat (1×) für Hylailer Feuer (Rezept ID 203).
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg108.c:consume`, `Quelle/seg055.c:buy_screen`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich ein gewöhnliches Nahrungsmittel.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-90"></a>
### 90. Bonbons (`ITEM_ID_BONBONS`)

- **Im Code:** Gegenstands-ID 90 (`ITEM_ID_BONBONS`, Rohname in ITEMNAME `BONBONS..`). ITEMS.DAT: Nahrung; Gewicht 10 Unzen; Grundpreis 1 Silbertaler (= 10 Heller). Nahrung (Essen: senkt den Hunger um 2 Punkte beim Einnehmen). Handel: Sortimentswert 6, damit bei 20 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 6).
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg108.c:consume`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich ein gewöhnliches Nahrungsmittel.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-91"></a>
### 91. Weinflasche (`ITEM_ID_WEINFLASCHE`)

- **Im Code:** Gegenstands-ID 91 (`ITEM_ID_WEINFLASCHE`, Rohname in ITEMNAME `WEINFLASCHE..N`). ITEMS.DAT: Nahrung; Gewicht 40 Unzen; Grundpreis 5 Heller (= 5 Heller). Nahrung (Getränk: senkt den Durst um 5 Punkte beim Einnehmen). Handel: Sortimentswert 14, damit bei 46 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 14). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (4×), Reiseereignis Ottarje–Orvil, Zwingfeste (6×), Spinnenhöhle (12×), Kultstätte des Namenlosen (3×), Piratenhöhle (3×). Alchimie-Zutat (1×) für Schlaftrunk (Rezept ID 242). Alchimie: Wird eine Weinflasche als Zutat verbraucht, bekommt der Brauer eine Glasflasche zurück. (`Quelle/seg104.c:hero_use_ingredients`) Einnehmen (Essen/Trinken-Menü): Der Held wird betrunken, die Flasche ist verbraucht. (`Quelle/seg108.c:consume`) Reiseereignis Berghütte mit grummelndem Jäger (Ottarje–Orvil): Nur wenn die Gruppe eine Schnaps- oder Weinflasche dabei hat, gibt es die Antwortmöglichkeiten, ihm etwas abzugeben (die Flasche wird nicht verbraucht). (`Quelle/seg111.c:tevent_059`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg108.c:consume`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg104.c:hero_use_ingredients`, `Quelle/seg111.c:tevent_059`
- **Deutung der KI:** Vermutlich vor allem Alchimie-Zutat. Trinken macht betrunken.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-92"></a>
### 92. Schnapsflasche (`ITEM_ID_SCHNAPSFLASCHE`)

- **Im Code:** Gegenstands-ID 92 (`ITEM_ID_SCHNAPSFLASCHE`, Rohname in ITEMNAME `SCHNAPSFLASCHE..N`). ITEMS.DAT: Nahrung; Gewicht 40 Unzen; Grundpreis 1 Silbertaler (= 10 Heller). Nahrung (Getränk: senkt den Durst um 2 Punkte beim Einnehmen). Handel: Sortimentswert 20, damit bei 56 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 20). Startausrüstung (`g_hero_startup_items`): Thorwaler. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (5×), Reiseereignis Ottarje–Orvil, Zwingfeste (4×), Wolfshöhle, Kultstätte des Namenlosen, Piratenhöhle (4×), Totenschiff, Reiseereignis Ala–Tjanset. Alchimie-Zutat (1×) für Heiltrank (Rezept ID 186). Alchimie-Zutat (1×) für Starker Heiltrank (Rezept ID 240). Alchimie-Zutat (1×) für Wunderkur (Rezept ID 241). Alchimie-Zutat (1×) für Gegengift (Rezept ID 202). Alchimie-Zutat (1×) für Zaubertrank (Rezept ID 206). Alchimie-Zutat (1×) für Zaubertrank (stark) (Rezept ID 243). Alchimie-Zutat (1×) für Miasthmaticum (Rezept ID 244). Alchimie-Zutat (1×) für MU-Elixier (Rezept ID 205). Alchimie-Zutat (1×) für KK-Elixier (Rezept ID 204). Alchimie-Zutat (2×) für Hylailer Feuer (Rezept ID 203). Orkbehausung: Beim Durchsuchen eines Bettes findet ein Held mit 10 % Wahrscheinlichkeit eine Schnapsflasche. (`Quelle/seg083.c:DNG08_search_bed`) Alchimie: Wird eine Schnapsflasche als Zutat verbraucht, bekommt der Brauer eine Glasflasche zurück. (`Quelle/seg104.c:hero_use_ingredients`) Einnehmen (Essen/Trinken-Menü): Der Held wird betrunken, die Flasche ist verbraucht. (`Quelle/seg108.c:consume`) Reiseereignis Berghütte mit grummelndem Jäger (Ottarje–Orvil): Nur wenn die Gruppe eine Schnaps- oder Weinflasche dabei hat, gibt es die Antwortmöglichkeiten, ihm etwas abzugeben (die Flasche wird nicht verbraucht). (`Quelle/seg111.c:tevent_059`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg108.c:consume`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg083.c:DNG08_search_bed`, `Quelle/seg104.c:hero_use_ingredients`, `Quelle/seg111.c:tevent_059`
- **Deutung der KI:** Vermutlich vor allem Alchimie-Zutat, die in vielen Rezepten steckt. Trinken macht betrunken.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-93"></a>
### 93. Hacke (`ITEM_ID_HACKE`)

- **Im Code:** Gegenstands-ID 93 (`ITEM_ID_HACKE`, Rohname in ITEMNAME `HACKE..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 200 Unzen; Grundpreis 20 Silbertaler (= 200 Heller). Handel: Sortimentswert 17, damit bei 52 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 17). Liegt in Truhen/Beutetabellen im Code: Verlassene Mine, Zwingfeste, Piratenhöhle, Hyggeliks Ruine. Zwingfeste: liegt in einer Nische auf Feld (3,3,3) (einmalig). (`Quelle/seg087.c:DNG14_handler`) Gehört zum Werkzeugsatz Schaufel, Hacke, Brecheisen und Wurfbeil. Nur wenn die Gruppe alle vier dabei hat, gilt sie als ausgerüstet. Verlassene Mine: Gang freiräumen 2 statt 6 Stunden. Einsturzstelle (schwer): mit Werkzeug 12, ohne 18 Stunden. Einsturzstelle (leicht): im Original dauert es MIT Werkzeug einen Tag und ohne 6 Stunden (vertauscht), mit Fix 122 (Fanpatch-Stufe „Wichtig“) richtig herum. Hyggeliks Ruine, Gang freiräumen: im Original mit Werkzeug 1 Stunde, ohne 30 Minuten (vertauscht), mit Fix 122 (Fanpatch-Stufe „Wichtig“) richtig herum. (`Quelle/seg091.c:DNG13_unblock_passage`, `Quelle/seg091.c:DNG13_collapsing_ceiling`, `Quelle/seg091.c:DNG13_collapsing_ceiling_hard`, `Quelle/seg089.c:DNG15_clear_way`) Hyggeliks Ruine: Eine rostige Spitzhacke steht in einer Ecke zum Mitnehmen (einmalig). (`Quelle/seg089.c:DNG15_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg087.c:DNG14_handler`, `Quelle/seg091.c:DNG13_unblock_passage`, `Quelle/seg091.c:DNG13_collapsing_ceiling`, `Quelle/seg091.c:DNG13_collapsing_ceiling_hard`, `Quelle/seg089.c:DNG15_clear_way`, `Quelle/seg089.c:DNG15_handler`
- **Deutung der KI:** Vermutlich nur als Teil des vierteiligen Werkzeugsatzes nützlich.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-94"></a>
### 94. Praiosamulett (`ITEM_ID_PRAIOS_AMULETT`)

- **Im Code:** Gegenstands-ID 94 (`ITEM_ID_PRAIOS_AMULETT`, Rohname in ITEMNAME `PRAIOSAMULETT..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 15 Dukaten (= 1500 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Spinnenhöhle (2×). Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-95"></a>
### 95. Laute (`ITEM_ID_LAUTE`)

- **Im Code:** Gegenstands-ID 95 (`ITEM_ID_LAUTE`, Rohname in ITEMNAME `LAUTE..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 80 Unzen; Grundpreis 15 Dukaten (= 1500 Heller). Handel: Sortimentswert 3, damit bei 13 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 3). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (2×). Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-96"></a>
### 96. Wintermantel (`ITEM_ID_WINTERMANTEL`)

- **Im Code:** Gegenstands-ID 96 (`ITEM_ID_WINTERMANTEL`, Rohname in ITEMNAME `WINTERM.ANTEL.ÄNTEL`). ITEMS.DAT: Rüstung; Gewicht 120 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[20]`: RS 1, BE 1. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 12, damit bei 15 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 12). Liegt in Truhen/Beutetabellen im Code: Piratenhöhle. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-97"></a>
### 97. Netz (`ITEM_ID_NETZ`)

- **Im Code:** Gegenstands-ID 97 (`ITEM_ID_NETZ`, Rohname in ITEMNAME `NETZ..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 35 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: Sortimentswert 11, damit bei 37 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 11). Piratenhöhle (Manrek): Wirft ein Held das Netz auf Feld (9,9) weg, gilt das als Opfer an Efferd/Swafnir (gs_dng11_swafnir_sacrifice). Im Original zählt das auch, wenn gar nichts weggeworfen wurde. Mit Fix 126 (Fanpatch-Stufe „Wichtig“) nur, wenn wirklich etwas weggeworfen wurde. (`Quelle/seg105.c:drop_item`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:drop_item`
- **Deutung der KI:** Sieht so aus, als ob das Netz an einer bestimmten Stelle der Piratenhöhle als Opfergabe für Swafnir gedacht ist.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-98"></a>
### 98. Wurfmesser (`ITEM_ID_WURFMESSER`)

- **Im Code:** Gegenstands-ID 98 (`ITEM_ID_WURFMESSER`, Rohname in ITEMNAME `WURFMESSER..`). ITEMS.DAT: Waffe; Gewicht 10 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Waffe, Waffenart Wurfwaffe (Untertyp 8), Waffentabelle `g_weapon_stats_table[24]`: TP 1W6, kein KK-Zuschlag, Bruchfaktor 4, AT -3, PA -4, Fernkampf-Entfernungstabelle Nr. 0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 14, damit bei 16 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Startausrüstung (`g_hero_startup_items`): Gaukler. Liegt in Truhen/Beutetabellen im Code: Zwingfeste, Verfallene Herberge, Totenschiff (3×). Wird als Wurfwaffe eingesetzt: Nach dem Wurf fliegt die Waffe aus der rechten Hand (liegt in der linken Hand eine gleiche, rückt sie nach). Bis zu 30 geworfene Waffen merkt sich der Kampf und gibt sie nach einem gewonnenen Kampf an die Gruppe zurück (nur die Gegenstandsnummer, nicht ihr Zustand). (`Quelle/seg041.c:range_attack_check_ammo`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:range_attack_check_ammo`
- **Deutung der KI:** Vermutlich eine typische Gaukler-Wurfwaffe, die nach dem Kampf zurückkommt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-99"></a>
### 99. Sichel (`ITEM_ID_SICHEL`)

- **Im Code:** Gegenstands-ID 99 (`ITEM_ID_SICHEL`, Rohname in ITEMNAME `SICHEL..N`). ITEMS.DAT: Waffe; Gewicht 30 Unzen; Grundpreis 25 Silbertaler (= 250 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[25]`: TP 1W6+2, KK-Zuschlag ab KK 17 (je Punkt darüber +1 TP), Bruchfaktor 5, AT -3, PA -4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 10, damit bei 13 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 10). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-100"></a>
### 100. Sense (`ITEM_ID_SENSE`)

- **Im Code:** Gegenstands-ID 100 (`ITEM_ID_SENSE`, Rohname in ITEMNAME `SENSE..N`). ITEMS.DAT: Waffe; Gewicht 100 Unzen; Grundpreis 35 Silbertaler (= 350 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[26]`: TP 1W6+3, KK-Zuschlag ab KK 17 (je Punkt darüber +1 TP), Bruchfaktor 6, AT -3, PA -4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 10, damit bei 13 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 10). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-101"></a>
### 101. Kriegshammer (`ITEM_ID_KRIEGSHAMMER`)

- **Im Code:** Gegenstands-ID 101 (`ITEM_ID_KRIEGSHAMMER`, Rohname in ITEMNAME `KRIEGSH.AMMER.ÄMMER`). ITEMS.DAT: Waffe; Gewicht 150 Unzen; Grundpreis 100 Silbertaler (= 1000 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[27]`: TP 2W6+3, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 2, AT -2, PA -4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Thorwaler, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 7, damit bei 9 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 7). Liegt in Truhen/Beutetabellen im Code: Hyggeliks Ruine. Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit stumpfer Waffe dargestellt. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-102"></a>
### 102. Dreizack (`ITEM_ID_DREIZACK`)

- **Im Code:** Gegenstands-ID 102 (`ITEM_ID_DREIZACK`, Rohname in ITEMNAME `DREIZACK..E`). ITEMS.DAT: Waffe; Gewicht 90 Unzen; Grundpreis 45 Silbertaler (= 450 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[28]`: TP 1W6+3, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 3, AT +0, PA -3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 6, damit bei 7 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 6). Liegt in Truhen/Beutetabellen im Code: Totenschiff. Piratenhöhle (Manrek): Wirft ein Held den Dreizack auf Feld (9,9) weg, gilt das als Opfer an Efferd/Swafnir (gs_dng11_swafnir_sacrifice). Im Original zählt das auch, wenn gar nichts weggeworfen wurde. Mit Fix 126 (Fanpatch-Stufe „Wichtig“) nur, wenn wirklich etwas weggeworfen wurde. (`Quelle/seg105.c:drop_item`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg105.c:drop_item`
- **Deutung der KI:** Sieht so aus, als ob der Dreizack an einer bestimmten Stelle der Piratenhöhle als Opfergabe für Swafnir gedacht ist.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-103"></a>
### 103. Hellebarde (`ITEM_ID_HELLEBARDE`)

- **Im Code:** Gegenstands-ID 103 (`ITEM_ID_HELLEBARDE`, Rohname in ITEMNAME `HELLEBARDE..N`). ITEMS.DAT: Waffe; Gewicht 150 Unzen; Grundpreis 75 Silbertaler (= 750 Heller). Waffe, Waffenart Axt (Untertyp 4), Waffentabelle `g_weapon_stats_table[29]`: TP 1W6+4, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 5, AT -1, PA -3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Thorwaler, Zwerg, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 12, damit bei 15 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 12). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Zwingfeste. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-104"></a>
### 104. Dreschflegel (`ITEM_ID_DRESCHFLEGEL`)

- **Im Code:** Gegenstands-ID 104 (`ITEM_ID_DRESCHFLEGEL`, Rohname in ITEMNAME `DRESCHFLEGEL..`). ITEMS.DAT: Waffe; Gewicht 100 Unzen; Grundpreis 15 Silbertaler (= 150 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[30]`: TP 1W6+2, KK-Zuschlag ab KK 13 (je Punkt darüber +1 TP), Bruchfaktor 6, AT -2, PA -3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Zwerg, Hexe, Magier. Handel: Sortimentswert 14, damit bei 16 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-105"></a>
### 105. Zweililien (`ITEM_ID_ZWEILILIEN`)

- **Im Code:** Gegenstands-ID 105 (`ITEM_ID_ZWEILILIEN`, Rohname in ITEMNAME `ZWEILILIEN..`). ITEMS.DAT: Waffe; Gewicht 80 Unzen; Grundpreis 95 Silbertaler (= 950 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[31]`: TP 1W6+3, KK-Zuschlag ab KK 18 (je Punkt darüber +1 TP), Bruchfaktor 4, AT -1, PA -1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 1, damit bei 1 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 1). Liegt in Truhen/Beutetabellen im Code: Zwingfeste. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-106"></a>
### 106. Ochsenherde (`ITEM_ID_OCHSENHERDE`)

- **Im Code:** Gegenstands-ID 106 (`ITEM_ID_OCHSENHERDE`, Rohname in ITEMNAME `OCHSENHERDE..N`). ITEMS.DAT: Waffe; Gewicht 240 Unzen; Grundpreis 110 Silbertaler (= 1100 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[32]`: TP 3W6+3, KK-Zuschlag ab KK 17 (je Punkt darüber +1 TP), Bruchfaktor 4, AT -3, PA -4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Thorwaler, Zwerg, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 5, damit bei 7 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 5). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-107"></a>
### 107. Basilikenzunge (`ITEM_ID_BASILISKENZUNGE`)

- **Im Code:** Gegenstands-ID 107 (`ITEM_ID_BASILISKENZUNGE`, Rohname in ITEMNAME `BASILIKENZUNGE..N`). ITEMS.DAT: Waffe; Gewicht 25 Unzen; Grundpreis 25 Silbertaler (= 250 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[33]`: TP 1W6+1, KK-Zuschlag ab KK 16 (je Punkt darüber +1 TP), Bruchfaktor 4, AT -2, PA -3. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 4, damit bei 6 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 4). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-108"></a>
### 108. Ogerfänger (`ITEM_ID_OGERFAENGER`)

- **Im Code:** Gegenstands-ID 108 (`ITEM_ID_OGERFAENGER`, Rohname in ITEMNAME `OGERFÄNGER..`). ITEMS.DAT: Waffe; Gewicht 30 Unzen; Grundpreis 95 Silbertaler (= 950 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[34]`: TP 1W6+2, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 4, AT -2, PA -3. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 10, damit bei 13 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 10). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-109"></a>
### 109. Mengbilar (`ITEM_ID_MENGBILAR`)

- **Im Code:** Gegenstands-ID 109 (`ITEM_ID_MENGBILAR`, Rohname in ITEMNAME `MENGBILAR..E`). ITEMS.DAT: Waffe; Gewicht 20 Unzen; Grundpreis 110 Silbertaler (= 1100 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[35]`: TP 1W6+1, KK-Zuschlag ab KK 16 (je Punkt darüber +1 TP), Bruchfaktor 7, AT -3, PA -4. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 2, damit bei 3 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 2). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Totenschiff. Ersetzt nach dem ersten Treffer den vergifteten Kukris-Mengbilar (ID 200) in der rechten Hand. (`Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die im Code nur als Ersatz für den verbrauchten Kukris-Mengbilar vorkommt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-110"></a>
### 110. Schwerer Dolch (`ITEM_ID_SCHWERER_DOLCH`)

- **Im Code:** Gegenstands-ID 110 (`ITEM_ID_SCHWERER_DOLCH`, Rohname in ITEMNAME `SCHWERE.R DOLCH. DOLCHE`). ITEMS.DAT: Waffe; Gewicht 30 Unzen; Grundpreis 30 Silbertaler (= 300 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[36]`: TP 1W6+2, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 2, AT -1, PA -2. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 18, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 18). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Rovamund–Nordvest, Wolfshöhle, Totenschiff (2×). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-111"></a>
### 111. Rondrakamm (`ITEM_ID_RONDRAKAMM`)

- **Im Code:** Gegenstands-ID 111 (`ITEM_ID_RONDRAKAMM`, Rohname in ITEMNAME `RONDRAK.AMM.ÄMME`). ITEMS.DAT: Waffe; Gewicht 150 Unzen; Grundpreis 140 Silbertaler (= 1400 Heller). Waffe, Waffenart Zweihänder (Untertyp 6), Waffentabelle `g_weapon_stats_table[37]`: TP 2W6+2, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 3, AT -2, PA -2. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Thorwaler, Zwerg, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 6, damit bei 7 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 6). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-112"></a>
### 112. Entermesser (`ITEM_ID_ENTERMESSER`)

- **Im Code:** Gegenstands-ID 112 (`ITEM_ID_ENTERMESSER`, Rohname in ITEMNAME `ENTERMESSER..`). ITEMS.DAT: Waffe; Gewicht 70 Unzen; Grundpreis 45 Silbertaler (= 450 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[38]`: TP 1W6+3, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 2, AT +0, PA -1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 18, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 18). Liegt in Truhen/Beutetabellen im Code: Daspotaschatz (2×), Piratenhöhle (5×), Totenschiff (3×). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-113"></a>
### 113. Bastardschwert (`ITEM_ID_BASTARDSCHWERT`)

- **Im Code:** Gegenstands-ID 113 (`ITEM_ID_BASTARDSCHWERT`, Rohname in ITEMNAME `BASTARDSCHWERT..ER`). ITEMS.DAT: Waffe; Gewicht 140 Unzen; Grundpreis 110 Silbertaler (= 1100 Heller). Waffe, Waffenart Zweihänder (Untertyp 6), Waffentabelle `g_weapon_stats_table[39]`: TP 1W6+5, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), Bruchfaktor 2, AT -1, PA -2. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Thorwaler, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 14, damit bei 16 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ala–Tjanset. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-114"></a>
### 114. Tuzakmesser (`ITEM_ID_TUZAKMESSER`)

- **Im Code:** Gegenstands-ID 114 (`ITEM_ID_TUZAKMESSER`, Rohname in ITEMNAME `TUZAKMESSER..`). ITEMS.DAT: Waffe; Gewicht 135 Unzen; Grundpreis 200 Silbertaler (= 2000 Heller). Waffe, Waffenart Zweihänder (Untertyp 6), Waffentabelle `g_weapon_stats_table[40]`: TP 1W6+6, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 1, AT -2, PA -2. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Thorwaler, Zwerg, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 2, damit bei 3 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 2). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-115"></a>
### 115. Rabenschnabel (`ITEM_ID_RABENSCHNABEL`)

- **Im Code:** Gegenstands-ID 115 (`ITEM_ID_RABENSCHNABEL`, Rohname in ITEMNAME `RABENSCHN.ABEL.ÄBEL`). ITEMS.DAT: Waffe; Gewicht 90 Unzen; Grundpreis 110 Silbertaler (= 1100 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[41]`: TP 1W6+4, KK-Zuschlag ab KK 16 (je Punkt darüber +1 TP), Bruchfaktor 3, AT -1, PA -3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 4, damit bei 6 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 4). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-116"></a>
### 116. Brabakbengel (`ITEM_ID_BRABAKBENGEL`)

- **Im Code:** Gegenstands-ID 116 (`ITEM_ID_BRABAKBENGEL`, Rohname in ITEMNAME `BRABAKBENGEL..`). ITEMS.DAT: Waffe; Gewicht 120 Unzen; Grundpreis 80 Silbertaler (= 800 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[42]`: TP 1W6+5, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), Bruchfaktor 1, AT -1, PA -2. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 4, damit bei 6 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 4). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-117"></a>
### 117. Rapier (`ITEM_ID_RAPIER`)

- **Im Code:** Gegenstands-ID 117 (`ITEM_ID_RAPIER`, Rohname in ITEMNAME `RAPIER..E`). ITEMS.DAT: Waffe; Gewicht 35 Unzen; Grundpreis 60 Silbertaler (= 600 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[43]`: TP 1W6+3, KK-Zuschlag ab KK 16 (je Punkt darüber +1 TP), Bruchfaktor 4, AT +0, PA -1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 13, damit bei 15 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 13). Startausrüstung (`g_hero_startup_items`): Streuner, Auelf. Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-118"></a>
### 118. Khunchomer (`ITEM_ID_KHUNCHOMER`)

- **Im Code:** Gegenstands-ID 118 (`ITEM_ID_KHUNCHOMER`, Rohname in ITEMNAME `KHUNCHOMER..`). ITEMS.DAT: Waffe; Gewicht 70 Unzen; Grundpreis 70 Silbertaler (= 700 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[44]`: TP 1W6+4, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), Bruchfaktor 2, AT +0, PA -1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 6, damit bei 7 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 6). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-119"></a>
### 119. Doppelkhunchomer (`ITEM_ID_DOPPELKHUNCHOMER`)

- **Im Code:** Gegenstands-ID 119 (`ITEM_ID_DOPPELKHUNCHOMER`, Rohname in ITEMNAME `DOPPELKHUNCHOMER..`). ITEMS.DAT: Waffe; Gewicht 150 Unzen; Grundpreis 125 Silbertaler (= 1250 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[45]`: TP 1W6+6, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 3, AT -1, PA -3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Thorwaler, Zwerg, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: Sortimentswert 2, damit bei 3 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 2). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-120"></a>
### 120. Schleuder (`ITEM_ID_SCHLEUDER`)

- **Im Code:** Gegenstands-ID 120 (`ITEM_ID_SCHLEUDER`, Rohname in ITEMNAME `SCHLEUDER..N`). ITEMS.DAT: Waffe; Gewicht 10 Unzen; Grundpreis 15 Silbertaler (= 150 Heller). Waffe, Waffenart Schusswaffe (Untertyp 7), Waffentabelle `g_weapon_stats_table[46]`: TP 1W6+2, kein KK-Zuschlag, Bruchfaktor 0, AT +0, PA +0, Fernkampf-Entfernungstabelle Nr. 1. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ottarje–Orvil. Die Schleuder verlangt in der linken Hand den Gegenstand Nr. 999, den es nicht gibt (Gegenstandsnummern gehen nur bis 254). Sie kann deshalb nie schießen und meldet immer Meldung: keine Steine in der linken Hand (`Quelle/seg041.c:range_attack_check_ammo`) Kann im Original vergiftet werden (als einzige Fernwaffe). Mit Fix 224 (Fanpatch-Stufe „nur Alle“) nicht mehr. (`Quelle/seg107.c:item_apply_weapon_poison`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg041.c:range_attack_check_ammo`, `Quelle/seg107.c:item_apply_weapon_poison`
- **Deutung der KI:** Sieht so aus, als ob die Schleuder im Original überhaupt nicht schießen kann. Ein Programmierfehler, den der Code-Kommentar auch so nennt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-121"></a>
### 121. Seil (`ITEM_ID_SEIL`)

- **Im Code:** Gegenstands-ID 121 (`ITEM_ID_SEIL`, Rohname in ITEMNAME `SEIL..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 55 Unzen; Grundpreis 7 Silbertaler (= 70 Heller). Handel: Sortimentswert 20, damit bei 56 von 56 Krämern im Angebot (Händler mit Sortimentsstufe ≤ 20). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota (2×), Verlassene Mine (2×), Zwingfeste, Spinnenhöhle (2×), Wolfshöhle (2×), Piratenhöhle (4×), Totenschiff (3×). Für Abstiege in Dungeons (Fallgruben, Schächte, auch einige Reiseereignisse) reicht ein Seil oder eine Strickleiter bei irgendeinem Helden. Ein Magier der Gruppe mit Stabzauber-Stufe über 2 ersetzt beides. (`Quelle/seg075.c:DNG_check_climb_tools`) Auf dem Totenschiff kann man durch einen Spalt in der Decke nur mit Seil oder Strickleiter nach oben klettern. Mit Fix 92 (Fanpatch-Stufe „Wichtig“) hilft dort auch ein Magier mit Stabzauber ab Stufe 3, wie an den Gruben. (`Quelle/seg077.c:DNG01_handler`) Kultstätte des Namenlosen: Um einen in die Fallgrube gestürzten Helden heraufzuholen, braucht die Gruppe 2 Seile/Strickleitern, oder 1 Seil/Strickleiter plus einen Magier mit Stabzauber über Stufe 2 (der Magier verwandelt seinen Stab in ein Seil). Mit nur einem Seil: Meldung, dass ein Seil für die Tiefe nicht reicht (`Quelle/seg081.c:DNG09_pitfall`) Zwingfeste, Loch auf Feld (2,4,13): „Angeseilt klettern“ wird nur angeboten, wenn die Gruppe zusammen mindestens 5 Seile/Strickleitern/Magier mit Stabzauber über Stufe 2 hat; dann stürzt nur ab, wem eine Klettern-Probe misslingt (1W6 LE). Ohne Seil: Klettern +4, bei Misserfolg 2W6+4 LE. Am eingestürzten Gang auf (3,10,10)/(3,14,10): mit Seil oder Strickleiter taucht zuerst ein gewählter Held (Schwimmen +8) und spannt ein Seil, die anderen folgen mit leichterer Probe. Ohne Seil müssen alle die schwere Probe ablegen. (`Quelle/seg087.c:DNG14_handler`) Reiseereignis Bach ohne Brücke (Breida–Tjoila und Auplog–Varnheim): Zum Brückenbau braucht die Gruppe mindestens 2 Seile/Strickleitern, eine Axt aus der Axtliste und einen Hammer. Dabei werden 2 Seile/Strickleitern verbraucht, und der Bau gelingt, wenn mindestens 3 Wildnisleben-Proben gelingen (im Original zählen dabei auch Helden anderer Gruppen und tote, Fix 153 (Fanpatch-Stufe „Wichtig“)). (`Quelle/seg109.c:TRV_barrier`) Reiseereignis Erdrutsch (Ottarje–Orvil), Überklettern: Wem die Klettern-Probe +3 misslingt, der stürzt. Schwer stürzt er (1W9+3 LE) im Original, wenn die Gruppe kein Seil hat ODER irgendwer eine Strickleiter (sinnwidrige Bedingung), sonst leicht (1W8 LE). Mit Fix 158 (Fanpatch-Stufe „Wichtig“) schwer nur ohne Seil und ohne Strickleiter. (`Quelle/seg111.c:tevent_057`) Reiseereignis Pass mit Abstieg in eine Klamm (Ottarje–Orvil), Abseilen: Mit mindestens 3 Seilen/Strickleitern gelingt es (alle verlieren 3 LE, 3 Seile/Strickleitern werden verbraucht). Mit genau 2 und einem Helden mit Stabzauber ab Stufe 3 gelingt es ebenfalls, ohne Verbrauch. Sonst Meldung: nicht genug Seil (`Quelle/seg111.c:tevent_060`) Reiseereignis Klamm (Rybon–Thoss): Nur mit Seil oder Strickleiter in der Gruppe gibt es die dritte Antwort, sich an einem Seil hinüberzuhangeln (ein Held springt mit Akrobatik-Probe vor, die anderen brauchen eine GE-Probe -4). Das Seil wird nicht verbraucht. (`Quelle/seg113.c:tevent_098`) Reiseereignis steiler Hang (Skelellen–Phexcaer), Klettern: Wem die Klettern-Probe +1 misslingt, der fängt sich mit Seil in der Gruppe im Seil (1W11+1 LE), ohne Seil stürzt er schwer (1W11+4 LE). Strickleitern zählen hier nicht. (`Quelle/seg113.c:tevent_107`) Reiseereignis Felswand (Skelellen–Phexcaer): Wem die Klettern-Probe misslingt, der fängt sich mit Seil in der Gruppe im Seil (1W10 LE), sonst schwerer Sturz. Strickleitern zählen hier nicht. (`Quelle/seg114.c:tevent_110`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg075.c:DNG_check_climb_tools`, `Quelle/seg077.c:DNG01_handler`, `Quelle/seg081.c:DNG09_pitfall`, `Quelle/seg087.c:DNG14_handler`, `Quelle/seg109.c:TRV_barrier`, `Quelle/seg111.c:tevent_057`, `Quelle/seg111.c:tevent_060`, `Quelle/seg113.c:tevent_098`, `Quelle/seg113.c:tevent_107`, `Quelle/seg114.c:tevent_110`
- **Deutung der KI:** Vermutlich einer der wichtigsten Ausrüstungsgegenstände: An vielen Kletterstellen in Dungeons und auf Reisen geht ohne Seil wenig, und einige Ereignisse brauchen sogar zwei oder drei.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-122"></a>
### 122. Shurinknolle (`ITEM_ID_SHURINKNOLLE`)

- **Im Code:** Gegenstands-ID 122 (`ITEM_ID_SHURINKNOLLE`, Rohname in ITEMNAME `SHURINKNOLLE..N`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 8 Dukaten (= 800 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 15, damit bei 16 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 15). Alchimie-Zutat (1×) für Gegengift (Rezept ID 202). Alchimie-Zutat (1×) für Vomicum (Rezept ID 169). Alchimie-Zutat (2×) für Expurgicum (Rezept ID 167). Alchimie-Zutat (1×) für Miasthmaticum (Rezept ID 244). Kräutersuche beim Lagern (`g_gather_herbs_table`): 12 % Chance, dann Pflanzenkunde-Probe +8 (leichter je Suchstunde), bei Erfolg 1 bis 2 Stück. Kräuterplatz im Reiseereignis Nr. 91 (Felsteyn–Einsiedlersee): Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg115.c:tevent_091`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich vor allem Handelsware oder Alchimie-Zutat; eine eigene Wirkung ist im Code nicht zu sehen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-123"></a>
### 123. Belmartblatt (`ITEM_ID_BELMART_BLATT`)

- **Im Code:** Gegenstands-ID 123 (`ITEM_ID_BELMART_BLATT`, Rohname in ITEMNAME `BELMARTBL.ATT.ÄTTER`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 12 Dukaten (= 1200 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 14, damit bei 14 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Alchimie-Zutat (1×) für Wunderkur (Rezept ID 241). Kräutersuche beim Lagern (`g_gather_herbs_table`): 12 % Chance, dann Pflanzenkunde-Probe +8 (leichter je Suchstunde), bei Erfolg 1 bis 3 Stück. Kräuterplatz im Reiseereignis Nr. 13 (Rovamund–Nordvest): Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Heilkunde Krankheiten: Der Heiler braucht das passende Kraut im eigenen Inventar, sonst Meldung: die nötigen Kräuter fehlen. Das Kraut wird nur bei gelungener Heilung verbraucht. Belmartblatt hilft gegen Dumpfschädel (es wird als erstes genommen, wenn mehrere passende Kräuter da sind). (`Quelle/seg104.c:has_herb_for_disease`) Einnehmen (Essen/Trinken-Menü): das erste Gift, an dem der Held leidet (Reihenfolge der Giftnummern), wird geheilt, sofern es eines der Gifte 1–5 ist (Shurinknollengift, Arax, Angstgift, Schlafgift, Goldleim). Meldung Meldung, der Held sei jetzt widerstandsfähiger gegen Krankheiten und Gifte Ein dazu gesetzter Kräuterwert (herbs) wird laut Code-Kommentar nirgends ausgewertet, die angekündigte Widerstandskraft gibt es also nicht. Wird verbraucht. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg110.c:tevent_013`, `Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Sieht so aus, als ob Belmart gegen ein paar Gifte und gegen Dumpfschädel hilft. Die versprochene „Widerstandskraft“ wird aber nirgends ausgewertet.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-124"></a>
### 124. Donfstengel (`ITEM_ID_DONFSTENGEL`)

- **Im Code:** Gegenstands-ID 124 (`ITEM_ID_DONFSTENGEL`, Rohname in ITEMNAME `DONFSTENGEL..`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 11, damit bei 11 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 11). Alchimie-Zutat (1×) für Wunderkur (Rezept ID 241). Alchimie-Zutat (1×) für MU-Elixier (Rezept ID 205). Kräutersuche beim Lagern (`g_gather_herbs_table`): 10 % Chance, dann Pflanzenkunde-Probe +9 (leichter je Suchstunde), bei Erfolg 1 bis 2 Stück. Kräuterplatz im Reiseereignis Nr. 85: Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Einnehmen wird verweigert: Meldung: unbehandelt ungenießbar Heilkunde Krankheiten: Der Heiler braucht das passende Kraut im eigenen Inventar, sonst Meldung: die nötigen Kräuter fehlen. Das Kraut wird nur bei gelungener Heilung verbraucht. Donfstengel hilft gegen Paralyse (einziges Mittel) und gegen Dumpfschädel. (`Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg104.c:talent_cure_disease`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg113.c:tevent_085`, `Quelle/seg108.c:consume`, `Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg104.c:talent_cure_disease`
- **Deutung der KI:** Vermutlich das einzige Mittel gegen Paralyse. Roh essen kann man den Donfstengel nicht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-125"></a>
### 125. Menchalkaktus (`ITEM_ID_MENCHALKAKTUS`)

- **Im Code:** Gegenstands-ID 125 (`ITEM_ID_MENCHALKAKTUS`, Rohname in ITEMNAME `MENCHALKAKT.US.EEN`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 10, damit bei 10 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 10). Alchimie-Zutat (2×) für Gegengift (Rezept ID 202). Heilkunde Krankheiten: Der Heiler braucht das passende Kraut im eigenen Inventar, sonst Meldung: die nötigen Kräuter fehlen. Das Kraut wird nur bei gelungener Heilung verbraucht. Menchalkaktus hilft gegen Dumpfschädel. (`Quelle/seg104.c:has_herb_for_disease`) Einnehmen (Essen/Trinken-Menü): das erste Gift, an dem der Held leidet (Reihenfolge der Giftnummern), wird geheilt, sofern es eines der Gifte 1–10 ist (also jedes). Meldung Meldung, der Held sei jetzt widerstandsfähiger gegen Krankheiten und Gifte Der dazu gesetzte Kräuterwert wird nirgends ausgewertet. Wird verbraucht. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich das beste Kraut gegen Gifte, weil es jedes erste Gift heilt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-126"></a>
### 126. Alraune (`ITEM_ID_ALRAUNE`)

- **Im Code:** Gegenstands-ID 126 (`ITEM_ID_ALRAUNE`, Rohname in ITEMNAME `ALRAUNE..N`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 8 Silbertaler (= 80 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 10, damit bei 10 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 10). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Reiseereignis Ottarje–Orvil (2×). Alchimie-Zutat (1×) für Starker Heiltrank (Rezept ID 240). Alchimie-Zutat (1×) für Wunderkur (Rezept ID 241). Alchimie-Zutat (1×) für Gegengift (Rezept ID 202). Alchimie-Zutat (2×) für Zaubertrank (Rezept ID 206). Alchimie-Zutat (3×) für Zaubertrank (stark) (Rezept ID 243). Alchimie-Zutat (1×) für Vomicum (Rezept ID 169). Alchimie-Zutat (2×) für Expurgicum (Rezept ID 167). Alchimie-Zutat (1×) für Miasthmaticum (Rezept ID 244). Alchimie-Zutat (1×) für MU-Elixier (Rezept ID 205). Alchimie-Zutat (1×) für KK-Elixier (Rezept ID 204). Kräutersuche beim Lagern (`g_gather_herbs_table`): 5 % Chance, dann Pflanzenkunde-Probe +15 (leichter je Suchstunde), bei Erfolg 1 bis 1 Stück. Kräuterplatz im Reiseereignis Nr. 21: Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg110.c:tevent_021`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich vor allem Handelsware oder Alchimie-Zutat; eine eigene Wirkung ist im Code nicht zu sehen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-127"></a>
### 127. Atmonblüte (`ITEM_ID_ATMONBLUETE`)

- **Im Code:** Gegenstands-ID 127 (`ITEM_ID_ATMONBLUETE`, Rohname in ITEMNAME `ATMONBLÜTE..N`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 30 Dukaten (= 3000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 8, damit bei 7 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 8). Einnehmen (Essen/Trinken-Menü): Alle 10 Körperlichen Talente (Akrobatik bis Zechen) +2 für 5 Stunden. Wird verbraucht. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein Kraut zur Leistungssteigerung bei körperlichen Talenten (5 Stunden).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-128"></a>
### 128. Ilmenblatt (`ITEM_ID_ILMENBLATT`)

- **Im Code:** Gegenstands-ID 128 (`ITEM_ID_ILMENBLATT`, Rohname in ITEMNAME `ILMENBL.ATT.ÄTTER`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 11 Dukaten (= 1100 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 8, damit bei 7 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 8). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Reiseereignis Ottarje–Orvil. Alchimie-Zutat (1×) für Miasthmaticum (Rezept ID 244). Alchimie-Zutat (1×) für MU-Elixier (Rezept ID 205). Einnehmen wird verweigert: Meldung: unbehandelt ungenießbar
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich vor allem Handelsware oder Alchimie-Zutat; eine eigene Wirkung ist im Code nicht zu sehen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-129"></a>
### 129. Finagebäumchen (`ITEM_ID_FINAGEBAEMCHEN`)

- **Im Code:** Gegenstands-ID 129 (`ITEM_ID_FINAGEBAEMCHEN`, Rohname in ITEMNAME `FINAGEBÄUMCHEN..`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 8 Dukaten (= 800 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 6, damit bei 3 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 6). Alchimie-Zutat (1×) für MU-Elixier (Rezept ID 205). Alchimie-Zutat (1×) für KK-Elixier (Rezept ID 204). Einnehmen wird verweigert: Meldung: unbehandelt ungenießbar Heilkunde Krankheiten: Der Heiler braucht das passende Kraut im eigenen Inventar, sonst Meldung: die nötigen Kräuter fehlen. Das Kraut wird nur bei gelungener Heilung verbraucht. Finagebäumchen hilft gegen Dumpfschädel. (`Quelle/seg104.c:has_herb_for_disease`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg108.c:consume`, `Quelle/seg104.c:has_herb_for_disease`
- **Deutung der KI:** Vermutlich vor allem Alchimie-Zutat. Roh ist es ungenießbar, hilft aber gegen Dumpfschädel.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-130"></a>
### 130. Jorugawurzel (`ITEM_ID_JORUGAWURZEL`)

- **Im Code:** Gegenstands-ID 130 (`ITEM_ID_JORUGAWURZEL`, Rohname in ITEMNAME `JORUGAWURZEL..N`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 18 Dukaten (= 1800 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 6, damit bei 3 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 6). Startausrüstung (`g_hero_startup_items`): Druide. Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ottarje–Orvil. Alchimie-Zutat (1×) für Wunderkur (Rezept ID 241). Kräutersuche beim Lagern (`g_gather_herbs_table`): 15 % Chance, dann Pflanzenkunde-Probe +10 (leichter je Suchstunde), bei Erfolg 1 bis 1 Stück. Kräuterplatz im Reiseereignis Nr. 70: Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Im Original wirkt das wegen eines Zahlenüberlaufs (Gegenstandsnummer über 127) nie, erst mit Fix 161. Kräuterplatz im Reiseereignis Nr. 140 (Querverbindung): Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Im Original wirkt das wegen eines Zahlenüberlaufs (Gegenstandsnummer über 127) nie, erst mit Fix 161. Einnehmen wird verweigert: Meldung: unbehandelt ungenießbar Heilkunde Krankheiten: Der Heiler braucht das passende Kraut im eigenen Inventar, sonst Meldung: die nötigen Kräuter fehlen. Das Kraut wird nur bei gelungener Heilung verbraucht. Jorugawurzel hilft gegen Tollwut (einziges Mittel), gegen Dumpfschädel, und zusammen mit einem Gulmondblatt gegen Schlachtenfieber (dann werden beide verbraucht). (`Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg104.c:talent_cure_disease`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg112.c:tevent_070`, `Quelle/seg116.c:tevent_140`, `Quelle/seg108.c:consume`, `Quelle/seg104.c:has_herb_for_disease`, `Quelle/seg104.c:talent_cure_disease`
- **Deutung der KI:** Vermutlich ein wichtiges Heilkraut gegen Tollwut und Schlachtenfieber. Roh essen kann man es nicht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-131"></a>
### 131. Thonnysblüte (`ITEM_ID_THONNYSBLUETE`)

- **Im Code:** Gegenstands-ID 131 (`ITEM_ID_THONNYSBLUETE`, Rohname in ITEMNAME `THONNYSBLÜTE..N`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 35 Dukaten (= 3500 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 3, damit bei 1 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 3). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ottarje–Orvil. Alchimie-Zutat (1×) für Zaubertrank (Rezept ID 206). Alchimie-Zutat (1×) für Zaubertrank (stark) (Rezept ID 243). Kräutersuche beim Lagern (`g_gather_herbs_table`): 3 % Chance, dann Pflanzenkunde-Probe +15 (leichter je Suchstunde), bei Erfolg 1 bis 1 Stück. Kräuterplatz im Reiseereignis Nr. 112 (Skelellen–Phexcaer): Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Im Original wirkt das wegen eines Zahlenüberlaufs (Gegenstandsnummer über 127) nie, erst mit Fix 161. Einnehmen: wird verbraucht, ohne Wirkung (kein Fall im Code, es kommt keine Meldung). Meditation (LE in AE umwandeln): Wer kein Magier ist, braucht eine Thonnysblüte im Inventar, sonst Meldung: keine Thonnys dabei. Die Blüte wird verbraucht, sobald man eine Zahl eingibt (auch wenn die Probe danach misslingt). (`Quelle/seg098.c:use_magic`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg114.c:tevent_112`, `Quelle/seg108.c:consume`, `Quelle/seg098.c:use_magic`
- **Deutung der KI:** Sieht so aus, als ob Nicht-Magier die Thonnysblüte zum Meditieren brauchen. Sie ist bei jedem Versuch weg.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-132"></a>
### 132. Lotusblüte (`ITEM_ID_LOTUSBLUETE`)

- **Im Code:** Gegenstands-ID 132 (`ITEM_ID_LOTUSBLUETE`, Rohname in ITEMNAME `LOTUSBLÜTE..N`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 2, damit bei 1 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 2). Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich vor allem Handelsware oder Alchimie-Zutat; eine eigene Wirkung ist im Code nicht zu sehen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-133"></a>
### 133. Zauberstab (`ITEM_ID_ZAUBERSTAB`)

- **Im Code:** Gegenstands-ID 133 (`ITEM_ID_ZAUBERSTAB`, Rohname in ITEMNAME `ZAUBERST.AB.ÄBE`). ITEMS.DAT: Waffe, persönlich (nicht ablegbar), magisch; Gewicht 70 Unzen; Grundpreis 100 Dukaten (= 10000 Heller). Waffe, Waffenart Speer/Stab (Untertyp 5), Waffentabelle `g_weapon_stats_table[63]`: TP 1W6+1, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), unzerbrechlich (BF -99), AT +0, PA +0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Startausrüstung (`g_hero_startup_items`): Magier. Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit stumpfer Waffe dargestellt. Hat den Waffen-Untertyp Speer, wird aber ausdrücklich nicht wie ein Speer als Wurf-/Fernkampfwaffe behandelt. (`Quelle/seg039.c:FIG_weapon_gfx_id_ranged`) Zählt im Nahkampf trotz Untertyp Speer als normale Nahkampfwaffe. (`Quelle/seg041.c:FIG_weapon_gfx_id_melee`) Mit Fix 390 (Fanpatch-Stufe „nur Alle“) setzt der Kampf den Bruchfaktor dieser Waffe vor jeder Bruchprüfung (Attacke, Parade) auf -99 (unzerbrechlich), und der Zauber Eisenrost (von Helden und von Gegnern) kann sie nicht zerbrechen. Hintergrund laut Fix-Kommentar: bei Curians Stab stand ein falscher Bruchfaktor 5 im Gegenstand. (`Quelle/seg042.c:fix_set_unbreakable`, `Quelle/seg043.c:fix_set_unbreakable_pa`, `Quelle/seg100.c:fix_unbreakable_weapon`, `Quelle/seg102.c:fix_unbreakable_weapon`) Nur Grafik: Ein Magier mit Zauberstab wird im Kampf mit der Animation ohne Waffe gezeigt. (`Quelle/seg044.c:FANI_prepare_fight_hero_ani`) Die Gegenstandsbeschreibung zeigt die aktuelle Stabzauber-Stufe des Magiers (Angabe, der wievielte Stabzauber auf ihm liegt). (`Quelle/seg106.c:print_item_description`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg106.c:startup_equipment`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`, `Quelle/seg039.c:FIG_weapon_gfx_id_ranged`, `Quelle/seg042.c:fix_set_unbreakable`, `Quelle/seg043.c:fix_set_unbreakable_pa`, `Quelle/seg100.c:fix_unbreakable_weapon`, `Quelle/seg102.c:fix_unbreakable_weapon`, `Quelle/seg044.c:FANI_prepare_fight_hero_ani`, `Quelle/seg106.c:print_item_description`
- **Deutung der KI:** Vermutlich das zentrale Werkzeug des Magiers für die Stabzauber. Mit Fix 390 ist er wirklich unzerbrechlich.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-134"></a>
### 134. Skraja (`ITEM_ID_SKRAJA`)

- **Im Code:** Gegenstands-ID 134 (`ITEM_ID_SKRAJA`, Rohname in ITEMNAME `SKRAJA..S`). ITEMS.DAT: Waffe; Gewicht 90 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[47]`: TP 1W6+3, KK-Zuschlag ab KK 13 (je Punkt darüber +1 TP), Bruchfaktor 4, AT +0, PA -2. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 18, damit bei 20 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 18). Startausrüstung (`g_hero_startup_items`): Thorwaler. Liegt in Truhen/Beutetabellen im Code: Zwingfeste (3×), Verfallene Herberge, Piratenhöhle (3×). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-135"></a>
### 135. Kriegsbeil (`ITEM_ID_KRIEGSBEIL`)

- **Im Code:** Gegenstands-ID 135 (`ITEM_ID_KRIEGSBEIL`, Rohname in ITEMNAME `KRIEGSBEIL..E`). ITEMS.DAT: Waffe; Gewicht 120 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Waffe, Waffenart Axt (Untertyp 4), Waffentabelle `g_weapon_stats_table[48]`: TP 1W6+4, KK-Zuschlag ab KK 13 (je Punkt darüber +1 TP), Bruchfaktor 2, AT -2, PA -4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Zwergenfeste (2×), Zwingfeste (2×), Piratenhöhle (3×). Zählt als Axt für die Reiseereignisse Brückenbau (Breida–Tjoila, Auplog–Varnheim) und umgestürzter Baum (`g_travel_event_axes`). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg109.c:g_travel_event_axes`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-136"></a>
### 136. Orknase (`ITEM_ID_ORKNASE`)

- **Im Code:** Gegenstands-ID 136 (`ITEM_ID_ORKNASE`, Rohname in ITEMNAME `ORKNASE..N`). ITEMS.DAT: Waffe; Gewicht 120 Unzen; Grundpreis 60 Silbertaler (= 600 Heller). Waffe, Waffenart Axt (Untertyp 4), Waffentabelle `g_weapon_stats_table[49]`: TP 1W6+5, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), Bruchfaktor 2, AT -1, PA -3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 16, damit bei 19 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 16). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Zwingfeste (3×), Wolfshöhle, Piratenhöhle. Zwingfeste: liegt auf Feld (4,5,13) am Boden (einmalig). (`Quelle/seg087.c:DNG14_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg087.c:DNG14_handler`
- **Deutung der KI:** Vermutlich eine normale Orkwaffe, die man in der Zwingfeste findet.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-137"></a>
### 137. Schneidzahn (`ITEM_ID_SCHNEIDZAHN`)

- **Im Code:** Gegenstands-ID 137 (`ITEM_ID_SCHNEIDZAHN`, Rohname in ITEMNAME `SCHNEIDZ.AHN.ÄHNE`). ITEMS.DAT: Waffe; Gewicht 50 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Waffe, Waffenart Wurfwaffe (Untertyp 8), Waffentabelle `g_weapon_stats_table[50]`: TP 1W6+4, kein KK-Zuschlag, Bruchfaktor 0, AT +0, PA +0, Fernkampf-Entfernungstabelle Nr. 6. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 14, damit bei 16 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 14). Schneidzahn hat in der Waffentabelle keinen Fernkampf-Eintrag (ranged_index = -1). Im Original wird mit ihm daher keine Wurfwaffen-Entfernungsrechnung gemacht. Mit Fix 34 (Fanpatch-Stufe „Wichtig“) nimmt das Spiel dafür die Entfernungstabelle von Wurfbeil/Wurfaxt. (`Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`) Wird als Wurfwaffe eingesetzt: Nach dem Wurf fliegt die Waffe aus der rechten Hand (liegt in der linken Hand eine gleiche, rückt sie nach). Bis zu 30 geworfene Waffen merkt sich der Kampf und gibt sie nach einem gewonnenen Kampf an die Gruppe zurück (nur die Gegenstandsnummer, nicht ihr Zustand). (`Quelle/seg041.c:range_attack_check_ammo`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`, `Quelle/seg041.c:range_attack_check_ammo`
- **Deutung der KI:** Sieht so aus, als ob Schneidzahn als Wurfwaffe gedacht ist, im Original aber keine Entfernungsrechnung bekommt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-138"></a>
### 138. Robbentöter (`ITEM_ID_ROBBENTOETER`)

- **Im Code:** Gegenstands-ID 138 (`ITEM_ID_ROBBENTOETER`, Rohname in ITEMNAME `ROBBENTÖTER..`). ITEMS.DAT: Waffe; Gewicht 70 Unzen; Grundpreis 60 Silbertaler (= 600 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[51]`: TP 1W6+3, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 2, AT +0, PA +0. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 1, damit bei 1 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 1). Startausrüstung (`g_hero_startup_items`): Firnelf. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg106.c:startup_equipment`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-139"></a>
### 139. Wolfmesser (`ITEM_ID_WOLFSMESSER`)

- **Im Code:** Gegenstands-ID 139 (`ITEM_ID_WOLFSMESSER`, Rohname in ITEMNAME `WOLFMESSER..`). ITEMS.DAT: Waffe; Gewicht 50 Unzen; Grundpreis 90 Silbertaler (= 900 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[52]`: TP 1W6+3, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 2, AT +0, PA -1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: Sortimentswert 1, damit bei 1 von 20 Waffenhändlern im Angebot (Händler mit Sortimentsstufe ≤ 1). Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit Messer/Dolch dargestellt. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-140"></a>
### 140. Hexenbesen (`ITEM_ID_HEXENBESEN`)

- **Im Code:** Gegenstands-ID 140 (`ITEM_ID_HEXENBESEN`, Rohname in ITEMNAME `HEXENBESEN..`). ITEMS.DAT: Waffe, persönlich (nicht ablegbar); Gewicht 60 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[63]`: TP 1W6+1, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), unzerbrechlich (BF -99), AT +0, PA +0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Startausrüstung (`g_hero_startup_items`): Hexe. Nur Grafik: Im Kampf wird der Held damit als Kämpfer mit stumpfer Waffe dargestellt. Mit Fix 390 (Fanpatch-Stufe „nur Alle“) setzt der Kampf den Bruchfaktor dieser Waffe vor jeder Bruchprüfung (Attacke, Parade) auf -99 (unzerbrechlich), und der Zauber Eisenrost (von Helden und von Gegnern) kann sie nicht zerbrechen. Hintergrund laut Fix-Kommentar: bei Curians Stab stand ein falscher Bruchfaktor 5 im Gegenstand. (`Quelle/seg042.c:fix_set_unbreakable`, `Quelle/seg043.c:fix_set_unbreakable_pa`, `Quelle/seg100.c:fix_unbreakable_weapon`, `Quelle/seg102.c:fix_unbreakable_weapon`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg106.c:startup_equipment`, `Quelle/seg041.c:FIG_weapon_gfx_id_melee`, `Quelle/seg042.c:fix_set_unbreakable`, `Quelle/seg043.c:fix_set_unbreakable_pa`, `Quelle/seg100.c:fix_unbreakable_weapon`, `Quelle/seg102.c:fix_unbreakable_weapon`
- **Deutung der KI:** Vermutlich die Hexenwaffe. Mit Fix 390 ist sie wirklich unzerbrechlich.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-141"></a>
### 141. Lotusgift (`ITEM_ID_LOTUSGIFT`)

- **Im Code:** Gegenstands-ID 141 (`ITEM_ID_LOTUSGIFT`, Rohname in ITEMNAME `LOTUSGIFT..`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 100 Dukaten (= 10000 Heller). Benutzbar-Eintrag 8 (`g_usable_items_table`): Benutzen ruft `item_apply_weapon_poison()` auf. Gruppe Kraut/Gift (Untertyp 0). Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Krieger. Handel: Sortimentswert 2, damit bei 1 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 2). Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt Benutzen: wird auf den Gegenstand in der rechten Hand aufgetragen. Das geht mit jedem Gegenstand dort (das Spiel prüft nicht, ob es eine Waffe ist), nur nicht mit Kurzbogen, Langbogen und Armbrust (mit Fix 224 (Fanpatch-Stufe „nur Alle“) auch nicht mit der Schleuder). Mit leerer rechter Hand: Meldung: nichts zum Vergiften in der rechten Hand. Mit Fix 400 (Fanpatch-Stufe „nur Alle“) gilt auch hier die Klassensperre (z. B. für Krieger). Die Waffe bekommt 5 Ladungen: Jeder Treffer mit mindestens 1 Schadenspunkt richtet zusätzlich 10 × Giftstärke 10 = 100 Schadenspunkte an und verbraucht eine Ladung. Das Gift ist verbraucht, zurück kommt eine Glasflasche. (`Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg108.c:consume`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob Lotusgift im Code nur sehr hohen Zusatzschaden macht (10 × Giftstärke).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-142"></a>
### 142. Kukris (`ITEM_ID_KUKRIS`)

- **Im Code:** Gegenstands-ID 142 (`ITEM_ID_KUKRIS`, Rohname in ITEMNAME `KUKRIS..`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 90 Dukaten (= 9000 Heller). Benutzbar-Eintrag 8 (`g_usable_items_table`): Benutzen ruft `item_apply_weapon_poison()` auf. Gruppe Kraut/Gift (Untertyp 0). Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Krieger. Handel: Sortimentswert 2, damit bei 1 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 2). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt Benutzen: wird auf den Gegenstand in der rechten Hand aufgetragen. Das geht mit jedem Gegenstand dort (das Spiel prüft nicht, ob es eine Waffe ist), nur nicht mit Kurzbogen, Langbogen und Armbrust (mit Fix 224 (Fanpatch-Stufe „nur Alle“) auch nicht mit der Schleuder). Mit leerer rechter Hand: Meldung: nichts zum Vergiften in der rechten Hand. Mit Fix 400 (Fanpatch-Stufe „nur Alle“) gilt auch hier die Klassensperre (z. B. für Krieger). Die Waffe bekommt 5 Ladungen: Jeder Treffer mit mindestens 1 Schadenspunkt richtet zusätzlich 10 × Giftstärke 12 = 120 Schadenspunkte an und verbraucht eine Ladung. Das Gift ist verbraucht, zurück kommt eine Glasflasche. (`Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg108.c:consume`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob Kukris im Code nur sehr hohen Zusatzschaden macht (10 × Giftstärke).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-143"></a>
### 143. Bannstaub (`ITEM_ID_BANNSTAUB`)

- **Im Code:** Gegenstands-ID 143 (`ITEM_ID_BANNSTAUB`, Rohname in ITEMNAME `BANNSTAUB..`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 250 Dukaten (= 25000 Heller). Benutzbar-Eintrag 8 (`g_usable_items_table`): Benutzen ruft `item_apply_weapon_poison()` auf. Gruppe Kraut/Gift (Untertyp 0). Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Krieger. Handel: Sortimentswert 1, damit bei 1 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 1). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt Benutzen: wird auf den Gegenstand in der rechten Hand aufgetragen. Das geht mit jedem Gegenstand dort (das Spiel prüft nicht, ob es eine Waffe ist), nur nicht mit Kurzbogen, Langbogen und Armbrust (mit Fix 224 (Fanpatch-Stufe „nur Alle“) auch nicht mit der Schleuder). Mit leerer rechter Hand: Meldung: nichts zum Vergiften in der rechten Hand. Mit Fix 400 (Fanpatch-Stufe „nur Alle“) gilt auch hier die Klassensperre (z. B. für Krieger). Die Waffe bekommt 5 Ladungen: Jeder Treffer mit mindestens 1 Schadenspunkt richtet zusätzlich 10 × Giftstärke 15 = 150 Schadenspunkte an und verbraucht eine Ladung. Das Gift ist verbraucht, zurück kommt eine Glasflasche. (`Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg108.c:consume`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob Bannstaub im Code das stärkste Waffengift ist (10 × Giftstärke 15).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-144"></a>
### 144. Kroetenschemelgift (`ITEM_ID_KROETENSCHEMELGIFT`)

- **Im Code:** Gegenstands-ID 144 (`ITEM_ID_KROETENSCHEMELGIFT`, Rohname in ITEMNAME `KROETENSCHEMELGIFT..`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 15 Dukaten (= 1500 Heller). Benutzbar-Eintrag 8 (`g_usable_items_table`): Benutzen ruft `item_apply_weapon_poison()` auf. Gruppe Kraut/Gift (Untertyp 0). Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Krieger. Handel: Sortimentswert 15, damit bei 16 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 15). Einnehmen wird verweigert: Meldung, dass der Held das Gift nicht in den Mund nimmt Benutzen: wird auf den Gegenstand in der rechten Hand aufgetragen. Das geht mit jedem Gegenstand dort (das Spiel prüft nicht, ob es eine Waffe ist), nur nicht mit Kurzbogen, Langbogen und Armbrust (mit Fix 224 (Fanpatch-Stufe „nur Alle“) auch nicht mit der Schleuder). Mit leerer rechter Hand: Meldung: nichts zum Vergiften in der rechten Hand. Mit Fix 400 (Fanpatch-Stufe „nur Alle“) gilt auch hier die Klassensperre (z. B. für Krieger). Die Waffe bekommt 5 Ladungen: Jeder Treffer mit mindestens 1 Schadenspunkt richtet zusätzlich 10 × Giftstärke 4 = 40 Schadenspunkte an und verbraucht eine Ladung. Das Gift ist verbraucht, zurück kommt eine Glasflasche. (`Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg055.c:buy_screen`, `Quelle/seg108.c:consume`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob Krötenschemelgift im Code nur Zusatzschaden macht (10 × Giftstärke).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-145"></a>
### 145. Heiltrank (`ITEM_ID_HEILTRANK`)

- **Im Code:** Gegenstands-ID 145 (`ITEM_ID_HEILTRANK`, Rohname in ITEMNAME `HEILTR.ANK.ÄNKE`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: Sortimentswert 18, damit bei 17 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 18). Liegt in Truhen/Beutetabellen im Code: Zwergenfeste, Zwingfeste (2×), Verfallene Herberge (2×), Spinnenhöhle, Drachenhort (3×), Piratenhöhle (3×), Totenschiff. Kann gebraut werden (Rezept ID 186). Drachenhort: beim Plündern des Horts 2 Stück, als Lohn des Drachen für den Platinschlüssel nochmals 2 Stück. (`Quelle/seg085.c:DNG10_handler`) Einnehmen (Essen/Trinken-Menü): Heilt 10 LE (höchstens bis zum Maximum), zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg085.c:DNG10_handler`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich der Standard-Heiltrank (10 LE).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-146"></a>
### 146. Starker Heiltrank (`ITEM_ID_STARKER_HEILTRANK`)

- **Im Code:** Gegenstands-ID 146 (`ITEM_ID_STARKER_HEILTRANK`, Rohname in ITEMNAME `STARKE.R HEILTRANK. HEILTRÄNKE`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 24 Dukaten (= 2400 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: Sortimentswert 10, damit bei 10 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 10). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota, Reiseereignis Ottarje–Orvil (2×), Zwingfeste, Wolfshöhle, Daspotaschatz (2×), Drachenhort. Kann gebraut werden (Rezept ID 240). Einnehmen (Essen/Trinken-Menü): Heilt 1W20+10 LE (höchstens bis zum Maximum), zurück kommt eine Bronzeflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich der stärkere Heiltrank (1W20+10 LE).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-147"></a>
### 147. MU-Elixier (`ITEM_ID_MU_ELIXIER`)

- **Im Code:** Gegenstands-ID 147 (`ITEM_ID_MU_ELIXIER`, Rohname in ITEMNAME `MU-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: Sortimentswert 9, damit bei 9 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 9). Liegt in Truhen/Beutetabellen im Code: Orkbehausung. Kann gebraut werden (Rezept ID 205). Ruine des Schwarzmagiers: liegt auf Feld (1,6,1) (dort erscheint zuerst eine Elfengestalt, die allen Helden der Gruppe MU +3 gibt), einmal mitnehmbar. (`Quelle/seg082.c:DNG07_handler`) Einnehmen (Essen/Trinken-Menü): MU +5 für 1 Stunde, zurück kommt eine Glasflasche. Im Original addieren sich mehrere gleiche Elixiere (je +5). Mit Fix 404 (Fanpatch-Stufe „Wichtig“) verlängert ein weiteres nur die Dauer. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg082.c:DNG07_handler`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein kurzzeitiger Mutschub. Im Original lassen sich mehrere stapeln.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-148"></a>
### 148. KL-Elixier (`ITEM_ID_KL_ELIXIER`)

- **Im Code:** Gegenstands-ID 148 (`ITEM_ID_KL_ELIXIER`, Rohname in ITEMNAME `KL-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: Sortimentswert 9, damit bei 9 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 9). Liegt in Truhen/Beutetabellen im Code: Verfallene Herberge, Piratenhöhle. Einnehmen (Essen/Trinken-Menü): KL +5 für 1 Stunde, zurück kommt eine Glasflasche. Im Original addieren sich mehrere gleiche Elixiere (je +5). Mit Fix 404 (Fanpatch-Stufe „Wichtig“) verlängert ein weiteres nur die Dauer. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein kurzzeitiger Klugheitsschub. Im Original lassen sich mehrere stapeln.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-149"></a>
### 149. CH-Elixier (`ITEM_ID_CH_ELIXIER`)

- **Im Code:** Gegenstands-ID 149 (`ITEM_ID_CH_ELIXIER`, Rohname in ITEMNAME `CH-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: Sortimentswert 9, damit bei 9 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 9). Liegt in Truhen/Beutetabellen im Code: Verfallene Herberge (2×). Einnehmen (Essen/Trinken-Menü): CH +5 für 1 Stunde, zurück kommt eine Glasflasche. Im Original addieren sich mehrere gleiche Elixiere (je +5). Mit Fix 404 (Fanpatch-Stufe „Wichtig“) verlängert ein weiteres nur die Dauer. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein kurzzeitiger Charismaschub. Im Original lassen sich mehrere stapeln.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-150"></a>
### 150. FF-Elixier (`ITEM_ID_FF_ELIXIER`)

- **Im Code:** Gegenstands-ID 150 (`ITEM_ID_FF_ELIXIER`, Rohname in ITEMNAME `FF-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: Sortimentswert 9, damit bei 9 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 9). Einnehmen (Essen/Trinken-Menü): FF +5 für 1 Stunde, zurück kommt eine Glasflasche. Im Original addieren sich mehrere gleiche Elixiere (je +5). Mit Fix 404 (Fanpatch-Stufe „Wichtig“) verlängert ein weiteres nur die Dauer. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein kurzzeitiger Fingerfertigkeitsschub. Im Original lassen sich mehrere stapeln.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-151"></a>
### 151. GE-Elixier (`ITEM_ID_GE_ELIXIER`)

- **Im Code:** Gegenstands-ID 151 (`ITEM_ID_GE_ELIXIER`, Rohname in ITEMNAME `GE-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: Sortimentswert 9, damit bei 9 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 9). Einnehmen (Essen/Trinken-Menü): GE +5 für 1 Stunde, zurück kommt eine Glasflasche. Im Original addieren sich mehrere gleiche Elixiere (je +5). Mit Fix 404 (Fanpatch-Stufe „Wichtig“) verlängert ein weiteres nur die Dauer. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein kurzzeitiger Gewandtheitsschub. Im Original lassen sich mehrere stapeln.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-152"></a>
### 152. IN-Elixier (`ITEM_ID_IN_ELIXIER`)

- **Im Code:** Gegenstands-ID 152 (`ITEM_ID_IN_ELIXIER`, Rohname in ITEMNAME `IN-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: Sortimentswert 9, damit bei 9 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 9). Einnehmen (Essen/Trinken-Menü): IN +5 für 1 Stunde, zurück kommt eine Glasflasche. Im Original addieren sich mehrere gleiche Elixiere (je +5). Mit Fix 404 (Fanpatch-Stufe „Wichtig“) verlängert ein weiteres nur die Dauer. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein kurzzeitiger Intuitionsschub. Im Original lassen sich mehrere stapeln.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-153"></a>
### 153. KK-Elixier (`ITEM_ID_KK_ELIXIER`)

- **Im Code:** Gegenstands-ID 153 (`ITEM_ID_KK_ELIXIER`, Rohname in ITEMNAME `KK-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: Sortimentswert 9, damit bei 9 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 9). Liegt in Truhen/Beutetabellen im Code: Verfallene Herberge, Daspotaschatz. Kann gebraut werden (Rezept ID 204). Einnehmen (Essen/Trinken-Menü): KK +5 für 1 Stunde, zurück kommt eine Glasflasche. Im Original addieren sich mehrere gleiche Elixiere (je +5). Mit Fix 404 (Fanpatch-Stufe „Wichtig“) verlängert ein weiteres nur die Dauer. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein kurzzeitiger Körperkraftschub. Im Original lassen sich mehrere stapeln.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-154"></a>
### 154. Zaubertrank (`ITEM_ID_ZAUBERTRANK`)

- **Im Code:** Gegenstands-ID 154 (`ITEM_ID_ZAUBERTRANK`, Rohname in ITEMNAME `ZAUBERTR.ANK.ÄNKE`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: Sortimentswert 18, damit bei 17 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 18). Liegt in Truhen/Beutetabellen im Code: Zwergenfeste, Zwingfeste (5×), Spinnenhöhle (4×), Kultstätte des Namenlosen, Drachenhort (2×), Piratenhöhle. Kann gebraut werden (Rezept ID 206). Drachenhort: beim Plündern des Horts 1 Stück, als Lohn des Drachen für den Platinschlüssel nochmals 1 Stück. (`Quelle/seg085.c:DNG10_handler`) Einnehmen (Essen/Trinken-Menü): Nur für Zauberkundige (Hexe, Druide, Magier, Elfen): bis zu 10 AE zurück. Andere: Meldung, dass das wenig Sinn hatte (Trank trotzdem verbraucht). Zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg085.c:DNG10_handler`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich nur für Zauberkundige sinnvoll (bis 10 AE).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-155"></a>
### 155. Zaubertrank (stark) (`ITEM_ID_ZAUBERTRANK_STARK`)

- **Im Code:** Gegenstands-ID 155 (`ITEM_ID_ZAUBERTRANK_STARK`, Rohname in ITEMNAME `ZAUBERTR.ANK (STARK).ÄNKE`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: Sortimentswert 10, damit bei 10 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 10). Liegt in Truhen/Beutetabellen im Code: Zwingfeste (5×), Daspotaschatz (2×), Drachenhort. Kann gebraut werden (Rezept ID 243). Einnehmen (Essen/Trinken-Menü): Nur für Zauberkundige: bis zu 30 AE zurück. Andere: Meldung, dass das wenig Sinn hatte (Trank trotzdem verbraucht). Zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich nur für Zauberkundige sinnvoll (bis 30 AE).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-156"></a>
### 156. Olginwurzel (`ITEM_ID_OLGINWURZEL`)

- **Im Code:** Gegenstands-ID 156 (`ITEM_ID_OLGINWURZEL`, Rohname in ITEMNAME `OLGINWURZEL..N`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 2 Dukaten (= 200 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 4, damit bei 2 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 4). Einnehmen wird verweigert: Meldung: unbehandelt ungenießbar Heilkunde Krankheiten: Der Heiler braucht das passende Kraut im eigenen Inventar, sonst Meldung: die nötigen Kräuter fehlen. Das Kraut wird nur bei gelungener Heilung verbraucht. Olginwurzel hilft gegen Dumpfschädel. (`Quelle/seg104.c:has_herb_for_disease`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/seg108.c:consume`, `Quelle/seg104.c:has_herb_for_disease`
- **Deutung der KI:** Vermutlich vor allem gegen Dumpfschädel nützlich. Roh ist sie ungenießbar.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-157"></a>
### 157. Kairanhalm (`ITEM_ID_KAIRANHALM`)

- **Im Code:** Gegenstands-ID 157 (`ITEM_ID_KAIRANHALM`, Rohname in ITEMNAME `KAIRANHALM..E`). ITEMS.DAT: benutzbar, stapelbar, Kraut/Gift/Trank; Gewicht 1 Unzen; Grundpreis 15 Silbertaler (= 150 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Kraut/Gift (Untertyp 0). Handel: Sortimentswert 4, damit bei 2 von 17 Kräuterhändlern im Angebot (Händler mit Sortimentsstufe ≤ 4). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ottarje–Orvil. Alchimie-Zutat (2×) für Zaubertrank (Rezept ID 206). Alchimie-Zutat (3×) für Zaubertrank (stark) (Rezept ID 243). Alchimie-Zutat (1×) für Miasthmaticum (Rezept ID 244). Kräutersuche beim Lagern (`g_gather_herbs_table`): 1 % Chance, dann Pflanzenkunde-Probe +18 (leichter je Suchstunde), bei Erfolg 1 bis 1 Stück. Kräuterplatz im Reiseereignis Nr. 134 (Einsiedlersee (Rundweg)): Dort sind die Chancen für dieses Kraut besser (+10 % Fundchance, +1 Höchstmenge). Im Original wirkt das wegen eines Zahlenüberlaufs (Gegenstandsnummer über 127) nie, erst mit Fix 161. Einnehmen wird verweigert: Meldung: unbehandelt ungenießbar
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg055.c:buy_screen`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg051.c:gather_herbs`, `Quelle/seg116.c:tevent_134`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich vor allem Handelsware oder Alchimie-Zutat; eine eigene Wirkung ist im Code nicht zu sehen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-158"></a>
### 158. Bastardschwert (magisch) (`ITEM_ID_BASTARDSCHWERT__MAGIC`)

- **Im Code:** Gegenstands-ID 158 (`ITEM_ID_BASTARDSCHWERT__MAGIC`, Rohname in ITEMNAME `BASTARDSCHWERT..ER`). ITEMS.DAT: Waffe, benutzbar, magisch; Gewicht 50 Unzen; Grundpreis 110 Silbertaler (= 1100 Heller). Waffe, Waffenart Zweihänder (Untertyp 6), Waffentabelle `g_weapon_stats_table[54]`: TP 1W6+5, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), Bruchfaktor -5, AT -2, PA -3. Als benutzbar markiert, aber ohne Benutzen-Funktion (Eintrag 54). Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Thorwaler, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 4 und gibt einen eigenen Text aus (SPELLTXT Nr. 66, sinngemäß: eine hervorragende Waffe). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-159"></a>
### 159. Orknase (magisch) (`ITEM_ID_ORKNASE__MAGIC`)

- **Im Code:** Gegenstands-ID 159 (`ITEM_ID_ORKNASE__MAGIC`, Rohname in ITEMNAME `ORKNASE..N`). ITEMS.DAT: Waffe, magisch; Gewicht 120 Unzen; Grundpreis 60 Silbertaler (= 600 Heller). Waffe, Waffenart Axt (Untertyp 4), Waffentabelle `g_weapon_stats_table[55]`: TP 1W6+10, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), unzerbrechlich (BF -99), AT -2, PA -8. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 4 und gibt einen eigenen Text aus (SPELLTXT Nr. 67, sinngemäß: mehr Wucht auf Kosten der Handhabung). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Mit Fix 390 (Fanpatch-Stufe „nur Alle“) setzt der Kampf den Bruchfaktor dieser Waffe vor jeder Bruchprüfung (Attacke, Parade) auf -99 (unzerbrechlich), und der Zauber Eisenrost (von Helden und von Gegnern) kann sie nicht zerbrechen. Hintergrund laut Fix-Kommentar: bei Curians Stab stand ein falscher Bruchfaktor 5 im Gegenstand. (`Quelle/seg042.c:fix_set_unbreakable`, `Quelle/seg043.c:fix_set_unbreakable_pa`, `Quelle/seg100.c:fix_unbreakable_weapon`, `Quelle/seg102.c:fix_unbreakable_weapon`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg042.c:fix_set_unbreakable`, `Quelle/seg043.c:fix_set_unbreakable_pa`, `Quelle/seg100.c:fix_unbreakable_weapon`, `Quelle/seg102.c:fix_unbreakable_weapon`
- **Deutung der KI:** Vermutlich eine magische Variante der Orknase, die erst mit Fix 390 wirklich unzerbrechlich ist.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-160"></a>
### 160. Kurzschwert (magisch) (`ITEM_ID_KURZSCHWERT__MAGIC`)

- **Im Code:** Gegenstands-ID 160 (`ITEM_ID_KURZSCHWERT__MAGIC`, Rohname in ITEMNAME `KURZSCHWERT..ER`). ITEMS.DAT: Waffe, magisch; Gewicht 40 Unzen; Grundpreis 45 Silbertaler (= 450 Heller). Waffe, Waffenart Schwert (Untertyp 3), Waffentabelle `g_weapon_stats_table[56]`: TP 1W6+2, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), unzerbrechlich (BF -99), AT +2, PA -1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 4 und gibt einen eigenen Text aus (SPELLTXT Nr. 68, sinngemäß: verbessert die Attacke entscheidend). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Mit Fix 390 (Fanpatch-Stufe „nur Alle“) setzt der Kampf den Bruchfaktor dieser Waffe vor jeder Bruchprüfung (Attacke, Parade) auf -99 (unzerbrechlich), und der Zauber Eisenrost (von Helden und von Gegnern) kann sie nicht zerbrechen. Hintergrund laut Fix-Kommentar: bei Curians Stab stand ein falscher Bruchfaktor 5 im Gegenstand. (`Quelle/seg042.c:fix_set_unbreakable`, `Quelle/seg043.c:fix_set_unbreakable_pa`, `Quelle/seg100.c:fix_unbreakable_weapon`, `Quelle/seg102.c:fix_unbreakable_weapon`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg042.c:fix_set_unbreakable`, `Quelle/seg043.c:fix_set_unbreakable_pa`, `Quelle/seg100.c:fix_unbreakable_weapon`, `Quelle/seg102.c:fix_unbreakable_weapon`
- **Deutung der KI:** Vermutlich eine magische Variante des Kurzschwerts, die erst mit Fix 390 wirklich unzerbrechlich ist.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-161"></a>
### 161. Sichel (magisch) (`ITEM_ID_SICHEL__MAGIC`)

- **Im Code:** Gegenstands-ID 161 (`ITEM_ID_SICHEL__MAGIC`, Rohname in ITEMNAME `SICHEL..N`). ITEMS.DAT: Waffe, magisch; Gewicht 30 Unzen; Grundpreis 25 Silbertaler (= 250 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[25]`: TP 1W6+2, KK-Zuschlag ab KK 17 (je Punkt darüber +1 TP), Bruchfaktor 5, AT -3, PA -4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 6 und gibt einen eigenen Text aus (SPELLTXT Nr. 69, sinngemäß: ein druidisches Artefakt, das den Weg zu Kräutern weist (der Analysierende ist sich nicht sicher)). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Wirkt schon im Inventar: Beim Erhalt bekommt der Held Pflanzenkunde +3, beim Weggeben/Verlieren wieder -3. (`Quelle/seg105.c:give_new_item_to_hero`, `Quelle/seg105.c:drop_item`) Beim Weitergeben an einen anderen Helden wandert der Bonus Pflanzenkunde +3 mit. (`Quelle/seg106.c:pass_item`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg105.c:give_new_item_to_hero`, `Quelle/seg105.c:drop_item`, `Quelle/seg106.c:pass_item`
- **Deutung der KI:** Sieht so aus, als ob die Sichel schon im Gepäck wirkt (Pflanzenkunde +3). Sie lohnt sich für den Kräutersammler der Gruppe.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-162"></a>
### 162. Amulett (gegen Ignifaxius) (`ITEM_ID_AMULETT__ANTI_IGNIFAXIUS`)

- **Im Code:** Gegenstands-ID 162 (`ITEM_ID_AMULETT__ANTI_IGNIFAXIUS`, Rohname in ITEMNAME `AMULETT..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird), magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Rovamund–Nordvest. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 2 und gibt einen eigenen Text aus (SPELLTXT Nr. 54, sinngemäß: dient der Abwehr von Ignifaxius). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`
- **Deutung der KI:** Sieht so aus, als ob Analüs eine Wirkung verspricht, die im Code an keiner Stelle abgefragt wird.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-163"></a>
### 163. Amulett (MR-Bonus) (`ITEM_ID_AMULETT__MR_BONUS`)

- **Im Code:** Gegenstands-ID 163 (`ITEM_ID_AMULETT__MR_BONUS`, Rohname in ITEMNAME `AMULETT..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird), magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 2 und gibt einen eigenen Text aus (SPELLTXT Nr. 70, sinngemäß: ein Schutzamulett, besonders gegen Verwandlungen und Kampfzauber). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Bei einem Stufenaufstieg, der die Magieresistenz ändert, schlägt das Spiel für das Amulett im Inventar wieder 5 MR auf (im Original höchstens einmal, mit Fix 250 (Fanpatch-Stufe „Wichtig“) je Amulett). Im Original vergleicht der Stufenaufstieg den MR-Wert mitsamt Gegenstandsboni mit dem neuen Grundwert, deshalb kann der Bonus dort verloren gehen. Fix 250 (Fanpatch-Stufe „Wichtig“) vergleicht den Grundwert. (`Quelle/seg050.c:level_up`) Wirkt schon im Inventar: Beim Erhalt bekommt der Held MR +5, beim Weggeben/Verlieren wieder -5 (je Amulett). (`Quelle/seg105.c:give_new_item_to_hero`, `Quelle/seg105.c:drop_item`) Beim Weitergeben an einen anderen Helden wandert der Bonus MR +5 mit. (`Quelle/seg106.c:pass_item`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg050.c:level_up`, `Quelle/seg105.c:give_new_item_to_hero`, `Quelle/seg105.c:drop_item`, `Quelle/seg106.c:pass_item`
- **Deutung der KI:** Sieht so aus, als ob das Amulett schon im Gepäck wirkt (MR +5). Im Original kann der Bonus beim Stufenanstieg verloren gehen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-164"></a>
### 164. Knochen mit Rune (`ITEM_ID_KNOCHEN_MIT_RUNE`)

- **Im Code:** Gegenstands-ID 164 (`ITEM_ID_KNOCHEN_MIT_RUNE`, Rohname in ITEMNAME `KNOCHEN MIT RUNE..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird), magisch; Gewicht 10 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ottarje–Orvil. Taucht der Knochen mit Rune in der Beuteliste eines Kampfes auf, bricht die Beuteliste im Original an dieser Stelle ab (alle Gegenstände dahinter werden nie angeboten). Mit Fix 16 (Fanpatch-Stufe „nur Alle“) wird nur der Knochen aus der Liste genommen. (`Quelle/seg035.c:FIG_loot_enemies`) Umbrik Siebenstein fragt im Gespräch nach dem Knochen mit Rune. Hat die Gruppe ihn, nimmt Umbrik ihn an sich (er verschwindet aus dem Inventar). (`Quelle/seg072.c:INF_yasma_umbrik_isleif`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg035.c:FIG_loot_enemies`, `Quelle/seg072.c:INF_yasma_umbrik_isleif`
- **Deutung der KI:** Vermutlich ein Questgegenstand für Umbrik. Er bringt im Original die Kampfbeute durcheinander.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-165"></a>
### 165. Ring (rot) (`ITEM_ID_RING__RED`)

- **Im Code:** Gegenstands-ID 165 (`ITEM_ID_RING__RED`, Rohname in ITEMNAME `RING..E`). ITEMS.DAT: benutzbar, magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzbar-Eintrag 1 (`g_usable_items_table`): Benutzen ruft `item_invoke_arcano()` auf, 5 Ladungen. Benutzen: Der Benutzer wählt einen Helden der Gruppe, auf den der Zauber Arcano Psychostabilis wirkt (wie beim Zaubern, siehe Zauberliste); das kostet eine von 5 Ladungen. Sind alle Ladungen weg: Meldung: die gespeicherte Magie ist aufgebraucht Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ottarje–Orvil. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 3 und gibt einen eigenen Text aus (SPELLTXT Nr. 56, sinngemäß: ein Schutzring, der die magischen Abwehrkräfte erhöht). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Bei einem Stufenaufstieg, der die Magieresistenz ändert, schlägt das Spiel den MR-Bonus von 2 für den roten Ring im Original nur in der linken Hand wieder auf (in der rechten Hand geht er verloren). Mit Fix 250 (Fanpatch-Stufe „Wichtig“) zählen beide Hände. (`Quelle/seg050.c:level_up`) Angelegt (an einem Körperplatz oder in einer Hand): MR +2, beim Ablegen wieder -2 (in jeder Hand, also mit zwei roten Ringen +4). (`Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_invoke_arcano`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg050.c:level_up`, `Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`
- **Deutung der KI:** Vermutlich ein MR-Ring (+2 in jeder Hand). Im Original geht der Bonus in der rechten Hand beim Stufenanstieg verloren.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-166"></a>
### 166. Expurgicum (`ITEM_ID_EXPURGICUM`)

- **Im Code:** Gegenstands-ID 166 (`ITEM_ID_EXPURGICUM`, Rohname in ITEMNAME `EXPURGICUM..`). ITEMS.DAT: benutzbar; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzbar-Eintrag 8 (`g_usable_items_table`): Benutzen ruft `item_apply_weapon_poison()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ottarje–Orvil. Kann gebraut werden (Rezept ID 167). Benutzen: wird auf den Gegenstand in der rechten Hand aufgetragen. Das geht mit jedem Gegenstand dort (das Spiel prüft nicht, ob es eine Waffe ist), nur nicht mit Kurzbogen, Langbogen und Armbrust (mit Fix 224 (Fanpatch-Stufe „nur Alle“) auch nicht mit der Schleuder). Mit leerer rechter Hand: Meldung: nichts zum Vergiften in der rechten Hand. Mit Fix 400 (Fanpatch-Stufe „nur Alle“) gilt auch hier die Klassensperre (z. B. für Krieger). Der nächste Treffer mit mindestens 1 Schadenspunkt richtet einmalig 1W6+2 Schadenspunkte mehr an. Zurück kommt eine Bronzeflasche. (`Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Vermutlich ein einmaliger Zusatzschaden für den nächsten Treffer (1W6+2).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-167"></a>
### 167. Rezept für Expurgicum (`ITEM_ID_REZEPT_FUER_EXPURGICUM`)

- **Im Code:** Gegenstands-ID 167 (`ITEM_ID_REZEPT_FUER_EXPURGICUM`, Rohname in ITEMNAME `REZEPT FÜR EXPURGICUM..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ottarje–Orvil, Verfallene Herberge. Rezept für Expurgicum (ID 166): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Bronzeflasche, 1× Lakritze, 2× Shurinknolle, 2× Alraune; 0 AE, Alchimie-Probe +8, Dauer 48 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um Expurgicum brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-168"></a>
### 168. Vomicum (`ITEM_ID_VOMICUM`)

- **Im Code:** Gegenstands-ID 168 (`ITEM_ID_VOMICUM`, Rohname in ITEMNAME `VOMICUM..`). ITEMS.DAT: benutzbar; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzbar-Eintrag 8 (`g_usable_items_table`): Benutzen ruft `item_apply_weapon_poison()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Kann gebraut werden (Rezept ID 169). Benutzen: wird auf den Gegenstand in der rechten Hand aufgetragen. Das geht mit jedem Gegenstand dort (das Spiel prüft nicht, ob es eine Waffe ist), nur nicht mit Kurzbogen, Langbogen und Armbrust (mit Fix 224 (Fanpatch-Stufe „nur Alle“) auch nicht mit der Schleuder). Mit leerer rechter Hand: Meldung: nichts zum Vergiften in der rechten Hand. Mit Fix 400 (Fanpatch-Stufe „nur Alle“) gilt auch hier die Klassensperre (z. B. für Krieger). Der nächste Treffer mit mindestens 1 Schadenspunkt richtet einmalig 1W20+5 Schadenspunkte mehr an. Zurück kommt eine Bronzeflasche. (`Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_apply_weapon_poison`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Vermutlich ein einmaliger kräftiger Zusatzschaden für den nächsten Treffer (1W20+5).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-169"></a>
### 169. Rezept für Vomicum (`ITEM_ID_REZEPT_FUER_VOMICUM`)

- **Im Code:** Gegenstands-ID 169 (`ITEM_ID_REZEPT_FUER_VOMICUM`, Rohname in ITEMNAME `REZEPT FÜR VOMICUM..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Verfallene Herberge (2×), Reiseereignis Ala–Tjanset. Rezept für Vomicum (ID 168): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Bronzeflasche, 1× Öl, 1× Shurinknolle, 1× Alraune; 0 AE, Alchimie-Probe +5, Dauer 8 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um Vomicum brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-170"></a>
### 170. Dokument (Einhorn) (`ITEM_ID_DOKUMENT__UNICORN`)

- **Im Code:** Gegenstands-ID 170 (`ITEM_ID_DOKUMENT__UNICORN`, Rohname in ITEMNAME `DOKUMENT..E`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 3 (`g_usable_items_table`): Benutzen ruft `item_read_document()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Felsteyn–Orkanger. Benutzen (Lesen) zeigt nur einen Text: ein Hinweis auf die Hilfe des Einhorns Weitere Wirkung hat das Lesen nicht. (`Quelle/seg107.c:item_read_document`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_document`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein Hinweis-Dokument ohne eigene Spielwirkung.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-171"></a>
### 171. Silberner Stirnreif (Fassung 1) (`ITEM_ID_SILBERNER_STIRNREIF__1`)

- **Im Code:** Gegenstands-ID 171 (`ITEM_ID_SILBERNER_STIRNREIF__1`, Rohname in ITEMNAME `SILBERNE.R STIRNREIF. STIRNREIFEN`). ITEMS.DAT: benutzbar, magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzbar-Eintrag 4 (`g_usable_items_table`): Benutzen ruft `item_invoke_armatrutz()` auf, 3 Ladungen. Benutzen: Der Benutzer wählt einen Helden der Gruppe, auf den der Zauber Armatrutz wirkt (wie beim Zaubern, siehe Zauberliste); das kostet eine von 3 Ladungen. Sind alle Ladungen weg: Meldung: die gespeicherte Magie ist aufgebraucht Handel: bei keinem Händler im Angebot (Sortimentswert 0). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 4 und gibt einen eigenen Text aus (SPELLTXT Nr. 57, sinngemäß: elfische Machart, ein Armatrutz ist enthalten). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Darf als einer von drei Stirnreifen (IDs 171, 217, 245) auf dem Kopf getragen werden, obwohl er keine Rüstung ist. (`Quelle/seg105.c:can_hero_equip_item_at_slot`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_invoke_armatrutz`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg105.c:can_hero_equip_item_at_slot`
- **Deutung der KI:** Sieht so aus, als ob der Stirnreif beim Benutzen Armatrutz wirkt (siehe Benutzbar-Eintrag) und auf dem Kopf getragen werden darf.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-172"></a>
### 172. Säbel (magisch) (`ITEM_ID_SAEBEL__MAGIC`)

- **Im Code:** Gegenstands-ID 172 (`ITEM_ID_SAEBEL__MAGIC`, Rohname in ITEMNAME `SÄBEL..`). ITEMS.DAT: Waffe, magisch; Gewicht 60 Unzen; Grundpreis 60 Silbertaler (= 600 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[57]`: TP 1W6+3, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 0, AT +0, PA +0. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 3 und gibt einen eigenen Text aus (SPELLTXT Nr. 58, sinngemäß: besonders wirkungsvoll gegen Zombies, Skelette und Mumien). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Der magische Säbel richtet gegen Skelette und Zombies 1 Schadenspunkt mehr an. (`Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Vermutlich ein Säbel mit kleinem Bonus gegen Untote (Skelette und Zombies, +1).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-173"></a>
### 173. Amulett (Feuerbann) (`ITEM_ID_AMULETT__FEUERBANN`)

- **Im Code:** Gegenstands-ID 173 (`ITEM_ID_AMULETT__FEUERBANN`, Rohname in ITEMNAME `AMULETT..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird), magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 6 und gibt einen eigenen Text aus (SPELLTXT Nr. 59, sinngemäß: trägt den Hexenzauber Feuerbann und fängt Feuerschaden größtenteils ab). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg099.c:g_analues_items`
- **Deutung der KI:** Sieht so aus, als ob Analüs eine Wirkung verspricht, die im Code an keiner Stelle abgefragt wird.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-174"></a>
### 174. Amulett (Flim Flam) (`ITEM_ID_AMULETT__FLIM_FLAM`)

- **Im Code:** Gegenstands-ID 174 (`ITEM_ID_AMULETT__FLIM_FLAM`, Rohname in ITEMNAME `AMULETT..E`). ITEMS.DAT: benutzbar, magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzbar-Eintrag 5 (`g_usable_items_table`): Benutzen ruft `item_invoke_flim_flam()` auf, 6 Ladungen. Benutzen: wirkt den Zauber Flim Flam Funkel (wie beim Zaubern, siehe Zauberliste); das kostet eine von 6 Ladungen. Sind alle Ladungen weg: Meldung: die gespeicherte Magie ist aufgebraucht Handel: bei keinem Händler im Angebot (Sortimentswert 0). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 0 und gibt einen eigenen Text aus (SPELLTXT Nr. 60, sinngemäß: trägt Flim Flam Funkel, man braucht keine Fackeln oder Öllampen). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Liegt auf dem Totenschiff auf Feld (4,2,9). Wer es nimmt, verliert 100 Punkte Ansehen bei Boron. (`Quelle/seg077.c:DNG01_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_invoke_flim_flam`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg077.c:DNG01_handler`
- **Deutung der KI:** Sieht so aus, als ob das Amulett eine Lichtquelle ersetzt (Flim Flam, siehe Benutzbar-Eintrag). Das Mitnehmen kostet aber Ansehen bei Boron.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-175"></a>
### 175. Amulett (Travia) (`ITEM_ID_AMULETT__TRAVIA`)

- **Im Code:** Gegenstands-ID 175 (`ITEM_ID_AMULETT__TRAVIA`, Rohname in ITEMNAME `AMULETT..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird), magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 6 und gibt einen eigenen Text aus (SPELLTXT Nr. 61, sinngemäß: macht unempfindlich gegen Hunger und Durst). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Trägt ein Held das Travia-Amulett im Inventar, isst und trinkt er in der stündlichen Versorgung nicht (Hunger und Durst werden für ihn dort übersprungen). (`Quelle/seg002.c:herokeeping`) Wolfshöhle: findet ein Held in einem Spalt (er wird dabei von einer Ratte gebissen, -2 LE), einmalig. (`Quelle/seg080.c:DNG04_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg002.c:herokeeping`, `Quelle/seg080.c:DNG04_handler`
- **Deutung der KI:** Sieht so aus, als ob der Träger mit diesem Amulett nie essen oder trinken muss, wenigstens bei der stündlichen Versorgung.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-176"></a>
### 176. Schuldbuch (`ITEM_ID_SCHULDBUCH`)

- **Im Code:** Gegenstands-ID 176 (`ITEM_ID_SCHULDBUCH`, Rohname in ITEMNAME `SCHULDBUCH..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 6 (`g_usable_items_table`): Benutzen ruft `item_read_schuldbuch()` auf. Benutzen (Lesen): Beim ersten Lesen bekommt jeder Held 15 AP, und der Informantenstatus von Hjore Ahrensson wird aktualisiert (update_informer_cond). Danach zeigt es nur noch den Text (Beschreibung des Schuldbuchs). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Wolfshöhle. Im Gespräch mit Hjore Ahrensson prüft das Spiel, ob die Gruppe das Schuldbuch dabei hat. Davon hängt ab, wie das Gespräch weitergeht. (`Quelle/seg072.c:INF_jurge_hjore`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_schuldbuch`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg072.c:INF_jurge_hjore`
- **Deutung der KI:** Vermutlich ein Questgegenstand für das Gespräch mit Hjore Ahrensson.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-177"></a>
### 177. Rote Mondscheibe (`ITEM_ID_ROTE_MONDSCHEIBE`)

- **Im Code:** Gegenstands-ID 177 (`ITEM_ID_ROTE_MONDSCHEIBE`, Rohname in ITEMNAME `ROTE MONDSCHEIBE..N`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 20 Silbertaler (= 200 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Orkbehausung. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-178"></a>
### 178. Zweihänder (magisch) (`ITEM_ID_ZWEIHAENDER__MAGIC`)

- **Im Code:** Gegenstands-ID 178 (`ITEM_ID_ZWEIHAENDER__MAGIC`, Rohname in ITEMNAME `ZWEIHÄNDER..`). ITEMS.DAT: Waffe, magisch; Gewicht 160 Unzen; Grundpreis 50 Dukaten (= 5000 Heller). Waffe, Waffenart Zweihänder (Untertyp 6), Waffentabelle `g_weapon_stats_table[58]`: TP 2W6+4, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), unzerbrechlich (BF -99), AT -3, PA -4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Thorwaler, Hexe, Druide, Magier, Auelf, Firnelf, Waldelf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Orkbehausung. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 0 und gibt einen eigenen Text aus (SPELLTXT Nr. 62, sinngemäß: die Waffe scheint unzerbrechlich). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-179"></a>
### 179. Orkdokument (`ITEM_ID_ORKDOKUMENT`)

- **Im Code:** Gegenstands-ID 179 (`ITEM_ID_ORKDOKUMENT`, Rohname in ITEMNAME `ORKDOKUMENT..E`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 7 (`g_usable_items_table`): Benutzen ruft `item_read_orkdokument()` auf. Benutzen (Lesen): Mit gelungener Sprachen-Probe +4 des Lesers (oder wenn es schon einmal gelesen wurde): der Text erklärt, dass es ein Aufmarschplan der Orks ist, beim ersten Mal bekommt die Gruppe 20 AP. Gelesen erlaubt es im Endkampf-Ereignis, bis zum richtigen Tag zu warten (siehe Grimring). Sonst: Meldung, dass es nur unleserliches orkisches Gekritzel ist Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Orkbehausung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_orkdokument`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-180"></a>
### 180. Gegengift (`ITEM_ID_GEGENGIFT`)

- **Im Code:** Gegenstands-ID 180 (`ITEM_ID_GEGENGIFT`, Rohname in ITEMNAME `GEGENGIFT..`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 16 Dukaten (= 1600 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Reiseereignis Ottarje–Orvil, Verfallene Herberge (2×). Kann gebraut werden (Rezept ID 202). Einnehmen (Essen/Trinken-Menü): das erste Gift, an dem der Held leidet (Reihenfolge der Giftnummern), wird geheilt, sofern es eines der Gifte 1–4 ist (Shurinknollengift, Arax, Angstgift, Schlafgift; Goldleim, Krötenschemel, Lotus, Kukris und Bannstaub also nicht). Die Meldung Meldung, der Held sei wieder völlig in Ordnung kommt immer. Zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Sieht so aus, als ob das Gegengift nur gegen die ersten vier Gifte hilft, obwohl die Meldung immer volle Heilung verspricht.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-181"></a>
### 181. Das Schwert Grimring (`ITEM_ID_DAS_SCHWERT_GRIMRING`)

- **Im Code:** Gegenstands-ID 181 (`ITEM_ID_DAS_SCHWERT_GRIMRING`, Rohname in ITEMNAME `DAS SCHWERT GRIMRING..`). ITEMS.DAT: Waffe, magisch; Gewicht 80 Unzen; Grundpreis 100 Dukaten (= 10000 Heller). Waffe, Waffenart Schwert (Untertyp 3), Waffentabelle `g_weapon_stats_table[59]`: TP 1W6+4, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), unzerbrechlich (BF -99), AT +2, PA +2. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Im Kampfmenü wird für Grimring in der rechten Hand im Endkampf (FIGHT_ID_F144) eine um 5 höhere Schadensspanne angezeigt. (`Quelle/seg033.c:FIG_menu`) Grimring richtet gegen Orks 5 Schadenspunkte mehr an. (`Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`) Mit Fix 390 (Fanpatch-Stufe „nur Alle“) setzt der Kampf den Bruchfaktor dieser Waffe vor jeder Bruchprüfung (Attacke, Parade) auf -99 (unzerbrechlich), und der Zauber Eisenrost (von Helden und von Gegnern) kann sie nicht zerbrechen. Hintergrund laut Fix-Kommentar: bei Curians Stab stand ein falscher Bruchfaktor 5 im Gegenstand. (`Quelle/seg042.c:fix_set_unbreakable`, `Quelle/seg043.c:fix_set_unbreakable_pa`, `Quelle/seg100.c:fix_unbreakable_weapon`, `Quelle/seg102.c:fix_unbreakable_weapon`) Hyggeliks Ruine: Die Gruppe bekommt Grimring nach der Begegnung mit Hyggelik; das Spiel fragt so lange, bis das Schwert genommen ist. Jeder Held bekommt dazu 50 AP. (`Quelle/seg089.c:DNG15_handler`) Endkampf (Reiseereignis 144): Nur wenn ein Held Grimring bei sich hat, kommt es überhaupt zum Endkampf gegen den Orkchampion. Zum richtigen Zeitpunkt (ab 10. Peraine 17 n. Hal) oder, wenn man das Orkdokument gelesen hat, durch Warten bis dahin. Der Träger tritt an (der NSC auf Platz 7 weigert sich). Wird Grimring vorher abgelegt, kämpft im Original Platz 1, mit Fix 38 (Fanpatch-Stufe „Wichtig“) der erste handlungsfähige Held. (`Quelle/seg116.c:tevent_144`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg033.c:FIG_menu`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`, `Quelle/seg042.c:fix_set_unbreakable`, `Quelle/seg043.c:fix_set_unbreakable_pa`, `Quelle/seg100.c:fix_unbreakable_weapon`, `Quelle/seg102.c:fix_unbreakable_weapon`, `Quelle/seg089.c:DNG15_handler`, `Quelle/seg116.c:tevent_144`
- **Deutung der KI:** Vermutlich das wichtigste Questobjekt: Ohne Grimring gibt es keinen Endkampf. Gegen Orks macht es mehr Schaden.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-182"></a>
### 182. Totenkopfgürtel (`ITEM_ID_TOTENKOPFGUERTEL`)

- **Im Code:** Gegenstands-ID 182 (`ITEM_ID_TOTENKOPFGUERTEL`, Rohname in ITEMNAME `TOTENKOPFGÜRTEL..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird), magisch; Gewicht 10 Unzen; Grundpreis 20 Silbertaler (= 200 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Hat ein Held den Totenkopfgürtel irgendwo im Inventar, richten Angriffe von Zombies und Skeletten bei ihm 0 Schaden an. Bei jedem solchen Treffer verschwindet der Gürtel mit 4 % Wahrscheinlichkeit (Meldung Meldung, dass der Gürtel seine Wirkung verliert und verschwindet). (`Quelle/seg041.c:FIG_get_enemy_attack_damage`) Angelegt (an einem Körperplatz oder in einer Hand): TA (Totenangst) -4, beim Ablegen wieder +4. Für die Schutzwirkung gegen Skelette und Zombies (siehe unten) muss er dagegen nur im Inventar sein. (`Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg041.c:FIG_get_enemy_attack_damage`, `Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`
- **Deutung der KI:** Sieht so aus, als ob der Gürtel schon im Gepäck vor Skeletten und Zombies schützt. Er kann dabei aber verschwinden.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-183"></a>
### 183. Kraftgürtel (`ITEM_ID_KRAFTGUERTEL`)

- **Im Code:** Gegenstands-ID 183 (`ITEM_ID_KRAFTGUERTEL`, Rohname in ITEMNAME `KRAFTGÜRTEL..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird), magisch; Gewicht 10 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Drachenhort. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 7 und gibt einen eigenen Text aus (SPELLTXT Nr. 73, sinngemäß: verleiht einmalig immense Körperkraft). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Angelegt (an einem Körperplatz oder in einer Hand): KK +5, beim Ablegen wieder -5. Laut Code-Kommentar werden davon abhängige Werte (z. B. AT/PA-Grundwerte) dabei nicht neu berechnet. (`Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`
- **Deutung der KI:** Vermutlich ein KK-Bonus beim Tragen. Abhängige Werte werden laut Code-Kommentar nicht neu berechnet.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-184"></a>
### 184. Magischer Brotbeutel (`ITEM_ID_MAGISCHER_BROTBEUTEL`)

- **Im Code:** Gegenstands-ID 184 (`ITEM_ID_MAGISCHER_BROTBEUTEL`, Rohname in ITEMNAME `MAGISCHE.R BROTBEUTEL. BROTBEUTEL`). ITEMS.DAT: benutzbar, magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzbar-Eintrag 13 (`g_usable_items_table`): Benutzen ruft `item_use_beutel()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt der Magische Brotbeutel bei irgendeinem Helden derselben Gruppe, essen die Helden dieser Gruppe in der stündlichen Versorgung nichts (der Hunger wird dort gar nicht behandelt). (`Quelle/seg002.c:herokeeping`) Benutzen ruft dieselbe Funktion auf wie der Beutel (ID 221, Benutzbar-Eintrag 13): Auf Ebene 1 der Ruine des Schwarzmagiers öffnet sich eine Wand auf Feld (10,3), und es kommt die Staub-Meldung. Danach versucht das Spiel, einen Beutel (ID 221) beim Benutzer zu entfernen. Hat er keinen, wird ein ungültiger Inventarplatz (-1) angesprochen, die Folgen hängen vom Speicherinhalt ab. Der Brotbeutel selbst bleibt. (`Quelle/seg107.c:item_use_beutel`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_use_beutel`, `Quelle/seg002.c:herokeeping`
- **Deutung der KI:** Sieht so aus, als ob der Brotbeutel der ganzen Gruppe das Essen erspart. „Benutzen“ ist vermutlich ein Datenfehler (gleicher Eintrag wie der Staubbeutel).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-185"></a>
### 185. Magischer Wasserschlauch (`ITEM_ID_MAGISCHER_WASSERSCHLAUCH`)

- **Im Code:** Gegenstands-ID 185 (`ITEM_ID_MAGISCHER_WASSERSCHLAUCH`, Rohname in ITEMNAME `MAGISCHE.R WASSERSCHLAUCH.WASSERSCHLÄUCHE`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird), magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt der Magische Wasserschlauch bei irgendeinem Helden derselben Gruppe, trinken die Helden dieser Gruppe in der stündlichen Versorgung nichts (der Durst wird dort gar nicht behandelt). (`Quelle/seg002.c:herokeeping`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg002.c:herokeeping`
- **Deutung der KI:** Sieht so aus, als ob der Wasserschlauch der ganzen Gruppe das Trinken erspart.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-186"></a>
### 186. Rezept für Heiltrank (`ITEM_ID_REZEPT_FUER_HEILTRANK`)

- **Im Code:** Gegenstands-ID 186 (`ITEM_ID_REZEPT_FUER_HEILTRANK`, Rohname in ITEMNAME `REZEPT FÜR HEILTRANK..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Rezept für Heiltrank (ID 145): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Glasflasche, 2× Vierblättrige Einbeere, 1× Tarnele, 1× Schnapsflasche; 5 AE, Alchimie-Probe +2, Dauer 8 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um Heiltränke brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-187"></a>
### 187. Schreiben von Jadra (`ITEM_ID_SCHREIBEN_VON_JADRA`)

- **Im Code:** Gegenstands-ID 187 (`ITEM_ID_SCHREIBEN_VON_JADRA`, Rohname in ITEMNAME `SCHREIBEN VON JADRA..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 3 (`g_usable_items_table`): Benutzen ruft `item_read_document()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Gibt es in Thorwal von der Stadtverwaltung (Magistrat), einmalig. Mit diesem Schreiben oder dem Empfehlungsschreiben des Hetmanns bietet das Zeughaus in Thorwal die zweite Antwort an: die Gruppe darf dort mit einem Budget (15 D × Unterstützung des Hetmanns, mindestens 10 D) einkaufen. (`Quelle/seg068.c:THO_magistracy`, `Quelle/seg068.c:THO_arsenal`) Benutzen (Lesen) zeigt nur einen Text: der Hinweis, dass es Zugang zum Zeughaus verschafft Weitere Wirkung hat das Lesen nicht. (`Quelle/seg107.c:item_read_document`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_document`, `Quelle/seg068.c:THO_magistracy`, `Quelle/seg068.c:THO_arsenal`
- **Deutung der KI:** Vermutlich der Schlüssel zum Einkauf im Zeughaus von Thorwal.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-188"></a>
### 188. Amulett (gewöhnlich) (`ITEM_ID_AMULETT__ORDINARY`)

- **Im Code:** Gegenstands-ID 188 (`ITEM_ID_AMULETT__ORDINARY`, Rohname in ITEMNAME `AMULETT..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 20 Silbertaler (= 200 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 0 und gibt einen eigenen Text aus (SPELLTXT Nr. 71, sinngemäß: kein Zauber feststellbar). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg099.c:g_analues_items`
- **Deutung der KI:** Sieht so aus, als ob Analüs eine Wirkung verspricht, die im Code an keiner Stelle abgefragt wird.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-189"></a>
### 189. Lobpreisungen (`ITEM_ID_LOBPREISUNGEN`)

- **Im Code:** Gegenstands-ID 189 (`ITEM_ID_LOBPREISUNGEN`, Rohname in ITEMNAME `LOBPREISUNGEN..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 3 (`g_usable_items_table`): Benutzen ruft `item_read_document()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Kultstätte des Namenlosen, Piratenhöhle. Benutzen (Lesen) zeigt nur einen Text: ein finsteres Buch mit Lobpreisungen des Namenlosen Weitere Wirkung hat das Lesen nicht. (`Quelle/seg107.c:item_read_document`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_document`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein Fundstück ohne Wirkung, das nur einen Text zeigt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-190"></a>
### 190. Mitgliederliste (`ITEM_ID_MITGLIEDERLISTE`)

- **Im Code:** Gegenstands-ID 190 (`ITEM_ID_MITGLIEDERLISTE`, Rohname in ITEMNAME `MITGLIEDERLISTE..N`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 3 (`g_usable_items_table`): Benutzen ruft `item_read_document()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Kultstätte des Namenlosen. Benutzen (Lesen) zeigt nur einen Text: eine Mitgliederliste des Kultes Weitere Wirkung hat das Lesen nicht. (`Quelle/seg107.c:item_read_document`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_document`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein Fundstück ohne Wirkung, das nur einen Text zeigt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-191"></a>
### 191. Dokument (Fassung 2) (`ITEM_ID_DOKUMENT__2`)

- **Im Code:** Gegenstands-ID 191 (`ITEM_ID_DOKUMENT__2`, Rohname in ITEMNAME `DOKUMENT..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-192"></a>
### 192. Seekarte (`ITEM_ID_SEEKARTE`)

- **Im Code:** Gegenstands-ID 192 (`ITEM_ID_SEEKARTE`, Rohname in ITEMNAME `SEEKARTE..N`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 3 (`g_usable_items_table`): Benutzen ruft `item_read_document()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Piratenhöhle. Benutzen (Lesen) zeigt nur einen Text: Seekarten einer unbekannten Gegend Weitere Wirkung hat das Lesen nicht. (`Quelle/seg107.c:item_read_document`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_document`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein Fundstück ohne Wirkung, das nur einen Text zeigt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-193"></a>
### 193. Kukrisdolch (`ITEM_ID_KUKRISDOLCH`)

- **Im Code:** Gegenstands-ID 193 (`ITEM_ID_KUKRISDOLCH`, Rohname in ITEMNAME `KUKRISDOLCH..E`). ITEMS.DAT: Waffe; Gewicht 20 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[10]`: TP 1W6+1, KK-Zuschlag ab KK 15 (je Punkt darüber +1 TP), Bruchfaktor 3, AT -2, PA -3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Krieger. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Trifft ein Held mit dem Kukrisdolch einen Gegner, ist der Schaden 1000 (tödlich). Danach wird der Kukrisdolch entfernt und durch einen gewöhnlichen Dolch (ID 14) in der rechten Hand ersetzt. Das Gift wirkt also genau einmal. (`Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`) Wird in der Gegenstandsbeschreibung immer als Zusatz „vergiftet“ angezeigt. (`Quelle/seg106.c:print_item_description`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`, `Quelle/seg106.c:print_item_description`
- **Deutung der KI:** Sieht so aus, als ob der Kukrisdolch einen einzigen garantiert tödlichen Treffer hat und danach ein normaler Dolch ist.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-194"></a>
### 194. Schlüssel (gelb) (`ITEM_ID_SCHLUESSEL__YELLOW`)

- **Im Code:** Gegenstands-ID 194 (`ITEM_ID_SCHLUESSEL__YELLOW`, Rohname in ITEMNAME `SCHLÜSSEL..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 20 Silbertaler (= 200 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-195"></a>
### 195. Goldener Schlüssel (`ITEM_ID_GOLDENER_SCHLUESSEL`)

- **Im Code:** Gegenstands-ID 195 (`ITEM_ID_GOLDENER_SCHLUESSEL`, Rohname in ITEMNAME `GOLDEN.ER SCHLÜSSEL.E SCHLÜSSEL`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 20 Silbertaler (= 200 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Daspotaschatz: liegt in einem Geheimfach auf Feld (0,8,7) zusammen mit 30 D. Das Fach findet man mit einer Sinnesschärfe-Probe +10 irgendeines Helden der Gruppe (einmal gefunden, bleibt es gefunden). (`Quelle/seg081.c:DNG06_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg081.c:DNG06_handler`
- **Deutung der KI:** Vermutlich ein Fundstück aus dem Daspotaschatz. Ob er irgendwo etwas öffnet, ist im Code nicht zu sehen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-196"></a>
### 196. Helm (verflucht) (`ITEM_ID_HELM__CURSED`)

- **Im Code:** Gegenstands-ID 196 (`ITEM_ID_HELM__CURSED`, Rohname in ITEMNAME `HELM..E`). ITEMS.DAT: Rüstung; Gewicht 90 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Rüstung, Platz Kopf (Untertyp 0), `g_armor_stats_table[21]`: RS 1, BE 0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Angelegt (an einem Körperplatz oder in einer Hand): CH -1, beim Ablegen wieder +1. (`Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`
- **Deutung der KI:** Vermutlich ein verfluchter Helm mit kleinem CH-Malus.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-197"></a>
### 197. Kettenhemd (verflucht) (`ITEM_ID_KETTENHEMD__CURSED`)

- **Im Code:** Gegenstands-ID 197 (`ITEM_ID_KETTENHEMD__CURSED`, Rohname in ITEMNAME `KETTENHEMD..EN`). ITEMS.DAT: Rüstung, magisch; Gewicht 320 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[22]`: RS 2, BE 4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Hexe, Druide, Magier, Firnelf, Waldelf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Drachenhort. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 5 und gibt einen eigenen Text aus (SPELLTXT Nr. 75, sinngemäß: merkt, ob Untote die Waffen führen; was dann passiert, bleibt offen). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Jede volle Spielstunde verliert ein Held, der das verfluchte Kettenhemd am Körper trägt, 1 LE (nicht im Kerker, nicht wenn tot). (`Quelle/seg002.c:magical_chainmail_damage`) Im Kampf zieht das verfluchte Kettenhemd (am Körper getragen) zusätzlich 3 Schadenspunkte von Treffern durch Zombies und Skelette ab. (`Quelle/seg041.c:FIG_get_enemy_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg002.c:magical_chainmail_damage`, `Quelle/seg041.c:FIG_get_enemy_attack_damage`
- **Deutung der KI:** Sieht so aus, als ob das verfluchte Kettenhemd gegen Untote schützt, den Träger aber stündlich LE kostet.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-198"></a>
### 198. Kettenhemd (magisch) (`ITEM_ID_KETTENHEMD__MAGIC`)

- **Im Code:** Gegenstands-ID 198 (`ITEM_ID_KETTENHEMD__MAGIC`, Rohname in ITEMNAME `KETTENHEMD..EN`). ITEMS.DAT: Rüstung, magisch; Gewicht 320 Unzen; Grundpreis 100 Dukaten (= 10000 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[23]`: RS 5, BE 3. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Gaukler, Jäger, Streuner, Hexe, Druide, Magier, Firnelf, Waldelf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Daspotaschatz. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 15 und gibt einen eigenen Text aus (SPELLTXT Nr. 72, sinngemäß: etwas mit dem Wesen der Schwere, das Kettenhemd wirkt leicht). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-199"></a>
### 199. Schwert (magisch) (`ITEM_ID_SCHWERT__MAGIC`)

- **Im Code:** Gegenstands-ID 199 (`ITEM_ID_SCHWERT__MAGIC`, Rohname in ITEMNAME `SCHWERT..ER`). ITEMS.DAT: Waffe, magisch; Gewicht 80 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Waffe, Waffenart Schwert (Untertyp 3), Waffentabelle `g_weapon_stats_table[60]`: TP 1W6+4, KK-Zuschlag ab KK 14 (je Punkt darüber +1 TP), Bruchfaktor 0, AT +2, PA +0. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Drachenhort. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 5 und gibt einen eigenen Text aus (SPELLTXT Nr. 74, sinngemäß: besonders wirksam gegen Untote). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-200"></a>
### 200. Kukrismenglibar (`ITEM_ID_KUKRIS_MENGBILAR`)

- **Im Code:** Gegenstands-ID 200 (`ITEM_ID_KUKRIS_MENGBILAR`, Rohname in ITEMNAME `KUKRISMENGLIBAR..E`). ITEMS.DAT: Waffe; Gewicht 20 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Waffe, Waffenart Stichwaffe (Untertyp 2), Waffentabelle `g_weapon_stats_table[35]`: TP 1W6+1, KK-Zuschlag ab KK 16 (je Punkt darüber +1 TP), Bruchfaktor 7, AT -3, PA -4. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Krieger. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Trifft ein Held mit dem Kukris-Mengbilar einen Gegner, ist der Schaden 1000 (tödlich). Danach wird die Waffe entfernt und durch einen gewöhnlichen Mengbilar (ID 109) in der rechten Hand ersetzt. Das Gift wirkt also genau einmal. (`Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`) Wird in der Gegenstandsbeschreibung immer als Zusatz „vergiftet“ angezeigt. (`Quelle/seg106.c:print_item_description`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`, `Quelle/seg106.c:print_item_description`
- **Deutung der KI:** Sieht so aus, als ob der Kukris-Mengbilar einen einzigen garantiert tödlichen Treffer hat und danach ein normaler Mengbilar ist.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-201"></a>
### 201. Platinschlüssel (`ITEM_ID_PLATINSCHLUESSEL`)

- **Im Code:** Gegenstands-ID 201 (`ITEM_ID_PLATINSCHLUESSEL`, Rohname in ITEMNAME `PLATINSCHLÜSSEL..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Drachenhort: Bringt die Gruppe dem Drachen (nach seinem Auftrag) den Platinschlüssel, nimmt er ihn und gibt 2 Heiltränke, 1 Zaubertrank, 1 magischen Wurfdolch, 1 Kristallkugel und 200 D. (`Quelle/seg085.c:DNG10_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg085.c:DNG10_handler`
- **Deutung der KI:** Vermutlich der Schlüssel zur Belohnung des Drachen im Drachenhort.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-202"></a>
### 202. Rezept für Gegengift (`ITEM_ID_REZEPT_FUER_GEGENGIFT`)

- **Im Code:** Gegenstands-ID 202 (`ITEM_ID_REZEPT_FUER_GEGENGIFT`, Rohname in ITEMNAME `REZEPT FÜR GEGENGIFT..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Rezept für Gegengift (ID 180): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Glasflasche, 1× Schnapsflasche, 2× Menchalkaktus, 1× Alraune, 1× Shurinknolle; 10 AE, Alchimie-Probe +5, Dauer 24 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um Gegengift brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-203"></a>
### 203. Rezept für Hylailer Feuer (`ITEM_ID_REZEPT_FUER_HYLAILER_FEUER`)

- **Im Code:** Gegenstands-ID 203 (`ITEM_ID_REZEPT_FUER_HYLAILER_FEUER`, Rohname in ITEMNAME `REZEPT FÜR HYLAILER FEUER..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Verfallene Herberge, Piratenhöhle. Rezept für Hylailer Feuer (ID 239): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Glasflasche, 2× Schnapsflasche, 1× Lakritze, 2× Öl; 0 AE, Alchimie-Probe +5, Dauer 8 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um Hylailer Feuer brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-204"></a>
### 204. Rezept für Kraftelixier (`ITEM_ID_REZEPT_FUER_KRAFTELIXIER`)

- **Im Code:** Gegenstands-ID 204 (`ITEM_ID_REZEPT_FUER_KRAFTELIXIER`, Rohname in ITEMNAME `REZEPT FÜR KRAFTELIXIER..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Verfallene Herberge (2×). Rezept für KK-Elixier (ID 153): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Glasflasche, 1× Schnapsflasche, 1× Finagebäumchen, 1× Alraune, 2× Gulmond Blatt; 15 AE, Alchimie-Probe +8, Dauer 48 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um das Kraftelixier brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-205"></a>
### 205. Rezept für Mutelixier (`ITEM_ID_REZEPT_FUER_MUTELIXIER`)

- **Im Code:** Gegenstands-ID 205 (`ITEM_ID_REZEPT_FUER_MUTELIXIER`, Rohname in ITEMNAME `REZEPT FÜR MUTELIXIER..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Rezept für MU-Elixier (ID 147): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Glasflasche, 1× Schnapsflasche, 1× Finagebäumchen, 1× Alraune, 1× Donfstengel, 1× Ilmenblatt; 12 AE, Alchimie-Probe +7, Dauer 48 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um das Mutelixier brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-206"></a>
### 206. Rezept für Zaubertrank (`ITEM_ID_REZEPT_FUER_ZAUBERTRANK`)

- **Im Code:** Gegenstands-ID 206 (`ITEM_ID_REZEPT_FUER_ZAUBERTRANK`, Rohname in ITEMNAME `REZEPT FÜR ZAUBERTRANK..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Rezept für Zaubertrank (ID 154): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Glasflasche, 1× Schnapsflasche, 2× Alraune, 2× Kairanhalm, 1× Thonnysblüte; 7 AE, Alchimie-Probe +8, Dauer 48 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um Zaubertränke brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-207"></a>
### 207. Ring (blau) (`ITEM_ID_RING__BLUE`)

- **Im Code:** Gegenstands-ID 207 (`ITEM_ID_RING__BLUE`, Rohname in ITEMNAME `RING..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 50 Silbertaler (= 500 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Zwingfeste: liegt auf Feld (3,7,3) am Boden (einmalig). (`Quelle/seg087.c:DNG14_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg087.c:DNG14_handler`
- **Deutung der KI:** Vermutlich ein Fundstück aus der Zwingfeste. Ob der Ring eine Wirkung hat, ist im Code nicht zu sehen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-208"></a>
### 208. Schlüssel (silbern) (`ITEM_ID_SCHLUESSEL__SILVER`)

- **Im Code:** Gegenstands-ID 208 (`ITEM_ID_SCHLUESSEL__SILVER`, Rohname in ITEMNAME `SCHLÜSSEL..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-209"></a>
### 209. Eisenschlüssel (Fassung 1) (`ITEM_ID_EISENSCHLUESSEL__1`)

- **Im Code:** Gegenstands-ID 209 (`ITEM_ID_EISENSCHLUESSEL__1`, Rohname in ITEMNAME `EISENSCHLÜSSEL..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Öffnet in der Piratenhöhle eine bestimmte Tür. Im Original prüft das Spiel dafür das Feld (0,12,8), auf dem keine Tür ist, der Schlüssel wirkt dort also nie. Mit Fix 432 (Fanpatch-Stufe „Wichtig“) wirkt er an der Tür auf (0,12,7). Es gibt zwei verschiedene Eisenschlüssel, nur dieser (ID 209) zählt hier. (`Quelle/seg076.c:DNG_door`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg076.c:DNG_door`
- **Deutung der KI:** Vermutlich der Schlüssel für eine Tür in der Piratenhöhle. Im Original ist er wegen des falschen Felds wirkungslos.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-210"></a>
### 210. Eisenschlüssel (Fassung 2) (`ITEM_ID_EISENSCHLUESSEL__2`)

- **Im Code:** Gegenstands-ID 210 (`ITEM_ID_EISENSCHLUESSEL__2`, Rohname in ITEMNAME `EISENSCHLÜSSEL..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-211"></a>
### 211. Roter Schlüssel (`ITEM_ID_ROTER_SCHLUESSEL`)

- **Im Code:** Gegenstands-ID 211 (`ITEM_ID_ROTER_SCHLUESSEL`, Rohname in ITEMNAME `ROTE.R SCHLÜSSEL. SCHLÜSSEL`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Spinnenhöhle. Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich aus dem Pen-&-Paper-Regelwerk übernommen; im Computerspiel nur Ballast oder Verkaufsware.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-212"></a>
### 212. Bronzeschlüssel (`ITEM_ID_BRONZESCHLUESSEL`)

- **Im Code:** Gegenstands-ID 212 (`ITEM_ID_BRONZESCHLUESSEL`, Rohname in ITEMNAME `BRONZESCHLÜSSEL..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 10 Silbertaler (= 100 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Orkbehausung: Drei Truhen (DNG08_chest03, 04, 05) öffnen sich, wenn irgendein Held den Bronzeschlüssel hat oder dem ersten handlungsfähigen Helden eine Schlösser-Probe +5 gelingt, sonst löst die Truhe ihre Falle aus. In chest03 liegen 853 Silbertaler. (`Quelle/seg083.c:DNG08_chest03_open`, `Quelle/seg083.c:DNG08_chest04_open`, `Quelle/seg083.c:DNG08_chest05_open`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg083.c:DNG08_chest03_open`, `Quelle/seg083.c:DNG08_chest04_open`, `Quelle/seg083.c:DNG08_chest05_open`
- **Deutung der KI:** Vermutlich der Schlüssel zu drei Truhen in der Orkbehausung. Ein guter Schlossknacker geht auch.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-213"></a>
### 213. Silberhelm (`ITEM_ID_SILBERHELM`)

- **Im Code:** Gegenstands-ID 213 (`ITEM_ID_SILBERHELM`, Rohname in ITEMNAME `SILBERHELM..E`). ITEMS.DAT: Rüstung; Gewicht 180 Unzen; Grundpreis 150 Silbertaler (= 1500 Heller). Rüstung, Platz Kopf (Untertyp 0), `g_armor_stats_table[2]`: RS 3, BE 2. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Wolfshöhle: liegt auf Feld (0,5,11) zum Mitnehmen (einmalig). (`Quelle/seg080.c:DNG04_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/seg080.c:DNG04_handler`
- **Deutung der KI:** Vermutlich ein guter Helm aus der Wolfshöhle. Besondere Wirkungen sind im Code nicht zu sehen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-214"></a>
### 214. Silberstreitkolben (`ITEM_ID_SILBERSTREITKOLBEN`)

- **Im Code:** Gegenstands-ID 214 (`ITEM_ID_SILBERSTREITKOLBEN`, Rohname in ITEMNAME `SILBERSTREITKOLBEN..`). ITEMS.DAT: Waffe; Gewicht 110 Unzen; Grundpreis 30 Dukaten (= 3000 Heller). Waffe, Waffenart Hiebwaffe (Untertyp 1), Waffentabelle `g_weapon_stats_table[19]`: TP 1W6+4, KK-Zuschlag ab KK 13 (je Punkt darüber +1 TP), Bruchfaktor 1, AT +0, PA -2. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Der Silberstreitkolben richtet gegen Skelette 4 Schadenspunkte mehr an (gegen Zombies nicht). (`Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/seg041.c:FIG_get_hero_weapon_attack_damage`
- **Deutung der KI:** Vermutlich eine Waffe gegen Skelette (+4).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-215"></a>
### 215. Silberschmuck (magisch) (`ITEM_ID_SILBERSCHMUCK__MAGIC`)

- **Im Code:** Gegenstands-ID 215 (`ITEM_ID_SILBERSCHMUCK__MAGIC`, Rohname in ITEMNAME `SILBERSCHMUCK..STÜCKE`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird), magisch; Gewicht 10 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 3 und gibt einen eigenen Text aus (SPELLTXT Nr. 63, sinngemäß: ein guter Schutz gegen Untote). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Angelegt (an einem Körperplatz oder in einer Hand): TA (Totenangst) -2, beim Ablegen wieder +2. (`Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`
- **Deutung der KI:** Vermutlich ein Schmuckstück, das die Totenangst senkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-216"></a>
### 216. Speer (magisch) (`ITEM_ID_SPEER__MAGIC`)

- **Im Code:** Gegenstands-ID 216 (`ITEM_ID_SPEER__MAGIC`, Rohname in ITEMNAME `SPEER..E`). ITEMS.DAT: Waffe, magisch; Gewicht 80 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Waffe, Waffenart Speer/Stab (Untertyp 5), Waffentabelle `g_weapon_stats_table[61]`: TP 1W6+3, KK-Zuschlag ab KK 16 (je Punkt darüber +1 TP), Bruchfaktor 5, AT +1, PA +1. Klassensperre (`g_forbidden_item_ids_table`): nicht benutzbar für Hexe, Druide, Magier. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Spinnenhöhle. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 3 und gibt einen eigenen Text aus (SPELLTXT Nr. 64, sinngemäß: Macht über Spinnentiere). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/datseg.c:g_forbidden_item_ids_table`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`
- **Deutung der KI:** Vermutlich eine gewöhnliche Waffe, die nur über ihre Tabellenwerte wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-217"></a>
### 217. Stirnreif (MR-Bonus) (`ITEM_ID_STIRNREIF__MR_BONUS`)

- **Im Code:** Gegenstands-ID 217 (`ITEM_ID_STIRNREIF__MR_BONUS`, Rohname in ITEMNAME `STIRNREIF..EN`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird), magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Kultstätte des Namenlosen. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 4 und gibt einen eigenen Text aus (SPELLTXT Nr. 65, sinngemäß: ein dauerhafter Arcano Psychostabilis liegt darauf). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Bei einem Stufenaufstieg, der die Magieresistenz ändert, schlägt das Spiel für den getragenen Stirnreif (Kopf) wieder 2 MR auf. (`Quelle/seg050.c:level_up`) Angelegt (an einem Körperplatz oder in einer Hand): MR +2, beim Ablegen wieder -2. (`Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`) Darf als einer von drei Stirnreifen (IDs 171, 217, 245) auf dem Kopf getragen werden, obwohl er keine Rüstung ist. (`Quelle/seg105.c:can_hero_equip_item_at_slot`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg050.c:level_up`, `Quelle/seg105.c:add_equip_boni`, `Quelle/seg105.c:unequip`, `Quelle/seg105.c:can_hero_equip_item_at_slot`
- **Deutung der KI:** Vermutlich ein MR-Stirnreif (+2).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-218"></a>
### 218. Wurfdolch (magisch) (`ITEM_ID_WURFDOLCH__MAGIC`)

- **Im Code:** Gegenstands-ID 218 (`ITEM_ID_WURFDOLCH__MAGIC`, Rohname in ITEMNAME `WURFDOLCH..E`). ITEMS.DAT: Waffe, magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Waffe, Waffenart Wurfwaffe (Untertyp 8), Waffentabelle `g_weapon_stats_table[62]`: TP 1W6, kein KK-Zuschlag, Bruchfaktor 4, AT -3, PA -4, Fernkampf-Entfernungstabelle Nr. 7. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 6 und gibt einen eigenen Text aus (SPELLTXT Nr. 78, sinngemäß: erleichtert das Zielen sehr). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Wird als Wurfwaffe eingesetzt: Nach dem Wurf fliegt die Waffe aus der rechten Hand (liegt in der linken Hand eine gleiche, rückt sie nach). Bis zu 30 geworfene Waffen merkt sich der Kampf und gibt sie nach einem gewonnenen Kampf an die Gruppe zurück (nur die Gegenstandsnummer, nicht ihr Zustand). Ob er magisch erkannt war, geht dabei verloren, weil nur die Nummer gespeichert wird. (`Quelle/seg041.c:range_attack_check_ammo`) Drachenhort: Lohn des Drachen für den Platinschlüssel. Im Original gibt es ihn zusätzlich schon beim Plündern des Horts; mit Fix 361 (Fanpatch-Stufe „nur Alle“) nur noch als Lohn (der Text des Horts nennt im Datenpatch 1.2 keinen Dolch mehr). (`Quelle/seg085.c:DNG10_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_weapon_stats_table`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg041.c:range_attack_check_ammo`, `Quelle/seg085.c:DNG10_handler`
- **Deutung der KI:** Vermutlich eine Wurfwaffe als Drachenlohn. Ob sie magisch erkannt war, geht nach dem Wurf verloren.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-219"></a>
### 219. Goldschlüssel (`ITEM_ID_GOLDSCHLUESSEL`)

- **Im Code:** Gegenstands-ID 219 (`ITEM_ID_GOLDSCHLUESSEL`, Rohname in ITEMNAME `GOLDSCHLÜSSEL..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 20 Silbertaler (= 200 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt auf dem Totenschiff in der Truhe auf (3,13,11). Mit Fix 97 (Fanpatch-Stufe „Wichtig“) bekommt man ihn dort nur einmal (sonst bei jedem Öffnen erneut). (`Quelle/seg077.c:DNG01_chest06_loot`) Öffnet auf dem Totenschiff die Truhe auf Feld (0,12,9): Der erste handlungsfähige Held der Gruppe muss den Schlüssel haben, sonst löst die Truhe ihre Falle aus. Ist der Schlüssel als zerbrochen markiert, passiert gar nichts. (`Quelle/seg077.c:g_dng01_specialchests`, `Quelle/seg092.c:use_key_on_chest`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg077.c:DNG01_chest06_loot`, `Quelle/seg077.c:g_dng01_specialchests`, `Quelle/seg092.c:use_key_on_chest`
- **Deutung der KI:** Vermutlich ein Fundstück aus dem Totenschiff. Ob er etwas öffnet, ist im Code nicht zu sehen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-220"></a>
### 220. Ring (grün) (`ITEM_ID_RING__GREEN`)

- **Im Code:** Gegenstands-ID 220 (`ITEM_ID_RING__GREEN`, Rohname in ITEMNAME `RING..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird), magisch; Gewicht 10 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 5 und gibt einen eigenen Text aus (SPELLTXT Nr. 77, sinngemäß: schützt eine gewisse Zeit vor magischem Feuer). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Drachenhort: Am Hebel auf Feld (1,14,2) verbrennt sich im Original der erste Held die Hände (-2 LE). Mit Fix 218 (Fanpatch-Stufe „Wichtig“) passiert nichts, wenn irgendein Held der Gruppe den grünen Ring hat (Meldung, dass nichts passiert). (`Quelle/seg085.c:DNG10_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg085.c:DNG10_handler`
- **Deutung der KI:** Sieht so aus, als ob der grüne Ring im Original gar nichts bewirkt. Mit Fix 218 schützt er wenigstens am Hebel im Drachenhort.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-221"></a>
### 221. Beutel (`ITEM_ID_BEUTEL`)

- **Im Code:** Gegenstands-ID 221 (`ITEM_ID_BEUTEL`, Rohname in ITEMNAME `BEUTEL..`). ITEMS.DAT: benutzbar, magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzbar-Eintrag 13 (`g_usable_items_table`): Benutzen ruft `item_use_beutel()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 0 und gibt einen eigenen Text aus (SPELLTXT Nr. 80, sinngemäß: der Analysierende versteht den Zauber nicht). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Benutzen: Der Staub wird davongeweht und der Beutel ist verbraucht. Nur auf Ebene 1 (Ebene 0 im Code) der Ruine des Schwarzmagiers öffnet das eine Wand auf Feld (10,3). Benutzt man ihn anderswo, ist der Beutel trotzdem weg, und der tiefere Teil der Ruine ist auf normalem Weg nicht mehr erreichbar. (`Quelle/seg107.c:item_use_beutel`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_use_beutel`, `Quelle/seg099.c:g_analues_items`
- **Deutung der KI:** Sieht so aus, als ob der Beutel nur an einer einzigen Stelle der Ruine des Schwarzmagiers sinnvoll ist. Anderswo benutzt ist er verloren.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-222"></a>
### 222. Buch (Piratenabrechnungen) (`ITEM_ID_BUCH__PIRATE_ACCOUNTING`)

- **Im Code:** Gegenstands-ID 222 (`ITEM_ID_BUCH__PIRATE_ACCOUNTING`, Rohname in ITEMNAME `B.UCH.ÜCHER`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 3 (`g_usable_items_table`): Benutzen ruft `item_read_document()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Benutzen (Lesen) zeigt nur einen Text: Aufzeichnungen über Beuteeinnahmen Weitere Wirkung hat das Lesen nicht. (`Quelle/seg107.c:item_read_document`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_document`
- **Deutung der KI:** Vermutlich ein Fundstück ohne Wirkung, das nur einen Text zeigt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-223"></a>
### 223. Antikrankheitselixier (`ITEM_ID_ANTIKRANKHEITSELIXIER`)

- **Im Code:** Gegenstands-ID 223 (`ITEM_ID_ANTIKRANKHEITSELIXIER`, Rohname in ITEMNAME `ANTIKRANKHEITSELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Drachenhort. Analüs Arcanstruktur erkennt ihn ab Probenergebnis 15 und gibt einen eigenen Text aus (SPELLTXT Nr. 76, sinngemäß: wirkt gegen Krankheiten). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Einnehmen (Essen/Trinken-Menü): Heilt die erste Krankheit des Helden (jede Krankheit). Im Original wird ohne Krankheit ein unbenutzter Eintrag verändert (folgenlos), Fix 227 (Fanpatch-Stufe „nur Alle“) lässt das. Zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein Allheilmittel gegen die erste Krankheit des Helden.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-224"></a>
### 224. Buch (Kaisersprüche Hals) (`ITEM_ID_BUCH__KAISERSPRUECHE_HALS`)

- **Im Code:** Gegenstands-ID 224 (`ITEM_ID_BUCH__KAISERSPRUECHE_HALS`, Rohname in ITEMNAME `B.UCH.ÜCHER`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 3 (`g_usable_items_table`): Benutzen ruft `item_read_document()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Daspotaschatz: liegt auf Feld (0,14,5) neben dem Donnerbalken zum Mitnehmen (einmalig). (`Quelle/seg081.c:DNG06_handler`) Benutzen (Lesen) zeigt nur einen Text: der Buchtitel „Kaisersprüche Hals“ Weitere Wirkung hat das Lesen nicht. (`Quelle/seg107.c:item_read_document`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_document`, `Quelle/seg081.c:DNG06_handler`
- **Deutung der KI:** Vermutlich ein Fundstück ohne Wirkung, das nur einen Text zeigt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-225"></a>
### 225. Kristall (`ITEM_ID_KRISTALL`)

- **Im Code:** Gegenstands-ID 225 (`ITEM_ID_KRISTALL`, Rohname in ITEMNAME `KRISTALL..E`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 5 Unzen; Grundpreis 20 Silbertaler (= 200 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Spinnenhöhle. Spinnenhöhle: Die Truhe auf Feld (1,10,7) öffnet sich nur, wenn der erste handlungsfähige Held genau 4 Kristalle im Rucksack trägt (Kristalle sind nicht stapelbar), sonst löst sie eine Falle aus. Am Altar auf (1,1,12) kann eine Gruppe mit Kristallen diese auf den Altar legen: dann werden ALLE Kristalle der Gruppe entfernt und es beginnt ein Kampf (FIGHT_ID_F051_14C). Mit Fix 353 (Fanpatch-Stufe „nur Alle“) gibt es dort zusätzlich die Antwort später wiederkommen (`Quelle/seg079.c:DNG03_chest12_loot`, `Quelle/seg079.c:DNG03_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg079.c:DNG03_chest12_loot`, `Quelle/seg079.c:DNG03_handler`
- **Deutung der KI:** Sieht so aus, als ob man genau vier Kristalle braucht, um eine Truhe in der Spinnenhöhle zu öffnen. Am Altar kosten sie alle und bringen einen Kampf.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-226"></a>
### 226. MU-Elixier (schädliche Fassung) (`ITEM_ID_MU_ELIXIER__BAD`)

- **Im Code:** Gegenstands-ID 226 (`ITEM_ID_MU_ELIXIER__BAD`, Rohname in ITEMNAME `MU-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Ruine des Schwarzmagiers: liegt auf Feld (2,1,11) als „kleine Phiole am Boden“ zum Mitnehmen (einmalig). Für den Spieler sieht sie aus wie das echte Mut-Elixier. (`Quelle/seg082.c:DNG07_handler`) Einnehmen (Essen/Trinken-Menü): MU -7 für 1 Stunde (Meldung Meldung über den Verlust von 7 Punkten), zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg082.c:DNG07_handler`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Sieht so aus, als ob diese Phiole eine Falle ist: Sie sieht aus wie das MU-Elixier, senkt den Mut aber.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-227"></a>
### 227. KL-Elixier (schädliche Fassung) (`ITEM_ID_KL_ELIXIER__BAD`)

- **Im Code:** Gegenstands-ID 227 (`ITEM_ID_KL_ELIXIER__BAD`, Rohname in ITEMNAME `KL-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Einnehmen (Essen/Trinken-Menü): KL -7 für 1 Stunde (Meldung Meldung über den Verlust von 7 Punkten), zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich eine schädliche Variante des KL-Elixiers.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-228"></a>
### 228. CH-Elixier (schädliche Fassung) (`ITEM_ID_CH_ELIXIER__BAD`)

- **Im Code:** Gegenstands-ID 228 (`ITEM_ID_CH_ELIXIER__BAD`, Rohname in ITEMNAME `CH-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Einnehmen (Essen/Trinken-Menü): CH -7 für 1 Stunde (Meldung Meldung über den Verlust von 7 Punkten), zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich eine schädliche Variante des CH-Elixiers.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-229"></a>
### 229. FF-Elixier (schädliche Fassung) (`ITEM_ID_FF_ELIXIER__BAD`)

- **Im Code:** Gegenstands-ID 229 (`ITEM_ID_FF_ELIXIER__BAD`, Rohname in ITEMNAME `FF-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Einnehmen (Essen/Trinken-Menü): FF -7 für 1 Stunde (Meldung Meldung über den Verlust von 7 Punkten), zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich eine schädliche Variante des FF-Elixiers.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-230"></a>
### 230. GE-Elixier (schädliche Fassung) (`ITEM_ID_GE_ELIXIER__BAD`)

- **Im Code:** Gegenstands-ID 230 (`ITEM_ID_GE_ELIXIER__BAD`, Rohname in ITEMNAME `GE-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Einnehmen (Essen/Trinken-Menü): GE -7 für 1 Stunde (Meldung Meldung über den Verlust von 7 Punkten), zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich eine schädliche Variante des GE-Elixiers.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-231"></a>
### 231. IN-Elixier (schädliche Fassung) (`ITEM_ID_IN_ELIXIER__BAD`)

- **Im Code:** Gegenstands-ID 231 (`ITEM_ID_IN_ELIXIER__BAD`, Rohname in ITEMNAME `IN-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Einnehmen (Essen/Trinken-Menü): IN -7 für 1 Stunde (Meldung Meldung über den Verlust von 7 Punkten), zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich eine schädliche Variante des IN-Elixiers.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-232"></a>
### 232. KK-Elixier (schädliche Fassung) (`ITEM_ID_KK_ELIXIER__BAD`)

- **Im Code:** Gegenstands-ID 232 (`ITEM_ID_KK_ELIXIER__BAD`, Rohname in ITEMNAME `KK-ELIXIER..E`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 40 Dukaten (= 4000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Einnehmen (Essen/Trinken-Menü): KK -7 für 1 Stunde (Meldung Meldung über den Verlust von 7 Punkten), zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich eine schädliche Variante des KK-Elixiers.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-233"></a>
### 233. Robe (grün, Fassung 2) (`ITEM_ID_ROBE__GREEN_2`)

- **Im Code:** Gegenstands-ID 233 (`ITEM_ID_ROBE__GREEN_2`, Rohname in ITEMNAME `ROBE..N`). ITEMS.DAT: Rüstung; Gewicht 50 Unzen; Grundpreis 5 Silbertaler (= 50 Heller). Rüstung, Platz Körper (Untertyp 2), `g_armor_stats_table[10]`: RS 0, BE 0. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-234"></a>
### 234. Goldener Schild (`ITEM_ID_GOLDENER_SCHILD`)

- **Im Code:** Gegenstands-ID 234 (`ITEM_ID_GOLDENER_SCHILD`, Rohname in ITEMNAME `GOLDENE.R SCHILD. SCHILDE`). ITEMS.DAT: Rüstung; Gewicht 200 Unzen; Grundpreis 25 Dukaten (= 2500 Heller). Rüstung, Platz linke Hand (Schild) (Untertyp 9), `g_armor_stats_table[1]`: RS 2, BE 1. Keine Klassensperre (jede Heldenklasse darf ihn benutzen). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Zählt in der Zwergenfeste auf Feld (1,2,14) beim Wegwerfen als Opfer für Ingerimm (Äxte und Rüstungen mit RS über 1). Liegt in Truhen/Beutetabellen im Code: Kultstätte des Namenlosen. Keine eigene Abfrage dieses Gegenstands im Code: Er wirkt nur über die Werte oben.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_armor_stats_table`, `Quelle/seg105.c:is_item_pleasing_ingerimm`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein gewöhnliches Rüstungsteil, das nur über RS und BE wirkt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-235"></a>
### 235. Empfehlungsschreiben (Hetmann) (`ITEM_ID_EMPFEHLUNGSSCHREIBEN__HETMAN`)

- **Im Code:** Gegenstands-ID 235 (`ITEM_ID_EMPFEHLUNGSSCHREIBEN__HETMAN`, Rohname in ITEMNAME `EMPFEHLUNGSSCHREIBEN..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 3 (`g_usable_items_table`): Benutzen ruft `item_read_document()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Gibt Hetmann Tronde in Thorwal, wenn man den Hauptauftrag annimmt. Mit diesem Schreiben oder dem Schreiben von Jadra bietet das Zeughaus in Thorwal die zweite Antwort an: die Gruppe darf dort mit einem Budget (15 D × Unterstützung des Hetmanns, mindestens 10 D) einkaufen. (`Quelle/seg069.c:THO_hetmann`, `Quelle/seg068.c:THO_arsenal`) Jurge Torfinsson prüft im Gespräch dreimal, ob die Gruppe das Empfehlungsschreiben des Hetmanns dabei hat; davon hängt ab, wie das Gespräch weitergeht. (`Quelle/seg072.c:has_hetman_letter`, `Quelle/seg072.c:INF_jurge_hjore`) Benutzen (Lesen) zeigt nur einen Text: eine Empfehlung des Hetmanns Tronde Weitere Wirkung hat das Lesen nicht. (`Quelle/seg107.c:item_read_document`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_document`, `Quelle/seg069.c:THO_hetmann`, `Quelle/seg068.c:THO_arsenal`, `Quelle/seg072.c:has_hetman_letter`, `Quelle/seg072.c:INF_jurge_hjore`
- **Deutung der KI:** Vermutlich der Nachweis des Hauptauftrags, der im Zeughaus und bei Jurge Türen öffnet.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-236"></a>
### 236. Wunderkur (`ITEM_ID_WUNDERKUR`)

- **Im Code:** Gegenstands-ID 236 (`ITEM_ID_WUNDERKUR`, Rohname in ITEMNAME `WUNDERKUR..EN`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 60 Dukaten (= 6000 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Verfallene Herberge (2×). Kann gebraut werden (Rezept ID 241). Einnehmen (Essen/Trinken-Menü): Hebt den Verlust an maximaler LE durch Hunger/Durst auf, füllt die LE ganz auf, heilt die erste Krankheit nur, wenn es Dumpfschädel oder Blaue Keuche ist, und heilt das erste Gift, wenn es eines der Gifte 1–6 ist (bis Krötenschemel). Zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich der stärkste Heiltrank: volle LE, er hebt Hungerschäden auf und heilt manche Gifte und Krankheiten.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-237"></a>
### 237. Schlaftrunk (`ITEM_ID_SCHLAFTRUNK`)

- **Im Code:** Gegenstands-ID 237 (`ITEM_ID_SCHLAFTRUNK`, Rohname in ITEMNAME `SCHLAFTR.UNK.ÜNKE`). ITEMS.DAT: benutzbar, Kraut/Gift/Trank; Gewicht 10 Unzen; Grundpreis 6 Dukaten (= 600 Heller). Benutzen wirkt wie Einnehmen (für Kräuter und Tränke ruft „Benutzen“ dieselbe Funktion auf). Gruppe Trank/Elixier (Untertyp 1). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Kann gebraut werden (Rezept ID 242). Einnehmen (Essen/Trinken-Menü): Doppelte Regeneration in den nächsten 3 Nächten. Zurück kommt eine Glasflasche. (`Quelle/seg108.c:consume`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg107.c:use_item`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg108.c:consume`
- **Deutung der KI:** Vermutlich ein Regenerationstrank für drei Nächte.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-238"></a>
### 238. Miasthmaticum (`ITEM_ID_MIASTHMATICUM`)

- **Im Code:** Gegenstands-ID 238 (`ITEM_ID_MIASTHMATICUM`, Rohname in ITEMNAME `MIASTHMATICUM..`). ITEMS.DAT: benutzbar; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzbar-Eintrag 9 (`g_usable_items_table`): Benutzen ruft `item_use_miasthmaticum()` auf. Benutzen außerhalb des Kampfes: Meldung: nur im Kampf zu benutzen Die Wirkung im Kampf steht unten. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Kann gebraut werden (Rezept ID 244). Im Kampf: Liegt er in der linken Hand, fragt „Gegenstand benutzen“ nach einem Zielfeld (`g_attack_items`). Im Kampf aus der linken Hand benutzt: Meldung über giftigen Nebel auf dem Zielfeld, das Ziel (Gegner oder Held) verliert 1W6+4 LE, keine Probe. Das Miasthmaticum ist danach verbraucht. (`Quelle/seg043.c:FIG_use_item`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_use_miasthmaticum`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg033.c:FIG_menu`, `Quelle/seg043.c:FIG_use_item`
- **Deutung der KI:** Vermutlich ein Kampfgegenstand mit mittlerem, sicherem Schaden.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-239"></a>
### 239. Hylailer Feuer (`ITEM_ID_HYLAILER_FEUER`)

- **Im Code:** Gegenstands-ID 239 (`ITEM_ID_HYLAILER_FEUER`, Rohname in ITEMNAME `HYLAILER FEUER..`). ITEMS.DAT: benutzbar; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzbar-Eintrag 10 (`g_usable_items_table`): Benutzen ruft `item_use_hylailer_feuer()` auf. Benutzen außerhalb des Kampfes: Meldung: nur im Kampf zu benutzen Die Wirkung im Kampf steht unten. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Zwingfeste (6×). Kann gebraut werden (Rezept ID 203). Im Kampf: Liegt er in der linken Hand, fragt „Gegenstand benutzen“ nach einem Zielfeld (`g_attack_items`). Im Kampf aus der linken Hand benutzt: das Ziel (Gegner oder Held) verliert genau 20 LE, keine Probe. Das Hylailer Feuer ist danach verbraucht. (`Quelle/seg043.c:FIG_use_item`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_use_hylailer_feuer`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg033.c:FIG_menu`, `Quelle/seg043.c:FIG_use_item`
- **Deutung der KI:** Vermutlich ein Kampfgegenstand mit hohem, sicherem Schaden (20 LE).
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-240"></a>
### 240. Rezept für Starken Heiltrank (`ITEM_ID_REZEPT_FUER_STARKEN_HEILTRANK`)

- **Im Code:** Gegenstands-ID 240 (`ITEM_ID_REZEPT_FUER_STARKEN_HEILTRANK`, Rohname in ITEMNAME `REZEPT FÜR STARKEN HEILTRANK..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Rezept für Starker Heiltrank (ID 146): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Bronzeflasche, 2× Wirselkraut, 1× Tarnele, 1× Schnapsflasche, 1× Alraune; 12 AE, Alchimie-Probe +5, Dauer 24 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`) Reiseereignis Sumpf mit Olimone (Phexcaer–Skelellen): Sie schenkt der Gruppe dieses Rezept. Hat danach keiner das Rezept (voller Rucksack), greift das Original auf einen ungültigen Helden zu (Text mit Müllnamen). Fix 171 (Fanpatch-Stufe „nur Alle“) nennt dann den ersten handlungsfähigen Helden. (`Quelle/seg114.c:tevent_114`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/seg104.c:g_alchemy_recipes`, `Quelle/seg114.c:tevent_114`
- **Deutung der KI:** Vermutlich das Rezept, das man von Olimone geschenkt bekommt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-241"></a>
### 241. Rezept für Wunderkur (`ITEM_ID_REZEPT_FUER_WUNDERKUR`)

- **Im Code:** Gegenstands-ID 241 (`ITEM_ID_REZEPT_FUER_WUNDERKUR`, Rohname in ITEMNAME `REZEPT FÜR WUNDERKUR..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Rezept für Wunderkur (ID 236): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Glasflasche, 1× Schnapsflasche, 2× Wirselkraut, 1× Donfstengel, 1× Belmartblatt, 1× Alraune, 1× Jorugawurzel; 25 AE, Alchimie-Probe +10, Dauer 72 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um die Wunderkur brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-242"></a>
### 242. Rezept für Schlaftrunk (`ITEM_ID_REZEPT_FUER_SCHLAFTRUNK`)

- **Im Code:** Gegenstands-ID 242 (`ITEM_ID_REZEPT_FUER_SCHLAFTRUNK`, Rohname in ITEMNAME `REZEPT FÜR SCHLAFTRUNK..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Rezept für Schlaftrunk (ID 237): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Glasflasche, 1× Weinflasche, 1× Gulmond Blatt, 1× Tarnele; 2 AE, Alchimie-Probe +2, Dauer 8 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um den Schlaftrunk brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-243"></a>
### 243. Rezept für Star. Zaubertrank (`ITEM_ID_REZEPT_FUER_STARKER_ZAUBERTRANK`)

- **Im Code:** Gegenstands-ID 243 (`ITEM_ID_REZEPT_FUER_STARKER_ZAUBERTRANK`, Rohname in ITEMNAME `REZEPT FÜR STAR. ZAUBERTRANK..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Verfallene Herberge (2×). Rezept für Zaubertrank (stark) (ID 155): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Glasflasche, 1× Schnapsflasche, 3× Alraune, 3× Kairanhalm, 1× Thonnysblüte; 15 AE, Alchimie-Probe +12, Dauer 120 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um den starken Zaubertrank brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-244"></a>
### 244. Rezept für Miasthmaticum (`ITEM_ID_REZEPT_FUER_MIASTHMATICUM`)

- **Im Code:** Gegenstands-ID 244 (`ITEM_ID_REZEPT_FUER_MIASTHMATICUM`, Rohname in ITEMNAME `REZEPT FÜR MIASTHMATICUM..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 2 (`g_usable_items_table`): Benutzen ruft `item_read_recipe()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Rezept für Miasthmaticum (ID 238): Brauen braucht Alchimieset, dieses Rezept und die Zutaten 1× Glasflasche, 1× Schnapsflasche, 1× Kairanhalm, 1× Ilmenblatt, 1× Shurinknolle, 1× Alraune; 0 AE, Alchimie-Probe +6, Dauer 48 Stunden. AE und Zutaten sind auch bei Misslingen weg (dann kommt nur die Flasche zurück). Benutzen (Lesen) zeigt nur die Zutatenliste (Hinweis, was sich damit herstellen lässt). Zum Brauen muss das Rezept im Inventar sein (siehe Alchimie). (`Quelle/seg107.c:item_read_recipe`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_recipe`, `Quelle/seg104.c:g_alchemy_recipes`
- **Deutung der KI:** Vermutlich nur nötig, um das Miasthmaticum brauen zu können.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-245"></a>
### 245. Silberner Stirnreif (Fassung 2) (`ITEM_ID_SILBERNER_STIRNREIF__2`)

- **Im Code:** Gegenstands-ID 245 (`ITEM_ID_SILBERNER_STIRNREIF__2`, Rohname in ITEMNAME `SILBERNER STIRNREIF..`). ITEMS.DAT: benutzbar, magisch; Gewicht 10 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzbar-Eintrag 4 (`g_usable_items_table`): Benutzen ruft `item_invoke_armatrutz()` auf, 3 Ladungen. Benutzen: Der Benutzer wählt einen Helden der Gruppe, auf den der Zauber Armatrutz wirkt (wie beim Zaubern, siehe Zauberliste); das kostet eine von 3 Ladungen. Sind alle Ladungen weg: Meldung: die gespeicherte Magie ist aufgebraucht Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Spinnenhöhle (2×). Analüs Arcanstruktur erkennt ihn ab Probenergebnis 4 und gibt einen eigenen Text aus (SPELLTXT Nr. 57, sinngemäß: elfische Machart, ein Armatrutz ist enthalten). Das ist nur ein Text, ob die beschriebene Wirkung im Code existiert, steht in den anderen Angaben dieses Eintrags. Darf als einer von drei Stirnreifen (IDs 171, 217, 245) auf dem Kopf getragen werden, obwohl er keine Rüstung ist. (`Quelle/seg105.c:can_hero_equip_item_at_slot`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_invoke_armatrutz`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg099.c:g_analues_items`, `Quelle/seg105.c:can_hero_equip_item_at_slot`
- **Deutung der KI:** Sieht so aus, als ob der Stirnreif beim Benutzen Armatrutz wirkt (siehe Benutzbar-Eintrag) und auf dem Kopf getragen werden darf.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-246"></a>
### 246. Buch (Heptagon) (`ITEM_ID_BUCH__HEPTAGON`)

- **Im Code:** Gegenstands-ID 246 (`ITEM_ID_BUCH__HEPTAGON`, Rohname in ITEMNAME `BUCH..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 10 Dukaten (= 1000 Heller). Benutzbar-Eintrag 11 (`g_usable_items_table`): Benutzen ruft `item_read_spellbook_heptagon()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Spinnenhöhle. Benutzen (Lesen): magische Ströme fließen aus dem Buch Der Leser bekommt +2 auf den Zauber Heptagon und Krötenei (unabhängig von Klasse), danach verschwindet das Buch. (`Quelle/seg107.c:item_read_spellbook_heptagon`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_spellbook_heptagon`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`
- **Deutung der KI:** Vermutlich ein einmaliger Lernschub für Heptagon und Krötenei (+2), für jeden Leser.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-247"></a>
### 247. Empfehlungsschreiben (Siebenstein) (`ITEM_ID_EMPFEHLUNGSSCHREIBEN__SIEBENSTEIN`)

- **Im Code:** Gegenstands-ID 247 (`ITEM_ID_EMPFEHLUNGSSCHREIBEN__SIEBENSTEIN`, Rohname in ITEMNAME `EMPFEHLUNGSSCHREIBEN..`). ITEMS.DAT: benutzbar; Gewicht 5 Unzen; Grundpreis 2 Silbertaler (= 20 Heller). Benutzbar-Eintrag 3 (`g_usable_items_table`): Benutzen ruft `item_read_document()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Gibt Umbrik Siebenstein am Ende seines Gesprächs. Tiomar Swafnildsson prüft im Gespräch zweimal, ob die Gruppe dieses Schreiben dabei hat; davon hängt ab, wie das Gespräch weitergeht. (`Quelle/seg072.c:INF_yasma_umbrik_isleif`, `Quelle/seg072.c:INF_eliane_tiomar`) Benutzen (Lesen) zeigt nur einen Text: Umbriks Empfehlung an Tiomar Weitere Wirkung hat das Lesen nicht. (`Quelle/seg107.c:item_read_document`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_read_document`, `Quelle/seg072.c:INF_yasma_umbrik_isleif`, `Quelle/seg072.c:INF_eliane_tiomar`
- **Deutung der KI:** Vermutlich ein Questgegenstand für das Gespräch mit Tiomar.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-248"></a>
### 248. Schwarze Statuette (`ITEM_ID_SCHWARZE_STATUETTE`)

- **Im Code:** Gegenstands-ID 248 (`ITEM_ID_SCHWARZE_STATUETTE`, Rohname in ITEMNAME `SCHWARZE STATUETTE..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 10 Unzen; Grundpreis 20 Dukaten (= 2000 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Im Gespräch mit Eliane Windenbeck prüft das Spiel, ob die Gruppe die schwarze Statuette dabei hat. Davon hängt ab, wie das Gespräch weitergeht. (`Quelle/seg072.c:INF_eliane_tiomar`) Kultstätte des Namenlosen: Die Gruppe kann die schwarze Statuette mitnehmen oder zerstören. (`Quelle/seg084.c:DNG09_handler`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/seg072.c:INF_eliane_tiomar`, `Quelle/seg084.c:DNG09_handler`
- **Deutung der KI:** Vermutlich ein Questgegenstand für das Gespräch mit Eliane Windenbeck.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-249"></a>
### 249. Laterne (brennend) (`ITEM_ID_LATERNE__LIT`)

- **Im Code:** Gegenstands-ID 249 (`ITEM_ID_LATERNE__LIT`, Rohname in ITEMNAME `LATERNE..N`). ITEMS.DAT: benutzbar; Gewicht 15 Unzen; Grundpreis 3 Silbertaler (= 30 Heller). Benutzbar-Eintrag 12 (`g_usable_items_table`): Benutzen ruft `item_ignite()` auf. Handel: bei keinem Händler im Angebot (Sortimentswert 0). Alle 15 Spielminuten wird der Brennzähler der brennenden Laterne um 1 verringert; erreicht er 0, wird sie wieder zur nicht brennenden Laterne (ID 25). (`Quelle/seg002.c:sub_light_timers`) Entsteht beim Anzünden einer Laterne (ID 25) mit Öl, Brennzähler 100 (25 Stunden). (`Quelle/seg101.c:spell_brenne`) Licht in Dungeons (ohne Dunkelheit-Zauber, ohne Flim Flam und ohne Magier mit Stabzauber ab Stufe 2): Die Helligkeit richtet sich nach der Lichtquelle mit dem meisten Restbrennwert. Eine brennende Fackel zählt mit ihrem Brennzähler (10 = volles Licht, dann immer dunkler), eine brennende Laterne mit Brennzähler/10 (bleibt also viel länger hell). Im Original zählen auch Lichtquellen von Helden anderer Gruppen. Mit Fix 347 (Fanpatch-Stufe „nur Alle“) nur die der aktiven Gruppe. (`Quelle/seg106.c:get_max_light_time`, `Quelle/seg075.c:DNG_lights`) Benutzen der brennenden Laterne füllt sie nach: Mit Öl im Inventar wird der Brennzähler wieder auf 100 gesetzt, das Öl verbraucht, eine Bronzeflasche kommt zurück. Im Original sucht das Spiel Öl und Laterne beim zuletzt zaubernden Helden statt beim Benutzer (falscher Inventarplatz möglich). Fix 225 (Fanpatch-Stufe „Wichtig“) sucht beim Benutzer. (`Quelle/seg107.c:item_ignite`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c:g_usable_items_table`, `Quelle/seg107.c:item_ignite`, `Quelle/seg002.c:sub_light_timers`, `Quelle/seg101.c:spell_brenne`, `Quelle/seg106.c:get_max_light_time`, `Quelle/seg075.c:DNG_lights`
- **Deutung der KI:** Vermutlich die bessere Lichtquelle, die sich mit Öl nachfüllen lässt.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-250"></a>
### 250. Dukaten (`ITEM_ID_DUKATEN`)

- **Im Code:** Gegenstands-ID 250 (`ITEM_ID_DUKATEN`, Rohname in ITEMNAME `DUKATEN..`). ITEMS.DAT: ohne Typ (Bit 7 gesetzt, das nicht ausgewertet wird); Gewicht 0 Unzen; Grundpreis 1 Dukaten (= 100 Heller). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Räuberhäuser in Daspota. Steht der Gegenstand „Dukaten“ in einer Truhe mit mehreren Gegenständen, bekommt die Gruppe statt eines Gegenstands Geld (1 D je Stück). (`Quelle/seg092.c:loot_multi_chest`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg092.c:loot_multi_chest`
- **Deutung der KI:** Vermutlich nur ein Platzhalter in Truhentabellen für Geld.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-251"></a>
### 251. 200 Pfeile (`ITEM_ID_200_PFEILE`)

- **Im Code:** Gegenstands-ID 251 (`ITEM_ID_200_PFEILE`, Rohname in ITEMNAME `200 PFEILE..`). ITEMS.DAT: ohne Typ; Gewicht 0 Unzen; Grundpreis keiner (0). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Piratenhöhle. Kommt nie als eigener Gegenstand ins Inventar: Wer ihn bekommt (z. B. aus einer Truhe), erhält stattdessen 200 Pfeile (ID 10). (`Quelle/seg105.c:give_new_item_to_group`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg105.c:give_new_item_to_group`
- **Deutung der KI:** Vermutlich nur ein Platzhalter für 200 Pfeile.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-252"></a>
### 252. 50 Bolzen (`ITEM_ID_50_BOLZEN`)

- **Im Code:** Gegenstands-ID 252 (`ITEM_ID_50_BOLZEN`, Rohname in ITEMNAME `50 BOLZEN..`). ITEMS.DAT: ohne Typ; Gewicht 0 Unzen; Grundpreis keiner (0). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Piratenhöhle. Kommt nie als eigener Gegenstand ins Inventar: Wer ihn bekommt (z. B. aus einer Truhe), erhält stattdessen 50 Bolzen (ID 13). (`Quelle/seg105.c:give_new_item_to_group`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg105.c:give_new_item_to_group`
- **Deutung der KI:** Vermutlich nur ein Platzhalter für 50 Bolzen.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-253"></a>
### 253. 20 Kletterhaken (`ITEM_ID_20_KLETTERHAKEN`)

- **Im Code:** Gegenstands-ID 253 (`ITEM_ID_20_KLETTERHAKEN`, Rohname in ITEMNAME `20 KLETTERHAKEN..`). ITEMS.DAT: ohne Typ; Gewicht 0 Unzen; Grundpreis keiner (0). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Liegt in Truhen/Beutetabellen im Code: Piratenhöhle. Kommt nie als eigener Gegenstand ins Inventar: Wer ihn bekommt (z. B. aus einer Truhe), erhält stattdessen 20 Kletterhaken (ID 40). (`Quelle/seg105.c:give_new_item_to_group`)
- **Codestellen:** `Quelle/datseg.h:struct item_stats`, `Quelle/datseg.c (Truhen- und Beutetabellen gs_…)`, `Quelle/seg105.c:give_new_item_to_group`
- **Deutung der KI:** Vermutlich nur ein Platzhalter für 20 Kletterhaken.
- **Status:** im Code geprüft, im Spiel unbestätigt

<a id="item-254"></a>
### 254. (Endmarke, kein Gegenstand) (`ITEM_ID__END`)

- **Im Code:** Gegenstands-ID 254 (`ITEM_ID__END`, Rohname in ITEMNAME ``). ITEMS.DAT: ohne Typ; Gewicht 0 Unzen; Grundpreis keiner (0). Handel: bei keinem Händler im Angebot (Sortimentswert 0). Keine Abfrage dieses Gegenstands im Code: Außer Gewicht und Preis hat er keine Spielwirkung.
- **Codestellen:** `Quelle/datseg.h:struct item_stats`
- **Deutung der KI:** Vermutlich kein echter Gegenstand, sondern nur die Endmarke der Liste.
- **Status:** im Code geprüft, im Spiel unbestätigt

