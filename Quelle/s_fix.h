/* s_fix.h - Fehlerbehebungen per Schalter (Projekt Schicksalsklinge, erzeugt von Werkzeuge/mkfix.py - nicht von Hand aendern)
 *
 * ALLE BEHEBUNGEN SIND IM SPIEL UNGETESTET. Tester willkommen (siehe Bauen/FIXES.md).
 *
 * Stufenschalter (BCC -D..., Vorgabe: Original):
 *   FIX_BUGS 0      Original, bytegleich mit BLADEM.EXE CD 3.02 de (Standard)
 *   FIX_BUGS 1      "wichtig": alle Fehler mit Auswirkung kritisch oder mittel
 *   FIX_BUGS 2      "alles": dazu alle Fehler mit Auswirkung egal
 *   FIX_EXPLOITS 1  auch Fehler beheben, die dem Spieler nuetzen (Vorgabe 1, sobald FIX_BUGS > 0); 0 = drinlassen
 * Einzelschalter: jeder FIX_nnnn laesst sich uebersteuern, z. B. -DFIX_0040=0 (aus) oder -DFIX_0004=1 (an).
 * Keine Behebung aendert Strukturen, das Spielstandformat, das Datensegment oder die Lage der residenten Segmente.
 *
 * Schalter   Stufe    Kategorie Exploit umgesetzt  Titel
 * ---------  -------  --------- ------- ---------  -----
 * FIX_0001   wichtig  mittel    -       nein       Speer wird als Fernkampfwaffe behandelt
 * FIX_0002   wichtig  mittel    -       nein       Wurfwaffe Schneidzahn hat keine Entfernungstabelle
 * FIX_0003   wichtig  mittel    -       nein       MR von 19 Gegnertypen wird mit 15W4 statt 1W6 ausgewuerfelt
 * FIX_0004   alles    egal      -       nein       Doppelgegner ueberschreibt beim Zug ueber eine Leiche deren Feldinhalt
 * FIX_0005   alles    egal      -       nein       Schwanzteil fliehender oder selbst getoeteter Doppelgegner bleibt auf dem Feld
 * (6)        -        egal      -       kein Fehler "Orig_BUG: I had this loop running infinitely"
 * FIX_0007   wichtig  kritisch  -       ja         Heldenzug ohne verbliebene Heldenphasen: Endlosschleife moeglich
 * FIX_0008   alles    egal      -       ja         Schwanzteil eines durch eigenen Patzer getoeteten Doppelgegners bleibt stehen
 * FIX_0009   alles    egal      -       ja         Abfrage g_max_enemies != 0 nach dem Sieg ist nie wahr
 * FIX_0010   alles    egal      -       nein       Fluchtverteilung uebergeht den NSC auf Platz 6
 * FIX_0011   wichtig  mittel    -       ja         Zauberziel: begehbare Objekte (Id >= 50) werden nicht abgewiesen
 * FIX_0012   alles    egal      -       ja         Flucht ueber den oberen und unteren Rand kostet ein BP mehr
 * FIX_0013   alles    egal      -       ja         Fluchtfeld wird mit dem Inhalt einer frueheren Cursorposition geprueft
 * FIX_0014   alles    egal      -       ja         Verschachtelte ifs: tote Figur auf dem Zielfeld ueberspringt die Weg- und BP-Pruefung
 * FIX_0015   wichtig  kritisch  -       ja         Bei genau 30 Beutestuecken schreibt "MEHR" ueber einen nicht gesetzten Zeiger
 * FIX_0016   alles    egal      -       ja         Knochen mit Rune beendet die Beuteliste
 * FIX_0017   alles    egal      -       ja         Meldung "ZIEL IST NICHT MEHR IN GUeLTIGER SCHUSSPOSITION" kann doppelt kommen
 * FIX_0018   wichtig  mittel    -       ja         Computerkampf: Helden greifen auch uebergelaufene Gegner an
 * FIX_0019   alles    egal      -       ja         Fluchtfelder (negative Ids) werden als Held gelesen
 * FIX_0020   alles    egal      -       ja         Umzingelt-Pruefung mit Grenze 25 statt 23
 * FIX_0021   wichtig  mittel    -       nein       Gegner schiessen auch auf direkt benachbarte Helden
 * FIX_0022   wichtig  kritisch  -       ja         Uebergelaufener Priester (Stufe 2) haengt das Spiel auf
 * (23)       -        egal      -       kein Fehler "Why unset 'tied' and not 'illusion'?"
 * FIX_0024   alles    egal      -       ja         best_viewdir moeglicherweise uninitialisiert
 * FIX_0025   wichtig  kritisch  -       ja         Fliehender Doppelgegner: Wegsuche haengt
 * FIX_0026   alles    egal      -       nein       memcpy 20 Byte in g_fig_move_pathdir[10]
 * FIX_0027   alles    egal      -       ja         Schwanzteile toter Doppelgegner sind nicht begehbar
 * FIX_0028   alles    egal      -       ja         Grenzpruefung: new_y > 23 doppelt, new_x > 23 fehlt
 * FIX_0029   alles    egal      ja      nein       Geworfene Waffen werden nur als Gegenstandsnummer gemerkt
 * FIX_0030   wichtig  kritisch  -       ja         Angstgift: Schreiben ueber nicht gesetzten Zeiger bei Held gegen Held
 * FIX_0031   alles    egal      -       ja         Held gegen Held: Werte des Verteidigers falsch berechnet
 * FIX_0032   alles    egal      -       ja         Paradepatzer setzt backfire_1 statt backfire_2
 * FIX_0033   alles    egal      -       nein       Waffenbruch bei gleichen Wuerfen trifft nie die Waffe eines Gegners
 * FIX_0034   wichtig  mittel    ja      ja         Wurfwaffe wird vor der Schadensberechnung abgelegt
 * FIX_0035   alles    egal      -       nein       double_size bleibt beim Rueckverwandeln eines Doppelgegners ungesetzt
 * FIX_0036   wichtig  mittel    -       ja         Gegner-Balsam heilt ueber das Maximum oder gar nicht
 * FIX_0037   wichtig  mittel    -       nein       Gegner-Plumbumbarum wirkt eine Stunde statt 5 Kampfrunden
 * FIX_0038   wichtig  mittel    -       ja         Endkampf: ohne Grimring kaempft Heldenplatz 0
 * FIX_0039   wichtig  mittel    -       ja         Akademie identifiziert und bricht Flueche nur ein einziges Mal im Spiel
 * FIX_0040   wichtig  kritisch  -       ja         Einbruch: "noch jemand frei?" zaehlt die gerade erwischte Gruppe mit
 * FIX_0041   alles    egal      -       ja         Verhaftung nur ab Platz 0, solange die Plaetze zur Gruppe gehoeren
 * FIX_0042   alles    egal      -       nein       Fehlende Haustypen 1 und 2: arr[1] bleibt ungesetzt
 * FIX_0043   wichtig  mittel    ja      ja         Statusleiste setzt 6 statt 4 Werte je Held, beim NSC wird die Krankheitstabelle ueberschrieben
 * FIX_0044   alles    egal      -       nein       Antworten werden gemischt, das Ergebnis aber nie benutzt
 * FIX_0045   wichtig  mittel    -       ja         Nicht vorhandene Antworten werden mitgemischt
 * FIX_0046   alles    egal      -       ja         Tempel: bei 20, 30, ... Helden eine leere Seite
 * FIX_0047   alles    egal      -       teilweise  leave_location() dreht die Gruppe auch dort um, wo sie nicht umdrehen soll
 * FIX_0048   wichtig  mittel    -       ja         Brauender NSC bleibt in der Herberge stecken
 * FIX_0049   wichtig  mittel    -       ja         NSC kann in der Herberge nicht zaubern, magic_act[6] schreibt hinter das Feld
 * FIX_0050   wichtig  mittel    -       ja         Brauende Helden: Einzelheld sieht sie nicht, nur der erste wird gefragt
 * FIX_0051   wichtig  mittel    -       ja         Nach dem Abholen gelten zusammengefuehrte Brauer weiter als "brauend"
 * FIX_0052   wichtig  mittel    -       ja         Herberge: Preisaufschlag fuer Einzelzimmer und Suite vom Schlafsaalpreis berechnet
 * FIX_0053   wichtig  mittel    -       teilweise  "Besonders guter Preis" beim Haendler verschlechtert den Verkauf
 * FIX_0054   wichtig  mittel    -       ja         Einkaufskorb: Platzbedarf falsch berechnet
 * FIX_0055   wichtig  mittel    -       nein       Volle Korbsumme wird bezahlt, auch wenn nicht alles verteilt werden konnte
 * FIX_0056   alles    egal      -       ja         Schmied zeigt das Geld des Helden, bezahlt wird aus der Gruppenkasse
 * FIX_0057   alles    egal      -       ja         Bei genau passendem Geld Feilschprobe fuer 0 %
 * FIX_0058   wichtig  mittel    -       nein       Tavernenmerker firstinfo, infopromise, round werden nie zurueckgesetzt
 * FIX_0059   wichtig  mittel    ja      ja         Taverne im Gespraech: Runde und Essen werden in Heller statt Silber abgezogen
 * FIX_0060   alles    egal      -       ja         Trinkprobe ueber Plaetze 0..n-1 statt ueber die Gruppenmitglieder
 * FIX_0061   alles    egal      -       ja         Praios-Wunder: Pruefung auf Zauberkundige wirkungslos
 * FIX_0062   alles    egal      -       ja         Praios und Hesinde heben Verwandlungen auch bei fremden Gruppen auf
 * FIX_0063   wichtig  mittel    -       ja         Firun-Wunder "keinen Hunger und Durst mehr" wirkt nur auf einen Helden
 * FIX_0064   alles    egal      -       ja         Taschendieb: falsche Textteile, das Ende "Dieb entkommt" erscheint nie
 * FIX_0065   wichtig  mittel    ja      ja         Strassenhaendler ueberschreibt den Typ des gerade betretenen Gebaeudes
 * FIX_0066   wichtig  mittel    ja      teilweise  Zeughaus: Stoerrebrandt-Schulden werden aus dem Staatsbudget bezahlt
 * FIX_0067   wichtig  kritisch  ja      ja         Zwingfeste: bei jedem Betreten 20 Dukaten
 * FIX_0068   wichtig  mittel    -       ja         Akademie erkennt einen Fluch beim Helden auf Platz 0 nicht
 * FIX_0069   alles    egal      -       ja         Fluchbrechung: Text 63 erscheint mit "%s"
 * FIX_0070   alles    egal      -       ja         Goettergunst ueber Index 0..13 statt 1..14
 * FIX_0071   alles    egal      -       ja         ESC ruft GUI_radio() mit -1 Optionen auf
 * FIX_0072   wichtig  mittel    ja      ja         Spielhaus: Gewinn und Verlust vertauscht
 * FIX_0073   alles    egal      -       nein       Spielhaus: "nach einigen Stunden", aber die Zeit steht
 * FIX_0074   alles    egal      -       nein       Phextempel: gs_ptemple_closed wird nie gesetzt
 * (75)       -        egal      -       kein Fehler "this would reactivate algrid"
 * FIX_0076   wichtig  mittel    -       ja         Informanten werden von "erledigt" (2) auf "bekannt" (1) zurueckgesetzt
 * FIX_0077   alles    egal      -       ja         Rueckgabewert bleibt oft uninitialisiert
 * FIX_0078   alles    egal      -       ja         Harikas Feilschbonus auch, wenn sie nicht dabei ist
 * FIX_0079   wichtig  mittel    -       ja         Stuendliche Sondertimer laufen bei Zeitspruengen nicht
 * FIX_0080   wichtig  kritisch  -       ja         Nur noch der NSC handlungsfaehig: das Spiel laeuft weiter
 * FIX_0081   alles    egal      -       ja         KK-Probe zaehlt Platz 0 ungeprueft
 * FIX_0082   alles    egal      -       ja         Gelber Pfeil der aktiven Gruppe wird vom violetten verdeckt
 * FIX_0083   alles    egal      -       ja         Sichtpruefung <= 16 laesst eine 17. Spalte neben der Karte zu
 * FIX_0084   wichtig  mittel    -       ja         Todesfalle (Totenschiff, Spinnenhoehle) beendet das Spiel auch fuer Gruppen draussen
 * FIX_0085   alles    egal      -       ja         Kletter-Meldung nennt Plaetze 0, 1, 2 ... ohne Gruppenpruefung
 * FIX_0086   alles    egal      -       ja         Proben und Hilfen zaehlen versteinerte und bewusstlose Helden mit (nur !dead geprueft)
 * FIX_0087   wichtig  mittel    ja      ja         Dietrich-Zaehler wird nach dem ersten Ebenenwechsel staendig zurueckgesetzt
 * FIX_0088   wichtig  mittel    -       ja         Klammerfehler: Feld (1,8,5) zeigt in jedem Dungeon den Spiegeltext
 * FIX_0089   alles    egal      -       ja         Symbol "Gruppen vereinen" bleibt nach dem Vereinen aktiv
 * FIX_0090   wichtig  mittel    ja      ja         Rueckwaertsschritt prueft halbdurchlaessige Waende nicht
 * FIX_0091   wichtig  mittel    ja      ja         Leeres Wasserfass fuellt trotzdem alle Schlaeuche
 * FIX_0092   wichtig  mittel    -       ja         Totenschiff-Spalt: nur Seil oder Strickleiter, kein Magierstab
 * FIX_0093   alles    egal      -       ja         Proben und Wirkungen mit get_hero(0) statt dem ersten handlungsfaehigen Helden
 * FIX_0094   wichtig  mittel    -       ja         Totenschiff: Deckenloch und Grube mit Zeilenbreite 32 statt 16 gesetzt
 * FIX_0095   alles    egal      -       ja         Totenschiff: Meldung vom sinkenden Schiff kommt nie
 * FIX_0096   alles    egal      -       ja         sprintf mit der Zahl 10 statt eines Namens
 * FIX_0097   wichtig  mittel    -       ja         Raetseltruhe: 10 AP Zusatzbelohnung werden nie vergeben
 * FIX_0098   alles    egal      -       ja         Klebriger Boden: "GELINGT ES, SICH LOSZUREISSEN" nur nach einem Fehlversuch
 * FIX_0099   alles    egal      -       nein       Wegteleportieren von Druckplatte oder Hebel wird nicht erkannt
 * FIX_0100   alles    egal      -       nein       Druckplatten der Verfallenen Herberge beruecksichtigen andere Gruppen nur halb
 * FIX_0101   alles    egal      -       ja         "IHR HOeRT EIN KNIRSCHEN" auch bei schon offener Wand
 * FIX_0102   wichtig  mittel    -       ja         weight_sum wird in der Gruppenschleife zurueckgesetzt
 * (103)      -        egal      -       kein Fehler "VORSICHT NACH DREI SCHRITT LINKS" - muesste es "rechts" heissen?
 * FIX_0104   alles    egal      -       nein       Beschwoerungsszene erscheint auch nach dem Kampf
 * FIX_0105   wichtig  mittel    -       ja         Teleporter und Schilfmeer treffen auch Helden anderer Gruppen
 * FIX_0106   wichtig  mittel    -       ja         Hebel und Geheimtuer verlangen Betreten UND Drehen im selben Schritt
 * FIX_0107   alles    egal      -       ja         Absturz in den Abgrund: hero_disappear() mit Position 0
 * FIX_0108   alles    egal      -       ja         Ratten beissen auch tote Helden
 * FIX_0109   alles    egal      -       ja         Armbrustfalle zaehlt nur Platz 0 und 1
 * FIX_0110   wichtig  mittel    ja      ja         Zwei-Gruppen-Raetsel pruefen nur x/y der anderen Gruppe
 * FIX_0111   wichtig  mittel    -       ja         Schleifen ueber 7 Plaetze beginnen beim ersten verfuegbaren Helden statt bei Platz 0
 * FIX_0112   alles    egal      -       nein       Merker "Raetseltruhe gepluendert" wird nie gesetzt
 * FIX_0113   alles    egal      -       ja         Hilferuf aus der Grube jede Minute statt alle 20 Minuten
 * FIX_0114   wichtig  mittel    -       ja         Armbrustfalle 4 zaehlt nicht herunter
 * FIX_0115   alles    egal      -       ja         Loser Stein: Zustand "entfernt" wird ueberschrieben
 * FIX_0116   alles    egal      -       ja         Trittplatte wirkt nur beim Entschaerfen
 * FIX_0117   wichtig  kritisch  ja      ja         Opferschale: ESC und grosse Eingaben (16-Bit-Ueberlauf) schenken Geld
 * (118)      -        egal      -       kein Fehler "possibly a bug: position (8,12) might be (8,11)"
 * FIX_0119   wichtig  mittel    -       ja         Geheimtueren: test_result bei falscher Hebelstellung uninitialisiert
 * FIX_0120   wichtig  mittel    -       nein       Zeitabfragen "day_timer % X == 0" greifen mehrmals hintereinander
 * FIX_0121   wichtig  mittel    -       ja         Verschuetteter Gang (Zustand 3) blockiert nicht
 * FIX_0122   wichtig  mittel    -       ja         Mit Werkzeug dauert das Freiraeumen laenger als ohne
 * FIX_0123   alles    egal      -       nein       Zaehler der Grubenversuche steht nicht im Spielstand
 * FIX_0124   wichtig  mittel    -       ja         Texte 31 und 32 fehlen in der deutschen OBER.DTX
 * (125)      -        egal      -       kein Fehler Deckeneinsturz (schwer): mit Werkzeug 18 statt 12 Stunden?
 * FIX_0126   wichtig  mittel    ja      ja         Opfer fuer Efferd und Ingerimm zaehlen auch ohne Wegwerfen
 * FIX_0127   wichtig  mittel    -       ja         Nur die erste Seeroute (Thorwal-Prem) wird zurueckgesetzt
 * FIX_0128   alles    egal      -       ja         Sprung bis Mitternacht (Swafnild) ueberspringt feste Ereignisse und verbraucht 25 Stunden Licht
 * FIX_0129   alles    egal      -       ja         Wachenwahl: ESC liest g_wildcamp_herbstatus[-1]
 * FIX_0130   alles    egal      -       ja         Bei weniger als 3 Stunden Schlaf wechseln die Wachen nie
 * FIX_0131   wichtig  mittel    -       nein       Seeweg Prem-Manrin nur 54 km lang
 * FIX_0132   alles    egal      -       ja         Hafenmeister schimpft auch beim Verlassen ueber fehlendes Geld
 * FIX_0133   alles    egal      -       ja         Hunger-/Durst-Meldungssperre des NSC wird nicht zurueckgesetzt
 * FIX_0134   wichtig  mittel    -       nein       Efferds Schutz verhindert den Krakenmolch, nicht die Piraten
 * FIX_0135   alles    egal      -       ja         Schiffsnamen werden bei jedem Blick neu ausgewuerfelt
 * FIX_0136   wichtig  mittel    ja      ja         Passagepreis in 16 Bit: lange Routen werden spottbillig
 * FIX_0137   wichtig  kritisch  -       ja         Schiffstempo in 16 Bit, Schnellsegler mit -106: Tempo 0 bringt das Spiel zum Absturz
 * FIX_0138   alles    egal      -       ja         Angezeigte Reisedauer nur in groben Stufen
 * FIX_0139   wichtig  mittel    -       nein       Hafen Hjalsingor: Route nach Overthorn fehlt in der Liste
 * FIX_0140   wichtig  mittel    -       nein       Hafen Leuchtturm Runin verweist auf eine Landweg-Liste
 * FIX_0141   alles    egal      -       nein       Ork-Monolith (Ereignis 71) steht auch auf dem Weg Oberorken-Felsteyn
 * FIX_0142   wichtig  mittel    ja      ja         Reiseereignisse: Lage wird nach Richtungswechsel falsch korrigiert
 * FIX_0143   wichtig  mittel    ja      nein       Gewaltmarsch-Tempo haengt von der schon gegangenen Strecke ab
 * (144)      -        egal      -       kein Fehler Gewaltmarsch zieht den LE-Verlust auf einmal ab
 * FIX_0145   alles    egal      -       ja         Umweg-Linie wird um einen Punkt versetzt zurueckgemalt
 * FIX_0146   wichtig  mittel    -       nein       Axt-Liste fuer Reiseereignisse
 * FIX_0147   alles    egal      -       nein       Rastplatz-Text ohne Platzhalter: Ortsbeschreibung und Kraeuterhinweis fehlen
 * FIX_0148   alles    egal      -       ja         Rastplatz: Status nur fuer den Anfuehrer geloescht
 * FIX_0149   wichtig  mittel    -       ja         replenish_stocks() bekommt die Heldenposition als Versuchszaehler
 * FIX_0150   wichtig  mittel    -       ja         "UMKEHREN" auf der Rueckreise behaelt die Rueckwaertsrichtung (9 Stellen)
 * FIX_0151   wichtig  mittel    -       ja         Furt: GE-Probe gilt fast immer als bestanden
 * FIX_0152   wichtig  mittel    -       ja         Faehrpreis waechst bei jedem Versuch weiter
 * FIX_0153   wichtig  mittel    -       ja         Gruppenproben zaehlen alle 7 Plaetze ohne Gruppen- und Todespruefung
 * (154)      -        egal      -       kein Fehler "Original-Glitch": gs_travel_detour = 1 ist zugleich die Nummer des Totenschiffs
 * FIX_0155   alles    egal      -       nein       Querverbindung: Herkunft wird nach Peilinen statt Kravik/Skelellen bestimmt
 * FIX_0156   alles    egal      -       ja         "AM UFER DES DES BODIR"
 * FIX_0157   alles    egal      -       nein       Brueckenbau Auplog-Varnheim: Pronomen im falschen Fall
 * FIX_0158   wichtig  mittel    -       ja         Steilhang: Strickleiter macht den Sturz schlimmer
 * FIX_0159   wichtig  mittel    ja      ja         Gorahs Huette: Silentium-Auswahl wird nie wiederholt
 * FIX_0160   alles    egal      -       ja         Huette bei Orvil: Frage passt nicht zu den Antworten
 * FIX_0161   wichtig  mittel    -       ja         Kraeuterbonus fuer Gegenstaende ueber 127 greift nie
 * FIX_0162   wichtig  mittel    ja      ja         Wegzoll 15 Dukaten insgesamt statt pro Person
 * FIX_0163   alles    egal      -       ja         Tatzelwurm-Spuren: Name aus einem Nullzeiger
 * FIX_0164   wichtig  kritisch  -       ja         Waten durch den Bach mit uninitialisiertem Helden
 * FIX_0165   wichtig  mittel    -       ja         Sturz in die Klamm: hero_disappear() mit der Antwortzahl als Position
 * FIX_0166   wichtig  mittel    ja      ja         Harpyien: Fluchtoption ist kein Versuch
 * FIX_0167   wichtig  mittel    -       ja         Nebeltal: Funktionsadresse als Held
 * FIX_0168   alles    egal      -       ja         Dialogbild fehlt nach einem Lagerkampf
 * FIX_0169   alles    egal      -       nein       "NACH GUT ACHT STUNDEN", vergehen aber 12
 * FIX_0170   wichtig  mittel    ja      ja         Firun-Opfer: Schlafbonus bleibt fuer spaetere Lager stehen
 * FIX_0171   alles    egal      -       ja         Olimone: get_hero(-1), wenn niemand das Rezept nehmen konnte
 * FIX_0172   wichtig  mittel    -       nein       Bruecke: Sturz in die Tiefe ist unerreichbar
 * FIX_0173   alles    egal      -       nein       Einsiedlersee: nur der erste Wasserschlauch wird gefuellt
 * FIX_0174   wichtig  kritisch  -       ja         Zustand 33 ist eine Endlosschleife
 * FIX_0175   alles    egal      -       nein       Landschaft 7 liest hinter die 7 Spalten der Begegnungstabelle
 * (176)      -        egal      -       kein Fehler Speer als Jagdwaffe
 * FIX_0177   wichtig  mittel    -       ja         AT- und PA-Proben verkehrt herum (Hoehlenbaer, Krakenmolch)
 * FIX_0178   wichtig  mittel    -       ja         Kvillotter beisst nie
 * FIX_0179   wichtig  mittel    -       ja         Krakenmolch: GE-Probe falsch ausgewertet
 * FIX_0180   wichtig  kritisch  -       ja         Krakenmolch: Endlosschleife, wenn alle Helden ueber Bord oder tot sind
 * FIX_0181   wichtig  mittel    -       ja         Zufallsbegegnungen: Vergleich verkehrt herum, fast alle unerreichbar
 * FIX_0182   alles    egal      -       ja         Krankheitsprobe mit uninitialisiertem Helden
 * FIX_0183   alles    egal      -       ja         Waldweg: Erfolgstext beim ersten Versuch unerreichbar
 * FIX_0184   wichtig  kritisch  -       ja         Verirrt im Wald: alle sieben Heldenplaetze sterben
 * FIX_0185   wichtig  mittel    -       ja         Kletterpartie: toedlicher Sturz bei knapp verfehlter Probe statt beim Patzer
 * FIX_0186   wichtig  mittel    -       ja         Visibili wirkt nur auf den Zaubernden, der Kampf entfaellt nur bei einem Einzelhelden
 * FIX_0187   alles    egal      -       ja         Transversalis-Kreuz darf eine Zeile unter die Karte
 * FIX_0188   alles    egal      -       ja         Meditation bei LE 6 oder 7 kostet LE und bringt keine AE
 * FIX_0189   wichtig  mittel    -       ja         Stabzauber: Abfrage "keine Stabzauber mehr" auf Stufe 7 statt 4
 * FIX_0190   wichtig  mittel    -       ja         Stabzauber-Probe prueft die zweite Eigenschaft doppelt
 * FIX_0191   wichtig  mittel    ja      ja         Foramen aus dem Zaubermenue ignoriert den Tuerzuschlag
 * FIX_0192   wichtig  mittel    -       ja         Gardianum kostet AE und schuetzt nicht
 * FIX_0193   alles    egal      -       ja         Spieltext "AUF %S RUHT ..." mit grossem S
 * FIX_0194   alles    egal      -       nein       "IST DOCH GARNICHT TOT" ohne Subjekt
 * FIX_0195   alles    egal      -       ja         Skelettarius: Leiche darunter verschwindet aus der Auswahl
 * FIX_0196   wichtig  kritisch  -       ja         Skelettarius belegt bei jedem Wirken 1288 Byte neu im Kampfobjektpuffer
 * FIX_0197   wichtig  mittel    -       ja         Foramen aus dem Zaubermenue wirkt nur nach geoeffnetem Tuermenue
 * FIX_0198   alles    egal      -       nein       Balsam: Mindestkosten 7 AE nicht einheitlich
 * FIX_0199   wichtig  mittel    ja      ja         Hexenspeichel: AE-Pruefung greift nie
 * FIX_0200   alles    egal      -       ja         Abbruch der Gegenstandswahl: inventory[-1]
 * FIX_0201   alles    egal      -       ja         Analues: volle AE-Kosten, wenn nur der Zuschlag die Probe kippt
 * FIX_0202   alles    egal      -       ja         "Eigenschaften seid gelesen" zeigt 80 % des Nahkampfschadens
 * FIX_0203   alles    egal      -       ja         Exposami vergleicht mit einem noch leeren Listeneintrag
 * FIX_0204   alles    egal      -       nein       Hexenknoten: Grafikfehler beim zweiten Wirken
 * FIX_0205   alles    egal      -       ja         Eisenrost auf ein Tier kostet volle AE
 * FIX_0206   alles    egal      -       ja         Spieltext "VERWANDLET"
 * FIX_0207   wichtig  mittel    ja      ja         Armatrutz: ein RS-Punkt mehr, als AE vorhanden
 * FIX_0208   alles    egal      -       ja         Paralue auf einen Helden prueft das alte Zauberziel
 * FIX_0209   alles    egal      -       nein       Salander auf zweiteilige Gegner: "strange effect"
 * FIX_0210   alles    egal      -       ja         Brenne entzuendet immer die erste Fackel oder Laterne
 * FIX_0211   wichtig  mittel    -       ja         g_ignite_mode wird nach dem Anzuenden nicht zurueckgesetzt
 * FIX_0212   wichtig  mittel    -       teilweise  Tollwut: Sanftmut ohne Pruefung des Zauberwerts
 * FIX_0213   wichtig  mittel    -       ja         Feuerbann wird gesetzt, aber nirgends ausgewertet
 * FIX_0214   wichtig  kritisch  -       teilweise  Magisches Bastardschwert ist als "benutzbar" markiert
 * FIX_0215   alles    egal      -       nein       Magischer Brotbeutel wirkt wie der magische Beutel
 * FIX_0216   alles    egal      -       ja         Truhenliste: 20 Byte je Name zu kurz
 * FIX_0217   alles    egal      -       ja         Mengenabfrage laesst sich nicht abbrechen
 * FIX_0218   wichtig  mittel    -       teilweise  Analues verspricht Wirkungen magischer Gegenstaende, die es nicht gibt
 * (219)      -        egal      -       kein Fehler Kraftguertel aktualisiert abhaengige Werte nicht
 * FIX_0220   wichtig  mittel    -       ja         Zaehler der belegten Inventarplaetze driftet
 * FIX_0221   wichtig  kritisch  -       ja         Zufaelliger Gegenstandsverlust: Endlosschleife ohne ablegbaren Gegenstand
 * FIX_0222   wichtig  kritisch  ja      ja         Tausch zweier Koerperplaetze ohne Pruefung und Boni
 * FIX_0223   alles    egal      -       ja         Wasserschlauch in der Hand wird nicht gefunden
 * FIX_0224   alles    egal      -       ja         Waffengift: Schleuder wird nicht ausgeschlossen
 * FIX_0225   wichtig  mittel    -       ja         Laterne nachfuellen: Oel beim letzten Zauberer gesucht, beim Benutzer entfernt
 * FIX_0226   alles    egal      -       teilweise  Tarnele: Meldung ohne Text
 * FIX_0227   alles    egal      -       ja         Antikrankheitselixier ohne Krankheit schreibt in disease[0]
 * (228)      -        egal      -       kein Fehler "What about herokeeping()?" im stuendlichen Timer
 * FIX_0229   wichtig  mittel    -       ja         h_index wird nur einmal vor der Schleife auf -1 gesetzt
 * FIX_0230   alles    egal      -       ja         Aufraeumen ungueltiger Ziele setzt nur den aktuellen Eintrag zurueck
 * FIX_0231   alles    egal      -       ja         Notbehelf bei 100 belegten Timern versagt im Kampf
 * FIX_0232   wichtig  mittel    -       nein       Gift wirkt nur in game_loop(), bei Zeitspruengen hoechstens einmal
 * FIX_0233   wichtig  mittel    -       ja         Heil-, Stab- und Lichttimer runden kleine Zeitspruenge weg
 * FIX_0234   wichtig  mittel    -       ja         Stundenzaehlung fuer Kettenhemd-Schaden und Essen falsch
 * FIX_0235   wichtig  kritisch  -       ja         Zufallsheld aus Plaetzen 0..n-1 statt aus den Gruppenmitgliedern
 * (236)      -        egal      -       kein Fehler Rote Anzeige des bleibenden LE-Verlusts (Bright-Eyes-Komfort)
 * FIX_0237   alles    egal      -       ja         BP-Anzeige bei Last ab 110 % falsch
 * FIX_0238   wichtig  mittel    -       ja         Erneute Vergiftung loescht die Protokollbytes
 * (239)      -        egal      -       kein Fehler "not a real BUG": keine Pruefung auf leeren Heldenplatz
 * FIX_0240   wichtig  mittel    -       ja         Erneute Erkrankung loescht die Protokollbytes, Verluste werden dauerhaft
 * (241)      -        egal      -       kein Fehler AT-Basis nicht nach DSA 3 (MU + KK + GE) / 5
 * FIX_0242   alles    egal      -       teilweise  Ohne waehlbaren Helden: Textbreite bleibt 3, geliefert wird Platz 0
 * FIX_0243   wichtig  mittel    -       ja         Gruppe teilen klappt nicht richtig, wenn der NSC in einer anderen Gruppe ist
 * FIX_0244   wichtig  kritisch  -       ja         Gruppenwechsel ohne Ausgang, wenn keine Gruppe frei handeln kann
 * FIX_0245   alles    egal      -       ja         Nachtwache folgt beim Tauschen nur einem der beiden Helden
 * FIX_0246   alles    egal      -       ja         Regeneration: Ergebnis 0 bleibt 0, -1 wird zu 1
 * (247)      -        egal      -       kein Fehler Brauen endet nur ohne Gift, Krankheit und Stabzaubersperre
 * FIX_0248   wichtig  kritisch  ja      ja         Stufenanstieg: ESC bei "WELCHE POSITIVE EIGENSCHAFT" bringt rund 168 000 Dukaten
 * FIX_0249   wichtig  mittel    -       ja         Stufenanstieg: ESC bei "WELCHE NEGATIVE EIGENSCHAFT" senkt die Intuition
 * FIX_0250   wichtig  mittel    -       ja         MR-Neuberechnung beim Stufenanstieg vertraegt sich nicht mit MR-Gegenstaenden
 * FIX_0251   alles    egal      -       ja         Zaubersteigerung: Zaehler i wird je Runde nicht zurueckgesetzt
 * FIX_0252   alles    egal      -       ja         Zufallszauber 1..86 statt 1..85
 * FIX_0253   wichtig  mittel    -       ja         Spielende: Merker fuer bleibenden LE-Verlust wird geloescht, LE-Maximum bleibt gesenkt
 * FIX_0254   wichtig  mittel    -       ja         set_mod_slot() mit who = 0 fuer einen anderen Helden
 * FIX_0255   wichtig  mittel    -       ja         Harikas Abschiedsgeschenk kostet Steigerungsversuche
 * FIX_0256   wichtig  mittel    -       ja         Entlassene NSC kommen nie wieder
 * (257)      -        egal      -       kein Fehler "Why STAFFSPELL?? BUG!" - staffspell_timer als Regenerationssperre
 * FIX_0258   alles    egal      -       ja         Misslungene Wundheilung setzt Wundfieber ohne Pruefung
 * FIX_0259   wichtig  mittel    -       ja         Brauen: Abtrennen des einzigen Helden einer Gruppe moeglich
 * FIX_0260   wichtig  kritisch  -       ja         Suche nach freier Gruppe ohne Grenze
 * FIX_0261   alles    egal      -       nein       "DA SCHEINT ABER WAS ZU FEHLEN" nennt die Zutat nicht
 * FIX_0262   wichtig  mittel    -       nein       Heilen Krankheiten: Wundfieber und Blaue Keuche nie heilbar, Erfolg wird als Misserfolg gemeldet
 * FIX_0263   alles    egal      -       ja         Textdatei wird bei totem Patienten nicht zurueckgeladen
 * FIX_0264   wichtig  mittel    -       ja         Verschwundener NSC: Datei und Timer um eins verschoben
 * FIX_0265   alles    egal      -       ja         Verschlimmerung zu Blauer Keuche mit 4 statt 5 %
 * FIX_0266   alles    egal      -       ja         Nach dem Wechsel zu Blauer Keuche noch Dumpfschaedel-Schaden am selben Tag
 * (267)      -        egal      -       kein Fehler "Original-Bug? forgot? time_counter = 0"
 * FIX_0268   alles    egal      -       nein       Blaue Keuche: "DAS LEBEN WIRD WOHL NIE WIEDER SEIN" immer
 * FIX_0269   wichtig  mittel    -       ja         Paralyse und Frostschaeden: Erholung mit umgekehrter Bedingung
 * FIX_0270   wichtig  mittel    -       ja         Schlachtfeldfieber endet nie
 * FIX_0271   alles    egal      -       nein       Zeitzaehler von Krankheiten und Giften koennen ueberlaufen
 * (272)      -        egal      -       kein Fehler "Not a bug, but useless": Markierung sex = 50
 * FIX_0273   wichtig  mittel    -       ja         Tollwut: Beruhigen und Sanftmut werten misslungene Proben als Erfolg
 * FIX_0274   wichtig  mittel    -       ja         Shurinknollengift und Bannstaub: Rueckgabe der Werte fast nie
 * FIX_0275   alles    egal      -       ja         Uebertrag der Subtraktion fehlt im High-Word der Titellaenge
 * FIX_0276   alles    egal      -       nein       NVF-Typ 1: Versatz mit nicht gelesenen Breiten und Hoehen
 * FIX_0277   alles    egal      -       nein       Rueckgabewert bei RLE-Bildern nicht gesetzt
 * FIX_0278   alles    egal      -       nein       Heldenbild-Abfrage reicht ein Feld zu weit (ACTION_ID_HERO_NPC + 1)
 * FIX_0279   alles    egal      -       nein       Lesen hinter icons_bak[9]
 * FIX_0280   alles    egal      -       ja         Bright-Eyes-Fix fuer die AE-Leiste prueft den falschen Merker
 * FIX_0281   alles    egal      -       ja         Anschnitt oben vergroessert die Bildhoehe, statt sie zu verkleinern (3 Stellen)
 * FIX_0282   alles    egal      -       ja         Wanduhr: Position laeuft bis 84, Tabelle hat nur 81 Werte
 * (283)      -        egal      -       kein Fehler "potential bug": nur dst von g_pic_copy wird gesichert
 * FIX_0284   alles    egal      -       ja         Zufallszahl kann 0 oder negativ werden
 * FIX_0285   alles    egal      -       nein       Modus 3: Anschnitt oben rueckt die Quelle um die Bildbreite statt um 320 vor
 * FIX_0286   alles    egal      -       nein       Toter OPL-Code: Timer 1 startet nie, wirkungslose Befehle
 * FIX_0287   alles    egal      -       ja         Leeres Tagebuch: Vergleich mit gs_diary_entries[-1]
 * FIX_0288   alles    egal      -       ja         Kopierhoehe line * 7 statt (line - startline) * 7
 * FIX_0289   alles    egal      -       ja         Speichern: do-while ohne Pruefung von findfirst
 * (290)      -        egal      -       kein Fehler "when using EMS something strange happens"
 * FIX_0291   alles    egal      -       nein       Letzte Figur liest das Ende hinter der TAB-Tabelle
 * FIX_0292   alles    egal      -       ja         Stadtdatei: liest 100 Byte mehr, als g_locations_tab fasst
 * FIX_0293   alles    egal      -       ja         Kein break nach dem Fund
 * FIX_0294   alles    egal      -       ja         Bereichspruefung clip_num > max statt >= max
 * FIX_0295   alles    egal      -       nein       Toter Zweig bei der Tiefe fuer Objekte 58..61
 * FIX_0296   alles    egal      -       ja         Dukaten-Anzeige laeuft ab 32768 D ueber
 * FIX_0297   alles    egal      -       nein       Bedingung state || mode != 2 ist immer wahr
 * FIX_0298   alles    egal      -       ja         Lesen vor g_hero_pic_posx bei Maus-x < 3
 * FIX_0299   alles    egal      -       ja         Kartenwerte ab 0xf0 lesen hinter die Tabelle
 * FIX_0300   alles    egal      -       nein       Gespiegelte Bilder werden am Rand von der falschen Seite beschnitten
 * (301)      -        egal      -       kein Fehler "not a bug, but useless"
 * FIX_0302   alles    egal      -       nein       Beugung der Namen: Bits mit + gesetzt, 2. und 3. Fall verwechselt
 * FIX_0303   alles    egal      -       nein       Pronomentabelle hat 7 statt 8 Eintraege
 * FIX_0304   alles    egal      -       ja         Spielstandname bearbeiten: Schreibstelle hinter dem Namen
 * FIX_0305   alles    egal      -       nein       Feldzugriff vor der Grenzpruefung (3 Stellen)
 * FIX_0306   alles    egal      -       ja         Abfrage "WIE VIELE DER ... SOLLEN ..." im falschen Fall
 * (307)      -        egal      -       kein Fehler Lichtquellen-Suche auch auf leeren Heldenplaetzen
 * FIX_0308   alles    egal      -       ja         Speichermeldung nennt 5000 Byte zu wenig
 * FIX_0309   wichtig  kritisch  -       ja         Fehlgeschlagener Start von GEN.EXE: Spiel laeuft ohne Puffer weiter
 * FIX_0310   alles    egal      -       nein       sprintf mit mehr Argumenten als Platzhaltern
 * FIX_0311   alles    egal      -       nein       Geflohener Gegner gilt als getoetet (Gorah)
 * FIX_0312   alles    egal      ja      ja         Gefesselte Gegner bleiben in der Kampfliste und kaempfen beim naechsten Betreten erneut
 * FIX_0313   wichtig  mittel    -       ja         Lagerueberfall: nur Heldenplatz 0 ist wach, eingeteilte Wachen zaehlen nicht
 * FIX_0314   wichtig  mittel    -       nein       Kampf: Waffe oder Gegenstand laesst sich nur tauschen, nicht ablegen
 * FIX_0315   alles    egal      -       nein       Schusslinie: bewusstlose und schlafende Figuren blockieren, Leichen nicht
 * FIX_0316   wichtig  mittel    -       ja         Zerbrochene Waffe bleibt in der Hand, Computerkampf wechselt nicht
 * FIX_0317   alles    egal      -       nein       Computerkampf: Held mit 1 BP oder weniger als 3 Rest-BP bleibt untaetig
 * FIX_0318   alles    egal      -       nein       Computerkampf zaubert ohne Blick auf das Ziel
 * FIX_0319   wichtig  mittel    -       ja         Computerkampf zaubert auch mit Zauberwert unter -5
 * FIX_0320   alles    egal      -       nein       Gegner fliehen auch ueber den rechten Rand, Helden nicht
 * FIX_0321   alles    egal      -       nein       Verschreckter Gegner ohne Fluchtweg tut nichts mehr
 * FIX_0322   wichtig  mittel    ja      ja         Alte Nahkampfoption ohne Nachbarschaftspruefung ("Schattenboxen")
 * FIX_0323   alles    egal      -       nein       Schwacher Heshthot (Runin) hat nach der LE-Kuerzung 0 LE
 * FIX_0324   alles    egal      -       nein       Keine Waffenresistenzen fuer Skelette und Mumien
 * FIX_0325   wichtig  mittel    ja      nein       Spinnendaemon gilt als Tier und ist fuer Waffengift empfaenglich
 * FIX_0326   wichtig  mittel    -       nein       Waffengift: jedes Gift macht 10 x Staerke Zusatzschaden
 * FIX_0327   alles    egal      ja      teilweise  Piratenangriff: Ergebnis von do_fight() wird nicht ausgewertet
 * (328)      -        egal      -       kein Fehler Computerkampf im Endkampf ohne Meldung gesperrt
 * (329)      -        egal      -       kein Fehler "Alte Optionen" fehlt im zweiten Menue eines Zuges
 * (330)      -        egal      -       kein Fehler Zufaellige Zugreihenfolge statt Initiative
 * FIX_0331   alles    egal      ja      nein       Markt- und Hafenangebot stehen nicht im Spielstand
 * FIX_0332   alles    egal      -       ja         Oeffnungszeit 8-19 Uhr gilt auch fuer Strassenhaendler und Treborn Kolberg
 * FIX_0333   alles    egal      -       nein       Tavernengeruechte in Serske: Lesen hinter dem Dateiende
 * FIX_0334   alles    egal      -       ja         Beleidigter Wirt wirft nicht hinaus
 * FIX_0335   alles    egal      -       nein       Wein und Schnaps ohne Zechenprobe
 * FIX_0336   alles    egal      -       ja         Essen in der Taverne verbraucht zusaetzlich Proviant
 * FIX_0337   wichtig  mittel    -       ja         Schmied Gurthag nach dem Kellerausgang dauerhaft unbenutzbar
 * FIX_0338   alles    egal      -       ja         Treborn Kolbergs Laden: Ortszeile leer oder fremd
 * FIX_0339   wichtig  mittel    ja      ja         Akademie: Bezahlung mit erstem gleichen Stueck, auch kaputt
 * FIX_0340   alles    egal      -       nein       Informant Tiomar mit Manrin statt Brendhil eingetragen
 * FIX_0341   wichtig  mittel    -       ja         Treborn Kolberg erscheint nach Algrids Hinweis nie, Olvir/Treborn wiederholt sich
 * FIX_0342   alles    egal      -       nein       Jurge Torfinsson: "nur Thorwaler" auch bei reinen Thorwalern
 * FIX_0343   alles    egal      -       ja         Taverne: Gewinn wird als unsinnige Ausgabe angezeigt
 * (344)      -        egal      -       kein Fehler Akrobatik und Falschspiel je nach Ort verschieden
 * FIX_0345   wichtig  mittel    -       nein       Kampf auf Spezialfeldern startet erst nach Drehen oder Menue
 * FIX_0346   wichtig  mittel    -       ja         Ereignisfelder loesen nach Gruppenwechsel erneut aus
 * FIX_0347   alles    egal      -       ja         Dungeonlicht: Lichtquellen anderer Gruppen zaehlen mit
 * FIX_0348   wichtig  mittel    -       nein       Lager in Dungeons: immer einer von vier allgemeinen Lagerkaempfen
 * FIX_0349   wichtig  mittel    ja      nein       Transversalis im Dungeon auf Felder ausserhalb des Plans
 * FIX_0350   wichtig  mittel    ja      nein       Verlaesst eine Teilgruppe das Totenschiff, sinkt es fuer die uebrigen nicht mehr
 * FIX_0351   wichtig  mittel    -       ja         Speerfallgrube (Spinnenhoehle): Spielende auch fuer Gruppen draussen
 * FIX_0352   alles    egal      -       ja         Giftdorn-Truhe nach sechs Ladungen ohne Text
 * FIX_0353   alles    egal      -       teilweise  Spinnenaltar: Auswahl ohne "spaeter", Text und Kampf wiederholen sich
 * FIX_0354   wichtig  mittel    -       ja         Gesperrte Tueren ohne Meldung, Daspota-Falltuer sperrt eine Tuer fuer immer
 * FIX_0355   wichtig  mittel    -       ja         Schatzlager nur beim ersten Betreten pluenderbar (Daspota-Schatz, Zwingfeste)
 * FIX_0356   alles    egal      -       nein       Zwingfeste: Anseilen erst ab fuenf Seilen, kein Seil verbraucht
 * FIX_0357   wichtig  mittel    -       ja         Oberorken-Wasserfalle: Raum zu klein, Ertrinken nur an 5-Minuten-Marken
 * FIX_0358   alles    egal      ja      teilweise  Oberorken-Wasserraum: Tueren beliebig oft knacken (je 1 AP)
 * FIX_0359   wichtig  mittel    ja      nein       Prem-Mine: AP fuer Deckenfelder beliebig oft
 * FIX_0360   alles    egal      -       nein       Orkhoehle: jedes Durchsuchen der Strohlager kostet CH -2
 * FIX_0361   alles    egal      ja      teilweise  Runin: Drachenhort und Platinschluessel-Auftrag unabhaengig
 * FIX_0362   wichtig  kritisch  ja      ja         Fluch der Goldmuenzen (Burgruine): jeder Kampf gilt sofort als gewonnen
 * (363)      -        egal      -       kein Fehler Spinnenhoehle: Teleport ueberspringt Feldereignisse
 * (364)      -        egal      -       kein Fehler Saeuregruben treffen nur die ersten beiden Helden
 * FIX_0365   wichtig  mittel    -       nein       Reiseziel nicht getrennt je Teilgruppe
 * FIX_0366   wichtig  mittel    ja      nein       Orkhoehle: Ausgang setzt die Gruppe direkt aufs Reiseziel
 * FIX_0367   wichtig  mittel    ja      ja         Weiterreisen ohne Nachtlager ueber unsichtbares Symbol
 * FIX_0368   wichtig  mittel    -       ja         Tavernenhinweis schaltet Reiseereignisse 66 und 75 ab
 * FIX_0369   alles    egal      -       ja         Nach Gorah dreht sich die Reiserichtung immer um
 * FIX_0370   wichtig  mittel    -       nein       Gorahs Haus nach Flucht der Helden unzugaenglich
 * FIX_0371   alles    egal      -       ja         Grenz- und Strassentexte ohne Richtungspruefung
 * FIX_0372   wichtig  mittel    ja      ja         Seereise: jeder Umweg-Wert fuehrt ins Totenschiff
 * FIX_0373   alles    egal      -       nein       Floss ohne Werkzeug baubar
 * FIX_0374   alles    egal      ja      nein       Berghuette: Jagd zweimal moeglich
 * FIX_0375   alles    egal      ja      ja         Jagd: Einzelheld schafft immer den Blattschuss
 * (376)      -        egal      -       kein Fehler Gorah-Versteck ohne Weiterreise
 * (377)      -        egal      -       kein Fehler Gorahs Truhe: immer der erste Held oeffnet
 * (378)      -        egal      -       kein Fehler Nachtrast immer bis 8 Uhr
 * FIX_0379   wichtig  mittel    -       nein       Axxeleratus: zweite Aktion wiederholt den Angriff ohne Zielwahl
 * FIX_0380   wichtig  mittel    -       nein       Boeser Blick setzt dauerhaft zwei Attacken
 * FIX_0381   alles    egal      -       nein       Boeser Blick auf Leichen moeglich
 * FIX_0382   wichtig  mittel    -       ja         Grosse Gier ist nicht umgesetzt
 * FIX_0383   wichtig  mittel    -       ja         Transversalis auf der Landreise kostet AE ohne Wirkung
 * FIX_0384   alles    egal      ja      nein       Herberge: Regenerationssperre nach Magie haengt am Heldenplatz
 * FIX_0385   wichtig  mittel    -       ja         Curian zaubert im Kampf nur im Computerkampf mit Magie
 * FIX_0386   alles    egal      -       teilweise  Analues erkennt nur 28 Gegenstaende, Grimring nicht
 * (387)      -        egal      -       kein Fehler Horriphobus: verschreckter Gegner gilt nicht als besiegt
 * (388)      -        egal      -       kein Fehler Zauberproben als Summenprobe (Boeser Blick gelingt oft)
 * (389)      -        egal      -       kein Fehler Kampfzauber ausserhalb des Kampfes waehlbar
 * FIX_0390   alles    egal      -       teilweise  Unzerbrechliche Waffen koennen zerbrechen (Held gegen Held, Curians Stab)
 * FIX_0391   alles    egal      -       nein       Magier duerfen den verfluchten Lederhelm tragen
 * FIX_0392   alles    egal      -       nein       Zwei Kriegsbeile mit verschiedenen Werten
 * FIX_0393   wichtig  mittel    -       nein       Steinschleuder ist unbenutzbar
 * FIX_0394   alles    egal      -       ja         Wegwerfen: hoechstens 99 Stueck, Mengenabfrage auch bei einem Stueck
 * FIX_0395   alles    egal      -       nein       Uebergabe auf vollen 99er-Stapel: ohne Meldung nichts
 * FIX_0396   alles    egal      -       nein       Kraeuterfund bei vollem Rucksack verloren
 * FIX_0397   alles    egal      ja      ja         Tausch zwischen Helden prueft nur die Traglast des Empfaengers
 * FIX_0398   alles    egal      -       ja         Zweihaender von rechter in linke Hand: falsche Meldung
 * FIX_0399   alles    egal      -       nein       Schild mit Zauberstab oder Hexenbesen kombinierbar
 * FIX_0400   alles    egal      -       ja         Waffengift ueber BENUTZEN umgeht die Klassensperre
 * FIX_0401   wichtig  kritisch  -       ja         Entlassene Helden hinterlassen Luecken in den Heldenplaetzen
 * FIX_0402   wichtig  mittel    ja      ja         Aufgesparte Zaubersteigerungen laufen ueber 127
 * FIX_0403   alles    egal      -       ja         Stufenaufstieg erst ab 101 statt 100 AP
 * FIX_0404   wichtig  mittel    ja      ja         Elixiere stapeln sich ohne Obergrenze
 * FIX_0405   alles    egal      -       ja         Angriffsart bleibt nach dem Kampf im Helden stehen
 * FIX_0406   wichtig  mittel    -       nein       Ruestungsbehinderung wirkt auf keine Talentprobe
 * FIX_0407   alles    egal      -       ja         Gruppengeld: Rest der Teilung verfaellt
 * (408)      -        egal      -       kein Fehler Zaubersteigerungen bei Nichtmagiern ("Waldelf-Jaeger-Fehler")
 * FIX_0409   alles    egal      -       nein       Namensbeugung: Eigennamen mit Artikel, Adjektive ungebeugt
 * FIX_0410   alles    egal      -       ja         Kampfmusik laeuft nach direkt gestarteten Dungeonkaempfen weiter
 * FIX_0411   alles    egal      -       nein       Umlaute im Spielstandnamen ergeben "Diskette voll"
 * FIX_0412   alles    egal      -       nein       Namenlose Tage im Tagebuch als -5. bis -1. Swafnir
 * FIX_0413   alles    egal      -       nein       Zeitvorspulen ohne Bremse
 * FIX_0414   wichtig  mittel    -       teilweise  Einhorn: Held als Zeiger im Spielstand
 * FIX_0415   alles    egal      -       nein       Schiffsreise mit Haendlermusik
 * FIX_0416   alles    egal      -       nein       Zufallswurf vor der Erledigt-Pruefung
 * (417)      -        egal      -       kein Fehler Textfenster schliessen bei jeder Taste
 * FIX_0418   alles    egal      -       ja         Schatzkarte: wer nur Hjores Kartenteil 9 hat, bekommt "keine Kartenteile gefunden"
 * FIX_0425   alles    egal      -       ja         Tavernenschlaegerei schreibt ueber gs_town_outlawed_flags hinaus in die Rauswurf-Merker der Herbergen
 * FIX_0426   wichtig  mittel    -       ja         Spiel laden im Tempel: leave_location() laeuft ueber den geladenen Spielstand
 * FIX_0432   wichtig  mittel    ja      ja         Manrek-Schatzkammertuer: Sperre und Eisenschluessel auf falschem Feld (0,12,8) statt (0,12,7)
 * FIX_0433   wichtig  mittel    -       ja         Totenschiff-Falltueren loesen neben der Grube aus und schieben die Gruppe (bei (1,14,5) in die Wand)
 * FIX_0434   alles    egal      -       ja         Speerfallgrube der Spinnenhoehle: eine Stunde Herausklettern auch, wenn niemand gestuerzt ist
 * FIX_0435   alles    egal      -       ja         Runin: loser Stein vor dem Hebel (0,8,1) gilt erst nach Betaetigen des Hebels als entfernt
 * FIX_0436   alles    egal      -       nein       STRASSE.LTX ist groesser als der 10000-Byte-Textpuffer
 */
