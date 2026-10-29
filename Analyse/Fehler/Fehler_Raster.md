# Fehlerraster BLADEM.EXE 3.02 – Originalfehler eingeordnet

Stand 20.10.2026. Grundlage: 310 eigene Einträge aus `Originalfehler.md` plus 107 aus dem Netz (Nr. 311–417), jeder an der Stelle im Quelltext geprüft. Die Einzelbewertung steht in `Fehler_Raster.tsv` (eine Zeile je Nummer, Spalte `netz` = Nummern der Netzmeldungen). Der Quelltext ist unverändert.

## Matrix

| | kritisch | mittel | egal | Summe |
|---|---:|---:|---:|---:|
| **sicher** | 18 | 115 | 204 | 337 |
| **unsicher** | 7 | 32 | 41 | 80 |
| **Summe** | 25 | 147 | 245 | 417 |

- **Quelle:** 132 neu, 161 nur Bright-Eyes, 17 beide, 107 Netz. Kombinationen stehen in der Spalte `quelle` („… + Netz“).
- **Im Netz bekannt:** 109 unserer 310 Einträge, 201 nur bei uns. Von den 132 eigenen Neufunden kannte das Netz 20, 112 kannte vorher niemand.
- **Kein Fehler:** 36 Einträge (alle in „egal“ gezählt): 6, 23, 75, 103, 118, 125, 144, 154, 176, 219, 228, 236, 239, 241, 247, 257, 267, 272, 283, 290, 301, 307, 328, 329, 330, 344, 363, 364, 376, 377, 378, 387, 388, 389, 408, 417.
- **Exploits:** 47 Einträge: 29, 34, 43, 59, 65, 66, 67, 72, 87, 90, 91, 110, 117, 126, 136, 142, 143, 159, 162, 166, 170, 191, 199, 207, 222, 248, 312, 322, 325, 327, 331, 339, 349, 350, 358, 359, 361, 362, 366, 367, 372, 374, 375, 384, 397, 402, 404.
- **Bereiche:** Kampf 58, Stadt 54, Dungeon 68, Reise 73, Magie 39, Gegenstände 25, Helden 55, Technik 45.
- **Patch a) (alles):** 381 Nummern mit Fix. **Patch b) (kritisch + mittel):** 172 Nummern.
- Die Auswahl „Wichtigste Fehler je Feld“ unten deckt Nr. 1–310 ab, die Netzeinträge stehen im Abschnitt „Aus dem Netz“.

## Wichtigste Fehler je Feld

### sicher · kritisch (15)

| Nr. | Stelle | Kern | So stellt man es nach |
|---:|---|---|---|
| 22 | seg037.c FIG_select_mspell | Übergelaufener Priester (Stufe 2) hängt das Spiel auf. Wird ein Priester der Stufe 2 mit Böser Blick auf die Seite der Helden gezogen, bleibt das Spiel in seinem Zug hängen. | Böser Blick (Held) auf einen Priester Stufe 2 mit AE ≥ 5 |
| 25 | seg038.c FIG_find_path_to_target_backtrack | Fliehender Doppelgegner: Wegsuche hängt. Kämpfe mit zweiteiligen Gegnern (Drachen, Schlangen) können einfrieren, wenn ein solcher Gegner flieht. | Kampf gegen einen Höhlendrachen bis zu dessen Flucht |
| 40 | seg025.c do_house | Einbruch: „noch jemand frei?“ zählt die gerade erwischte Gruppe mit. Sitzen alle anderen Gruppen schon im Kerker und wird auch die letzte freie Gruppe beim Einbruch erwischt, hängt das Spiel. | Gruppe teilen, Gruppe B beim Einbruch erwischen lassen, in derselben Nacht mit Gruppe A ebenfalls einbrechen und erwischt werden |
| 67 | seg068.c THO_ugdalf | Zwingfeste: bei jedem Betreten 20 Dukaten. Solange der Ugdalf-Auftrag läuft, bekommt die Gruppe bei jedem Betreten der Zwingfeste erneut 20 Dukaten. | Auftrag von Ugdalf annehmen, Zwingfeste mehrmals betreten und verlassen |
| 80 | seg002.c check_group u. a. | Nur noch der NSC handlungsfähig: das Spiel läuft weiter. Werden in der Verfallenen Herberge alle außer dem NSC versteinert (oder verschwinden bzw. sterben sie an der Steilwand), läuft das Spiel mit dem NSC allein weiter. Gruppenwechsel und Zufallsheld hängen dann. | Verfallene Herberge, Versteinerungsfalle mit hoher NSC-MR (Curian + roter Ring), danach Gruppe wechseln |
| 137 | seg064.c get_passage_travel_hours u. a. | Schiffstempo in 16 Bit, Schnellsegler mit −106: Tempo 0 bringt das Spiel zum Absturz. Je nach Wetter wird eine Schnellsegler-Passage mit absurder Dauer angezeigt, beim Buchen stürzt das Spiel mit Division durch 0 ab. Andere Schiffe fahren mit falschem Tempo. | Hafen mit Schnellsegler, Wetter 6/2 (Zufall), Passage buchen |
| 164 | seg113.c tevent_098 | Waten durch den Bach mit uninitialisiertem Helden. Beim Durchwaten des Bachs kann zufälliger Speicher verändert werden oder das Spiel hängen. | Reiseereignis 98, Waten wählen |
| 180 | seg117.c octopus_attack | Krakenmolch: Endlosschleife, wenn alle Helden über Bord oder tot sind. Werden alle Helden über Bord gerissen oder sterben, hängt das Spiel. | – |
| 184 | seg118.c tevent_078 u. a. | Verirrt im Wald: alle sieben Heldenplätze sterben. Findet eine Teilgruppe nach fünf Tagen nicht aus dem Wald, sterben auch alle Helden der anderen Gruppen, das Spiel ist aus. | Gruppe teilen, mit einer Teilgruppe auf dem Waldweg (Ereignis 78) fünfmal die Orientierung verfehlen |
| 196 | seg099.c spell_skelettarius u. a. | Skelettarius belegt bei jedem Wirken 1288 Byte neu im Kampfobjektpuffer. Wird Skelettarius in einem Kampf oft gewirkt, läuft der Grafikpuffer über, das Spiel kann abstürzen. | Kampf mit vielen Leichen, wiederholt Skelettarius |
| 214 | items_dat.cpp ITEMS.DAT (magisches Bastardschwert) u. a. | Magisches Bastardschwert ist als „benutzbar“ markiert. Wer das magische Bastardschwert über BENUTZEN verwendet, bringt das Spiel zum Absturz. | Magisches Bastardschwert im Inventar, Menü BENUTZEN |
| 221 | seg105.c lose_random_item | Zufälliger Gegenstandsverlust: Endlosschleife ohne ablegbaren Gegenstand. Hat ein Held keinen ablegbaren Gegenstand und trifft ihn ein Verlust (z. B. Bad in der Furt), hängt das Spiel. | Held ohne Gegenstände (oder nur persönliche), Furt durchqueren und baden gehen |
| 235 | seg002.c get_random_hero | Zufallsheld aus Plätzen 0..n−1 statt aus den Gruppenmitgliedern. Der NSC wird nie als Zufallsheld getroffen. Lebt in der Gruppe nur noch der NSC, hängt das Spiel bei der nächsten Falle. | Nur der NSC lebt noch, eine Falle mit Zufallsopfer auslösen |
| 244 | seg049.c GRP_switch_to_next | Gruppenwechsel ohne Ausgang, wenn keine Gruppe frei handeln kann. Sind alle Gruppen im Kerker oder handlungsunfähig, hängt das Spiel nach Ereignissen, im Menü erscheint die Meldung endlos. | – |
| 248 | seg050.c level_up | Stufenanstieg: ESC bei „WELCHE POSITIVE EIGENSCHAFT“ bringt rund 168 000 Dukaten. Bricht man beim Stufenanstieg die Wahl der zu steigernden Eigenschaft mit ESC oder rechter Maustaste ab, bekommt der Held etwa 168 427 Dukaten und keine Eigenschaft. | Held mit Stufenaufstieg, bei der Frage nach der positiven Eigenschaft rechte Maustaste |

