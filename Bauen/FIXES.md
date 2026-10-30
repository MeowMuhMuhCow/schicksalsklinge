# Fehlerbehebungen per Schalter (FIXES) - Die Schicksalsklinge

**ALLE BEHEBUNGEN SIND IM SPIEL UNGETESTET.** Sie sind am Quelltext entworfen, uebersetzt, statisch geprueft (Datensegment, Lage der residenten Segmente, Overlay-Einspruenge: `Werkzeuge/fixcheck.py`) und starten in DOSBox, sind aber noch in keinem Spiel ausprobiert worden. **Tester willkommen** - Rueckmeldungen bitte mit Nummer.

Quelle der Tabelle: `Werkzeuge/fix_schalter.tsv`, erzeugt von `Werkzeuge/mkfix.py` aus `Analyse/Fehler/Fehler_Raster.tsv`, `Werkzeuge/fix_status.tsv` und den Kommentaren `/* Fix n: ... */` im Quelltext.

**Stufen:** `FIX_BUGS=1` ("wichtig") schaltet alle Behebungen der Kategorien *kritisch* und *mittel* ein, `FIX_BUGS=2` ("alles") zusaetzlich die Kategorie *egal*. `FIX_EXPLOITS=0` laesst die als Exploit markierten Fehler (die dem Spieler nuetzen) im Spiel; Vorgabe ist `FIX_EXPLOITS=1`, sobald `FIX_BUGS > 0`. Jeder Schalter `FIX_nnnn` laesst sich einzeln uebersteuern (`-DFIX_0040=0`). `FIX_BUGS=0` (Vorgabe) ergibt das bytegleiche Original (BLADEM.EXE CD 3.02 de, MD5 4ce035f95ead01a8b2553b4487544e5d).

**Bauen:** `Bauen\BAUEN.bat wichtig` bzw. `Bauen\BAUEN.bat alles` -> `Build\BLADEM_FIX_WICHTIG.EXE` bzw. `Build\BLADEM_FIX_ALLES.EXE`; starten mit `BLADEM_FIX_WICHTIG_STARTEN.bat` / `BLADEM_FIX_ALLES_STARTEN.bat` (eigener Ordner, eigene Testspielstaende).

**Grenzen:** Keine Behebung aendert Strukturen, das Spielstandformat, das Datensegment, die Spieldaten oder die Lage der residenten Segmente. Fehler, die das braeuchten, und Fehler mit unklarer Absicht (Balance) sind mit Grund als "nein" gefuehrt. Der Kopierschutz ist unberuehrt.

**Spalten:** Stufe = ab welcher Stufe der Schalter an ist; umgesetzt = ja / teilweise / nein; "Aenderung / Grund" nennt die Kommentare an den `#if FIX_nnnn`-Stellen bzw. warum nicht oder nur teilweise behoben wurde.

## Uebersicht

| Stufe | ja | teilweise | nein | kein Fehler |
|---|---|---|---|---|
| wichtig (kritisch + mittel) | 124 | 5 | 44 | - |
| alles (zusaetzlich egal) | 118 | 5 | 85 | - |
| - (laut Raster kein Fehler) | - | - | - | 36 |

Nr. 51 ist dieselbe Stelle wie Nr. 50 (`FIX_0051` folgt `FIX_0050`). In den residenten Modulen (seg001..seg007) darf kein Segment laenger werden; in seg002 entfallen dafuer mit den Behebungen nie aufgerufene Funktionen (Schalter `FIX_PLATZ_SEG002`, aus den Schaltern des Moduls abgeleitet).

## Tabelle