#ifndef S_FIX_H
#define S_FIX_H

#ifndef FIX_BUGS
#define FIX_BUGS 0
#endif
#ifndef FIX_EXPLOITS
#if FIX_BUGS > 0
#define FIX_EXPLOITS 1
#else
#define FIX_EXPLOITS 0
#endif
#endif

/* Vorgaben je Stufe (X = Exploit: nur mit FIX_EXPLOITS) */
#define FIX_STUFE_W  (FIX_BUGS >= 1)
#define FIX_STUFE_A  (FIX_BUGS >= 2)
#define FIX_STUFE_WX (FIX_BUGS >= 1 && FIX_EXPLOITS)
#define FIX_STUFE_AX (FIX_BUGS >= 2 && FIX_EXPLOITS)

/* Zusatzfunktion, keine Fehlerbehebung: Probenlog. Jede Probe wird als Zahlenzeile an PROBEN.CSV im Spielordner
 * angehaengt, aber nur, wenn diese Datei schon da ist (Datei anlegen = Log an). Spiel und Spielstand bleiben gleich. */
#ifndef FIX_PROBENLOG
#define FIX_PROBENLOG FIX_STUFE_W
#endif

#ifndef FIX_0001
#define FIX_0001 FIX_STUFE_W
#endif
#ifndef FIX_0002
#define FIX_0002 FIX_STUFE_W
#endif
#ifndef FIX_0003
#define FIX_0003 FIX_STUFE_W
#endif
#ifndef FIX_0004
#define FIX_0004 FIX_STUFE_A
#endif
#ifndef FIX_0005
#define FIX_0005 FIX_STUFE_A
#endif
#ifndef FIX_0007
#define FIX_0007 FIX_STUFE_W
#endif
#ifndef FIX_0008
#define FIX_0008 FIX_STUFE_A
#endif
#ifndef FIX_0009
#define FIX_0009 FIX_STUFE_A
#endif
#ifndef FIX_0010
#define FIX_0010 FIX_STUFE_A
#endif
#ifndef FIX_0011
#define FIX_0011 FIX_STUFE_W
#endif
#ifndef FIX_0012
#define FIX_0012 FIX_STUFE_A
#endif
#ifndef FIX_0013
#define FIX_0013 FIX_STUFE_A
#endif
#ifndef FIX_0014
#define FIX_0014 FIX_STUFE_A
#endif
#ifndef FIX_0015
#define FIX_0015 FIX_STUFE_W
#endif
#ifndef FIX_0016
#define FIX_0016 FIX_STUFE_A
#endif
#ifndef FIX_0017
#define FIX_0017 FIX_STUFE_A
#endif
#ifndef FIX_0018
#define FIX_0018 FIX_STUFE_W
#endif
#ifndef FIX_0019
#define FIX_0019 FIX_STUFE_A
#endif
#ifndef FIX_0020
#define FIX_0020 FIX_STUFE_A
#endif
#ifndef FIX_0021
#define FIX_0021 FIX_STUFE_W
#endif
#ifndef FIX_0022
#define FIX_0022 FIX_STUFE_W
#endif
#ifndef FIX_0024
#define FIX_0024 FIX_STUFE_A
#endif
#ifndef FIX_0025
#define FIX_0025 FIX_STUFE_W
#endif
#ifndef FIX_0026
#define FIX_0026 FIX_STUFE_A
#endif
#ifndef FIX_0027
#define FIX_0027 FIX_STUFE_A
#endif
#ifndef FIX_0028
#define FIX_0028 FIX_STUFE_A
#endif
#ifndef FIX_0029
#define FIX_0029 FIX_STUFE_AX
#endif
#ifndef FIX_0030
#define FIX_0030 FIX_STUFE_W
#endif
#ifndef FIX_0031
#define FIX_0031 FIX_STUFE_A
#endif
#ifndef FIX_0032
#define FIX_0032 FIX_STUFE_A
#endif
#ifndef FIX_0033
#define FIX_0033 FIX_STUFE_A
#endif
#ifndef FIX_0034
#define FIX_0034 FIX_STUFE_WX
#endif
#ifndef FIX_0035
#define FIX_0035 FIX_STUFE_A
#endif
#ifndef FIX_0036
#define FIX_0036 FIX_STUFE_W
#endif
#ifndef FIX_0037
#define FIX_0037 FIX_STUFE_W
#endif
#ifndef FIX_0038
#define FIX_0038 FIX_STUFE_W
#endif
#ifndef FIX_0039
#define FIX_0039 FIX_STUFE_W
#endif
#ifndef FIX_0040
#define FIX_0040 FIX_STUFE_W
#endif
#ifndef FIX_0041
#define FIX_0041 FIX_STUFE_A
#endif
#ifndef FIX_0042
#define FIX_0042 FIX_STUFE_A
#endif
#ifndef FIX_0043
#define FIX_0043 FIX_STUFE_WX
#endif
#ifndef FIX_0044
#define FIX_0044 FIX_STUFE_A
#endif
#ifndef FIX_0045
#define FIX_0045 FIX_STUFE_W
#endif
#ifndef FIX_0046
#define FIX_0046 FIX_STUFE_A
#endif
#ifndef FIX_0047
#define FIX_0047 FIX_STUFE_A
#endif
#ifndef FIX_0048
#define FIX_0048 FIX_STUFE_W
#endif
#ifndef FIX_0049
#define FIX_0049 FIX_STUFE_W
#endif
#ifndef FIX_0050
#define FIX_0050 FIX_STUFE_W
#endif
#ifndef FIX_0051
#define FIX_0051 FIX_0050	/* dieselbe Stelle */
#endif
#ifndef FIX_0052
#define FIX_0052 FIX_STUFE_W
#endif
#ifndef FIX_0053
#define FIX_0053 FIX_STUFE_W
#endif
#ifndef FIX_0054
#define FIX_0054 FIX_STUFE_W
#endif
#ifndef FIX_0055
#define FIX_0055 FIX_STUFE_W
#endif
#ifndef FIX_0056
#define FIX_0056 FIX_STUFE_A
#endif
#ifndef FIX_0057
#define FIX_0057 FIX_STUFE_A
#endif
#ifndef FIX_0058
#define FIX_0058 FIX_STUFE_W
#endif
#ifndef FIX_0059
#define FIX_0059 FIX_STUFE_WX
#endif
#ifndef FIX_0060
#define FIX_0060 FIX_STUFE_A
#endif
#ifndef FIX_0061
#define FIX_0061 FIX_STUFE_A
#endif
#ifndef FIX_0062
#define FIX_0062 FIX_STUFE_A
#endif
#ifndef FIX_0063
#define FIX_0063 FIX_STUFE_W
#endif
#ifndef FIX_0064
#define FIX_0064 FIX_STUFE_A
#endif
#ifndef FIX_0065
#define FIX_0065 FIX_STUFE_WX
#endif
#ifndef FIX_0066
#define FIX_0066 FIX_STUFE_WX
#endif
#ifndef FIX_0067
#define FIX_0067 FIX_STUFE_WX
#endif
#ifndef FIX_0068
#define FIX_0068 FIX_STUFE_W
#endif
#ifndef FIX_0069
#define FIX_0069 FIX_STUFE_A
#endif
#ifndef FIX_0070
#define FIX_0070 FIX_STUFE_A
#endif
#ifndef FIX_0071
#define FIX_0071 FIX_STUFE_A
#endif
#ifndef FIX_0072
#define FIX_0072 FIX_STUFE_WX
#endif
#ifndef FIX_0073
#define FIX_0073 FIX_STUFE_A
#endif
#ifndef FIX_0074
#define FIX_0074 FIX_STUFE_A
#endif
#ifndef FIX_0076
#define FIX_0076 FIX_STUFE_W
#endif
#ifndef FIX_0077
#define FIX_0077 FIX_STUFE_A
#endif
#ifndef FIX_0078
#define FIX_0078 FIX_STUFE_A
#endif
#ifndef FIX_0079
#define FIX_0079 FIX_STUFE_W
#endif
#ifndef FIX_0080
#define FIX_0080 FIX_STUFE_W
#endif
#ifndef FIX_0081
#define FIX_0081 FIX_STUFE_A
#endif
#ifndef FIX_0082
#define FIX_0082 FIX_STUFE_A
#endif
#ifndef FIX_0083
#define FIX_0083 FIX_STUFE_A
#endif
#ifndef FIX_0084
#define FIX_0084 FIX_STUFE_W
#endif
#ifndef FIX_0085
#define FIX_0085 FIX_STUFE_A
#endif
#ifndef FIX_0086
#define FIX_0086 FIX_STUFE_A
#endif
#ifndef FIX_0087
#define FIX_0087 FIX_STUFE_WX
#endif
#ifndef FIX_0088
#define FIX_0088 FIX_STUFE_W
#endif
#ifndef FIX_0089
#define FIX_0089 FIX_STUFE_A
#endif
#ifndef FIX_0090
#define FIX_0090 FIX_STUFE_WX
#endif
#ifndef FIX_0091
#define FIX_0091 FIX_STUFE_WX
#endif
#ifndef FIX_0092
#define FIX_0092 FIX_STUFE_W
#endif
#ifndef FIX_0093
#define FIX_0093 FIX_STUFE_A
#endif
#ifndef FIX_0094
#define FIX_0094 FIX_STUFE_W
#endif
#ifndef FIX_0095
#define FIX_0095 FIX_STUFE_A
#endif
#ifndef FIX_0096
#define FIX_0096 FIX_STUFE_A
#endif
#ifndef FIX_0097
#define FIX_0097 FIX_STUFE_W
#endif
#ifndef FIX_0098
#define FIX_0098 FIX_STUFE_A
#endif
#ifndef FIX_0099
#define FIX_0099 FIX_STUFE_A
#endif
#ifndef FIX_0100
#define FIX_0100 FIX_STUFE_A
#endif
#ifndef FIX_0101
#define FIX_0101 FIX_STUFE_A
#endif
#ifndef FIX_0102
#define FIX_0102 FIX_STUFE_W
#endif
#ifndef FIX_0104
#define FIX_0104 FIX_STUFE_A
#endif
#ifndef FIX_0105
#define FIX_0105 FIX_STUFE_W
#endif
#ifndef FIX_0106
#define FIX_0106 FIX_STUFE_W
#endif
#ifndef FIX_0107
#define FIX_0107 FIX_STUFE_A
#endif
#ifndef FIX_0108
#define FIX_0108 FIX_STUFE_A
#endif
#ifndef FIX_0109
#define FIX_0109 FIX_STUFE_A
#endif
#ifndef FIX_0110
#define FIX_0110 FIX_STUFE_WX
#endif
#ifndef FIX_0111
#define FIX_0111 FIX_STUFE_W
#endif
#ifndef FIX_0112
#define FIX_0112 FIX_STUFE_A
#endif
#ifndef FIX_0113
#define FIX_0113 FIX_STUFE_A
#endif
#ifndef FIX_0114
#define FIX_0114 FIX_STUFE_W
#endif
#ifndef FIX_0115
#define FIX_0115 FIX_STUFE_A
#endif
#ifndef FIX_0116
#define FIX_0116 FIX_STUFE_A
#endif
#ifndef FIX_0117
#define FIX_0117 FIX_STUFE_WX
#endif
#ifndef FIX_0119
#define FIX_0119 FIX_STUFE_W
#endif
#ifndef FIX_0120
#define FIX_0120 FIX_STUFE_W
#endif
#ifndef FIX_0121
#define FIX_0121 FIX_STUFE_W
#endif
#ifndef FIX_0122
#define FIX_0122 FIX_STUFE_W
#endif
#ifndef FIX_0123
#define FIX_0123 FIX_STUFE_A
#endif
#ifndef FIX_0124
#define FIX_0124 FIX_STUFE_W
#endif
#ifndef FIX_0126
#define FIX_0126 FIX_STUFE_WX
#endif
#ifndef FIX_0127
#define FIX_0127 FIX_STUFE_W
#endif
#ifndef FIX_0128
#define FIX_0128 FIX_STUFE_A
#endif
#ifndef FIX_0129
#define FIX_0129 FIX_STUFE_A
#endif
#ifndef FIX_0130
#define FIX_0130 FIX_STUFE_A
#endif
#ifndef FIX_0131
#define FIX_0131 FIX_STUFE_W
#endif
#ifndef FIX_0132
#define FIX_0132 FIX_STUFE_A
#endif
#ifndef FIX_0133
#define FIX_0133 FIX_STUFE_A
#endif
#ifndef FIX_0134
#define FIX_0134 FIX_STUFE_W
#endif
#ifndef FIX_0135
#define FIX_0135 FIX_STUFE_A
#endif
#ifndef FIX_0136
#define FIX_0136 FIX_STUFE_WX
#endif
#ifndef FIX_0137
#define FIX_0137 FIX_STUFE_W
#endif
#ifndef FIX_0138
#define FIX_0138 FIX_STUFE_A
#endif
#ifndef FIX_0139
#define FIX_0139 FIX_STUFE_W
#endif
#ifndef FIX_0140
#define FIX_0140 FIX_STUFE_W
#endif
#ifndef FIX_0141
#define FIX_0141 FIX_STUFE_A
#endif
#ifndef FIX_0142
#define FIX_0142 FIX_STUFE_WX
#endif
#ifndef FIX_0143
#define FIX_0143 FIX_STUFE_WX
#endif
#ifndef FIX_0145
#define FIX_0145 FIX_STUFE_A
#endif
#ifndef FIX_0146
#define FIX_0146 FIX_STUFE_W
#endif
#ifndef FIX_0147
#define FIX_0147 FIX_STUFE_A
#endif
#ifndef FIX_0148
#define FIX_0148 FIX_STUFE_A
#endif
#ifndef FIX_0149
#define FIX_0149 FIX_STUFE_W
#endif
#ifndef FIX_0150
#define FIX_0150 FIX_STUFE_W
#endif
#ifndef FIX_0151
#define FIX_0151 FIX_STUFE_W
#endif
#ifndef FIX_0152
#define FIX_0152 FIX_STUFE_W
#endif
#ifndef FIX_0153
#define FIX_0153 FIX_STUFE_W
#endif
#ifndef FIX_0155
#define FIX_0155 FIX_STUFE_A
#endif
#ifndef FIX_0156
#define FIX_0156 FIX_STUFE_A
#endif
#ifndef FIX_0157
#define FIX_0157 FIX_STUFE_A
#endif
#ifndef FIX_0158
#define FIX_0158 FIX_STUFE_W
#endif
#ifndef FIX_0159
#define FIX_0159 FIX_STUFE_WX
#endif
#ifndef FIX_0160
#define FIX_0160 FIX_STUFE_A
#endif
#ifndef FIX_0161
#define FIX_0161 FIX_STUFE_W
#endif
#ifndef FIX_0162
#define FIX_0162 FIX_STUFE_WX
#endif
#ifndef FIX_0163
#define FIX_0163 FIX_STUFE_A
#endif
#ifndef FIX_0164
#define FIX_0164 FIX_STUFE_W
#endif
#ifndef FIX_0165
#define FIX_0165 FIX_STUFE_W
#endif
#ifndef FIX_0166
#define FIX_0166 FIX_STUFE_WX
#endif
#ifndef FIX_0167
#define FIX_0167 FIX_STUFE_W
#endif
#ifndef FIX_0168
#define FIX_0168 FIX_STUFE_A
#endif
#ifndef FIX_0169
#define FIX_0169 FIX_STUFE_A
#endif
#ifndef FIX_0170
#define FIX_0170 FIX_STUFE_WX
#endif
#ifndef FIX_0171
#define FIX_0171 FIX_STUFE_A
#endif
#ifndef FIX_0172
#define FIX_0172 FIX_STUFE_W
#endif
#ifndef FIX_0173
#define FIX_0173 FIX_STUFE_A
#endif
#ifndef FIX_0174
#define FIX_0174 FIX_STUFE_W
#endif
#ifndef FIX_0175
#define FIX_0175 FIX_STUFE_A
#endif
#ifndef FIX_0177
#define FIX_0177 FIX_STUFE_W
#endif
#ifndef FIX_0178
#define FIX_0178 FIX_STUFE_W
#endif
#ifndef FIX_0179
#define FIX_0179 FIX_STUFE_W
#endif
#ifndef FIX_0180
#define FIX_0180 FIX_STUFE_W
#endif
#ifndef FIX_0181
#define FIX_0181 FIX_STUFE_W
#endif
#ifndef FIX_0182
#define FIX_0182 FIX_STUFE_A
#endif
#ifndef FIX_0183
#define FIX_0183 FIX_STUFE_A
#endif
#ifndef FIX_0184
#define FIX_0184 FIX_STUFE_W
#endif
#ifndef FIX_0185
#define FIX_0185 FIX_STUFE_W
#endif
#ifndef FIX_0186
#define FIX_0186 FIX_STUFE_W
#endif
#ifndef FIX_0187
#define FIX_0187 FIX_STUFE_A
#endif
#ifndef FIX_0188
#define FIX_0188 FIX_STUFE_A
#endif
#ifndef FIX_0189
#define FIX_0189 FIX_STUFE_W
#endif
#ifndef FIX_0190
#define FIX_0190 FIX_STUFE_W
#endif
#ifndef FIX_0191
#define FIX_0191 FIX_STUFE_WX
#endif
#ifndef FIX_0192
#define FIX_0192 FIX_STUFE_W
#endif
#ifndef FIX_0193
#define FIX_0193 FIX_STUFE_A
#endif
#ifndef FIX_0194
#define FIX_0194 FIX_STUFE_A
#endif
#ifndef FIX_0195
#define FIX_0195 FIX_STUFE_A
#endif
#ifndef FIX_0196
#define FIX_0196 FIX_STUFE_W
#endif
#ifndef FIX_0197
#define FIX_0197 FIX_STUFE_W
#endif
#ifndef FIX_0198
#define FIX_0198 FIX_STUFE_A
#endif
#ifndef FIX_0199
#define FIX_0199 FIX_STUFE_WX
#endif
#ifndef FIX_0200
#define FIX_0200 FIX_STUFE_A
#endif
#ifndef FIX_0201
#define FIX_0201 FIX_STUFE_A
#endif
#ifndef FIX_0202
#define FIX_0202 FIX_STUFE_A
#endif
#ifndef FIX_0203
#define FIX_0203 FIX_STUFE_A
#endif
#ifndef FIX_0204
#define FIX_0204 FIX_STUFE_A
#endif
#ifndef FIX_0205
#define FIX_0205 FIX_STUFE_A
#endif
#ifndef FIX_0206
#define FIX_0206 FIX_STUFE_A
#endif
#ifndef FIX_0207
#define FIX_0207 FIX_STUFE_WX
#endif
#ifndef FIX_0208
#define FIX_0208 FIX_STUFE_A
#endif
#ifndef FIX_0209
#define FIX_0209 FIX_STUFE_A
#endif
#ifndef FIX_0210
#define FIX_0210 FIX_STUFE_A
#endif
#ifndef FIX_0211
#define FIX_0211 FIX_STUFE_W
#endif
#ifndef FIX_0212
#define FIX_0212 FIX_STUFE_W
#endif
#ifndef FIX_0213
#define FIX_0213 FIX_STUFE_W
#endif
#ifndef FIX_0214
#define FIX_0214 FIX_STUFE_W
#endif
#ifndef FIX_0215
#define FIX_0215 FIX_STUFE_A
#endif
#ifndef FIX_0216
#define FIX_0216 FIX_STUFE_A
#endif
#ifndef FIX_0217
#define FIX_0217 FIX_STUFE_A
#endif
#ifndef FIX_0218
#define FIX_0218 FIX_STUFE_W
#endif
#ifndef FIX_0220
#define FIX_0220 FIX_STUFE_W
#endif
#ifndef FIX_0221
#define FIX_0221 FIX_STUFE_W
#endif
#ifndef FIX_0222
#define FIX_0222 FIX_STUFE_WX
#endif
#ifndef FIX_0223
#define FIX_0223 FIX_STUFE_A
#endif
#ifndef FIX_0224
#define FIX_0224 FIX_STUFE_A
#endif
#ifndef FIX_0225
#define FIX_0225 FIX_STUFE_W
#endif
#ifndef FIX_0226
#define FIX_0226 FIX_STUFE_A
#endif
#ifndef FIX_0227
#define FIX_0227 FIX_STUFE_A
#endif
#ifndef FIX_0229
#define FIX_0229 FIX_STUFE_W
#endif
#ifndef FIX_0230
#define FIX_0230 FIX_STUFE_A
#endif
#ifndef FIX_0231
#define FIX_0231 FIX_STUFE_A
#endif
#ifndef FIX_0232
#define FIX_0232 FIX_STUFE_W
#endif
#ifndef FIX_0233
#define FIX_0233 FIX_STUFE_W
#endif
#ifndef FIX_0234
#define FIX_0234 FIX_STUFE_W
#endif
#ifndef FIX_0235
#define FIX_0235 FIX_STUFE_W
#endif
#ifndef FIX_0237
#define FIX_0237 FIX_STUFE_A
#endif
#ifndef FIX_0238
#define FIX_0238 FIX_STUFE_W
#endif
#ifndef FIX_0240
#define FIX_0240 FIX_STUFE_W
#endif
#ifndef FIX_0242
#define FIX_0242 FIX_STUFE_A
#endif
#ifndef FIX_0243
#define FIX_0243 FIX_STUFE_W
#endif
#ifndef FIX_0244
#define FIX_0244 FIX_STUFE_W
#endif
#ifndef FIX_0245
#define FIX_0245 FIX_STUFE_A
#endif
#ifndef FIX_0246
#define FIX_0246 FIX_STUFE_A
#endif
#ifndef FIX_0248
#define FIX_0248 FIX_STUFE_WX
#endif
#ifndef FIX_0249
#define FIX_0249 FIX_STUFE_W
#endif
#ifndef FIX_0250
#define FIX_0250 FIX_STUFE_W
#endif
#ifndef FIX_0251
#define FIX_0251 FIX_STUFE_A
#endif
#ifndef FIX_0252
#define FIX_0252 FIX_STUFE_A
#endif
#ifndef FIX_0253
#define FIX_0253 FIX_STUFE_W
#endif
#ifndef FIX_0254
#define FIX_0254 FIX_STUFE_W
#endif
#ifndef FIX_0255
#define FIX_0255 FIX_STUFE_W
#endif
#ifndef FIX_0256
#define FIX_0256 FIX_STUFE_W
#endif
#ifndef FIX_0258
#define FIX_0258 FIX_STUFE_A
#endif
#ifndef FIX_0259
#define FIX_0259 FIX_STUFE_W
#endif
#ifndef FIX_0260
#define FIX_0260 FIX_STUFE_W
#endif
#ifndef FIX_0261
#define FIX_0261 FIX_STUFE_A
#endif
#ifndef FIX_0262
#define FIX_0262 FIX_STUFE_W
#endif
#ifndef FIX_0263
#define FIX_0263 FIX_STUFE_A
#endif
#ifndef FIX_0264
#define FIX_0264 FIX_STUFE_W
#endif
#ifndef FIX_0265
#define FIX_0265 FIX_STUFE_A
#endif
#ifndef FIX_0266
#define FIX_0266 FIX_STUFE_A
#endif
#ifndef FIX_0268
#define FIX_0268 FIX_STUFE_A
#endif
#ifndef FIX_0269
#define FIX_0269 FIX_STUFE_W
#endif
#ifndef FIX_0270
#define FIX_0270 FIX_STUFE_W
#endif
#ifndef FIX_0271
#define FIX_0271 FIX_STUFE_A
#endif
#ifndef FIX_0273
#define FIX_0273 FIX_STUFE_W
#endif
#ifndef FIX_0274
#define FIX_0274 FIX_STUFE_W
#endif
#ifndef FIX_0275
#define FIX_0275 FIX_STUFE_A
#endif
#ifndef FIX_0276
#define FIX_0276 FIX_STUFE_A
#endif
#ifndef FIX_0277
#define FIX_0277 FIX_STUFE_A
#endif
#ifndef FIX_0278
#define FIX_0278 FIX_STUFE_A
#endif
#ifndef FIX_0279
#define FIX_0279 FIX_STUFE_A
#endif
#ifndef FIX_0280
#define FIX_0280 FIX_STUFE_A
#endif
#ifndef FIX_0281
#define FIX_0281 FIX_STUFE_A
#endif
#ifndef FIX_0282
#define FIX_0282 FIX_STUFE_A
#endif
#ifndef FIX_0284
#define FIX_0284 FIX_STUFE_A
#endif
#ifndef FIX_0285
#define FIX_0285 FIX_STUFE_A
#endif
#ifndef FIX_0286
#define FIX_0286 FIX_STUFE_A
#endif
#ifndef FIX_0287
#define FIX_0287 FIX_STUFE_A
#endif
#ifndef FIX_0288
#define FIX_0288 FIX_STUFE_A
#endif
#ifndef FIX_0289
#define FIX_0289 FIX_STUFE_A
#endif
#ifndef FIX_0291
#define FIX_0291 FIX_STUFE_A
#endif
#ifndef FIX_0292
#define FIX_0292 FIX_STUFE_A
#endif
#ifndef FIX_0293
#define FIX_0293 FIX_STUFE_A
#endif
#ifndef FIX_0294
#define FIX_0294 FIX_STUFE_A
#endif
#ifndef FIX_0295
#define FIX_0295 FIX_STUFE_A
#endif
#ifndef FIX_0296
#define FIX_0296 FIX_STUFE_A
#endif
#ifndef FIX_0297
#define FIX_0297 FIX_STUFE_A
#endif
#ifndef FIX_0298
#define FIX_0298 FIX_STUFE_A
#endif
#ifndef FIX_0299
#define FIX_0299 FIX_STUFE_A
#endif
#ifndef FIX_0300
#define FIX_0300 FIX_STUFE_A
#endif
#ifndef FIX_0302
#define FIX_0302 FIX_STUFE_A
#endif
#ifndef FIX_0303
#define FIX_0303 FIX_STUFE_A
#endif
#ifndef FIX_0304
#define FIX_0304 FIX_STUFE_A
#endif
#ifndef FIX_0305
#define FIX_0305 FIX_STUFE_A
#endif
#ifndef FIX_0306
#define FIX_0306 FIX_STUFE_A
#endif
#ifndef FIX_0308
#define FIX_0308 FIX_STUFE_A
#endif
#ifndef FIX_0309
#define FIX_0309 FIX_STUFE_W
#endif
#ifndef FIX_0310
#define FIX_0310 FIX_STUFE_A
#endif
#ifndef FIX_0311
#define FIX_0311 FIX_STUFE_A
#endif
#ifndef FIX_0312
#define FIX_0312 FIX_STUFE_AX
#endif
#ifndef FIX_0313
#define FIX_0313 FIX_STUFE_W
#endif
#ifndef FIX_0314
#define FIX_0314 FIX_STUFE_W
#endif
#ifndef FIX_0315
#define FIX_0315 FIX_STUFE_A
#endif
#ifndef FIX_0316
#define FIX_0316 FIX_STUFE_W
#endif
#ifndef FIX_0317
#define FIX_0317 FIX_STUFE_A
#endif
#ifndef FIX_0318
#define FIX_0318 FIX_STUFE_A
#endif
#ifndef FIX_0319
#define FIX_0319 FIX_STUFE_W
#endif
#ifndef FIX_0320
#define FIX_0320 FIX_STUFE_A
#endif
#ifndef FIX_0321
#define FIX_0321 FIX_STUFE_A
#endif
#ifndef FIX_0322
#define FIX_0322 FIX_STUFE_WX
#endif
#ifndef FIX_0323
#define FIX_0323 FIX_STUFE_A
#endif
#ifndef FIX_0324
#define FIX_0324 FIX_STUFE_A
#endif
#ifndef FIX_0325
#define FIX_0325 FIX_STUFE_WX
#endif
#ifndef FIX_0326
#define FIX_0326 FIX_STUFE_W
#endif
#ifndef FIX_0327
#define FIX_0327 FIX_STUFE_AX
#endif
#ifndef FIX_0331
#define FIX_0331 FIX_STUFE_AX
#endif
#ifndef FIX_0332
#define FIX_0332 FIX_STUFE_A
#endif
#ifndef FIX_0333
#define FIX_0333 FIX_STUFE_A
#endif
#ifndef FIX_0334
#define FIX_0334 FIX_STUFE_A
#endif
#ifndef FIX_0335
#define FIX_0335 FIX_STUFE_A
#endif
#ifndef FIX_0336
#define FIX_0336 FIX_STUFE_A
#endif
#ifndef FIX_0337
#define FIX_0337 FIX_STUFE_W
#endif
#ifndef FIX_0338
#define FIX_0338 FIX_STUFE_A
#endif
#ifndef FIX_0339
#define FIX_0339 FIX_STUFE_WX
#endif
#ifndef FIX_0340
#define FIX_0340 FIX_STUFE_A
#endif
#ifndef FIX_0341
#define FIX_0341 FIX_STUFE_W
#endif
#ifndef FIX_0342
#define FIX_0342 FIX_STUFE_A
#endif
#ifndef FIX_0343
#define FIX_0343 FIX_STUFE_A
#endif
#ifndef FIX_0345
#define FIX_0345 FIX_STUFE_W
#endif
#ifndef FIX_0346
#define FIX_0346 FIX_STUFE_W
#endif
#ifndef FIX_0347
#define FIX_0347 FIX_STUFE_A
#endif
#ifndef FIX_0348
#define FIX_0348 FIX_STUFE_W
#endif
#ifndef FIX_0349
#define FIX_0349 FIX_STUFE_WX
#endif
#ifndef FIX_0350
#define FIX_0350 FIX_STUFE_WX
#endif
#ifndef FIX_0351
#define FIX_0351 FIX_STUFE_W
#endif
#ifndef FIX_0352
#define FIX_0352 FIX_STUFE_A
#endif
#ifndef FIX_0353
#define FIX_0353 FIX_STUFE_A
#endif
#ifndef FIX_0354
#define FIX_0354 FIX_STUFE_W
#endif
#ifndef FIX_0355
#define FIX_0355 FIX_STUFE_W
#endif
#ifndef FIX_0356
#define FIX_0356 FIX_STUFE_A
#endif
#ifndef FIX_0357
#define FIX_0357 FIX_STUFE_W
#endif
#ifndef FIX_0358
#define FIX_0358 FIX_STUFE_AX
#endif
#ifndef FIX_0359
#define FIX_0359 FIX_STUFE_WX
#endif
#ifndef FIX_0360
#define FIX_0360 FIX_STUFE_A
#endif
#ifndef FIX_0361
#define FIX_0361 FIX_STUFE_AX
#endif
#ifndef FIX_0362
#define FIX_0362 FIX_STUFE_WX
#endif
#ifndef FIX_0365
#define FIX_0365 FIX_STUFE_W
#endif
#ifndef FIX_0366
#define FIX_0366 FIX_STUFE_WX
#endif
#ifndef FIX_0367
#define FIX_0367 FIX_STUFE_WX
#endif
#ifndef FIX_0368
#define FIX_0368 FIX_STUFE_W
#endif
#ifndef FIX_0369
#define FIX_0369 FIX_STUFE_A
#endif
#ifndef FIX_0370
#define FIX_0370 FIX_STUFE_W
#endif
#ifndef FIX_0371
#define FIX_0371 FIX_STUFE_A
#endif
#ifndef FIX_0372
#define FIX_0372 FIX_STUFE_WX
#endif
#ifndef FIX_0373
#define FIX_0373 FIX_STUFE_A
#endif
#ifndef FIX_0374
#define FIX_0374 FIX_STUFE_AX
#endif
#ifndef FIX_0375
#define FIX_0375 FIX_STUFE_AX
#endif
#ifndef FIX_0379
#define FIX_0379 FIX_STUFE_W
#endif
#ifndef FIX_0380
#define FIX_0380 FIX_STUFE_W
#endif
#ifndef FIX_0381
#define FIX_0381 FIX_STUFE_A
#endif
#ifndef FIX_0382
#define FIX_0382 FIX_STUFE_W
#endif
#ifndef FIX_0383
#define FIX_0383 FIX_STUFE_W
#endif
#ifndef FIX_0384
#define FIX_0384 FIX_STUFE_AX
#endif
#ifndef FIX_0385
#define FIX_0385 FIX_STUFE_W
#endif
#ifndef FIX_0386
#define FIX_0386 FIX_STUFE_A
#endif
#ifndef FIX_0390
#define FIX_0390 FIX_STUFE_A
#endif
#ifndef FIX_0391
#define FIX_0391 FIX_STUFE_A
#endif
#ifndef FIX_0392
#define FIX_0392 FIX_STUFE_A
#endif
#ifndef FIX_0393
#define FIX_0393 FIX_STUFE_W
#endif
#ifndef FIX_0394
#define FIX_0394 FIX_STUFE_A
#endif
#ifndef FIX_0395
#define FIX_0395 FIX_STUFE_A
#endif
#ifndef FIX_0396
#define FIX_0396 FIX_STUFE_A
#endif
#ifndef FIX_0397
#define FIX_0397 FIX_STUFE_AX
#endif
#ifndef FIX_0398
#define FIX_0398 FIX_STUFE_A
#endif
#ifndef FIX_0399
#define FIX_0399 FIX_STUFE_A
#endif
#ifndef FIX_0400
#define FIX_0400 FIX_STUFE_A
#endif
#ifndef FIX_0401
#define FIX_0401 FIX_STUFE_W
#endif
#ifndef FIX_0402
#define FIX_0402 FIX_STUFE_WX
#endif
#ifndef FIX_0403
#define FIX_0403 FIX_STUFE_A
#endif
#ifndef FIX_0404
#define FIX_0404 FIX_STUFE_WX
#endif
#ifndef FIX_0405
#define FIX_0405 FIX_STUFE_A
#endif
#ifndef FIX_0406
#define FIX_0406 FIX_STUFE_W
#endif
#ifndef FIX_0407
#define FIX_0407 FIX_STUFE_A
#endif
#ifndef FIX_0409
#define FIX_0409 FIX_STUFE_A
#endif
#ifndef FIX_0410
#define FIX_0410 FIX_STUFE_A
#endif
#ifndef FIX_0411
#define FIX_0411 FIX_STUFE_A
#endif
#ifndef FIX_0412
#define FIX_0412 FIX_STUFE_A
#endif
#ifndef FIX_0413
#define FIX_0413 FIX_STUFE_A
#endif
#ifndef FIX_0414
#define FIX_0414 FIX_STUFE_W
#endif
#ifndef FIX_0415
#define FIX_0415 FIX_STUFE_A
#endif
#ifndef FIX_0416
#define FIX_0416 FIX_STUFE_A
#endif
#ifndef FIX_0418
#define FIX_0418 FIX_STUFE_A
#endif
#ifndef FIX_0425
#define FIX_0425 FIX_STUFE_A
#endif
#ifndef FIX_0426
#define FIX_0426 FIX_STUFE_W
#endif
#ifndef FIX_0432
#define FIX_0432 FIX_STUFE_WX
#endif
#ifndef FIX_0433
#define FIX_0433 FIX_STUFE_W
#endif
#ifndef FIX_0434
#define FIX_0434 FIX_STUFE_A
#endif
#ifndef FIX_0435
#define FIX_0435 FIX_STUFE_A
#endif
#ifndef FIX_0436
#define FIX_0436 FIX_STUFE_A
#endif

/* SEG002 ist resident: ohne Platz fuer die Behebungen entfallen dort nie aufgerufene Funktionen */
#ifndef FIX_PLATZ_SEG002
#define FIX_PLATZ_SEG002 (FIX_0039 || FIX_0079 || FIX_0080 || FIX_0127 || FIX_0128 || FIX_0229 || FIX_0230 || FIX_0231 || FIX_0233 || FIX_0234 || FIX_0235 || FIX_0280 || FIX_0403 || FIX_0407 || FIX_PROBENLOG)
#endif

#endif