### unsicher · kritisch (5)

| Nr. | Stelle | Kern | So stellt man es nach |
|---:|---|---|---|
| 7 | seg032.c FIG_do_round | Heldenzug ohne verbliebene Heldenphasen: Endlosschleife möglich. Unter seltenen Umständen bleibt der Kampf hängen, wenn die Helden keine Aktion mehr haben, die Gegner aber schon. | – |
| 15 | seg035.c FIG_loot_enemies | Bei genau 30 Beutestücken schreibt „MEHR“ über einen nicht gesetzten Zeiger. Bei sehr viel Beute (genau 30 Stück) kann das Blättern auf Seite 2 Speicher zerstören oder abstürzen. | Kampf mit 30 Beutestücken, Seite 2 aufrufen |
| 30 | seg041.c FIG_get_hero_weapon_attack_damage | Angstgift: Schreiben über nicht gesetzten Zeiger bei Held gegen Held. Schlägt ein verhexter Held mit Angstgift-Waffe einen anderen Helden, wird zufälliger Speicher verändert. | – |
| 174 | seg116.c TLK_old_woman | Zustand 33 ist eine Endlosschleife. Erreicht das Gespräch mit der alten Frau je Zustand 33, hängt das Spiel. | – |
| 309 | seg120.c call_gen | Fehlgeschlagener Start von GEN.EXE: Spiel läuft ohne Puffer weiter. Lässt sich die Heldenerschaffung nicht starten, läuft das Spiel ohne globalen Puffer, Maus und Timer weiter und stürzt vermutlich ab. | – |

### sicher · mittel (90)

Auswahl: ausnutzbar oder mit klarem Weg zum Nachstellen (32 von 90).