| Nr | Titel | Kategorie | Exploit | Stufe | umgesetzt | Aenderung / Grund | Dateien |
|---|---|---|---|---|---|---|---|
| 1 | Speer wird als Fernkampfwaffe behandelt | mittel | nein | wichtig | nein | unsicher (Wurfspeer moeglich); braucht Aenderungen der Waffentabelle im Datensegment |  |
| 2 | Wurfwaffe Schneidzahn hat keine Entfernungstabelle | mittel | nein | wichtig | nein | braucht eine Fernkampftabelle im Datensegment |  |
| 3 | MR von 19 Gegnertypen wird mit 15W4 statt 1W6 ausgewürfelt | mittel | nein | wichtig | nein | Datenfehler in MONSTER.DAT (Spieldaten) |  |
| 4 | Doppelgegner überschreibt beim Zug über eine Leiche deren Feldinhalt | egal | nein | alles | nein | residentes Modul seg005 hat keinen Platz (die Behebung wuerde das Segment um 6 Byte verlaengern, dort gibt es keinen toten Code zum Ausgleich) |  |
| 5 | Schwanzteil fliehender oder selbst getöteter Doppelgegner bleibt auf dem Feld | egal | nein | alles | nein | residentes Modul seg005 hat keinen Platz (+68 Byte, kein toter Code zum Ausgleich); der Teil fuer den Patzer-Tod steht unter Nr. 8 (seg032) |  |
| 6 | „Orig_BUG: I had this loop running infinitely“ | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 7 | Heldenzug ohne verbliebene Heldenphasen: Endlosschleife möglich | kritisch | nein | wichtig | ja | erst auf 0 Heldenphasen pruefen und den Zug an die Gegner geben (wie im Gegnerzweig) | SEG032.C |
| 8 | Schwanzteil eines durch eigenen Patzer getöteten Doppelgegners bleibt stehen | egal | nein | alles | ja | Schwanzteil eines durch eigenen Patzer getoeteten Doppelgegners vom Schachbrett nehmen | SEG032.C |
| 9 | Abfrage g_max_enemies != 0 nach dem Sieg ist nie wahr | egal | nein | alles | ja | Kuerzung merken, die Abfrage nach dem Sieg braucht sie; gemerkte Kuerzung pruefen (g_max_enemies ist hier schon 0) | SEG032.C |
| 10 | Fluchtverteilung übergeht den NSC auf Platz 6 | egal | nein | alles | nein | Mit allen 7 Plaetzen koennte der NSC allein durch einen anderen Ausgang fliehen und eine reine NSC-Gruppe bilden - die kennt das Spiel nicht (siehe Nr. 80, 243); ohne Fix bleibt der NSC bei der Restgruppe, folgenlos |  |
| 11 | Zauberziel: begehbare Objekte (Id ≥ 50) werden nicht abgewiesen | mittel | nein | wichtig | ja | begehbare Objekte (Id >= 50) wie im Angriffszweig abweisen | SEG033.C |
| 12 | Flucht über den oberen und unteren Rand kostet ein BP mehr | egal | nein | alles | ja | wie am linken Rand reicht ein BP mehr als der Weg | SEG034.C |
| 13 | Fluchtfeld wird mit dem Inhalt einer früheren Cursorposition geprüft | egal | nein | alles | ja | im Fluchtfall den gerade gelesenen Inhalt des Randfelds pruefen | SEG034.C |
| 14 | Verschachtelte ifs: tote Figur auf dem Zielfeld überspringt die Weg- und BP-Prüfung | egal | nein | alles | ja | if-Kette flach (Bright-Eyes-Fix), damit Weg und BP auch bei Leichen geprueft werden | SEG034.C |
| 15 | Bei genau 30 Beutestücken schreibt „MEHR“ über einen nicht gesetzten Zeiger | kritisch | nein | wichtig | ja | auch loot_names[30] belegen, dorthin schreibt "MEHR" auf Seite 2 bei genau 30 Stueck | SEG035.C |
| 16 | Knochen mit Rune beendet die Beuteliste | egal | nein | alles | ja | den Knochen mit Rune aus der Beuteliste nehmen, statt die Liste an ihm abzubrechen | SEG035.C |
| 17 | Meldung „ZIEL IST NICHT MEHR IN GÜLTIGER SCHUSSPOSITION“ kann doppelt kommen | egal | nein | alles | ja | Schusslinie nur pruefen, wenn die Meldung nicht schon kam | SEG035.C |
| 18 | Computerkampf: Helden greifen auch übergelaufene Gegner an | mittel | nein | wichtig | ja | uebergelaufene Gegner (Boeser Blick) kaempfen auf der Seite der Helden; uebergelaufene Gegner nicht angreifen; ein uebergelaufener Gegner steht in der Schusslinie wie ein Held | SEG036.C |
| 19 | Fluchtfelder (negative Ids) werden als Held gelesen | egal | nein | alles | ja | Fluchtfelder (negative Ids) sind kein Held, sie beenden die Suche; Fluchtfelder (negative Ids) sind kein Held (Bright-Eyes-Fix) | SEG036.C SEG037.C |
| 20 | Umzingelt-Prüfung mit Grenze 25 statt 23 | egal | nein | alles | ja | Grenze 23 (das Feld ist 0..23) | SEG036.C |
| 21 | Gegner schießen auch auf direkt benachbarte Helden | mittel | nein | wichtig | nein | Absicht unklar (unsicher): ob Gegner im Handgemenge nicht schiessen duerfen, ist eine Regelfrage |  |
| 22 | Übergelaufener Priester (Stufe 2) hängt das Spiel auf | kritisch | nein | wichtig | ja | ohne einen Zauber, den dieser Gegner auf seiner Seite einsetzen kann, gar nicht erst suchen | SEG037.C |
| 23 | „Why unset 'tied' and not 'illusion'?“ | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 24 | best_viewdir möglicherweise uninitialisiert | egal | nein | alles | ja | best_viewdir vorbelegen (Bright-Eyes-Fix) | SEG038.C |
| 25 | Fliehender Doppelgegner: Wegsuche hängt | kritisch | nein | wichtig | ja | beim Fluchtziel im ersten Rueckschritt keinen Platz fuer den Schwanz verlangen, die | SEG038.C |
| 26 | memcpy 20 Byte in g_fig_move_pathdir[10] | egal | nein | alles | nein | das Zielfeld liegt im Datensegment (darf nicht wachsen); die 10 Byte dahinter (g_fig_enemy_parry_action_used[0..9]) werden nie benutzt, lange Wege brauchen die 20 Byte - folgenlos |  |
| 27 | Schwanzteile toter Doppelgegner sind nicht begehbar | egal | nein | alles | ja | tote Schwanzteile begehbar machen (Bright-Eyes-Fix) | SEG038.C |
| 28 | Grenzprüfung: new_y > 23 doppelt, new_x > 23 fehlt | egal | nein | alles | ja | rechten Rand pruefen (Bright-Eyes-Fix) | SEG038.C |
| 29 | Geworfene Waffen werden nur als Gegenstandsnummer gemerkt | egal | ja | alles | nein | braucht Platz fuer ganze Inventareintraege und Besitzer je Wurf - neue Daten im Datensegment |  |
| 30 | Angstgift: Schreiben über nicht gesetzten Zeiger bei Held gegen Held | kritisch | nein | wichtig | ja | Angstgift wirkt nur auf Gegner, beim Helden ist enemy_p nicht gesetzt | SEG041.C |
| 31 | Held gegen Held: Werte des Verteidigers falsch berechnet | egal | nein | alles | ja | Ruestungsbehinderung des Angegriffenen, nicht des Angreifers; PA-Zuschlag der Waffe (wie in seg043 und WERTE PRUEFEN) | SEG042.C |
| 32 | Paradepatzer setzt backfire_1 statt backfire_2 | egal | nein | alles | ja | den Freischlag fuehrt der Angreifer aus, also ihn animieren | SEG042.C |
| 33 | Waffenbruch bei gleichen Würfen trifft nie die Waffe eines Gegners | egal | nein | alles | nein | Designfrage (unsicher): Gegner haben keinen Bruchfaktor, vermutlich bewusst |  |
| 34 | Wurfwaffe wird vor der Schadensberechnung abgelegt | mittel | ja | wichtig | ja | Wurfwaffe erst nach der Schadensberechnung ablegen (Bright-Eyes-Fix, Original-Bug 32) | SEG042.C |
| 35 | double_size bleibt beim Rückverwandeln eines Doppelgegners ungesetzt | egal | nein | alles | nein | ohne Wirkung: FIG_remove_from_list(..., 1) laesst die alten Werte des Gegners in g_fig_list_elem stehen, double_size ist also schon richtig |  |
| 36 | Gegner-Balsam heilt über das Maximum oder gar nicht | mittel | nein | wichtig | ja | halben Verlust, mindestens 7, hoechstens bis zum Maximum heilen (Bright-Eyes-Fix) | SEG102.C |
| 37 | Gegner-Plumbumbarum wirkt eine Stunde statt 5 Kampfrunden | mittel | nein | wichtig | nein | Absicht unklar (unsicher): ob der Gegnerzauber laenger wirken soll als der Heldenzauber, ist nicht belegbar |  |
| 38 | Endkampf: ohne Grimring kämpft Heldenplatz 0 | mittel | nein | wichtig | ja | dann den ersten handlungsfaehigen Helden der Gruppe nehmen | SEG116.C |
| 39 | Akademie identifiziert und bricht Flüche nur ein einziges Mal im Spiel | mittel | nein | wichtig | ja | Akademie identifiziert und bricht Flueche am naechsten Tag wieder | SEG002.C |
| 40 | Einbruch: „noch jemand frei?“ zählt die gerade erwischte Gruppe mit | kritisch | nein | wichtig | ja | nur Helden anderer Gruppen zaehlen, die aktive wird ja gerade verhaftet | SEG025.C |
| 41 | Verhaftung nur ab Platz 0, solange die Plätze zur Gruppe gehören | egal | nein | alles | ja | alle 7 Plaetze der aktiven Gruppe verhaften (auch den NSC) | SEG025.C |
| 42 | Fehlende Haustypen 1 und 2: arr[1] bleibt ungesetzt | egal | nein | alles | nein | ohne Wirkung: keine Stadt hat die Kombination (Haustyp 1 und 2 fehlen) |  |
| 43 | Statusleiste setzt 6 statt 4 Werte je Held, beim NSC wird die Krankheitstabelle überschrieben | mittel | ja | wichtig | ja | das Feld hat 4 Werte je Held (sonst wird beim NSC g_disease_strength ueberschrieben) | SEG029.C |
| 44 | Antworten werden gemischt, das Ergebnis aber nie benutzt | egal | nein | alles | nein | Absicht unklar: die gemischten Antworten zu benutzen waere eine sichtbare Spielaenderung, das Mischen zu streichen aendert nichts Sichtbares |  |
| 45 | Nicht vorhandene Antworten werden mitgemischt | mittel | nein | wichtig | ja | fehlende Antworten auf txt_offset setzen, dann schuetzt sie die Pruefung beim Mischen | SEG031.C |
| 46 | Tempel: bei 20, 30, … Helden eine leere Seite | egal | nein | alles | ja | auch bei genau 20, 30, ... Helden zurueck zur ersten Seite | SEG047.C |
| 47 | leave_location() dreht die Gruppe auch dort um, wo sie nicht umdrehen soll | egal | nein | alles | teilweise | Drehung nach dem Lager zuruecknehmen (Bright-Eyes-Fix zu Original-Bug 26); Drehung von leave_location() zuruecknehmen (Bright-Eyes-Fix zu Original-Bug 27) - nach dem Lager in Stadt und Dungeon und nach dem Strassenhaendler wird die Drehung zurueckgenommen (Bright-Eyes-Fix); die mehrfachen Aufrufe in Haeusern mit Taverne und Herberge (THO_tav_inn_combi, do_special_buildings) bleiben, dort haengt die Richtung vom Ablauf ab | SEG052.C SEG067.C |
| 48 | Brauender NSC bleibt in der Herberge stecken | mittel | nein | wichtig | ja | auch den NSC (Platz 6) finden (Bright-Eyes-Fix) | SEG054.C |
| 49 | NSC kann in der Herberge nicht zaubern, magic_act[6] schreibt hinter das Feld | mittel | nein | wichtig | ja | Platz fuer den NSC (Bright-Eyes-Fix) | SEG054.C |
| 50 | Brauende Helden: Einzelheld sieht sie nicht, nur der erste wird gefragt | mittel | nein | wichtig | ja | eine Gruppe mit der aktiven vereinen (der Rumpf von GRP_merge() fuer eine einzelne Gruppe); jeden brauenden Helden im Haus einzeln behandeln, auch beim Einzelhelden; nur die | SEG054.C |
| 51 | Nach dem Abholen gelten zusammengeführte Brauer weiter als „brauend“ | mittel | nein | wichtig | ja | eine Gruppe mit der aktiven vereinen (der Rumpf von GRP_merge() fuer eine einzelne Gruppe); jeden brauenden Helden im Haus einzeln behandeln, auch beim Einzelhelden; nur die (dieselbe Stelle wie Nr. 50 (FIX_0051 folgt FIX_0050)) | SEG054.C |
| 52 | Herberge: Preisaufschlag für Einzelzimmer und Suite vom Schlafsaalpreis berechnet | mittel | nein | wichtig | ja | Aufschlag vom eigenen Preis | SEG054.C |
| 53 | „Besonders guter Preis“ beim Händler verschlechtert den Verkauf | mittel | nein | wichtig | teilweise | der "besonders gute Preis" darf den Verkauf nicht verschlechtern; gelungenes Feilschen verschlechtert den Verkauf nicht mehr - der gute Preis verschlechtert den Verkauf nicht mehr (Haendler setzen g_price_modificator nicht mehr auf 3); welche Verguenstigung gemeint war (Rabatt beim Kauf oder mehr Erloes), ist nicht belegbar (Spielbalance) | SEG055.C |
| 54 | Einkaufskorb: Platzbedarf falsch berechnet | mittel | nein | wichtig | ja | Stapelbarkeit des Korbeintrags, 99 Stueck je Platz; die zusaetzlich gekaufte Ware mitzaehlen; die neue Ware mitzaehlen | SEG056.C |
| 55 | Volle Korbsumme wird bezahlt, auch wenn nicht alles verteilt werden konnte | mittel | nein | wichtig | nein | unsicher: Meldung 566 ("DAS NAECHSTE MAL SOLLTET IHR VORHER ETWAS PLATZ ...") deutet auf Absicht; mit Nr. 54 passt der Korb jetzt zum freien Platz |  |
| 56 | Schmied zeigt das Geld des Helden, bezahlt wird aus der Gruppenkasse | egal | nein | alles | ja | das Geld der Gruppe zeigen, daraus wird bezahlt | SEG058.C |
| 57 | Bei genau passendem Geld Feilschprobe für 0 % | egal | nein | alles | ja | auch bei genau passendem Geld | SEG058.C |
| 58 | Tavernenmerker firstinfo, infopromise, round werden nie zurückgesetzt | mittel | nein | wichtig | nein | unsicher: die Wirkung der Merker auf den Dialog ist nicht geprueft; sie stehen im Spielstand und koennen gewollt dauerhaft sein |  |
| 59 | Taverne im Gespräch: Runde und Essen werden in Heller statt Silber abgezogen | mittel | ja | wichtig | ja | der Preis ist in Silbertalern genannt, das Geld wird in Hellern gefuehrt; Silberstuecke wie in do_tavern(), also mal 10 Heller | SEG060.C |
| 60 | Trinkprobe über Plätze 0..n−1 statt über die Gruppenmitglieder | egal | nein | alles | ja | alle 7 Plaetze durchgehen, Zustand 109 prueft die Gruppe; nur Helden der aktiven Gruppe | SEG060.C |
| 61 | Praios-Wunder: Prüfung auf Zauberkundige wirkungslos | egal | nein | alles | ja | alle Helden der Gruppe pruefen, ein Zauberkundiger verhindert das Praios-Wunder (Bright-Eyes-Fix) | SEG062.C |
| 62 | Praios und Hesinde heben Verwandlungen auch bei fremden Gruppen auf | egal | nein | alles | ja | nur Helden der aktiven Gruppe ohne Goetterzorn (wie im renegade-Zweig) | SEG062.C |
| 63 | Firun-Wunder „keinen Hunger und Durst mehr“ wirkt nur auf einen Helden | mittel | nein | wichtig | ja | alle Helden der Gruppe sind satt (der Text spricht alle an) | SEG062.C |
| 64 | Taschendieb: falsche Textteile, das Ende „Dieb entkommt“ erscheint nie | egal | nein | alles | ja | Texte 24, 25, 29, 30 (26..28 sind die dreiteilige Geschichte oben) | SEG067.C |
| 65 | Straßenhändler überschreibt den Typ des gerade betretenen Gebäudes | mittel | ja | wichtig | ja | Gebaeudetyp sichern (Bright-Eyes-Fix, Original-Bug 24); Gebaeudetyp zurueckschreiben | SEG067.C |
| 66 | Zeughaus: Störrebrandt-Schulden werden aus dem Staatsbudget bezahlt | mittel | ja | wichtig | teilweise | im Zeughaus wird mit dem Budget des Hetmanns eingekauft, keine Schuldenabfrage - im Zeughaus keine Schuldenabfrage mehr (das Budget des Hetmanns zahlt keine Stoerrebrandt-Schulden); dass der Rest des Budgets auf ganze Dukaten abgerundet wird, bleibt (gs_arsenal_money steht in Dukaten im Spielstand) | SEG055.C |
| 67 | Zwingfeste: bei jedem Betreten 20 Dukaten | kritisch | ja | wichtig | ja | das Geld gibt es nur einmal (Bright-Eyes-Fix); der Zugang haengt weiter an !gs_dng14_ugdalf_done | SEG068.C |
| 68 | Akademie erkennt einen Fluch beim Helden auf Platz 0 nicht | mittel | nein | wichtig | ja | -1 = niemand verflucht, damit auch Platz 0 erkannt wird | SEG068.C |
| 69 | Fluchbrechung: Text 63 erscheint mit „%s“ | egal | nein | alles | ja | den Namen des verfluchten Helden einsetzen | SEG068.C |
| 70 | Göttergunst über Index 0..13 statt 1..14 | egal | nein | alles | ja | Goetter 1..14 (Praios bis Ifirn) | SEG069.C SEG082.C SEG084.C |
| 71 | ESC ruft GUI_radio() mit −1 Optionen auf | egal | nein | alles | ja | eigene Variable fuer die Antwort, answer bleibt die Zahl der Optionen (Bright-Eyes-Fix) | SEG070.C |
| 72 | Spielhaus: Gewinn und Verlust vertauscht | mittel | ja | wichtig | ja | verloren wird, wenn hoechstens die Haelfte beim Falschspiel Erfolg hatte (Bright-Eyes-Fix) | SEG070.C |
| 73 | Spielhaus: „nach einigen Stunden“, aber die Zeit steht | egal | nein | alles | nein | wie viele Stunden "einige" sein sollen, ist nicht belegbar |  |
| 74 | Phextempel: gs_ptemple_closed wird nie gesetzt | egal | nein | alles | nein | unsicher: ob die Geweihte nach der Beleidigung fuer immer verschwinden und der Tempel geschlossen bleiben soll, ist nicht belegbar |  |
| 75 | „this would reactivate algrid“ | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 76 | Informanten werden von „erledigt“ (2) auf „bekannt“ (1) zurückgesetzt | mittel | nein | wichtig | ja | einen Informanten nur von 0 (unbekannt) auf 1 (bekannt) setzen, nicht von 2 (erledigt) zurueck | V302DE.H |
| 77 | Rückgabewert bleibt oft uninitialisiert | egal | nein | alles | ja | vorbelegen, sonst wird ein Stapelwert zurueckgegeben | SEG073.C |
| 78 | Harikas Feilschbonus auch, wenn sie nicht dabei ist | egal | nein | alles | ja | Harika hilft nur, wenn sie in der aktiven Gruppe und handlungsfaehig ist | SEG103.C |
| 79 | Stündliche Sondertimer laufen bei Zeitsprüngen nicht | mittel | nein | wichtig | ja | die stuendlichen Sondertimer aus do_timers() (Einhorn, Sphaerenriss, Orkmist-Faesser),; stuendliche Sondertimer mitzaehlen | SEG002.C |
| 80 | Nur noch der NSC handlungsfähig: das Spiel läuft weiter | kritisch | nein | wichtig | ja | die Pruefung von game_loop() anstossen, die den NSC allein nicht als handlungsfaehige; die Spielschleife prueft die Gruppen (der NSC allein zaehlt dort nicht); der NSC allein versucht es nicht noch einmal; die Spielschleife prueft die Gruppe | SEG002.C SEG113.C SEG116.C |
| 81 | KK-Probe zählt Platz 0 ungeprüft | egal | nein | alles | ja | die ersten beiden handlungsfaehigen Helden der aktiven Gruppe | SEG047.C |
| 82 | Gelber Pfeil der aktiven Gruppe wird vom violetten verdeckt | egal | nein | alles | ja | gelben Pfeil zuletzt zeichnen (Bright-Eyes-Fix zu Original-Bug 31) | SEG074.C |
| 83 | Sichtprüfung <= 16 lässt eine 17. Spalte neben der Karte zu | egal | nein | alles | ja | sichtbar sind die Spalten 0..15 | SEG074.C |
| 84 | Todesfalle (Totenschiff, Spinnenhöhle) beendet das Spiel auch für Gruppen draußen | mittel | nein | wichtig | ja | nur die Helden der Gruppe im Dungeon kommen um (wie beim verlorenen Kampf auf Reisen: | SEG075.C |
| 85 | Kletter-Meldung nennt Plätze 0, 1, 2 … ohne Gruppenprüfung | egal | nein | alles | ja | nur die handlungsfaehigen Helden der aktiven Gruppe aufzaehlen (wie count_heroes_available_in_group()) | SEG075.C |
| 86 | Proben und Hilfen zählen versteinerte und bewusstlose Helden mit (nur !dead geprüft) | egal | nein | alles | ja | nur handlungsfaehige Helden (nicht versteinert, bewusstlos ...); nur ein handlungsfaehiger Magier hilft mit dem Stab; nur handlungsfaehige Helden; nur handlungsfaehige Zauberkundige; versteinerte, bewusstlose usw. Helden kaempfen nicht mit | SEG025.C SEG075.C SEG081.C SEG098.C SEG103.C SEG117.C |
| 87 | Dietrich-Zähler wird nach dem ersten Ebenenwechsel ständig zurückgesetzt | mittel | ja | wichtig | ja | den Ebenenwechsel nur einmal auswerten | SEG076.C |
| 88 | Klammerfehler: Feld (1,8,5) zeigt in jedem Dungeon den Spiegeltext | mittel | nein | wichtig | ja | beide Felder nur in Hyggeliks Ruine | SEG076.C |
| 89 | Symbol „Gruppen vereinen“ bleibt nach dem Vereinen aktiv | egal | nein | alles | ja | Symbol ausgrauen (Bright-Eyes-Fix zu Original-Bug 19) | SEG076.C |
| 90 | Rückwärtsschritt prüft halbdurchlässige Wände nicht | mittel | ja | wichtig | ja | rueckwaerts gilt das Richtungsflag der Gegenrichtung (wie vorwaerts) | SEG076.C |
| 91 | Leeres Wasserfass füllt trotzdem alle Schläuche | mittel | ja | wichtig | ja | erst nachsehen, was im Fass ist: aus einem leeren Fass nichts fuellen, | SEG076.C |
| 92 | Totenschiff-Spalt: nur Seil oder Strickleiter, kein Magierstab | mittel | nein | wichtig | nein | Designluecke (unsicher): ob der verwandelte Magierstab auch am Spalt helfen soll, ist nicht belegbar |  |
| 93 | Proben und Wirkungen mit get_hero(0) statt dem ersten handlungsfähigen Helden | egal | nein | alles | ja | der erste handlungsfaehige Held klettert; der erste handlungsfaehige Held (Bright-Eyes-Fix zu Original-Bug 66); der erste handlungsfaehige Held, mit seiner Position (vgl. Nr. 254) | SEG077.C SEG078.C SEG080.C |
| 94 | Totenschiff: Deckenloch und Grube mit Zeilenbreite 32 statt 16 gesetzt | mittel | nein | wichtig | ja | Dungeonkarten sind 16 Felder breit | SEG077.C |
| 95 | Totenschiff: Meldung vom sinkenden Schiff kommt nie | egal | nein | alles | ja | leave_dungeon() hat gs_deathtrap schon geloescht (Bright-Eyes-Fix zu Original-Bug 30) | SEG077.C |
| 96 | sprintf mit der Zahl 10 statt eines Namens | egal | nein | alles | ja | den Namen statt der Zahl 10 einsetzen (Bright-Eyes-Fix) | SEG077.C |
| 97 | Rätseltruhe: 10 AP Zusatzbelohnung werden nie vergeben | mittel | nein | wichtig | ja | den Goldschluessel nur einmal (die Truhe gilt nach Fix 97 als offen); die Zusatzbelohnung (10 AP) freischalten (Bright-Eyes-Fix) | SEG077.C |
| 98 | Klebriger Boden: „GELINGT ES, SICH LOSZUREISSEN“ nur nach einem Fehlversuch | egal | nein | alles | ja | die Befreiungsmeldung immer zeigen (Bright-Eyes-Fix zu Original-Bug 65) | SEG078.C |
| 99 | Wegteleportieren von Druckplatte oder Hebel wird nicht erkannt | egal | nein | alles | nein | die Druckplatten muessten bei jedem Teleport und Gruppenwechsel in mehreren Dungeons neu ausgewertet werden - ein Umbau der Handler, kein eindeutiger Einzelfix |  |
| 100 | Druckplatten der Verfallenen Herberge berücksichtigen andere Gruppen nur halb | egal | nein | alles | nein | Neuauswertung aller Gruppen auf den Platten waere ein Umbau der Plattenlogik; der eindeutige Teil (Summe in der Schleife) ist Nr. 102 |  |
| 101 | „IHR HÖRT EIN KNIRSCHEN“ auch bei schon offener Wand | egal | nein | alles | ja | nur melden, wenn die Wand noch steht | SEG078.C |
| 102 | weight_sum wird in der Gruppenschleife zurückgesetzt | mittel | nein | wichtig | ja | Summe einmal vor der Schleife loeschen | SEG078.C |
| 103 | „VORSICHT NACH DREI SCHRITT LINKS“ – müsste es „rechts“ heißen? | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 104 | Beschwörungsszene erscheint auch nach dem Kampf | egal | nein | alles | nein | ob der Kampf schon stattfand, steht nur in der Kampfliste; welcher Kampf zur Szene gehoert, ist im Handler nicht verknuepft |  |
| 105 | Teleporter und Schilfmeer treffen auch Helden anderer Gruppen | mittel | nein | wichtig | ja | nur lebende Helden der aktiven Gruppe | SEG079.C SEG110.C |
| 106 | Hebel und Geheimtür verlangen Betreten UND Drehen im selben Schritt | mittel | nein | wichtig | ja | Betreten ODER Drehen (wie beim Hebel (1,3,11)) | SEG079.C |
| 107 | Absturz in den Abgrund: hero_disappear() mit Position 0 | egal | nein | alles | ja | die Position des abgestuerzten Helden | SEG080.C |
| 108 | Ratten beißen auch tote Helden | egal | nein | alles | ja | keine toten Helden | SEG080.C |
| 109 | Armbrustfalle zählt nur Platz 0 und 1 | egal | nein | alles | ja | die ersten beiden handlungsfaehigen Helden der Gruppe treffen | SEG081.C |
| 110 | Zwei-Gruppen-Rätsel prüfen nur x/y der anderen Gruppe | mittel | ja | wichtig | ja | die andere Gruppe muss in diesem Dungeon auf dieser Ebene stehen | SEG081.C SEG083.C SEG088.C |
| 111 | Schleifen über 7 Plätze beginnen beim ersten verfügbaren Helden statt bei Platz 0 | mittel | nein | wichtig | ja | bei Platz 0 beginnen; bei Platz 0 beginnen (hero zeigte noch auf den ersten handlungsfaehigen Helden) | SEG081.C SEG082.C SEG083.C |
| 112 | Merker „Rätseltruhe geplündert“ wird nie gesetzt | egal | nein | alles | nein | ob der Inhalt der Raetseltruhe immer ganz genommen wird, ist offen; der Merker wuerde uebrig gebliebene Gegenstaende sperren |  |
| 113 | Hilferuf aus der Grube jede Minute statt alle 20 Minuten | egal | nein | alles | ja | alle 20 Minuten | SEG084.C |
| 114 | Armbrustfalle 4 zählt nicht herunter | mittel | nein | wichtig | ja | wie die Fallen 1 bis 3 herunterzaehlen | SEG084.C |
| 115 | Loser Stein: Zustand „entfernt“ wird überschrieben | egal | nein | alles | ja | den Zustand "Stein entfernt" (2) nicht ueberschreiben | SEG085.C |
| 116 | Trittplatte wirkt nur beim Entschärfen | egal | nein | alles | nein | unsicher, als Absicht lesbar (Text 13 fragt nur nach dem Entschaerfen); ein Ausloesen bei unentdeckter Platte waere eine neue Falle |  |
| 117 | Opferschale: ESC und große Eingaben (16-Bit-Überlauf) schenken Geld | kritisch | ja | wichtig | ja | in 32 Bit rechnen; ESC (-1) und Ueberlaeufe (<= 0) bringen kein Geld | SEG085.C |
| 118 | „possibly a bug: position (8,12) might be (8,11)“ | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 119 | Geheimtüren: test_result bei falscher Hebelstellung uninitialisiert | mittel | nein | wichtig | ja | bei falscher Hebelstellung kein Zufallswert | SEG086.C |
| 120 | Zeitabfragen „day_timer % X == 0“ greifen mehrmals hintereinander | mittel | nein | wichtig | nein | unsicher, nicht gemessen; die zuletzt ausgewertete Zeitmarke zu merken braucht eine neue Variable (Datensegment) |  |
| 121 | Verschütteter Gang (Zustand 3) blockiert nicht | mittel | nein | wichtig | ja | der verschuettete Gang sperrt den Weg; Freiraeumen anbieten (Text 45) | SEG089.C |
| 122 | Mit Werkzeug dauert das Freiräumen länger als ohne | mittel | nein | wichtig | ja | mit Werkzeug 30 Minuten, ohne eine Stunde; mit Werkzeug 6 Stunden, ohne einen Tag | SEG089.C SEG091.C |
| 123 | Zähler der Grubenversuche steht nicht im Spielstand | egal | nein | alles | nein | der Zaehler muesste in den Spielstand (neues Feld) |  |
| 124 | Texte 31 und 32 fehlen in der deutschen OBER.DTX | mittel | nein | wichtig | nein | unsicher: die Texte fehlen in der Spieldatei OBER.DTX; welcher vorhandene Text passt, ist nicht belegbar |  |
| 125 | Deckeneinsturz (schwer): mit Werkzeug 18 statt 12 Stunden? | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 126 | Opfer für Efferd und Ingerimm zählen auch ohne Wegwerfen | mittel | ja | wichtig | ja | Opfer zaehlen nur, wenn wirklich etwas weggeworfen wurde | SEG105.C |
| 127 | Nur die erste Seeroute (Thorwal–Prem) wird zurückgesetzt | mittel | nein | wichtig | ja | alle Seerouten zuruecksetzen | SEG002.C |
| 128 | Sprung bis Mitternacht (Swafnild) überspringt feste Ereignisse und verbraucht 25 Stunden Licht | egal | nein | alles | ja | gewoehnlicher Zeitsprung bis Mitternacht (feste Uhrzeiten, Licht und Stunden werden richtig gezaehlt) | SEG002.C |
| 129 | Wachenwahl: ESC liest g_wildcamp_herbstatus[−1] | egal | nein | alles | ja | answer != -1 vor allen drei Abfragen | SEG051.C |
| 130 | Bei weniger als 3 Stunden Schlaf wechseln die Wachen nie | egal | nein | alles | ja | mindestens 1 Stunde je Wache, sonst wechselt die Wache nie | SEG051.C SEG052.C |
| 131 | Seeweg Prem–Manrin nur 54 km lang | mittel | nein | wichtig | nein | die Entfernung steht als Byte in der Tabelle g_sea_routes (Datensegment, Spielstand) |  |
| 132 | Hafenmeister schimpft auch beim Verlassen über fehlendes Geld | egal | nein | alles | ja | Geld nur pruefen, wenn die Gruppe etwas wissen will (nicht beim Verlassen) | SEG063.C |
| 133 | Hunger-/Durst-Meldungssperre des NSC wird nicht zurückgesetzt | egal | nein | alles | ja | auch den NSC | SEG063.C |
| 134 | Efferds Schutz verhindert den Krakenmolch, nicht die Piraten | mittel | nein | wichtig | nein | Auslegung (unsicher): ob Efferds Schutz auch vor Piraten schuetzen soll, ist nicht belegbar |  |
| 135 | Schiffsnamen werden bei jedem Blick neu ausgewürfelt | egal | nein | alles | ja | Schiffsname fest aus dem Preisaufschlag der Passage (Bright-Eyes-Fix) | SEG064.C |
| 136 | Passagepreis in 16 Bit: lange Routen werden spottbillig | mittel | ja | wichtig | ja | mit der Entfernung in 32 Bit multiplizieren | SEG064.C |
| 137 | Schiffstempo in 16 Bit, Schnellsegler mit −106: Tempo 0 bringt das Spiel zum Absturz | kritisch | nein | wichtig | ja | Grundtempo ohne Vorzeichen (150 statt -106), Produkt in 32 Bit, Ergebnis 1..127 | SEG064.C |
| 138 | Angezeigte Reisedauer nur in groben Stufen | egal | nein | alles | ja | erst multiplizieren, dann teilen (Bright-Eyes-Fix); Tempo 0 abfangen | SEG064.C |
| 139 | Hafen Hjalsingor: Route nach Overthorn fehlt in der Liste | mittel | nein | wichtig | nein | die Hafenliste steht im Datensegment (Tabelle g_harbors_linked_sea_routes_10) |  |
| 140 | Hafen Leuchtturm Runin verweist auf eine Landweg-Liste | mittel | nein | wichtig | nein | die Hafentabelle g_harbors steht im Datensegment |  |
| 141 | Ork-Monolith (Ereignis 71) steht auch auf dem Weg Oberorken–Felsteyn | egal | nein | alles | nein | Eintrag der Ereignistabelle im Datensegment, Datenfehler (unsicher) |  |
| 142 | Reiseereignisse: Lage wird nach Richtungswechsel falsch korrigiert | mittel | ja | wichtig | ja | Lage direkt nach dem Ereignis setzen, bei Richtungswechsel durch das Ereignis | SEG094.C |
| 143 | Gewaltmarsch-Tempo hängt von der schon gegangenen Strecke ab | mittel | ja | wichtig | nein | unsicher: welcher feste Wert gemeint war, ist nicht belegbar (197 waere langsamer als normal) |  |
| 144 | Gewaltmarsch zieht den LE-Verlust auf einmal ab | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 145 | Umweg-Linie wird um einen Punkt versetzt zurückgemalt | egal | nein | alles | ja | erst zurueckschreiben, dann weitersetzen (wie beim Zeichnen) | SEG094.C |
| 146 | Axt-Liste für Reiseereignisse | mittel | nein | wichtig | nein | die Axt-Liste liegt im Datensegment; welche Aexte gemeint sind, ist ausserdem nicht belegt |  |
| 147 | Rastplatz-Text ohne Platzhalter: Ortsbeschreibung und Kräuterhinweis fehlen | egal | nein | alles | nein | unsicher: ob FEATURE.LTX 15..19 als Formattexte gedacht sind, laesst sich ohne die Spieldaten nicht pruefen; Spieltext |  |
| 148 | Rastplatz: Status nur für den Anführer gelöscht | egal | nein | alles | ja | Status aller Helden zuruecksetzen, replenish_stocks() fragt selbst nach dem Helden (Plaetze 0..6) | SEG109.C |
| 149 | replenish_stocks() bekommt die Heldenposition als Versuchszähler | mittel | nein | wichtig | ja | zweiter Parameter ist der Versuchszaehler, nicht die Heldenposition | SEG109.C |
| 150 | „UMKEHREN“ auf der Rückreise behält die Rückwärtsrichtung (9 Stellen) | mittel | nein | wichtig | ja | Umkehren kehrt die aktuelle Richtung um | SEG109.C SEG113.C SEG114.C SEG115.C SEG116.C SEG118.C |
| 151 | Furt: GE-Probe gilt fast immer als bestanden | mittel | nein | wichtig | ja | gewoehnliche GE-Probe, jedes Ergebnis <= 0 ist misslungen | SEG109.C |
| 152 | Fährpreis wächst bei jedem Versuch weiter | mittel | nein | wichtig | ja | Preis bei jedem Versuch aus dem Grundpreis berechnen | SEG109.C |
| 153 | Gruppenproben zählen alle 7 Plätze ohne Gruppen- und Todesprüfung | mittel | nein | wichtig | ja | nur lebende Helden der aktiven Gruppe; nur lebende Helden der aktiven Gruppe (wie count_heroes_in_group()) | SEG109.C SEG116.C |
| 154 | „Original-Glitch“: gs_travel_detour = 1 ist zugleich die Nummer des Totenschiffs | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 155 | Querverbindung: Herkunft wird nach Peilinen statt Kravik/Skelellen bestimmt | egal | nein | alles | nein | folgenlos: ausgewertet wird nur Bit 0, das bei beiden Werten gleich ist |  |
| 156 | „AM UFER DES DES BODIR“ | egal | nein | alles | ja | fuehrendes "DES " des Flussnamens weglassen, der Furttext hat schon "AM UFER DES %s" | SEG110.C |
| 157 | Brückenbau Auplog–Varnheim: Pronomen im falschen Fall | egal | nein | alles | nein | Spieltext (FEATURE2.LTX) mit zwei statt drei Platzhaltern |  |
| 158 | Steilhang: Strickleiter macht den Sturz schlimmer | mittel | nein | wichtig | ja | schwerer Sturz nur ohne Seil und ohne Strickleiter | SEG111.C |
| 159 | Gorahs Hütte: Silentium-Auswahl wird nie wiederholt | mittel | ja | wichtig | ja | Nicht-Magier oder misslungener Silentium fuehren zurueck in die Auswahl | SEG111.C |
| 160 | Hütte bei Orvil: Frage passt nicht zu den Antworten | egal | nein | alles | ja | die zur Antwort passende Frage (Text 91) | SEG112.C |
| 161 | Kräuterbonus für Gegenstände über 127 greift nie | mittel | nein | wichtig | ja | Kraeuternummern ueber 127 (int8_t) richtig vergleichen | SEG051.C |
| 162 | Wegzoll 15 Dukaten insgesamt statt pro Person | mittel | ja | wichtig | ja | 15 Dukaten pro Person, wie im Text verlangt | SEG112.C |
| 163 | Tatzelwurm-Spuren: Name aus einem Nullzeiger | egal | nein | alles | ja | ohne Jaeger (hero == NULL) den ersten verfuegbaren Helden nennen | SEG113.C |
| 164 | Waten durch den Bach mit uninitialisiertem Helden | kritisch | nein | wichtig | ja | Held ist hier nicht gesetzt - den ersten verfuegbaren nehmen | SEG113.C |
| 165 | Sturz in die Klamm: hero_disappear() mit der Antwortzahl als Position | mittel | nein | wichtig | ja | Position des Springers uebergeben | SEG113.C |
| 166 | Harpyien: Fluchtoption ist kein Versuch | mittel | ja | wichtig | ja | "VERSUCHEN ZU FLIEHEN?" fuehrt zur Kriegskunst-Probe (else gehoert zur Antwort) | SEG113.C |
| 167 | Nebeltal: Funktionsadresse als Held | mittel | nein | wichtig | ja | Funktion aufrufen statt ihre Adresse als Helden zu nehmen | SEG113.C |
| 168 | Dialogbild fehlt nach einem Lagerkampf | egal | nein | alles | ja | Kopfbild nach dem Lager neu laden | SEG113.C SEG118.C |
| 169 | „NACH GUT ACHT STUNDEN“, vergehen aber 12 | egal | nein | alles | nein | unsicher: Umweg plus Rueckweg koennten gemeint sein |  |
| 170 | Firun-Opfer: Schlafbonus bleibt für spätere Lager stehen | mittel | ja | wichtig | ja | Schlafbonus gilt nur fuer dieses Lager | SEG114.C |
| 171 | Olimone: get_hero(−1), wenn niemand das Rezept nehmen konnte | egal | nein | alles | ja | hat niemand das Rezept, den ersten verfuegbaren Helden nennen statt get_hero(-1) | SEG114.C |
| 172 | Brücke: Sturz in die Tiefe ist unerreichbar | mittel | nein | wichtig | nein | unklar, welche Bedingung gemeint war: -99 hiesse ein patzender Hoehenangst-Wurf (also gerade keine Angst) laesst den Helden abstuerzen; test_talent() == -1 ist ebenfalls zufaellig eng |  |
| 173 | Einsiedlersee: nur der erste Wasserschlauch wird gefüllt | egal | nein | alles | nein | unsicher, ob Absicht |  |
| 174 | Zustand 33 ist eine Endlosschleife | kritisch | nein | wichtig | ja | statt Endlosschleife das Gespraech beenden | SEG116.C |
| 175 | Landschaft 7 liest hinter die 7 Spalten der Begegnungstabelle | egal | nein | alles | nein | Tabelle im Datensegment; welche Landschaft fuer Typ 7 gemeint war, ist nicht belegt (mit Fix 181 liest Typ 7 weiter die verschobene Spalte) |  |
| 176 | Speer als Jagdwaffe | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 177 | AT- und PA-Proben verkehrt herum (Höhlenbär, Krakenmolch) | mittel | nein | wichtig | ja | Attacke gelingt bei W20 <= AT; Schaden bei misslungener Parade (W20 > PA); Treffer bei W20 <= AT | SEG117.C |
| 178 | Kvillotter beißt nie | mittel | nein | wichtig | ja | das schlechteste misslungene Ergebnis merken, sein Held wird gebissen | SEG117.C |
| 179 | Krakenmolch: GE-Probe falsch ausgewertet | mittel | nein | wichtig | ja | misslungene GE-Probe = Wuergen, Patzer (-99) = Packen | SEG117.C |
| 180 | Krakenmolch: Endlosschleife, wenn alle Helden über Bord oder tot sind | kritisch | nein | wichtig | ja | Kampf endet, wenn kein Held mehr kaempft (sonst Endlosschleife) (anders als Bright-Eyes: die Schleife endet nur, ueber Bord Gerissene bleiben wie im Original in der Gruppe) | SEG117.C |
| 181 | Zufallsbegegnungen: Vergleich verkehrt herum, fast alle unerreichbar | mittel | nein | wichtig | ja | aufsteigende Schwellen: W100 <= Wert | SEG117.C |
| 182 | Krankheitsprobe mit uninitialisiertem Helden | egal | nein | alles | ja | KK des betroffenen Helden | SEG117.C |
| 183 | Waldweg: Erfolgstext beim ersten Versuch unerreichbar | egal | nein | alles | ja | beim ersten Versuch (answer == 2) der Text fuer den schnellen Erfolg | SEG118.C |
| 184 | Verirrt im Wald: alle sieben Heldenplätze sterben | kritisch | nein | wichtig | ja | nur die Helden der verirrten Gruppe | SEG118.C |
| 185 | Kletterpartie: tödlicher Sturz bei knapp verfehlter Probe statt beim Patzer | mittel | nein | wichtig | ja | toedlich ist nur der Patzer (-99) | SEG118.C |
| 186 | Visibili wirkt nur auf den Zaubernden, der Kampf entfällt nur bei einem Einzelhelden | mittel | nein | wichtig | ja | der Kampf faellt aus, wenn alle lebenden Helden der Gruppe unsichtbar sind; wie im Loesungsbuch: die ganze Gruppe wird unsichtbar, die Kosten (Spielrunden x 5 AE) zahlt der Zaubernde einmal (belegt durch das Loesungsbuch (S. 17 ff.: "macht die Gruppe unsichtbar", Kosten Spielrunden x 5 AE): alle lebenden Helden der Gruppe werden unsichtbar, der Kampf entfaellt, wenn alle unsichtbar sind (Version 1.1)) | SEG032.C SEG101.C |
| 187 | Transversalis-Kreuz darf eine Zeile unter die Karte | egal | nein | alles | ja | das Kreuz bleibt auf der Karte (Zeilen 0..15) | SEG074.C |
| 188 | Meditation bei LE 6 oder 7 kostet LE und bringt keine AE | egal | nein | alles | ja | ohne umwandelbare LE (ae <= 0) keine LE abziehen | SEG098.C |
| 189 | Stabzauber: Abfrage „keine Stabzauber mehr“ auf Stufe 7 statt 4 | mittel | nein | wichtig | ja | ab dem 5. Stabzauber (Zuschlag 99) gibt es keine mehr | SEG098.C |
| 190 | Stabzauber-Probe prüft die zweite Eigenschaft doppelt | mittel | nein | wichtig | ja | dritte Eigenschaft statt der zweiten ein zweites Mal | SEG098.C |
| 191 | Foramen aus dem Zaubermenü ignoriert den Türzuschlag | mittel | ja | wichtig | ja | Zuschlag der Tuer vor der Gruppe auch beim freien Foramen (Suche endet am Listenende -1) | SEG098.C |
| 192 | Gardianum kostet AE und schützt nicht | mittel | nein | wichtig | ja | der Gardianum-Schild gilt fuer einen Kampf; Staerke des Schildes = Stufe des Zaubernden + eingesetzte AE (Loesungsbuch); ausgewertet in seg102; der Gardianum-Schild faengt ab, die AE zahlt der Gegner trotzdem (belegt durch das Loesungsbuch (Schild gegen feindliche Fulminictus/Ignifaxius, Staerke Stufe + AE, 1 Kampf oder bis verbraucht): Schild faengt Schaden ab, wird zu Kampfbeginn geleert (Version 1.1)) | SEG032.C SEG099.C SEG102.C |
| 193 | Spieltext „AUF %S RUHT …“ mit großem S | egal | nein | alles | ja | %S im geladenen Text zu %s machen | SEG099.C |
| 194 | „IST DOCH GARNICHT TOT“ ohne Subjekt | egal | nein | alles | nein | Spieltext (SPELLTXT.LTX) ohne %s; ein %s laesst sich nicht ohne Textaenderung einfuegen |  |
| 195 | Skelettarius: Leiche darunter verschwindet aus der Auswahl | egal | nein | alles | ja | object_id des Ziels sichern (darunterliegende Leiche bleibt waehlbar) | SEG099.C |
| 196 | Skelettarius belegt bei jedem Wirken 1288 Byte neu im Kampfobjektpuffer | kritisch | nein | wichtig | ja | Grafikpuffer des ersetzten Gegners (je Gegner 0x508 Byte) fuer den Zombie wiederverwenden | SEG099.C |
| 197 | Foramen aus dem Zaubermenü wirkt nur nach geöffnetem Türmenü | mittel | nein | wichtig | ja | freies Foramen auch vor einer geschlossenen Tuer ohne geoeffnetes Tuermenue | SEG099.C |
| 198 | Balsam: Mindestkosten 7 AE nicht einheitlich | egal | nein | alles | nein | Balancefrage: ob 7 AE auch fuer weniger fehlende LE (oder bei voller LE) gelten sollen, ist nicht belegt |  |
| 199 | Hexenspeichel: AE-Prüfung greift nie | mittel | ja | wichtig | ja | hoechstens die fehlenden LE heilen, je LE 2 AE, und nicht mehr AE ausgeben, als da sind | SEG099.C |
| 200 | Abbruch der Gegenstandswahl: inventory[−1] | egal | nein | alles | ja | Abbruch der Gegenstandswahl (-1) nicht als Inventarplatz benutzen; Abbruch der Gegenstandswahl (-1) nicht als Inventarplatz benutzen (g_dtp2 ist schon leer) | SEG099.C SEG100.C |
| 201 | Analüs: volle AE-Kosten, wenn nur der Zuschlag die Probe kippt | egal | nein | alles | ja | am Zuschlag gescheitert -> halbe AE wie bei misslungenem Zauber | SEG099.C |
| 202 | „Eigenschaften seid gelesen“ zeigt 80 % des Nahkampfschadens | egal | nein | alles | ja | Nahkampfschaden ungekuerzt anzeigen | SEG100.C |
| 203 | Exposami vergleicht mit einem noch leeren Listeneintrag | egal | nein | alles | ja | nur beschriebene Eintraege vergleichen | SEG100.C |
| 204 | Hexenknoten: Grafikfehler beim zweiten Wirken | egal | nein | alles | nein | Breite/Hoehe und das Zuruecksetzen bei Kampfende braeuchten neuen Speicher im Datensegment bzw. Zugriff auf eine statische Variable aus einem anderen Modul |  |
| 205 | Eisenrost auf ein Tier kostet volle AE | egal | nein | alles | ja | keine Kosten, wie bei Bannbaladin | SEG100.C |
| 206 | Spieltext „VERWANDLET“ | egal | nein | alles | ja | Tippfehler im geladenen Text zur Laufzeit berichtigen | SEG100.C |
| 207 | Armatrutz: ein RS-Punkt mehr, als AE vorhanden | mittel | ja | wichtig | ja | groesster Bonus, dessen Quadrat die AE nicht uebersteigt | SEG101.C SEG102.C |
| 208 | Paralue auf einen Helden prüft das alte Zauberziel | egal | nein | alles | ja | erst das neue Ziel setzen, dann pruefen | SEG101.C |
| 209 | Salander auf zweiteilige Gegner: „strange effect“ | egal | nein | alles | nein | unsicher, im Spiel nicht nachgeprueft; Behandlung des Schwanzteils waere ein groesserer Eingriff in die Kampfliste |  |
| 210 | Brenne entzündet immer die erste Fackel oder Laterne | egal | nein | alles | ja | den benutzten Inventarplatz anzuenden, nicht die erste Fackel/Laterne | SEG101.C |
| 211 | g_ignite_mode wird nach dem Anzünden nicht zurückgesetzt | mittel | nein | wichtig | ja | Anzuendmodus zuruecksetzen, sonst findet "Brenne toter Stoff" danach nur noch diese Art | SEG107.C |
| 212 | Tollwut: Sanftmut ohne Prüfung des Zauberwerts | mittel | nein | wichtig | teilweise | Zauberwert pruefen (die Meldung "beherrscht den Zauber nicht" braeuchte einen neuen Text); Magier mit 4. Stabzauber zahlen 2 AE weniger (sub_ae_splash) - Zauberwert und Stabrabatt geprueft; die fehlende Meldung braeuchte einen neuen Text | SEG120.C |
| 213 | Feuerbann wird gesetzt, aber nirgends ausgewertet | mittel | nein | wichtig | ja | Dauer Stufe in Stunden (Loesungsbuch); die Wirkung wertet seg102 aus (belegt durch das Loesungsbuch ("halbiert evtl. auftretenden Feuerschaden", Dauer Stufe in Stunden) und Sternenschweif (feindlicher Ignifaxius halbiert bei Feuerbann): Schaden durch feindlichen Ignifaxius halbiert, Dauer Stufe in Stunden (Version 1.1)) | SEG101.C SEG102.C |
| 214 | Magisches Bastardschwert ist als „benutzbar“ markiert | kritisch | nein | wichtig | teilweise | Gegenstaende mit Nummer ausserhalb der Benutzen-Tabelle (magisches - Absturz verhindert (Meldung nicht benutzbar); das usable-Bit in ITEMS.DAT bleibt | SEG107.C |
| 215 | Magischer Brotbeutel wirkt wie der magische Beutel | egal | nein | alles | nein | Datenfehler in ITEMS.DAT, ohne regulaeren Fundort folgenlos |  |
| 216 | Truhenliste: 20 Byte je Name zu kurz | egal | nein | alles | ja | 30 Byte je Name wie in loot_simple_chest() | SEG092.C |
| 217 | Mengenabfrage lässt sich nicht abbrechen | egal | nein | alles | ja | ESC (oder eine negative Eingabe) nimmt nichts mit | SEG092.C |
| 218 | Analüs verspricht Wirkungen magischer Gegenstände, die es nicht gibt | mittel | nein | wichtig | nein | fehlende Gegenstandswirkungen waeren neue Spielmechanik; Texte stehen in den Spieldaten |  |
| 219 | Kraftgürtel aktualisiert abhängige Werte nicht | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 220 | Zähler der belegten Inventarplätze driftet | mittel | nein | wichtig | ja | Stapel hoechstens 99, beim Zusammenlegen den Platzzaehler senken; neuer Stapel beim Empfaenger belegt einen Platz (Zaehler an den drei Stellen richtig gefuehrt, Stapel beim Zusammenlegen auf 99 begrenzt; in alten Spielstaenden bereits verschobene Zaehler bleiben) | SEG106.C |
| 221 | Zufälliger Gegenstandsverlust: Endlosschleife ohne ablegbaren Gegenstand | kritisch | nein | wichtig | ja | ohne ablegbaren Gegenstand nichts verlieren (sonst Endlosschleife) | SEG105.C |
| 222 | Tausch zweier Körperplätze ohne Prüfung und Boni | kritisch | ja | wichtig | ja | Meldung "... KANN AN DIESER POSITION NICHT GETRAGEN WERDEN" (wie in move_item()); liegen beide Plaetze am Koerper, auch den verdraengten Gegenstand pruefen | SEG106.C |
| 223 | Wasserschlauch in der Hand wird nicht gefunden | egal | nein | alles | ja | auch den Wasserschlauch in der Hand finden | SEG106.C |
| 224 | Waffengift: Schleuder wird nicht ausgeschlossen | egal | nein | alles | ja | auch die Schleuder ist eine Fernwaffe | SEG107.C |
| 225 | Laterne nachfüllen: Öl beim letzten Zauberer gesucht, beim Benutzer entfernt | mittel | nein | wichtig | ja | Oel und Laterne beim Benutzer suchen, nicht beim letzten Zauberer | SEG107.C |
| 226 | Tarnele: Meldung ohne Text | egal | nein | alles | teilweise | keinen alten Text zeigen; ein leerer Text oeffnet kein Fenster - kein alter Text mehr; eine eigene Meldung braeuchte einen neuen Text | SEG108.C |
| 227 | Antikrankheitselixier ohne Krankheit schreibt in disease[0] | egal | nein | alles | ja | ohne Krankheit nichts aendern (disease[0] ist unbenutzt) | SEG108.C |
| 228 | „What about herokeeping()?“ im stündlichen Timer | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 229 | h_index wird nur einmal vor der Schleife auf −1 gesetzt | mittel | nein | wichtig | ja | Held fuer jeden Timer neu suchen | SEG002.C |
| 230 | Aufräumen ungültiger Ziele setzt nur den aktuellen Eintrag zurück | egal | nein | alles | ja | alle Eintraege des ungueltigen Ziels zuruecksetzen | SEG002.C |
| 231 | Notbehelf bei 100 belegten Timern versagt im Kampf | egal | nein | alles | ja | auch im Kampf (g_timers_disabled) wirklich freigeben | SEG002.C |
| 232 | Gift wirkt nur in game_loop(), bei Zeitsprüngen höchstens einmal | mittel | nein | wichtig | nein | Gift alle 5 Minuten auch waehrend Rast und Reise auszuwerten hiesse bei langer Rast dutzende Meldungsfenster und meist den Tod (Shurinknollengift endet nicht von selbst); ob das so gewollt ist, ist offen - im Zweifel nicht |  |
| 233 | Heil-, Stab- und Lichttimer runden kleine Zeitsprünge weg | mittel | nein | wichtig | ja | den Rest der angebrochenen 5 bzw. 15 Minuten mitrechnen | SEG002.C |
| 234 | Stundenzählung für Kettenhemd-Schaden und Essen falsch | mittel | nein | wichtig | ja | Zahl der ueberschrittenen vollen Stunden | SEG002.C |
| 235 | Zufallsheld aus Plätzen 0..n−1 statt aus den Gruppenmitgliedern | kritisch | nein | wichtig | ja | alle 7 Plaetze wuerfeln (auch den NSC); ohne lebenden Helden der Gruppe keine Endlosschleife | SEG002.C |
| 236 | Rote Anzeige des bleibenden LE-Verlusts (Bright-Eyes-Komfort) | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 237 | BP-Anzeige bei Last ab 110 % falsch | egal | nein | alles | ja | ab 110 % Last nur 1 BP, wie im Kampf (FIG_do_round) | SEG046.C |
| 238 | Erneute Vergiftung löscht die Protokollbytes | mittel | nein | wichtig | ja | die Protokollbytes der erlittenen Verluste nur bei einem gesunden Helden loeschen, | SEG047.C |
| 239 | „not a real BUG“: keine Prüfung auf leeren Heldenplatz | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 240 | Erneute Erkrankung löscht die Protokollbytes, Verluste werden dauerhaft | mittel | nein | wichtig | ja | die Protokollbytes der erlittenen Verluste nur bei einem gesunden Helden loeschen, | SEG047.C |
| 241 | AT-Basis nicht nach DSA 3 (MU + KK + GE) / 5 | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 242 | Ohne wählbaren Helden: Textbreite bleibt 3, geliefert wird Platz 0 | egal | nein | alles | teilweise | Textbreite zuruecksetzen; die Rueckgabe bleibt 0 (die Aufrufer pruefen nicht auf -1) - Textbreite wird zurueckgesetzt; die Rueckgabe bleibt 0 statt -1, weil alle Aufrufer das Ergebnis ungeprueft mit get_hero() benutzen | SEG047.C |
| 243 | Gruppe teilen klappt nicht richtig, wenn der NSC in einer anderen Gruppe ist | mittel | nein | wichtig | ja | handlungsfaehige Helden der aktiven Gruppe ohne den NSC (Platz 6) zaehlen; den NSC nicht mitzaehlen, egal in welcher Gruppe er ist (Bright-Eyes-Fix) | SEG049.C |
| 244 | Gruppenwechsel ohne Ausgang, wenn keine Gruppe frei handeln kann | kritisch | nein | wichtig | ja | nach einer vollen Runde ohne handlungsfaehige Gruppe nicht endlos weitersuchen: | SEG049.C |
| 245 | Nachtwache folgt beim Tauschen nur einem der beiden Helden | egal | nein | alles | ja | auch die Wache des zweiten Helden mitnehmen; auch die Wache auf dem Zielplatz mitnehmen | SEG049.C |
| 246 | Regeneration: Ergebnis 0 bleibt 0, −1 wird zu 1 | egal | nein | alles | ja | auch Ergebnis 0 auf 1 setzen | SEG049.C |
| 247 | Brauen endet nur ohne Gift, Krankheit und Stabzaubersperre | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 248 | Stufenanstieg: ESC bei „WELCHE POSITIVE EIGENSCHAFT“ bringt rund 168 000 Dukaten | kritisch | ja | wichtig | ja | Abbruch (ESC, rechte Maustaste) wiederholt die Abfrage | SEG050.C |
| 249 | Stufenanstieg: ESC bei „WELCHE NEGATIVE EIGENSCHAFT“ senkt die Intuition | mittel | nein | wichtig | ja | Abbruch (ESC, rechte Maustaste) wiederholt die Abfrage | SEG050.C |
| 250 | MR-Neuberechnung beim Stufenanstieg verträgt sich nicht mit MR-Gegenständen | mittel | nein | wichtig | ja | mit dem Grundwert vergleichen; die Boni der MR-Gegenstaende so aufschlagen wie | SEG050.C |
| 251 | Zaubersteigerung: Zähler i wird je Runde nicht zurückgesetzt | egal | nein | alles | ja | jede Runde wieder beim ersten Zauber beginnen | SEG050.C |
| 252 | Zufallszauber 1..86 statt 1..85 | egal | nein | alles | ja | nur echte Zauber 1..SP__END-1 | SEG050.C |
| 253 | Spielende: Merker für bleibenden LE-Verlust wird gelöscht, LE-Maximum bleibt gesenkt | mittel | nein | wichtig | ja | den Merker fuer den bleibenden LE-Verlust behalten (das LE-Maximum bleibt ja gesenkt), | SEG065.C |
| 254 | set_mod_slot() mit who = 0 für einen anderen Helden | mittel | nein | wichtig | ja | who = Position dieses Helden; Position des betroffenen Helden als Besitzer eintragen | SEG080.C SEG115.C |
| 255 | Harikas Abschiedsgeschenk kostet Steigerungsversuche | mittel | nein | wichtig | ja | Steigerungsversuche sichern, inc_talent_novice() zieht sie ab | SEG095.C |
| 256 | Entlassene NSC kommen nie wieder | mittel | nein | wichtig | ja | Zustand des NSC vor dem Loeschen merken, sonst wird der Timer immer -1 | SEG095.C |
| 257 | „Why STAFFSPELL?? BUG!“ – staffspell_timer als Regenerationssperre | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 258 | Misslungene Wundheilung setzt Wundfieber ohne Prüfung | egal | nein | alles | ja | eine laufende Erkrankung oder Genesung nicht von vorn beginnen | SEG103.C |
| 259 | Brauen: Abtrennen des einzigen Helden einer Gruppe möglich | mittel | nein | wichtig | ja | handlungsfaehige Helden der aktiven Gruppe ohne NSC (wie count_heroes_available_in_group_ignore_npc(); nur abtrennen, wenn in der aktiven Gruppe noch ein handlungsfaehiger Held bleibt | SEG104.C |
| 260 | Suche nach freier Gruppe ohne Grenze | kritisch | nein | wichtig | ja | erste freie der 6 Gruppen, -1 wenn alle belegt sind; Abtrennen nur anbieten, wenn eine der 6 Gruppen frei ist | SEG104.C |
| 261 | „DA SCHEINT ABER WAS ZU FEHLEN“ nennt die Zutat nicht | egal | nein | alles | nein | Spieltext (SPELLTXT.LTX) ohne %s; der Name laesst sich nicht ohne Textaenderung einfuegen |  |
| 262 | Heilen Krankheiten: Wundfieber und Blaue Keuche nie heilbar, Erfolg wird als Misserfolg gemeldet | mittel | nein | wichtig | nein | unsicher, ob die beiden Krankheiten per Talent heilbar sein sollen; eine eigene Meldung braeuchte einen neuen Text |  |
| 263 | Textdatei wird bei totem Patienten nicht zurückgeladen | egal | nein | alles | ja | Textdatei auch bei nicht heilbarem Patienten zurueckladen | SEG104.C |
| 264 | Verschwundener NSC: Datei und Timer um eins verschoben | mittel | nein | wichtig | ja | Datei und Timer des NSC selbst (npc_id 1..6, wie add_npc()/remove_npc()) | SEG113.C |
| 265 | Verschlimmerung zu Blauer Keuche mit 4 statt 5 % | egal | nein | alles | ja | 5 % wie vorgesehen | SEG119.C |
| 266 | Nach dem Wechsel zu Blauer Keuche noch Dumpfschädel-Schaden am selben Tag | egal | nein | alles | ja | nach der Verschlimmerung keinen Dumpfschaedel-Schaden mehr | SEG119.C |
| 267 | „Original-Bug? forgot? time_counter = 0“ | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 268 | Blaue Keuche: „DAS LEBEN WIRD WOHL NIE WIEDER SEIN“ immer | egal | nein | alles | nein | braucht einen eigenen Merker im Heldendatensatz (Spielstand) und der Bright-Eyes-Fix einen neuen Text |  |
| 269 | Paralyse und Frostschäden: Erholung mit umgekehrter Bedingung | mittel | nein | wichtig | ja | FIX_0269 = 1 dreht die Bedingung um (Verlust vorhanden -> zurueckgeben) | SEG119.C |
| 270 | Schlachtfeldfieber endet nie | mittel | nein | wichtig | ja | nach der Rueckgabe der KK wird der Held gesund | SEG119.C |
| 271 | Zeitzähler von Krankheiten und Giften können überlaufen | egal | nein | alles | nein | unsicher, Folgen nicht geprueft; die Ursache Schlachtfeldfieber ist mit Fix 270 behoben |  |
| 272 | „Not a bug, but useless“: Markierung sex = 50 | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 273 | Tollwut: Beruhigen und Sanftmut werten misslungene Proben als Erfolg | mittel | nein | wichtig | ja | nur handlungsfaehige Helden der Gruppe, CH-Probe gelungen (> 0); nur eine gelungene Probe (> 0) beruhigt | SEG120.C |
| 274 | Shurinknollengift und Bannstaub: Rückgabe der Werte fast nie | mittel | nein | wichtig | ja | alle 12 Takte (Klammern um den Rest) | SEG121.C |
| 275 | Übertrag der Subtraktion fehlt im High-Word der Titellänge | egal | nein | alles | ja | 32 Bit rechnen, der Uebertrag geht ins High-Word | SEG001.C |
| 276 | NVF-Typ 1: Versatz mit nicht gelesenen Breiten und Höhen | egal | nein | alles | nein | unsicher, ob NVF-Typ 1 mit Index > 0 im Spiel vorkommt; residentes Modul |  |
| 277 | Rückgabewert bei RLE-Bildern nicht gesetzt | egal | nein | alles | nein | folgenlos (kein Aufrufer wertet den Wert aus); residentes Modul |  |
| 278 | Heldenbild-Abfrage reicht ein Feld zu weit (ACTION_ID_HERO_NPC + 1) | egal | nein | alles | nein | folgenlos (der Wert kommt nie vor); residentes Modul |  |
| 279 | Lesen hinter icons_bak[9] | egal | nein | alles | nein | folgenlos (hoechstens ein unnoetiges Neuzeichnen); residentes Modul |  |
| 280 | Bright-Eyes-Fix für die AE-Leiste prüft den falschen Merker | egal | nein | alles | ja | AE-Leiste ausserhalb des Kampfes im 3D-Bildschirm neu zeichnen | SEG002.C |
| 281 | Anschnitt oben vergrößert die Bildhöhe, statt sie zu verkleinern (3 Stellen) | egal | nein | alles | ja | Anschnitt oben verkleinert die Hoehe | SEG003.C SEG066.C SEG075.C |
| 282 | Wanduhr: Position läuft bis 84, Tabelle hat nur 81 Werte | egal | nein | alles | ja | Teiler 810 = HOURS(12) / 80, die Positionen bleiben in 0..80 | SEG004.C |
| 283 | „potential bug“: nur dst von g_pic_copy wird gesichert | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 284 | Zufallszahl kann 0 oder negativ werden | egal | nein | alles | ja | ohne Vorzeichen rechnen (__abs__(-32768) bleibt negativ); Startwert wie bisher | SEG007.C |
| 285 | Modus 3: Anschnitt oben rückt die Quelle um die Bildbreite statt um 320 vor | egal | nein | alles | nein | unsicher, ob der Fall im Spiel vorkommt; Assembler-Modul mit fester Laenge |  |
| 286 | Toter OPL-Code: Timer 1 startet nie, wirkungslose Befehle | egal | nein | alles | nein | kein Fix noetig (toter Code) |  |
| 287 | Leeres Tagebuch: Vergleich mit gs_diary_entries[−1] | egal | nein | alles | ja | bei leerem Tagebuch nicht mit gs_diary_entries[-1] vergleichen | SEG024.C |
| 288 | Kopierhöhe line * 7 statt (line − startline) * 7 | egal | nein | alles | ja | nur die gedruckten Zeilen kopieren | SEG024.C |
| 289 | Speichern: do-while ohne Prüfung von findfirst | egal | nein | alles | ja | nur anhaengen, wenn findfirst() etwas gefunden hat | SEG026.C |
| 290 | „when using EMS something strange happens“ | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 291 | Letzte Figur liest das Ende hinter der TAB-Tabelle | egal | nein | alles | nein | unsicher und folgenlos: read_archive_file() begrenzt auf das Dateiende, ob die letzte Figur je so geladen wird, ist offen |  |
| 292 | Stadtdatei: liest 100 Byte mehr, als g_locations_tab fasst | egal | nein | alles | ja | nur so viel lesen, wie g_locations_tab fasst | SEG028.C |
| 293 | Kein break nach dem Fund | egal | nein | alles | ja | nach dem Fund aufhoeren (Bright-Eyes-Fix) | SEG031.C |
| 294 | Bereichsprüfung clip_num > max statt >= max | egal | nein | alles | ja | gueltig sind 0..clip_num_max-1; gueltig sind 0..ani_max_num-1 | SEG036.C SEG037.C SEG044.C |
| 295 | Toter Zweig bei der Tiefe für Objekte 58..61 | egal | nein | alles | nein | toter Zweig ohne Wirkung, kein Fehler im Spiel |  |
| 296 | Dukaten-Anzeige läuft ab 32768 D über | egal | nein | alles | ja | ab 32768 Dukaten die Dukaten als long ausgeben (Format "%dD %dS %dH": ab Zeichen 2 folgt "D ...") | SEG047.C |
| 297 | Bedingung state // mode != 2 ist immer wahr | egal | nein | alles | nein | ohne Wirkung: im else-Zweig ist state immer 1 |  |
| 298 | Lesen vor g_hero_pic_posx bei Maus-x < 3 | egal | nein | alles | ja | nicht vor die Tabelle lesen (Ergebnis wie bisher, hoechstens 0) | SEG049.C |
| 299 | Kartenwerte ab 0xf0 lesen hinter die Tabelle | egal | nein | alles | ja | nicht hinter die Tabelle lesen (Ergebnis bleibt 12) | SEG066.C |
| 300 | Gespiegelte Bilder werden am Rand von der falschen Seite beschnitten | egal | nein | alles | nein | unsicher, im Spiel nicht geprueft: welcher Ausschnitt eines gespiegelten Bildes richtig ist, liesse sich nur am Bild klaeren |  |
| 301 | „not a bug, but useless“ | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 302 | Beugung der Namen: Bits mit + gesetzt, 2. und 3. Fall verwechselt | egal | nein | alles | nein | unsicher: Bright-Eyes' Umbau aendert bei Gegenstaenden ohne Plural auch Artikel im 1./4. Fall; die Abfrage == 1 in GUI_name_inflect trifft ohnehin nur den 2. Fall - ob das Original hier absichtlich so formt, ist nicht belegbar |  |
| 303 | Pronomentabelle hat 7 statt 8 Einträge | egal | nein | alles | nein | braucht einen neuen Tabelleneintrag im Datensegment; folgenlos, der einzige Aufrufer nutzt den 4. Fall |  |
| 304 | Spielstandname bearbeiten: Schreibstelle hinter dem Namen | egal | nein | alles | ja | nur die Zeichen des alten Namens ausgeben, Schreibstelle direkt dahinter | SEG097.C |
| 305 | Feldzugriff vor der Grenzprüfung (3 Stellen) | egal | nein | alles | nein | folgenlos: nur Lesezugriffe hinter dem Feld, das Ergebnis wird nie benutzt |  |
| 306 | Abfrage „WIE VIELE DER … SOLLEN …“ im falschen Fall | egal | nein | alles | ja | "WIE VIELE DER ..." verlangt den 2. Fall | SEG106.C |
| 307 | Lichtquellen-Suche auch auf leeren Heldenplätzen | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 308 | Speichermeldung nennt 5000 Byte zu wenig | egal | nein | alles | ja | der wirkliche Fehlbetrag | SEG120.C |
| 309 | Fehlgeschlagener Start von GEN.EXE: Spiel läuft ohne Puffer weiter | kritisch | nein | wichtig | ja | nach dem Aufraeumen beenden statt ohne Puffer weiterzulaufen | SEG120.C |
| 310 | sprintf mit mehr Argumenten als Platzhaltern | egal | nein | alles | nein | kein Fix noetig (ueberzaehlige Argumente sind harmlos) |  |
| 311 | Geflohener Gegner gilt als getötet (Gorah) | egal | nein | alles | nein | braeuchte eigene Texte fuer die Flucht Gorahs; die Spielwerte (Rune, Huette) bei Flucht sind Auslegungssache |  |
| 312 | Gefesselte Gegner bleiben in der Kampfliste und kämpfen beim nächsten Betreten erneut | egal | ja | alles | ja | gefesselte Gegner eines gewonnenen Kampfes gelten als besiegt (sonst schreibt | SEG032.C |
| 313 | Lagerüberfall: nur Heldenplatz 0 ist wach, eingeteilte Wachen zählen nicht | mittel | nein | wichtig | ja | beim Lagerueberfall ist die eingeteilte Wache wach (gs_camp_incident = Nummer der Wache,; erst nach dem Kampf loeschen, FIG_init_heroes() weckt die Wache; im Dungeon erst nach dem Kampf loeschen, FIG_init_heroes() weckt die Wache | SEG039.C SEG051.C SEG052.C |
| 314 | Kampf: Waffe oder Gegenstand lässt sich nur tauschen, nicht ablegen | mittel | nein | wichtig | nein | Absicht unklar: beide Menuepunkte tauschen nur mit belegten Rucksackplaetzen; ein Ablegen auf einen leeren Platz waere eine neue Bedienung |  |
| 315 | Schusslinie: bewusstlose und schlafende Figuren blockieren, Leichen nicht | egal | nein | alles | nein | Absicht unklar (Spielbalance): ob bewusstlose oder schlafende Figuren die Schusslinie freigeben sollen, ist nicht belegbar |  |
| 316 | Zerbrochene Waffe bleibt in der Hand, Computerkampf wechselt nicht | mittel | nein | wichtig | ja | keine zerbrochene Waffe nehmen; ist die Waffe zerbrochen und liegt eine heile Nahkampfwaffe im Rucksack, zu ihr greifen | SEG036.C |
| 317 | Computerkampf: Held mit 1 BP oder weniger als 3 Rest-BP bleibt untätig | egal | nein | alles | nein | KI-Entscheidung des Computerkampfs, kein eindeutiger Fehler: nach einer Bewegung mit weniger als 3 Rest-BP ist kein Angriff mehr moeglich; ob der Held trotzdem weiterziehen soll, ist nicht belegbar |  |
| 318 | Computerkampf zaubert ohne Blick auf das Ziel | egal | nein | alles | nein | KI-Schwaeche, kein eindeutiger Fehler (unsicher); eine zielabhaengige Zauberwahl waere neues Verhalten |  |
| 319 | Computerkampf zaubert auch mit Zauberwert unter −5 | mittel | nein | wichtig | ja | einen Zauber mit Zauberwert unter -5 kann der Held nicht wirken (test_spell() liefert 0, | SEG036.C |
| 320 | Gegner fliehen auch über den rechten Rand, Helden nicht | egal | nein | alles | nein | Absicht nicht belegbar: die Fluchtraender unterscheiden sich bewusst je Hintergrund (Schiff), und Helden im Computerkampf fliehen ueber dieselbe Suche ebenfalls nach rechts |  |
| 321 | Verschreckter Gegner ohne Fluchtweg tut nichts mehr | egal | nein | alles | nein | kann Absicht sein (unsicher): ein umstellter Fliehender wehrt sich nicht |  |
| 322 | Alte Nahkampfoption ohne Nachbarschaftsprüfung („Schattenboxen“) | mittel | ja | wichtig | ja | alte Nahkampfoption nur, wenn das Ziel (Kopf oder Schwanz) noch direkt daneben steht | SEG033.C |
| 323 | Schwacher Heshthot (Runin) hat nach der LE-Kürzung 0 LE | egal | nein | alles | nein | Datenfehler in MONSTER.DAT (Spieldaten), kann gewollt sein (unsicher) |  |
| 324 | Keine Waffenresistenzen für Skelette und Mumien | egal | nein | alles | nein | Designaenderung (Resistenzen nach Handbuch), keine Fehlerbehebung |  |
| 325 | Spinnendämon gilt als Tier und ist für Waffengift empfänglich | mittel | ja | wichtig | nein | is_animal steht in MONSTER.DAT (Spieldaten); ob Daemonen giftimmun sein sollen, ist eine Regelfrage (unsicher) |  |
| 326 | Waffengift: jedes Gift macht 10 × Stärke Zusatzschaden | mittel | nein | wichtig | nein | eigene Wirkungen je Gift waeren neue Regeln; welche Wirkung gedacht war, ist nicht belegbar (Spielbalance) |  |
| 327 | Piratenangriff: Ergebnis von do_fight() wird nicht ausgewertet | egal | ja | alles | teilweise | nach verlorenem oder per VERLASSEN/LADEN beendetem Kampf kein Ausruestungsbildschirm - nach verlorenem oder abgebrochenem Kampf kein Ausruestungsbildschirm; welche Folgen eine Flucht haben soll, ist offen | SEG117.C |
| 328 | Computerkampf im Endkampf ohne Meldung gesperrt | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 329 | „Alte Optionen“ fehlt im zweiten Menü eines Zuges | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 330 | Zufällige Zugreihenfolge statt Initiative | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 331 | Markt- und Hafenangebot stehen nicht im Spielstand | egal | ja | alles | nein | Neuberechnen beim Laden aenderte das Angebot gegenueber dem Original auch ohne Ausnutzen; welcher Stand gelten soll, ist offen |  |
| 332 | Öffnungszeit 8–19 Uhr gilt auch für Straßenhändler und Treborn Kolberg | egal | nein | alles | ja | Strassenhaendler und Treborn Kolberg haben keine Ladenzeiten | SEG055.C |
| 333 | Tavernengerüchte in Serske: Lesen hinter dem Dateiende | egal | nein | alles | nein | Tabelle der Geruechte im Datensegment, Datenfehler |  |
| 334 | Beleidigter Wirt wirft nicht hinaus | egal | nein | alles | nein | Absicht unklar: der Merker steuert nur den Gespraechsbeginn mit dem Wirt; Zimmer und Essen zu verweigern waere eine neue Regel ohne eigenen Text |  |
| 335 | Wein und Schnaps ohne Zechenprobe | egal | nein | alles | nein | unsicher: ob Wein/Schnaps eine Zechenprobe haben sollen, ist nicht belegt (Balance) |  |
| 336 | Essen in der Taverne verbraucht zusätzlich Proviant | egal | nein | alles | ja | erst essen, dann die Stunde vergehen lassen (sonst isst herokeeping() vorher Proviant) | SEG059.C |
| 337 | Schmied Gurthag nach dem Kellerausgang dauerhaft unbenutzbar | mittel | nein | wichtig | nein | kann gewollt sein (unsicher): Gurthag spricht nach dem Kellerausgang nicht mehr mit der Gruppe |  |
| 338 | Treborn Kolbergs Laden: Ortszeile leer oder fremd | egal | nein | alles | nein | fuer Treborns Laden gibt es keinen eigenen Ortstext; ein Sonderfall braucht einen neuen Text |  |
| 339 | Akademie: Bezahlung mit erstem gleichen Stück, auch kaputt | mittel | ja | wichtig | ja | das Stueck, das academy_get_equal_item() angeboten hat: erstes heiles Stueck dieser Art,; mit dem angebotenen (heilen) Stueck bezahlen, nicht mit dem ersten gleichen | SEG068.C |
| 340 | Informant Tiomar mit Manrin statt Brendhil eingetragen | egal | nein | alles | nein | Datenfehler in der Tabelle g_informer_tab (Datensegment) |  |
| 341 | Treborn Kolberg erscheint nach Algrids Hinweis nie, Olvir/Treborn wiederholt sich | mittel | nein | wichtig | teilweise | die Begegnung nur einmal je Reise - die Olvir/Treborn-Begegnung wiederholt sich nicht mehr auf derselben Reise; dass Algrids Hinweis (Treborn = bekannt) sein erstes Treffen verhindert, bleibt - getrennte Merker braeuchten ein neues Feld im Spielstand | SEG094.C |
| 342 | Jurge Torfinsson: „nur Thorwaler“ auch bei reinen Thorwalern | egal | nein | alles | nein | der Satz steht fest im Dialog (Spieldaten) |  |
| 343 | Taverne: Gewinn wird als unsinnige Ausgabe angezeigt | egal | nein | alles | ja | nur eine Ausgabe anzeigen, keinen Gewinn | SEG059.C |
| 344 | Akrobatik und Falschspiel je nach Ort verschieden | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 345 | Kampf auf Spezialfeldern startet erst nach Drehen oder Menü | mittel | nein | wichtig | nein | der Kampf startet beim ersten Drehen oder Menue; ihn schon beim Ankommen auszuloesen braucht einen eigenen Merker (Datensegment) fuer alle Wege (Treppe, Falltuer, Teleporter) |  |
| 346 | Ereignisfelder lösen nach Gruppenwechsel erneut aus | mittel | nein | wichtig | nein | die Vergleichsposition gs_dng_pos_bak je Gruppe zu fuehren braucht neue Felder im Spielstand |  |
| 347 | Dungeonlicht: Lichtquellen anderer Gruppen zählen mit | egal | nein | alles | ja | nur Lichtquellen der aktiven Gruppe zaehlen | SEG106.C |
| 348 | Lager in Dungeons: immer einer von vier allgemeinen Lagerkämpfen | mittel | nein | wichtig | nein | unsicher; Kaempfe je Dungeon waeren neue Daten |  |
| 349 | Transversalis im Dungeon auf Felder außerhalb des Plans | mittel | ja | wichtig | nein | welche Dungeon-Feldtypen begehbar sind, ist ohne Pruefung an den Plaenen (Totenschiff) nicht sicher; eine falsche Sperre wuerde Teleporte verhindern |  |
| 350 | Verlässt eine Teilgruppe das Totenschiff, sinkt es für die übrigen nicht mehr | mittel | ja | wichtig | nein | die Todesfalle (gs_deathtrap) ist ein einziger Wert im Spielstand; sie je Gruppe zu fuehren aendert das Spielstandformat |  |
| 351 | Speerfallgrube (Spinnenhöhle): Spielende auch für Gruppen draußen | mittel | nein | wichtig | ja | nur diese Gruppe ist verloren; gibt es noch andere Helden, geht das Spiel mit ihnen weiter | SEG079.C |
| 352 | Giftdorn-Truhe nach sechs Ladungen ohne Text | egal | nein | alles | nein | fuer die leere Giftdorn-Truhe gibt es keinen eigenen Text |  |
| 353 | Spinnenaltar: Auswahl ohne „später“, Text und Kampf wiederholen sich | egal | nein | alles | nein | den Altarzustand zu merken braucht ein neues Feld im Spielstand |  |
| 354 | Gesperrte Türen ohne Meldung, Daspota-Falltür sperrt eine Tür für immer | mittel | nein | wichtig | nein | Absicht unklar: ob der gesicherte Stein die Tuer (0,11,1) sperren soll, ist nicht belegbar; eine Meldung braucht einen neuen Text |  |
| 355 | Schatzlager nur beim ersten Betreten plünderbar (Daspota-Schatz, Zwingfeste) | mittel | nein | wichtig | nein | der Rest muesste als Truhe gespeichert werden - neue Daten im Spielstand |  |
| 356 | Zwingfeste: Anseilen erst ab fünf Seilen, kein Seil verbraucht | egal | nein | alles | nein | Absicht unklar (unsicher): feste Grenze von fuenf Seilen |  |
| 357 | Oberorken-Wasserfalle: Raum zu klein, Ertrinken nur an 5-Minuten-Marken | mittel | nein | wichtig | nein | Raumgrenzen und Zeitpruefung der Falle sind Daten- und Designfragen; die richtige Ausdehnung ist nicht belegbar |  |
| 358 | Oberorken-Wasserraum: Türen beliebig oft knacken (je 1 AP) | egal | ja | alles | nein | unsicher: wie die Tueren wieder verschlossen werden, ist nicht gefunden; Absicht nicht belegbar |  |
| 359 | Prem-Mine: AP für Deckenfelder beliebig oft | mittel | ja | wichtig | nein | die Felder einzeln zu merken braucht neue Merker im Spielstand |  |
| 360 | Orkhöhle: jedes Durchsuchen der Strohlager kostet CH −2 | egal | nein | alles | nein | einmal je Lager braeuchte einen Merker je Strohlager (Spielstand) |  |
| 361 | Runin: Drachenhort und Platinschlüssel-Auftrag unabhängig | egal | ja | alles | nein | Absicht unklar: ob Hort und Auftragsbelohnung sich ausschliessen sollen, ist nicht belegbar |  |
| 362 | Fluch der Goldmünzen (Burgruine): jeder Kampf gilt sofort als gewonnen | kritisch | ja | wichtig | nein | der Merker ist ein Byte im Spielstand ohne Gruppenbezug; die Fluchwirkung je Gruppe zu fuehren aendert das Spielstandformat, und die gemeinte Wirkung des Fluchs ist unklar |  |
| 363 | Spinnenhöhle: Teleport überspringt Feldereignisse | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 364 | Säuregruben treffen nur die ersten beiden Helden | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 365 | Reiseziel nicht getrennt je Teilgruppe | mittel | nein | wichtig | nein | braucht je Gruppe gesicherte Werte, also eine Aenderung des Spielstands |  |
| 366 | Orkhöhle: Ausgang setzt die Gruppe direkt aufs Reiseziel | mittel | ja | wichtig | nein | welcher feste Ausgangsort gemeint war, ist nicht belegbar |  |
| 367 | Weiterreisen ohne Nachtlager über unsichtbares Symbol | mittel | ja | wichtig | ja | Weiterreisen nur ueber das sichtbare 7. Symbol (guter Lagerplatz) | SEG051.C |
| 368 | Tavernenhinweis schaltet Reiseereignisse 66 und 75 ab | mittel | nein | wichtig | nein | eigene Hinweis-Merker braeuchten neue Felder im Spielstand |  |
| 369 | Nach Gorah dreht sich die Reiserichtung immer um | egal | nein | alles | nein | welche Fahrtrichtung zu Umbrik zurueckfuehrt, ist aus dem Code nicht sicher abzuleiten |  |
| 370 | Gorahs Haus nach Flucht der Helden unzugänglich | mittel | nein | wichtig | nein | unklar, was nach einer Flucht der Helden gelten soll (Gorah lebt ja noch) |  |
| 371 | Grenz- und Straßentexte ohne Richtungsprüfung | egal | nein | alles | nein | welche Fahrtrichtung die Strecke als vorwaerts fuehrt, ist aus dem Code nicht sicher abzuleiten; im Zweifel nicht |  |
| 372 | Seereise: jeder Umweg-Wert führt ins Totenschiff | mittel | ja | wichtig | ja | nur der Umweg ins Totenschiff fuehrt dorthin; bei 99 (Gruppe tot, Kampf verlassen); bei 99 ist eine andere Gruppe aktiv, ihr Ort bleibt (wie bei der Landreise, seg093) | SEG063.C |
| 373 | Floß ohne Werkzeug baubar | egal | nein | alles | nein | unsicher, kann Absicht sein |  |
| 374 | Berghütte: Jagd zweimal möglich | egal | ja | alles | nein | do_wildcamp() setzt die Status selbst zurueck; sie zu uebernehmen braeuchte einen neuen Zustand (Datensegment) oder Eingriff in seg051 |  |
| 375 | Jagd: Einzelheld schafft immer den Blattschuss | egal | ja | alles | ja | mindestens die Haelfte (aufgerundet), ein Einzelheld muss treffen | SEG117.C |
| 376 | Gorah-Versteck ohne Weiterreise | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 377 | Gorahs Truhe: immer der erste Held öffnet | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 378 | Nachtrast immer bis 8 Uhr | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 379 | Axxeleratus: zweite Aktion wiederholt den Angriff ohne Zielwahl | mittel | nein | wichtig | nein | Absicht unklar (Spielbalance): Axxeleratus gibt eine zweite Aktion; ob sie neue BP und eine neue Wahl bekommen sollte, ist nicht belegbar - ein neues Menue mit neuen BP waere eine Regelaenderung |  |
| 380 | Böser Blick setzt dauerhaft zwei Attacken | mittel | nein | wichtig | nein | unklar, ob die zwei Attacken gewollt sind; zuruecksetzen ginge nur an allen Stellen, die renegade loeschen, und der Ausgangswert ist im Gegnerblatt nicht gespeichert |  |
| 381 | Böser Blick auf Leichen möglich | egal | nein | alles | nein | folgenlos (tote Gegner kaempfen nicht); welche Meldung/Kosten richtig waeren, ist offen |  |
| 382 | Große Gier ist nicht umgesetzt | mittel | nein | wichtig | ja | der Gegner verlaesst den Kampfplatz (Loesungsbuch), wie beim Horriphobus ueber die Flucht; Kosten 8 AE aus der Zaubertabelle. (belegt durch das Loesungsbuch ("Gegner verlaesst den Kampfplatz", 8 AE): Gegner flieht wie beim Horriphobus, ohne eigene Meldung (Version 1.1)) | SEG099.C |
| 383 | Transversalis auf der Landreise kostet AE ohne Wirkung | mittel | nein | wichtig | nein | unsicher: nicht geprueft, ob gs_town_id auf der Reise noch die Abreisestadt haelt |  |
| 384 | Herberge: Regenerationssperre nach Magie hängt am Heldenplatz | egal | ja | alles | nein | ein Merker am Helden braeuchte ein Feld in der Heldenstruktur (Spielstand); die Sperre selbst ist gewollt |  |
| 385 | Curian zaubert im Kampf nur im Computerkampf mit Magie | mittel | nein | wichtig | ja | ein NSC (Curian) zaubert auch im normalen Kampf, nicht nur im Computerkampf mit Magie | SEG036.C |
| 386 | Analüs erkennt nur 28 Gegenstände, Grimring nicht | egal | nein | alles | nein | braucht neue Eintraege und Texte (Spieldaten/Datensegment) |  |
| 387 | Horriphobus: verschreckter Gegner gilt nicht als besiegt | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 388 | Zauberproben als Summenprobe (Böser Blick gelingt oft) | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 389 | Kampfzauber außerhalb des Kampfes wählbar | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 390 | Unzerbrechliche Waffen können zerbrechen (Held gegen Held, Curians Stab) | egal | nein | alles | teilweise | unzerbrechliche Waffe (bf -99) des Angegriffenen bleibt unzerbrechlich - Paraden Held gegen Held pruefen jetzt bf -99; Curians Stab mit bf 5 steht in CURIAN.NPC (Spieldaten) und bleibt | SEG042.C |
| 391 | Magier dürfen den verfluchten Lederhelm tragen | egal | nein | alles | nein | Datenfehler (Sperrliste im Datensegment) |  |
| 392 | Zwei Kriegsbeile mit verschiedenen Werten | egal | nein | alles | nein | unsicher; Waffentabelle im Datensegment |  |
| 393 | Steinschleuder ist unbenutzbar | mittel | nein | wichtig | nein | es gibt keinen Munitionsgegenstand (Id 999 passt nicht in ein Byte); Bright-Eyes macht die Schleuder als Feature-Mod ohne Munition benutzbar - eine Designentscheidung, keine Fehlerbehebung |  |
| 394 | Wegwerfen: höchstens 99 Stück, Mengenabfrage auch bei einem Stück | egal | nein | alles | ja | bei einem einzelnen Stueck nicht nach der Menge fragen | SEG105.C |
| 395 | Übergabe auf vollen 99er-Stapel: ohne Meldung nichts | egal | nein | alles | nein | eine passende Meldung braeuchte einen neuen Text |  |
| 396 | Kräuterfund bei vollem Rucksack verloren | egal | nein | alles | nein | die Weitergabe an die Gruppe aenderte den Ablauf (Wegwerf-Abfrage); geringer Nutzen |  |
| 397 | Tausch zwischen Helden prüft nur die Traglast des Empfängers | egal | ja | alles | ja | beim Tausch auch die Traglast des Gebenden pruefen, wenn er dadurch schwerer wird | SEG106.C |
| 398 | Zweihänder von rechter in linke Hand: falsche Meldung | egal | nein | alles | ja | Tausch zwischen den Haenden kann keine Zweihandkollision ergeben | SEG106.C |
| 399 | Schild mit Zauberstab oder Hexenbesen kombinierbar | egal | nein | alles | nein | unsicher, ob Stab/Besen mit Schild gewollt verboten sein sollen (Stab ist als Speer gefuehrt) |  |
| 400 | Waffengift über BENUTZEN umgeht die Klassensperre | egal | nein | alles | ja | Klassensperre (z. B. Krieger und Gifte) auch beim Benutzen pruefen | SEG107.C |
| 401 | Entlassene Helden hinterlassen Lücken in den Heldenplätzen | kritisch | nein | wichtig | ja | die Heldenplaetze wieder lueckenlos sortieren (viele Stellen setzen das voraus) | SEG061.C |
| 402 | Aufgesparte Zaubersteigerungen laufen über 127 | mittel | ja | wichtig | ja | aufgesparte Zaubersteigerungen hoechstens 127 (signed char); hoechstens 127 (signed char) | SEG050.C |
| 403 | Stufenaufstieg erst ab 101 statt 100 AP | egal | nein | alles | ja | Aufstieg schon bei genau der geforderten AP-Zahl | SEG002.C |
| 404 | Elixiere stapeln sich ohne Obergrenze | mittel | ja | wichtig | ja | laufende Elixierwirkung (+5) auf derselben Eigenschaft des Helden suchen, -1 wenn keine; wirkt schon ein Elixier auf diese Eigenschaft, nur seine Dauer erneuern (kein weiteres +5) (eine laufende Elixierwirkung auf dieselbe Eigenschaft wird nur erneuert (Meldung bleibt)) | SEG108.C |
| 405 | Angriffsart bleibt nach dem Kampf im Helden stehen | egal | nein | alles | ja | Angriffsart (aggressiv/vorsichtig) nicht in den naechsten Kampf mitnehmen | SEG032.C |
| 406 | Rüstungsbehinderung wirkt auf keine Talentprobe | mittel | nein | wichtig | nein | unsicher; Behinderung bei Talentproben waere neue Spielregel (welche Talente, wie viel), nicht belegt |  |
| 407 | Gruppengeld: Rest der Teilung verfällt | egal | nein | alles | ja | der NSC allein bekommt den Betrag, nicht dazu; der Rest der Teilung geht an den ersten Helden | SEG002.C |
| 408 | Zaubersteigerungen bei Nichtmagiern („Waldelf-Jäger-Fehler“) | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
| 409 | Namensbeugung: Eigennamen mit Artikel, Adjektive ungebeugt | egal | nein | alles | nein | Grenze des Grammatiksystems; braucht neue Tabellen im Datensegment |  |
| 410 | Kampfmusik läuft nach direkt gestarteten Dungeonkämpfen weiter | egal | nein | alles | ja | im Dungeon nach jedem Kampf wieder die Dungeonmusik (bisher nur in DNG_fight()) | SEG032.C |
| 411 | Umlaute im Spielstandnamen ergeben „Diskette voll“ | egal | nein | alles | nein | aendert die Dateinamen von Spielstaenden mit Umlaut im Namen; was das Original dabei schreibt, haengt von Bytes vor der ctype-Tabelle ab - vorhandene Spielstaende koennten nicht mehr gefunden werden |  |
| 412 | Namenlose Tage im Tagebuch als −5. bis −1. Swafnir | egal | nein | alles | nein | die Formatstrings des Tagebuchs (Datensegment) verlangen Tag und Monatsnamen; eine eigene Zeile fuer die Namenlosen Tage braucht einen neuen Text |  |
| 413 | Zeitvorspulen ohne Bremse | egal | nein | alles | nein | keine Fehlfunktion des Spiels, haengt vom Rechner bzw. den DOSBox-Zyklen ab |  |
| 414 | Einhorn: Held als Zeiger im Spielstand | mittel | nein | wichtig | nein | unsicher; das Spielstandfeld haelt einen Zeiger - ein anderer Inhalt im selben Feld vertruege sich nicht mit vorhandenen Spielstaenden |  |
| 415 | Schiffsreise mit Händlermusik | egal | nein | alles | nein | es gibt keine eigene Reisemusik; welches Stueck gemeint war, ist nicht belegbar |  |
| 416 | Zufallswurf vor der Erledigt-Prüfung | egal | nein | alles | nein | folgenlos: do_fight() prueft selbst, ob der Kampf erledigt ist |  |
| 417 | Textfenster schließen bei jeder Taste | egal | nein | - | kein Fehler | laut Raster kein Fehler |  |
