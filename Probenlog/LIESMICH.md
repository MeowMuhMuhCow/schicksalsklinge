# Probenlog – sehen, was das Spiel würfelt

Zusatzfunktion der Bugfix-Fassung (WICHTIG und ALLES), ab Version 1.3. Jede Probe, die das Spiel würfelt, erscheint
live in einem zweiten Fenster und wird in einer Datei mitgeschrieben. Das gilt auch für die versteckten Proben,
die man sonst nie sieht, etwa Gefahrensinn oder Sinnesschärfe an Fallen.

```
17. Praios 15 Hal, 08:17 | TALIMEE zaubert Visibili (Zauberfertigkeit 3) → Probe auf KL/KL/GE (13/13/13), gesamt erleichtert um 3: 12, 10, 9 → gelungen, 11 Punkte übrig
```

## So geht's

| | Windows | Linux |
|---|---|---|
| Spielen mit Probenlog | `BLADEM_FIX_WICHTIG_MIT_PROBENLOG_STARTEN.bat` (bzw. `…_ALLES_…`) doppelklicken. Neben dem Spiel geht ein Fenster auf, in dem jede Probe erscheint | `bash Linux/starten.sh wichtig log` (bzw. `alles log`). Die Proben erscheinen im Terminal |
| Ohne Probenlog | wie bisher `BLADEM_FIX_WICHTIG_STARTEN.bat` | `bash Linux/starten.sh wichtig` |
| Hinterher ansehen | `Probenlog_Schicksalsklinge.html` im Browser öffnen und die Datei `PROBEN.CSV` hineinziehen: alle Proben als Tabelle, dazu Erfolgsquote je Held, Talent und Zauber. In Chrome/Edge kann die Seite auch live mitlesen | ebenso |

Die Datei liegt im Spielstand-Ordner der Fassung (`Build\Testspielstaende_Fix_Wichtig\PROBEN.CSV`, unter Linux
`Build/Linux_C_Fix_Wichtig/PROBEN.CSV`). Beim nächsten Start wandert sie nach `Build\Probenlogs`, mit Datum im Namen.

## Was drinsteht

| Probe | Angezeigt |
|---|---|
| Eigenschaftsprobe (MU, KL, KK …) | Eigenschaft, Wert, Zuschlag, der W20, Ergebnis |
| Talentprobe | Talent, Talentwert, Zuschlag, die drei Eigenschaften mit Werten, **jeder der drei W20**, Ergebnis mit Restpunkten |
| Fernkampf (Schuss- und Wurfwaffen) | Zielwert, W20, Treffer oder daneben |
| Zauberprobe | Zauber, Zauberfertigkeit, Zuschlag (mit Magieresistenz des Ziels), drei W20, Ergebnis |
| Kampf | jede Attacke und Parade, von Helden und Gegnern, mit W20 und AT- bzw. PA-Wert |
| Stufenanstieg (ab 1.5) | jeder Steigerungswurf für Talente und Zauber (alter Wert, Wurf, gelungen oder nicht), die gesteigerte positive Eigenschaft, die drei W20 beim Senken der negativen Eigenschaft, der LE-Wurf (bei Zauberkundigen die Punkte für LE und AE) und beim Magier der AE-Tausch. Im Anfängermodus, wo das Spiel selbst steigert, sieht man so erstmals, was passiert |
| Ereignisse (ab 1.5d) | die gewürfelte Gegnerzahl bei Kämpfen mit zufälliger Gegnerzahl (z. B. WILD8), der Überfallwurf im Nachtlager und beim Lagern im Dungeon (mit Schwelle, Wache, Travias Segen und in welcher Wache der Überfall kommt) und auf Reisen für jeden Tag der Wurf auf Zufallsbegegnung und auf Kampf |

Steigerungswurf: Das Spiel würfelt hier nicht 2W6 bzw. 3W6, sondern eine gleichverteilte Zahl von 2 bis 12 (Talentwert unter 11)
bzw. 3 bis 18 (ab 11). Gelungen ist die Steigerung, wenn der Wurf über dem bisherigen Wert liegt.

Gut zu wissen: Die Schicksalsklinge würfelt Talent- und Zauberproben nicht nach der Tischregel (jeder Wurf gegen seine
Eigenschaft). Sie zählt die drei W20 samt Zuschlag zusammen und vergleicht die Summe mit der Summe der drei
Eigenschaften. Das Log zeigt genau das.

## Technik

Das Spiel schreibt nur, wenn `PROBEN.CSV` schon da ist. Die Startdatei legt sie an, ohne Datei läuft alles wie
bisher. In der Datei stehen nur Zahlen und der Heldenname, eine Zeile pro Probe. Daraus machen das Fenster
(`probenlog_fenster.ps1`, unter Linux `probenlog.py`) und die HTML-Seite den Klartext. Die Proben laufen dabei exakt
gleich ab, mit gleich vielen Würfen in derselben Reihenfolge. Das Spielstandformat ist unverändert, und die
Standard-EXE bleibt bytegleich mit dem Original.

Zeilenformat: `Art;Tag;Monat;Jahr;Minute des Tages;Name;Werte…` mit Art 1 Eigenschaftsprobe
(Eigenschaft, Wert, Zuschlag, W20, Ergebnis), 3 Dreifachprobe (3 Eigenschaften, 3 Werte, Zuschlag, 3 W20, Ergebnis),
4 Talent (Talent, Talentwert bzw. Zielwert, Zuschlag, Fernkampf ja/nein, W20, Ergebnis), 5 Zauber (Zauber, ZfW,
Zuschlag), 6 Attacke und 7 Parade (Wert, W20, Gegnertyp, Richtung), 8 Stufenanstieg (Typ, Nr., alter Wert, Wurf, Ergebnis; Typ 1
Talent, 2 Zauber, 3 positive Eigenschaft +1, 4 negative Eigenschaft senken mit drei W20, 5 LE-Wurf, 6 AE-Tausch) und 9 Ereignis ohne Heldennamen: Typ 1 Gegnerzahl (Kampf, gewürfelte Obergrenze, Gegner in der
Kampfliste, davon im Kampf), 2 Nachtlager (W100, Schwelle, Travias Segen, Wache des Überfalls oder −1), 3 Lager im
Dungeon (W100, Schwelle, Stunden, Wache oder −1), 4 Reisebegegnung und 5 Reisekampf (W100, Schwelle, ja/nein).