| Nr. | Stelle | Kern | So stellt man es nach |
|---:|---|---|---|
| 3 | monster_dat.cpp MONSTER.DAT (MR-Würfel) u. a. | MR von 19 Gegnertypen wird mit 15W4 statt 1W6 ausgewürfelt. Zombies, Piraten-Steuermann und -Kapitän, Folterknecht, Krieger, Höhlendrache, Zwergenveteran, Orkchampion u. a. haben eine riesige Magieresistenz, Beherrschungs- und Schadenszauber prallen fast immer ab. | Gegen einen Zombie einen Zauber mit MR-Probe sprechen |
| 18 | seg036.c AFIG_can_attack_neighbour u. a. | Computerkampf: Helden greifen auch übergelaufene Gegner an. Im automatischen Kampf schlagen die Helden auch auf Gegner ein, die durch Böser Blick auf ihrer Seite kämpfen. | Böser Blick auf einen Gegner, dann Computerkampf |
| 34 | seg042.c FIG_do_hero_action u. a. | Wurfwaffe wird vor der Schadensberechnung abgelegt. Wurfwaffen treffen immer, ohne Probe, und machen nur 1W6 wie waffenlos (außer Wurfsterne im Stapel). | Wurfbeil werfen und den Kampfbericht ansehen |
| 39 | seg002.c timers_daily u. a. | Akademie identifiziert nur ein einziges Mal im Spiel. Nach der ersten Identifizierung in der Akademie heißt es „VERSUCHT ES MORGEN“ für immer. | In der Akademie zweimal einen Gegenstand identifizieren lassen, auch Tage später |
| 43 | seg029.c draw_status_line | Statusleiste setzt 6 statt 4 Werte je Held, beim NSC wird die Krankheitstabelle überschrieben. Mit NSC in der Gruppe zahlt der Heiler für die Heilung von Wundfieber 100 Heller drauf, und die Talentprobe auf Wundfieber wird leicht. | NSC in der Gruppe, Held mit Wundfieber zum Heiler bringen |
| 48 | seg054.c talk_inn | Brauender NSC bleibt in der Herberge stecken. Lässt man Curian in einer Herberge brauen, findet die Gruppe ihn dort nicht wieder, und seine Gruppe lässt sich nicht anwählen. Der NSC ist verloren. | Curian in einer Herberge ein Rezept brauen lassen, Gruppe verlassen und wiederkommen |
| 53 | seg055.c TLK_khandel u. a. | „Besonders guter Preis“ beim Händler verschlechtert den Verkauf. Nach dem erfolgreichen Händlergespräch („ICH MACHE EUCH EINEN BESONDERS GUTEN PREIS“) bekommt man beim Verkaufen nur 75 %, beim Kaufen ändert sich nichts. | Händler-Gespräch bis Zustand 11 (KHANDEL) bzw. 25 (WHANDEL), dann verkaufen |
| 54 | seg056.c buy_screen | Einkaufskorb: Platzbedarf falsch berechnet. Der Händler meldet „DIE GRUPPE KANN KEINE WEITEREN GEGENSTÄNDE AUFNEHMEN“, obwohl Platz ist, oder lässt zu viel in den Korb. | 50 Pfeile in den Korb, dann eine Waffe wählen |
| 59 | seg060.c TLK_tavern u. a. | Taverne im Gespräch: Runde und Essen werden in Heller statt Silber abgezogen. Im Tavernengespräch kosten die Lokalrunde und das Essen nur ein Zehntel des genannten Preises. | Taverne, Gespräch, Lokalrunde ausgeben |
| 65 | seg067.c city_event_6 u. a. | Straßenhändler überschreibt den Typ des gerade betretenen Gebäudes. Erscheint der Straßenhändler beim Betreten eines Gebäudes, ist das Gebäude danach verdorben: ein Tempel wird zum Praiostempel, eine Taverne kann negative Essenspreise haben. | – |
| 68 | seg068.c THO_academy | Akademie erkennt einen Fluch beim Helden auf Platz 0 nicht. Ist der erste Held verflucht, sagt Aranda „KEINER VON EUCH HAT EINEN FLUCH AUF SICH“. | Held auf Platz 0 verfluchen lassen, Akademie Thorwal |
| 72 | seg070.c PHX_spielhaus | Spielhaus: Gewinn und Verlust vertauscht. Im Spielhaus von Phexcaer verliert die Gruppe, wenn die Mehrheit beim Falschspiel Erfolg hat, und gewinnt, wenn sie versagt. | Gruppe mit schlechtem Falschspiel im Spielhaus |
| 87 | seg076.c DNG_door u. a. | Dietrich-Zähler wird nach dem ersten Ebenenwechsel ständig zurückgesetzt. Nach dem ersten Treppenwechsel kann man ein Schloss beliebig oft knacken, ohne dass die Dietriche beim 4. Versuch brechen. | Einmal die Ebene wechseln, dann vor einer Tür mehrfach Schlösser knacken |
| 91 | seg076.c DNG_waterbarrel | Leeres Wasserfass füllt trotzdem alle Schläuche. Aus einem leeren Fass kann man beliebig oft alle Wasserschläuche füllen, „DAS FASS IST NUN LEER“ kommt jedes Mal. | Fass leer trinken, dann „VORRÄTE AUFFRISCHEN“ |
| 110 | seg081.c DNG06_handler u. a. | Zwei-Gruppen-Rätsel prüfen nur x/y der anderen Gruppe. Eine zweite Gruppe, die irgendwo anders (andere Ebene, anderer Dungeon, Stadt) auf denselben Koordinaten steht, gilt als „am anderen Hebel“. | Zweite Gruppe in einer Stadt auf (3,6) parken, dann im Dungeon 6 den Hebel an (1,1,2) ziehen |
| 117 | seg085.c DNG10_handler | Opferschale: ESC schenkt der Gruppe 1 Silberstück. Bricht man die Eingabe an der Opferschale mit ESC ab, bekommt die Gruppe 10 Heller, beliebig oft. | Dungeon 10, Statue mit Opferschale, Eingabe mit ESC, wiederholen |
| 126 | seg105.c drop_item | Opfer für Efferd und Ingerimm zählen auch ohne Wegwerfen. An den Opferstellen (Piratenhöhle, Zwergenfeste) gilt ein Gegenstand als geopfert, auch wenn man „WIRKLICH WEGWERFEN?“ verneint. | Piratenhöhle (9,9), Dreizack wegwerfen, mit Nein antworten |
| 136 | seg064.c print_passage_price | Passagepreis in 16 Bit: lange Routen werden spottbillig. Auf langen Routen kostet eine Karracke oder Luxuspassage nur einen Bruchteil, z. B. 80 statt 735 Heller. | Prem, Karracke nach Hjalsingor |
| 139 | seg093.c g_harbors_linked_sea_routes_10 u. a. | Hafen Hjalsingor: Route nach Overthorn fehlt in der Liste. Nach der Schiffsreise von Overthorn nach Hjalsingor steht die Gruppe beim Verlassen des Hafens auf einem falschen Feld hinter den Häusern. | Schiff Overthorn → Hjalsingor, Hafen verlassen |
| 140 | seg093.c g_harbors | Hafen Leuchtturm Runin verweist auf eine Landweg-Liste. Nach der Schiffsreise zum Leuchtturm Runin steht die Gruppe auf einem entlegenen Feld des Ortes. | Schiff zum Leuchtturm Runin, Hafen verlassen |
| 142 | seg094.c trv_do_journey u. a. | Reiseereignisse: Lage wird nach Richtungswechsel falsch korrigiert. Einmalige Reiseereignisse lassen sich durch Umkehren wiederholen, gefährliche (z. B. Sümpfe mit Umkehr-Option) durch erneutes Umkehren umgehen, wiederholbare fallen nach dem Umkehren aus. | Weg mit Sumpf-Ereignis, Umkehren wählen, gleich wieder umkehren |
| 159 | seg111.c tevent_064 | Gorahs Hütte: Silentium-Auswahl wird nie wiederholt. Wählt man „Silentium“ mit einem Nicht-Magier oder misslingt der Zauber, nähert sich die Gruppe ohne Probe und ohne Hinterhalt, während ein gelungener Silentium den Hinterhalt auslöst. | Bei Gorahs Hütte „Silentium“ wählen und einen Krieger nehmen |
| 162 | seg112.c tevent_074 u. a. | Wegzoll 15 Dukaten insgesamt statt pro Person. Der Wegelagerer verlangt 15 Dukaten pro Person, die Gruppe zahlt aber nur 15 Dukaten insgesamt. | – |
| 166 | seg113.c tevent_099 u. a. | Harpyien: Fluchtoption ist kein Versuch. „VERSUCHEN ZU FLIEHEN?“ gelingt immer ohne Probe, die Kriegskunst-Probe kommt erst nach einem verlorenen Kampf. | Reiseereignis 99 oder 101, Flucht wählen |
| 170 | seg114.c tevent_113 | Firun-Opfer: Schlafbonus bleibt für spätere Lager stehen. Wer am Firun-Schrein opfert, schläft auch in späteren Wildnislagern besser (+1 bis +4), bis ein Rastplatz den Wert löscht. | – |
| 189 | seg098.c use_magic | Stabzauber: Abfrage „keine Stabzauber mehr“ auf Stufe 7 statt 4. Nach dem 4. Stabzauber versucht der Magier immer weiter, den 5. zu wirken: die Probe misslingt stets, jeder Versuch kostet halbe AE und 2 Stunden. | Magier mit 4 Stabzaubern, „Stabzauber“ wählen |
| 191 | seg098.c use_spell u. a. | Foramen aus dem Zaubermenü ignoriert den Türzuschlag. Wird Foramen frei aus dem Zaubermenü gesprochen, ist die Probe für jede Tür gleich leicht. | Verschlossene Tür mit hohem Foramen-Zuschlag, Foramen über das Zaubermenü |
| 199 | seg099.c spell_hexenspeichel | Hexenspeichel: AE-Prüfung greift nie. Mit Hexenspeichel kann eine Hexe mehr heilen, als ihre AE erlauben. | Hexe mit wenig AE, Hexenspeichel mit großer Menge |
| 207 | seg101.c spell_armatrutz u. a. | Armatrutz: ein RS-Punkt mehr, als AE vorhanden. Armatrutz bietet eine Stufe mehr an, als die AE bezahlen können, bei Gegnern ebenso. | Held mit 10 AE, Armatrutz mit Bonus 4 |
| 225 | seg107.c item_ignite u. a. | Laterne nachfüllen: Öl beim letzten Zauberer gesucht, beim Benutzer entfernt. Füllt ein Held seine Laterne nach und hat zuletzt ein anderer Held gezaubert, verschwindet beim Benutzer ein falscher Gegenstand. | Held A zaubert, Held B füllt seine Laterne nach |
| 243 | seg049.c GRP_split u. a. | Gruppe teilen klappt nicht richtig, wenn der NSC in einer anderen Gruppe ist. Zwei Helden lassen sich nicht trennen, und man kann nicht alle bis auf einen abspalten, wenn der NSC woanders steht. | NSC in eine eigene Gruppe, dann eine Zweiergruppe teilen |
| 250 | seg050.c level_up u. a. | MR-Neuberechnung beim Stufenanstieg verträgt sich nicht mit MR-Gegenständen. Mit Stirnreif, rotem Ring oder Amulett erscheint die MR-Meldung grundlos, und ein roter Ring in der rechten Hand kostet nach dem Ablegen dauerhaft 2 MR. | Roter Ring in die rechte Hand, Stufenaufstieg, Ring ablegen |

Übrige nach Bereich: **Kampf:** 2, 36; **Stadt:** 49, 50, 51, 52, 76; **Dungeon:** 79, 84, 88, 94, 97, 102, 105, 114, 119, 121, 122; **Reise:** 127, 149, 150, 151, 152, 153, 158, 161, 165, 167, 172, 177, 178, 179, 181, 185; **Magie:** 190, 192, 197, 211, 212, 213; **Gegenstände:** 218, 220; **Helden:** 229, 232, 233, 234, 240, 249, 253, 254, 255, 256, 259, 264, 269, 270, 273, 274

### unsicher · mittel (21)

| Nr. | Stelle | Kern | So stellt man es nach |
|---:|---|---|---|
| 1 | datseg.c g_weapon_stats_table u. a. | Speer wird als Fernkampfwaffe behandelt. Wer im Nahkampf mit dem Speer zusticht, macht nur Schaden, wenn eine Probe auf Schusswaffen gelingt. Geworfen wird der Speer abgelegt und trifft wie waffenlos. | – |
| 11 | seg033.c FIG_menu | Zauberziel: begehbare Objekte (Id ≥ 50) werden nicht abgewiesen. Ein Zauber auf ein begehbares Deko-Objekt wird angenommen und wirkt auf Speicher hinter der Gegnerliste. | – |
| 21 | seg037.c FIG_search_range_target | Gegner schießen auch auf direkt benachbarte Helden. Gegner mit Bogen oder Fernzauber schießen auch Helden auf dem Nachbarfeld an. | – |
| 37 | seg102.c mspell_plumbumbarum | Gegner-Plumbumbarum wirkt eine Stunde statt 5 Kampfrunden. Trifft ein Gegner einen Helden mit Plumbumbarum, bleibt dessen AT noch lange nach dem Kampf um 3 gesenkt. | – |
| 38 | seg116.c tevent_144 | Endkampf: ohne Grimring kämpft Heldenplatz 0. Legt man Grimring im Statusmenü vor dem Endkampf ab, wird der Held auf Platz 0 zum Hauptakteur, auch wenn er nicht dabei oder kampfunfähig ist. | – |
| 45 | seg031.c do_random_talk | Nicht vorhandene Antworten werden mitgemischt. In Zufallsgesprächen kann eine alte oder leere Antwort sichtbar werden, die in den falschen Zustand führt. | – |
| 55 | seg056.c buy_screen | Volle Korbsumme wird bezahlt, auch wenn nicht alles verteilt werden konnte. Passt nicht alles in die Rucksäcke, zahlt die Gruppe trotzdem den ganzen Korb. | – |
| 58 | seg060.c TLK_tavern | Tavernenmerker firstinfo, infopromise, round werden nie zurückgesetzt. Nach der ersten Information in irgendeiner Taverne verlaufen spätere Tavernengespräche dauerhaft anders. | – |
| 63 | seg062.c ask_miracle | Firun-Wunder „keinen Hunger und Durst mehr“ wirkt nur auf einen Helden. Der Text spricht alle an, satt bleibt aber nur ein zufälliger Held. | – |
| 66 | seg068.c THO_arsenal | Zeughaus: Störrebrandt-Schulden werden aus dem Staatsbudget bezahlt. Wer bei Störrebrandt Schulden hat, zahlt im Zeughaus 50 D davon mit dem Geld des Hetmanns. Der Rest des Budgets wird auf ganze Dukaten abgerundet. | – |
| 90 | seg076.c DNG_step | Rückwärtsschritt prüft halbdurchlässige Wände nicht. Einbahn-Wände lassen sich rückwärts gehend immer durchqueren. | – |
| 106 | seg079.c DNG03_handler u. a. | Hebel und Geheimtür verlangen Betreten UND Drehen im selben Schritt. Der Hebel an (0,9,11) in der Spinnenhöhle und die Geheimtür an (4,8) im Orkdungeon sind praktisch nicht zu benutzen bzw. zu finden. | – |
| 111 | seg081.c DNG09_pitfall u. a. | Schleifen über 7 Plätze beginnen beim ersten verfügbaren Helden statt bei Platz 0. Ist Platz 0 nicht handlungsfähig, werden Helden übersehen und Daten hinter dem Heldenfeld gelesen und verändert, z. B. MU +3 durch das Elixier, Essen am Kessel. | – |
| 120 | seg087.c DNG14_handler u. a. | Zeitabfragen „day_timer % X == 0“ greifen mehrmals hintereinander. Im Giftgasraum gibt es vermutlich mehr als 1 LE Schaden je 10 Minuten, die einsturzgefährdeten Stollen der Zwergenmine stürzen schneller ein. | – |
| 124 | seg090.c DNG12_handler u. a. | Texte 31 und 32 fehlen in der deutschen OBER.DTX. Ab dem dritten Sprung über die Speergrube und beim Durchbrechen erscheint ein falscher oder unsinniger Text. | – |
| 131 | seg063.c g_sea_routes | Seeweg Prem–Manrin nur 54 km lang. Die längste Seeroute der Karte ist im Spiel eine der kürzesten. | – |
| 134 | seg063.c sea_travel | Efferds Schutz verhindert den Krakenmolch, nicht die Piraten. Trotz „EFFERD GEWÄHRT EUCH SEINEN SCHUTZ AUF WASSER“ greifen Piraten an. | – |
| 143 | seg094.c trv_do_journey | Gewaltmarsch-Tempo hängt von der schon gegangenen Strecke ab. Wer den Gewaltmarsch spät auf einem Weg beginnt, ist um ein Vielfaches schneller. | Langer Weg, kurz vor dem Ziel Gewaltmarsch wählen |
| 222 | seg106.c move_item | Tausch zweier Körperplätze ohne Prüfung und Boni. Tauscht man zwei angelegte Gegenstände direkt, landet der verdrängte ohne Prüfung am anderen Platz, Rüstungsschutz und Boni können verrutschen. | – |
| 238 | seg047.c hero_gets_poisoned | Erneute Vergiftung löscht die Protokollbytes. Wird ein bereits vergifteter Held erneut mit demselben Gift vergiftet, bleiben die schon erlittenen Verluste dauerhaft. | – |
| 262 | seg104.c talent_cure_disease | Heilen Krankheiten: Wundfieber und Blaue Keuche nie heilbar, Erfolg wird als Misserfolg gemeldet. Gelingt die Probe auf Wundfieber oder Blaue Keuche, meldet das Spiel trotzdem „DAS HEILEN MISSLINGT“ mit 3 Schaden. | – |

### sicher · egal (150)

- **Echte, aber folgenlose oder kosmetische Fehler (134):** **Kampf:** 4, 5, 8, 9, 10, 12, 13, 14, 16, 17, 19, 20, 26, 27, 28, 29, 31, 32; **Stadt:** 41, 42, 44, 46, 47, 56, 57, 60, 61, 62, 64, 69, 70, 71, 73, 77, 78; **Dungeon:** 81, 82, 83, 85, 86, 89, 93, 95, 96, 98, 99, 100, 101, 104, 107, 108, 109, 112, 113, 115, 123; **Reise:** 128, 129, 130, 132, 133, 135, 138, 145, 147, 148, 155, 156, 157, 160, 163, 168, 171, 175, 182, 183; **Magie:** 187, 193, 194, 195, 198, 200, 201, 202, 203, 204, 206, 208, 210; **Gegenstände:** 215, 217, 223, 226, 227; **Helden:** 230, 231, 237, 242, 245, 246, 251, 252, 258, 260, 261, 263, 265, 266, 268; **Technik:** 275, 277, 278, 279, 280, 281, 282, 284, 286, 287, 288, 289, 292, 293, 294, 295, 296, 297, 298, 299, 303, 305, 306, 308, 310
- **Kein Fehler (16):** 103 („VORSICHT NACH DREI SCHRITT LINKS“ – müsste es „rechts“ heißen?), 118 („possibly a bug: position (8,12) might be (8,11)“), 125 (Deckeneinsturz (schwer): mit Werkzeug 18 statt 12 Stunden?), 144 (Gewaltmarsch zieht den LE-Verlust auf einmal ab), 146 (Axt-Liste für Reiseereignisse), 154 („Original-Glitch“: gs_travel_detour = 1 ist zugleich die Nummer des Totenschiffs), 176 (Speer als Jagdwaffe), 228 („What about herokeeping()?“ im stündlichen Timer), 236 (Rote Anzeige des bleibenden LE-Verlusts (Bright-Eyes-Komfort)), 239 („not a real BUG“: keine Prüfung auf leeren Heldenplatz), 241 (AT-Basis nicht nach DSA 3 (MU + KK + GE) / 5), 257 („Why STAFFSPELL?? BUG!“ – staffspell_timer als Regenerationssperre), 267 („Original-Bug? forgot? time_counter = 0“), 272 („Not a bug, but useless“: Markierung sex = 50), 301 („not a bug, but useless“), 307 (Lichtquellen-Suche auch auf leeren Heldenplätzen)

### unsicher · egal (29)

| Nr. | Stelle | Kern |
|---:|---|---|
| 6 | seg032.c FIG_choose_next_enemy | „Orig_BUG: I had this loop running infinitely“. Nichts nachweisbar. |
| 23 | seg037.c FIG_enemy_turn | „Why unset 'tied' and not 'illusion'?“. Nichts, ein gefesselter Gegner kommt mit 4 % je Zug frei. |
| 24 | seg038.c FIG_find_path_to_target_backtrack | best_viewdir möglicherweise uninitialisiert. Nichts Beobachtetes. |
| 33 | seg042.c FIG_do_hero_action | Waffenbruch bei gleichen Würfen trifft nie die Waffe eines Gegners. Gegnerwaffen gehen nie kaputt, eine unzerbrechliche Angreiferwaffe schützt auch die Waffe des Verteidigers. |
| 35 | seg043.c FIG_do_enemy_action | double_size bleibt beim Rückverwandeln eines Doppelgegners ungesetzt. Vermutlich nichts. |
| 74 | seg071.c PHX_phextempel | Phextempel: gs_ptemple_closed wird nie gesetzt. Nach der Beleidigung der Geweihten passiert nichts, der verschlossene Betraum (Text 60) erscheint nie. |
| 75 | seg072.c INF_treborn_unicorn u. a. | „this would reactivate algrid“. Nichts Fehlerhaftes: nach Treborns Hinweis kann man Algrid erneut ansprechen. |
| 92 | seg077.c DNG01_handler | Totenschiff-Spalt: nur Seil oder Strickleiter, kein Magierstab. Am Spalt im Totenschiff hilft der verwandelte Magierstab nicht, anders als an der Grube. |
| 116 | seg085.c DNG10_handler | Trittplatte wirkt nur beim Entschärfen. Unentdeckt oder in Ruhe gelassen tut die Trittplatte nichts, nur ein misslungenes Entschärfen löst die Bolzen aus. |
| 141 | seg093.c g_tevents_tab | Ork-Monolith (Ereignis 71) steht auch auf dem Weg Oberorken–Felsteyn. Auf dem Weg Oberorken–Felsteyn kann der Monolith „AM ANDEREN UFER DES THOSSEL“ erscheinen, nur einmal im Spiel. |
| 169 | seg113.c tevent_107 | „NACH GUT ACHT STUNDEN“, vergehen aber 12. Text und verstrichene Zeit passen nicht zusammen. |
| 173 | seg116.c tevent_137 | Einsiedlersee: nur der erste Wasserschlauch wird gefüllt. Am Seeufer wird nur ein Wasserschlauch je Held gefüllt. |
| 186 | seg032.c do_fight | Visibili verhindert den Kampf nur bei einem Einzelhelden. Sind alle Helden einer Gruppe mit Visibili unsichtbar, kommt es trotzdem zum Kampf. |
| 188 | seg098.c use_magic | Meditation bei LE 6 oder 7 kostet LE und bringt keine AE. Ein Magier mit 6 oder 7 LE verliert beim Meditieren noch LE bis 5, ohne AE zu bekommen. |
| 205 | seg100.c spell_eisenrost | Eisenrost auf ein Tier kostet volle AE. „TIERE HABEN KEINE WAFFEN“, die AE sind trotzdem weg. |
| 209 | seg101.c spell_salander | Salander auf zweiteilige Gegner: „strange effect“. Ein in einen Pilz verwandelter Drache sieht vermutlich seltsam aus (Schwanzteil bleibt). |
| 216 | seg092.c loot_chest | Truhenliste: 20 Byte je Name zu kurz. Lange Gegenstandsnamen laufen in den nächsten Eintrag, am Listenende über das Feld. |
| 219 | seg105.c unequip u. a. | Kraftgürtel aktualisiert abhängige Werte nicht. Nichts erkennbar. |
| 224 | seg107.c item_apply_weapon_poison | Waffengift: Schleuder wird nicht ausgeschlossen. Gift lässt sich auf eine Schleuder auftragen. |
| 247 | seg049.c GRP_hero_sleep | Brauen endet nur ohne Gift, Krankheit und Stabzaubersperre. Ein kranker oder vergifteter Held wird mit dem Brauen erst fertig, wenn er gesund ist. |
| 271 | seg119.c disease_effect u. a. | Zeitzähler von Krankheiten und Giften können überlaufen. Bei sehr langen Krankheiten (z. B. dem nie endenden Schlachtfeldfieber) wird der Zähler nach 127 Tagen negativ. |
| 276 | seg002.c process_nvf | NVF-Typ 1: Versatz mit nicht gelesenen Breiten und Höhen. Nichts, solange kein Bild vom Typ 1 mit Index > 0 geladen wird. |
| 283 | seg006.c FIG_draw_figures u. a. | „potential bug“: nur dst von g_pic_copy wird gesichert. Nichts erkennbar. |
| 285 | seg008.asm pic_copy (Modus 3) | Modus 3: Anschnitt oben rückt die Quelle um die Bildbreite statt um 320 vor. Vermutlich nichts, sonst ein verschobener Bildausschnitt im Kampfbildschirm. |
| 290 | seg027.c load_fight_figs | „when using EMS something strange happens“. Nichts nachweisbar. |
| 291 | seg027.c load_fight_figs | Letzte Figur liest das Ende hinter der TAB-Tabelle. Nichts, das Bild wird trotzdem geladen. |
| 300 | seg075.c DNG_draw_image | Gespiegelte Bilder werden am Rand von der falschen Seite beschnitten. Angeschnittene, gespiegelte Dungeonbilder zeigen vermutlich den falschen Ausschnitt. |
| 302 | seg096.c GUI_name_inflect_with_article u. a. | Beugung der Namen: Bits mit + gesetzt, 2. und 3. Fall verwechselt. Bei manchen Gegenständen und Gegnern stimmen Artikel oder Genitiv-S im 3. Fall nicht. |
| 304 | seg097.c GUI_enter_string | Spielstandname bearbeiten: Schreibstelle hinter dem Namen. Hängt man an einen vorhandenen Spielstandnamen Zeichen an, gehen sie verloren, solange man nicht erst die Rücktaste drückt. |

## Bright-Eyes-Markierungen: wo wir widersprechen

Von 178 Einträgen mit Bright-Eyes-Beteiligung bestätigen wir 148. Bei 30 widersprechen wir ganz oder teilweise:

| Nr. | Bright-Eyes | Urteil | Begründung |
|---:|---|---|---|
| 138 | Bright-Eyes (35): Angezeigte Reisedauer nur in groben Stufen | **Fix falsch** | Erst wird das Tempo auf km/h gerundet, dann geteilt. Die Wertelisten im Bright-Eyes-Kommentar gelten aber nur mit 32 Bit und Tempo 150, im Original bestimmen Überlauf und −106 die Werte (Nr. 137). Der Bright-Eyes-Fix ROUNDED_DIVISION(10 * distance, speed2) teilt bei Tempo 0 durch 0. |
| 151 | Bright-Eyes (47): Furt: GE-Probe gilt fast immer als bestanden | **Fix falsch** | Misserfolg nur bei test_attrib() == 0. test_attrib() = GE − (W20 + Zuschlag) + 1, also nur bei genau einem Wurf. Die 95 % gelten für GE − Zuschlag 0..18 (Bright-Eyes schreibt ≤ 19). Der Fix „< 0“ lässt das Ergebnis 0 (knapp misslungen) weiter gelingen. |
| 268 | Bright-Eyes (48): Blaue Keuche: „DAS LEBEN WIRD WOHL NIE WIEDER SEIN“ immer | **Fix falsch** | Der Befund stimmt. Der Fix nutzt aber log_3, das nur den vorübergehenden Abzug (AT/PA −4, KK/GE halbiert) markiert und schon am ersten Krankheitstag gesetzt wird. Der dauerhafte Verlust entsteht nur im 25-%-Zweig. Nach Heilung durch Heiler, Talent oder Kraut kommt die Meldung auch mit Fix. |
| 280 | Bright-Eyes (ohne Nr.): Bright-Eyes-Fix für die AE-Leiste prüft den falschen Merker | **Fix falsch** | Der Originalfehler (AE-Leiste fehlt außerhalb des Kampfes) stimmt. Der Fix fragt aber g_mouse1_doubleclick ab, das mit dem Bildschirmmodus nichts zu tun hat. Gemeint ist g_pp20_index == ARCHIVE_FILE_PLAYM_UK. |
| 179 | Bright-Eyes (21): Krakenmolch: GE-Probe falsch ausgewertet | **Begründung falsch** | Würgen nur bei test_attrib() == 0, Packen nur bei −1. Bright-Eyes schreibt „für GE <= 19 immer 100 %“, richtig ist GE >= 19. Der Fix (−99 = Packen, <= 0 = Würgen) stimmt. |
| 190 | Bright-Eyes (17): Stabzauber-Probe prüft die zweite Eigenschaft doppelt | **Begründung falsch** | Übergeben werden attrib1, attrib2, attrib2. Bright-Eyes schrieb „die erste wird doppelt geprüft, die zweite fehlt“, richtig ist: die zweite doppelt, die dritte fehlt. Der Fix (attrib3) stimmt. |
| 6 | Bright-Eyes (ohne Nr.): „Orig_BUG: I had this loop running infinitely“ | **kein Fehler** | Die Schleife endet, solange die Summe der attacks_left dem Zähler in FIG_do_round() entspricht. Skelettarius setzt attacks_left, Böser Blick ändert nur attacks. Eine Ursache für die beobachtete Endlosschleife ist im Code nicht zu finden. |
| 23 | Bright-Eyes (Frage ohne Nr.): „Why unset 'tied' and not 'illusion'?“ | **kein Fehler** | Die Deutung „Illusion verschwindet“ stammte aus einem falschen Kommentar. Das Lösen der Fessel (Band und Fessel) ist als Befreiungschance plausibel. |
| 75 | Bright-Eyes (ohne Nr., 2 Stellen): „this would reactivate algrid“ | **kein Fehler** | Algrids Dialog prüft gs_algrid_treborns_hint in Zustand 3 und 4. Das Zurücksetzen auf 1 macht gerade dieses Gespräch möglich, vermutlich gewollt. |
| 103 | Bright-Eyes (Frage ohne Nr., 2 Stellen): „VORSICHT NACH DREI SCHRITT LINKS“ – müsste es „rechts“ heißen? | **kein Fehler** | Blick nach Norden auf (12,5): links ist Westen, zur Geheimtür (8,4). Blick nach Süden auf (4,5): links ist Osten, ebenfalls zu x = 8. Beide Inschriften zeigen richtig, nach drei Schritten folgt das Feld der Tür. |
| 116 | Bright-Eyes (Frage ohne Nr.): Trittplatte wirkt nur beim Entschärfen | **kein Fehler** | Text 13 fragt nach dem Entschärfen, der Schaden folgt nur dort. Seltsam, aber als Absicht lesbar. |
| 118 | Bright-Eyes (Vermutung ohne Nr.): „possibly a bug: position (8,12) might be (8,11)“ | **kein Fehler** | Von (8,12) nach Norden blickt die Gruppe auf die Geheimtür (8,11), die geöffnet wird. Die beiden anderen Türen funktionieren genauso. |
| 125 | Bright-Eyes (18): Deckeneinsturz (schwer): mit Werkzeug 18 statt 12 Stunden? | **kein Fehler** | Fehlgelesen: has_items == 0 ergibt 18 h, mit Werkzeug 12 h. Der Bright-Eyes-Fix dreht es falsch herum und erzeugt erst den Fehler. Der echte Fehler dieser Art steckt eine Funktion weiter oben (Nr. 122). |
| 144 | Bright-Eyes (Hinweis ohne Nr.): Gewaltmarsch zieht den LE-Verlust auf einmal ab | **kein Fehler** | Bright-Eyes nennt es „brutal und nah an einem Fehler“. Es ist eine Designentscheidung. |
| 146 | Bright-Eyes (33): Axt-Liste für Reiseereignisse | **kein Fehler** | Designentscheidung von Attic, kein Programmfehler. Der Bright-Eyes-Fix ändert die Spielbalance. |
| 154 | Bright-Eyes (Hinweis ohne Nr., 2 Stellen): „Original-Glitch“: gs_travel_detour = 1 ist zugleich die Nummer des Totenschiffs | **kein Fehler** | gs_travel_detour != 0 beendet nur trv_do_journey(), danach startet do_travel_mode() die Querverbindung. Ein Dungeon wird nicht betreten. |
| 176 | Bright-Eyes (60): Speer als Jagdwaffe | **kein Fehler** | Jagd mit dem Speer ist plausibel. Bright-Eyes nimmt ihn im Rahmen von 60 heraus, für die Jagd ist das eine Designänderung, kein Fehlerfix. |
| 219 | Bright-Eyes (TODO ohne Nr., 2 Stellen): Kraftgürtel aktualisiert abhängige Werte nicht | **kein Fehler** | Der Gürtel ändert attrib[KK].current. update_atpa() rechnet mit .normal, AT/PA-Grundwert hängt also nie vom Gürtel ab. Werte wie die Tragkraft lesen .current ohnehin direkt. |
| 228 | Bright-Eyes (Frage ohne Nr.): „What about herokeeping()?“ im stündlichen Timer | **kein Fehler** | herokeeping() läuft bei jeder Eingabeabfrage. magical_chainmail_damage() setzt stündlich g_herokeeping_flag, das löst dort Essen und Trinken aus. Der stündliche Ablauf ist also vollständig. |
| 236 | Bright-Eyes (ohne Nr.): Rote Anzeige des bleibenden LE-Verlusts (Bright-Eyes-Komfort) | **kein Fehler** | Kein Fehler, sondern eine Komfortfunktion von Bright-Eyes (Formatstring auf %s umbiegen, Wert rot). |
| 239 | Bright-Eyes (ohne Nr.): „not a real BUG“: keine Prüfung auf leeren Heldenplatz | **kein Fehler** | Bright-Eyes nennt es selbst keinen echten Fehler. Ein leerer Platz bekommt höchstens Krankheitswerte, die nie gelesen werden. |
| 241 | Bright-Eyes (ohne Nr.): AT-Basis nicht nach DSA 3 (MU + KK + GE) / 5 | **kein Fehler** | DSA 1 kennt nur einen AT/PA-Grundwert (IN + KK + GE) / 5. Das ist eine Regelabweichung zu DSA 3, kein Programmfehler. |
| 247 | Bright-Eyes (Frage ohne Nr.): Brauen endet nur ohne Gift, Krankheit und Stabzaubersperre | **kein Fehler** | Die Bedingungen sind gewollt streng (staffspell_timer wirkt als Regenerationssperre). Ein Fehler ist nicht zu erkennen. |
| 257 | Bright-Eyes (TODO ohne Nr., 2 Stellen): „Why STAFFSPELL?? BUG!“ – staffspell_timer als Regenerationssperre | **kein Fehler** | staffspell_timer wird nur in GRP_hero_sleep() gelesen und wirkt dort als Sperre der Regeneration. heal_timer ist schon gesetzt. Vermutlich Absicht, nur der Name ist irreführend. |
| 267 | Bright-Eyes (Fragen ohne Nr., 16 Stellen): „Original-Bug? forgot? time_counter = 0“ | **kein Fehler** | Bei den Krankheiten setzt der Genesungsblock direkt darunter time_counter = 0. Bei den Giften setzt hero_gets_poisoned() den Zähler beim nächsten Vergiften auf 0, im Zustand HEALTHY wird nicht gezählt. |
| 272 | Bright-Eyes (ohne Nr.): „Not a bug, but useless“: Markierung sex = 50 | **kein Fehler** | Bright-Eyes nennt es selbst keinen Fehler. Der Umweg über sex = 50 findet den Helden nach dem Gruppenwechsel (der die Plätze umsortiert) wieder. |
| 283 | Bright-Eyes (Vermutung ohne Nr.): „potential bug“: nur dst von g_pic_copy wird gesichert | **kein Fehler** | Gesichert wird nur der Zielzeiger (bzw. zusätzlich das Rechteck). Die Aufrufer von do_pic_copy() setzen x1..y2 und src vor jeder Benutzung neu, ein Fehlbild ist nicht nachweisbar. |
| 290 | Bright-Eyes (ohne Nr.): „when using EMS something strange happens“ | **kein Fehler** | Im Code ist kein Grund für ein Fehlverhalten mit EMS zu erkennen. Die Behauptung ist unbelegt. |
| 301 | Bright-Eyes (ohne Nr.): „not a bug, but useless“ | **kein Fehler** | signpost_typeindex = 1 und gleich danach die Abfrage darauf: überflüssiger Code ohne Wirkung. |
| 307 | Bright-Eyes (Fix ohne Nr.): Lichtquellen-Suche auch auf leeren Heldenplätzen | **kein Fehler** | Leere Plätze sind genullt (remove_npc, Entlassen), eine brennende Fackel findet sich dort nicht. |

Außerdem präzisieren wir bei vielen bestätigten Markierungen die Folge (Spalte `begruendung` in der TSV), z. B. Nr. 235, Nr. 43, Nr. 137, Nr. 152, Nr. 78.

## Hinweise für die Patches

- **Bright-Eyes-Fix nicht übernehmen:** Nr. 125 (18, dreht die Zeiten falsch herum), Nr. 176 (Speer als Jagdwaffe), Nr. 146 (Axt-Liste, Geschmackssache). **Fix ändern:** Nr. 151 (47: `<= 0` statt `< 0`), Nr. 138 (35: Tempo 0 abfangen), Nr. 268 (48: eigener Merker statt log_3), Nr. 280 (AE-Leiste: Bildschirmmodus prüfen).
- **Kritische zuerst (Absturz, Hänger, großer Exploit):** Nr. 7, 15, 22, 25, 30, 40, 67, 80, 137, 164, 174, 180, 184, 196, 214, 221, 235, 244, 248, 309. Mehrere Hänger laufen über den Gruppenwechsel ohne Ausgang (Nr. 244) bzw. einen Helden, der nur noch der NSC ist (Nr. 80).
- **Gleiche Wurzel, gemeinsam beheben:** Nr. 137/138 (Seereise-Tempo und Anzeige), Nr. 238/240 (Gift- und Krankheitsprotokolle), Nr. 48/50/51 (Brauen in der Herberge), Nr. 54/55 (Händlerkorb und Bezahlung).
- **Spielstand:** Kein Fix ändert das Format des Spielstands. Alte Spielstände bleiben aber inhaltlich verdorben, wo der Fehler schon gewirkt hat (z. B. Nr. 248, 256, 220, 269).

## Aus dem Netz

Abgeglichen mit 328 Spielermeldungen (Crystals-DSA-Foren S1–S5, Steam S6, VOGONS S8, siehe `netz_quellen.md`), jede am Code geprüft (`netz_geprueft.tsv`). 214 Meldungen sind bestätigt oder teilweise bestätigt, alle sind jetzt einer Nummer zugeordnet.

- **Bekannt:** 109 unserer 310 Einträge stehen auch im Netz (89 von 178 Bright-Eyes-Einträgen, 20 von 132 eigenen Neufunden: Nr. 13, 18, 22, 40, 52, 68, 69, 88, 90, 102, 105, 106, 115, 117, 122, 199, 222, 250, 252, 274). 201 Einträge kannte das Netz nicht, darunter 112 eigene Funde, die vorher niemand beschrieben hatte.
- **Neu aufgenommen:** 107 Einträge (Nr. 311–417), davon 93 Fehler und 14 als „kein Fehler“ (Absicht/Design: 328, 329, 330, 344, 363, 364, 376, 377, 378, 387, 388, 389, 408, 417). Kritisch: Nr. 362, 401. Exploits: Nr. 312, 322, 325, 327, 331, 339, 349, 350, 358, 359, 361, 362, 366, 367, 372, 374, 375, 384, 397, 402, 404.
- **Wichtigste neue:** 362 Fluch der Goldmünzen: jeder Kampf gilt als gewonnen (Netz 305), 341 Treborn Kolberg erscheint nach Algrid nie (192), 401 entlassene Helden hinterlassen Lücken (49), 372 Seereise-Umweg führt ins Totenschiff (223), 359 Prem-Mine AP beliebig oft (283), 404 Elixiere ohne Obergrenze (120), 380 Böser Blick setzt dauerhaft zwei Attacken (40), 382 Große Gier wirkungslos (60), 326 Waffengift 10 × Stärke (47), 393 Steinschleuder unbenutzbar (95).
- **Widersprüche geklärt:**
  - Kraftgürtel (Netz 77/78/135/136): Der Fehler liegt in `move_item` (Nr. 222). Beim Umlegen zwischen Körperplätzen fehlt `unequip()` für den Quellplatz, `add_equip_boni()` wirkt jedes Mal. Nr. 222 jetzt **sicher, kritisch, Exploit**. Nr. 219 (atpa_base) bleibt kein Fehler, Netz 137 entsprechend „bestätigt, aber kein Fehler“.
  - Nr. 146 Axt-Liste (Netz 93): von „kein Fehler“ auf **Fehler mittel (unsicher)**, Orknasen sind WEAPON_TYPE_AXT.
  - Nr. 116 Trittplatte Runin (Netz 292): von „kein Fehler“ auf **Fehler egal (unsicher)**.
  - Nr. 307 Lichtsuche (Netz 157): bleibt kein Fehler für leere Plätze, der echte Fehler (andere Gruppen, Netz 156) ist neu Nr. 347.
  - Nr. 75 Algrid (Netz 195): bleibt kein Fehler, Netzurteil angeglichen.
  - Schwere angehoben: Nr. 117 Opferschale auf kritisch (16-Bit-Überlauf, Netz 295), Nr. 260 Brauen/7. Gruppe auf kritisch (Netz 146), Nr. 92 Bilge auf mittel (Netz 94). Nr. 39 um den Fluch-Merker ergänzt (Netz 185).
  - Netzurteil korrigiert: Netz 148 (kein Hänger, Vorprüfung Text 332), Netz 154 (doch Hänger, Nr. 40 kritisch), Netz 58 und 280 („nicht im Code“, aber = Nr. 3 und 124, Datenfehler).
- **Widerlegt (32):** 15 NSC greift im rondrianischen Endkampf ein (oder nicht), 17 Grimring bei Curian: Gepäckbildschirm vor Endkampf nicht verlassbar, 48 Kukris-Dolch wirkt nur bei einem Treffer, 53 Zombie-AP uneinheitlich, 62 Balsam im Kampf wirkungslos, 70 Transversalis nicht über unerkundete Felder, 73 Scharfes Auge: Fernkampfbonus nicht angezeigt, 82 Daspota-Orknase nicht als magisch markiert, 84 Silberner Stirnreif ohne MR-Wirkung, 89 Magisches Kettenhemd senkt AT/PA zusätzlich, 127 Gift aus Gorahs Truhe nicht heilbar, 128 Vergiftete Truhe im Namenlosengewölbe tötet bei Tastendruck, 131 Abgetrennter Held erhält AP (Daspota-Schatz), 133 MU-Senkung an der Wolfshöhle, 158 Heiler/Tempel: Geldüberlauf, unendlich Geld, 161 Stoerrebrandt-Bank: Zahlenüberlauf, 164 Sinkender Ankaufspreis für Goldschmuck, 167 Diebstahl im Charakterbildschirm ohne Folgen, 177 Diskette: Helden verlassen Taverne immer betrunken, 217 Firun-Tempel ohne Speichern/Laden, 228 Jagd/Kräutersuche beim eigenen Lager unmöglich, 237 Proviant/Schläuche verloren (Vilnheim–Phexcaer), 240 Beerdigung erhöht Ansehen bei Hesinde statt Boron, 251 Totenschiff: Heshthot-Truhe Absturz ohne Goldschlüssel, 253 Totenschiff/Spinnenhöhle: Fluch blockiert Spiel, 254 Spinnenhöhle (Diskette): Schrittzähler läuft in anderen Dungeons weiter, 272 Dietrich-Logik, 288 Rast-Icon fehlt vor Truhe/Tür, 297 Wiederkehrende Mumienkämpfe (Runin / Tempel des Namenlosen), 299 Heshthot in Runin flieht, 300 Verfallene Herberge: Odem Arcanum-Probe, 319 AP-Abzug beim Speichern außerhalb von Tempeln.
- **Nicht im Code oder ungeklärt (80):** Daten, Diskettenfassung, Emulator oder nicht nachstellbar, nicht aufgenommen. Tippfehler in Daten (Netz 264, 270, 309) und nicht spielrelevante is_magic-Werte (80, 85) ohne eigenen Eintrag.
