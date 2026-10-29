# Schicksalsklinge: Fehler im Original

**Das Schwarze Auge 1: Die Schicksalsklinge** (Attic, 1992), deutsche CD-Fassung 3.02. Alle Fehler, die bei der vollständigen Rekonstruktion des Quelltexts gefunden wurden. Jeder Eintrag ist am Quelltext geprüft. Stand 20.10.2026, MooMoo mit Claude.

**Spoiler-Warnung:** Die Liste nennt Orte, Ereignisse und Lösungen aus dem ganzen Spiel.

## Übersicht

381 Fehler, eingeordnet nach zwei Fragen: *Ist das sicher ein Fehler oder vielleicht Absicht?* und *Wie stark wirkt er sich im Spiel aus?*

| | kritisch | mittel | egal |
|---|---:|---:|---:|
| **sicher falsch** | 18 | 115 | 175 |
| **unsicher falsch** | 7 | 32 | 34 |

| Begriff | Bedeutung |
|---|---|
| sicher falsch | Der Code widerspricht sich selbst oder seiner offensichtlichen Absicht |
| unsicher | Könnte Absicht von Attic sein (Spielbalance, Design). Die Begründung steht dabei |
| kritisch | Absturz, Speicher überschrieben, Spielstand oder Ausrüstung verloren, Spieler betrogen oder plötzlich reich, Quest blockiert |
| mittel | spürbar falsches Spielverhalten: Text erscheint nie, Ereignis falsch, Werte falsch |
| egal | ohne sichtbare Folge, unerreichbar, toter Code |
| Im Netz bekannt | *ja* = in den durchgesehenen Sammlungen gemeldet (erste Meldung mit Link), *nein* = dort nicht gefunden |
| Exploit | Fehler, der dem Spieler nützt und gezielt ausnutzbar ist (47 Stück, siehe Cheat-Liste) |

**Im Netz bekannt:** 198 ja, 183 nein. Durchgesehen wurden vor allem die Crystals-DSA-Foren-Threads „Fehler und Bugs“ (2006–2025) und „Umfassender Bugfix-Patch“ von NRS (2016–2026) sowie einige Steam- und VOGONS-Threads (Liste im Anhang). Dazu kommen die Markierungen im Bright-Eyes-Projekt, die alle am Code nachgeprüft wurden.

36 Einträge haben sich bei der Prüfung als Absicht oder als kein Fehler erwiesen. Sie stehen am Ende der Liste.

## Cheat-Liste: Exploits

Fehler, die dem Spieler nützen. Was im Spiel passiert und warum. Schritt-für-Schritt-Anleitungen zum Nachstellen folgen mit den Tests im Spiel.

### Nr. 29: Geworfene Waffen werden nur als Gegenstandsnummer gemerkt

*Kampf · sicher, egal*

Nach dem Kampf bekommt man geworfene Waffen als neue Stücke zurück: zerbrochene sind wieder heil, Gift und „magisch erkannt“ sind weg.

**Warum:** g_fig_dropped_weapons speichert nur item_id, give_new_item_to_group() legt frische Gegenstände an. Auch der Werfer geht verloren.

**Im Netz bekannt:** nein

### Nr. 34: Wurfwaffe wird vor der Schadensberechnung abgelegt

*Kampf · sicher, mittel*

Wurfwaffen treffen immer, ohne Probe, und machen nur 1W6 wie waffenlos (außer Wurfsterne im Stapel).

**Warum:** range_attack_check_ammo(hero, 0) entfernt die Waffe aus der Hand, danach rechnet FIG_get_hero_weapon_attack_damage() mit leerer Hand.

**Im Netz bekannt:** ja: „Wurfwaffen werden als waffenloser Kampf gewürfelt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=30)

### Nr. 43: Statusleiste setzt 6 statt 4 Werte je Held, beim NSC wird die Krankheitstabelle überschrieben

*Städte & Orte · sicher, mittel*

Mit NSC in der Gruppe zahlt der Heiler für die Heilung von Wundfieber 100 Heller drauf, und die Talentprobe auf Wundfieber wird leicht.

**Warum:** for (j < 6) auf g_char_status_bars[i][4]. Für Platz 6 landen [6][4], [6][5] in g_disease_strength[0], [1] = −1. Heilerpreis = −1 * 10 Silber, money −= −100 Heller. Heilen Krankheiten auf Wundfieber hat Zuschlag −1 statt 6.

**Im Netz bekannt:** ja: „Heiler verlangt negativen Betrag (Wundfieber)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=17)

### Nr. 59: Taverne im Gespräch: Runde und Essen werden in Heller statt Silber abgezogen

*Städte & Orte · sicher, mittel*

Im Tavernengespräch kosten die Lokalrunde und das Essen nur ein Zehntel des genannten Preises.

**Warum:** Lokalrunde: Text 18/31 nennt 20 * crowdedness SILBERTALER, geprüft und abgezogen werden 20 * crowdedness Heller (Zustände 24 bis 40). Essen (Zustand 130): dieselbe Formel wie do_tavern() in seg059, dort Silber (* 10), hier als Heller abgezogen.

**Im Netz bekannt:** nein

### Nr. 65: Straßenhändler überschreibt den Typ des gerade betretenen Gebäudes

*Städte & Orte · sicher, mittel*

Erscheint der Straßenhändler beim Betreten eines Gebäudes, ist das Gebäude danach verdorben: ein Tempel wird zum Praiostempel, eine Taverne kann negative Essenspreise haben.

**Warum:** town_step() ruft city_event_switch() nach enter_location(). city_event_6 setzt gs_town_typeindex auf den Straßenhändler und stellt ihn nicht zurück. Typ 93 steht in keiner Tempelliste, also Praios. Chance etwa 1:900 je Betreten (8 bis 20 Uhr, Thorwal, Prem, Phexcaer, Oberorken).

**Im Netz bekannt:** nein

### Nr. 66: Zeughaus: Störrebrandt-Schulden werden aus dem Staatsbudget bezahlt

*Städte & Orte · unsicher, mittel*

Wer bei Störrebrandt Schulden hat, zahlt im Zeughaus 50 D davon mit dem Geld des Hetmanns. Der Rest des Budgets wird auf ganze Dukaten abgerundet.

**Warum:** Das Budget ersetzt während do_merchant() das Gruppengeld. Die Schuldenabfrage für Thorwal ab 100 D greift dort weiter und zieht 50 D ab.

**Im Netz bekannt:** nein

### Nr. 67: Zwingfeste: bei jedem Betreten 20 Dukaten

*Städte & Orte · sicher, kritisch*

Solange der Ugdalf-Auftrag läuft, bekommt die Gruppe bei jedem Betreten der Zwingfeste erneut 20 Dukaten.

**Warum:** gs_quest_ugdalf bleibt 1, bis das Verlies den Auftrag löst. Mit 2 (Fix) greift weiterhin !gs_dng14_ugdalf_done für den Zugang, das Geld aber nur einmal.

**Im Netz bekannt:** ja: „Zwingfeste: Dramosch-Vorauszahlung mehrfach“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=12)

### Nr. 72: Spielhaus: Gewinn und Verlust vertauscht

*Städte & Orte · sicher, mittel*

Im Spielhaus von Phexcaer verliert die Gruppe, wenn die Mehrheit beim Falschspiel Erfolg hat, und gewinnt, wenn sie versagt.

**Warum:** if ((count >> 1) < num_passed) → Verlust. Bright-Eyes kehrt die Bedingung ohne Erklärung um, der alte Kommentar beschrieb das Verhalten mit Fix.

**Im Netz bekannt:** ja: „Phexcaer-Spielhaus: Falschspiel-Ergebnis verkehrt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=29)

### Nr. 87: Dietrich-Zähler wird nach dem ersten Ebenenwechsel ständig zurückgesetzt

*Dungeons · sicher, mittel*

Nach dem ersten Treppenwechsel kann man ein Schloss beliebig oft knacken, ohne dass die Dietriche beim 4. Versuch brechen.

**Warum:** g_dng_level_changed wird in seg075 auf 1 gesetzt, nie auf 0. DNG_step() setzt dann bei jedem Aufruf g_lockpick_try_counter = 0. Der Vermerk „Why is this a global variable?“ ist eine Designfrage, wirksam wird erst der nie gelöschte Merker.

**Im Netz bekannt:** nein

### Nr. 90: Rückwärtsschritt prüft halbdurchlässige Wände nicht

*Dungeons · unsicher, mittel*

Einbahn-Wände lassen sich rückwärts gehend immer durchqueren.

**Warum:** Vorwärts wird bei DNG_TILE_SEMIPERMEABLE_WALL das Richtungsflag geprüft, rückwärts fehlt der Fall. Ob die Dungeonkarten solche Wände haben, ist nicht geprüft (seg085 setzt eine).

**Im Netz bekannt:** ja: „Einseitige Geheimtüren/Illusionswände rückwärts passierbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)

### Nr. 91: Leeres Wasserfass füllt trotzdem alle Schläuche

*Dungeons · sicher, mittel*

Aus einem leeren Fass kann man beliebig oft alle Wasserschläuche füllen, „DAS FASS IST NUN LEER“ kommt jedes Mal.

**Warum:** Der Schlauch wird gefüllt, bevor der Fassinhalt geprüft wird. Bei 0 Einheiten wird trotzdem gefüllt und wieder auf 0 gesetzt. Bright-Eyes notiert nur „check the value first“.

**Im Netz bekannt:** nein

### Nr. 110: Zwei-Gruppen-Rätsel prüfen nur x/y der anderen Gruppe

*Dungeons · sicher, mittel*

Eine zweite Gruppe, die irgendwo anders (andere Ebene, anderer Dungeon, Stadt) auf denselben Koordinaten steht, gilt als „am anderen Hebel“.

**Warum:** gs_groups_x/y wird verglichen, gs_groups_dungeon_id und gs_groups_dng_level nicht. In seg088 abgeschwächt, weil zusätzlich gs_dng15_reached_hands nötig ist.

**Im Netz bekannt:** nein

### Nr. 117: Opferschale: ESC und große Eingaben (16-Bit-Überlauf) schenken Geld

*Dungeons · sicher, kritisch*

An der Opferschale bringt ESC 10 Heller. Gibt man eine große Zahl ein (z. B. 3300), läuft die Rechnung über und die Gruppe bekommt rund 3250 Dukaten, beliebig oft.

**Warum:** GUI_input() liefert bei ESC −1, answer = −10. Die Geldprüfung besteht, set_party_money(Geld − (−10)). Netz 295: GUI_input(…, 6) liefert int16, answer = Eingabe * 10 läuft in 16 Bit über, negative Werte bestehen die Geldprüfung ebenso.

**Im Netz bekannt:** ja: „Runin-Drachenhöhle: Opferstock-Überlauf“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=6)

### Nr. 126: Opfer für Efferd und Ingerimm zählen auch ohne Wegwerfen

*Dungeons · sicher, mittel*

An den Opferstellen (Piratenhöhle, Zwergenfeste) gilt ein Gegenstand als geopfert, auch wenn man „WIRKLICH WEGWERFEN?“ verneint.

**Warum:** Die Prüfungen auf Dreizack/Netz bzw. Ingerimm-Gegenstände laufen unabhängig von retval.

**Im Netz bekannt:** nein

### Nr. 136: Passagepreis in 16 Bit: lange Routen werden spottbillig

*Reise & Ereignisse · sicher, mittel*

Auf langen Routen kostet eine Karracke oder Luxuspassage nur einen Bruchteil, z. B. 80 statt 735 Heller.

**Warum:** di (unsigned int) * distance läuft über 65535. Karracke (35 H je 10 km) auf Prem–Hjalsingor (210 km) ab price_mod 90, Luxuspassage (45 H) ab 115 km.

**Im Netz bekannt:** nein

### Nr. 142: Reiseereignisse: Lage wird nach Richtungswechsel falsch korrigiert

*Reise & Ereignisse · sicher, mittel*

Einmalige Reiseereignisse lassen sich durch Umkehren wiederholen, gefährliche (z. B. Sümpfe mit Umkehr-Option) durch erneutes Umkehren umgehen, wiederholbare fallen nach dem Umkehren aus.

**Warum:** Die nachträgliche Korrektur über last_tevent_id betrifft immer das zuletzt ausgelöste Ereignis, auch nicht wiederholbare, und auch, wenn das Ereignis selbst die Richtung umdrehte.

**Im Netz bekannt:** ja: „Feste Reiseereignisse wiederholen sich“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=9)

### Nr. 143: Gewaltmarsch-Tempo hängt von der schon gegangenen Strecke ab

*Reise & Ereignisse · unsicher, mittel*

Wer den Gewaltmarsch spät auf einem Weg beginnt, ist um ein Vielfaches schneller.

**Warum:** gs_travel_speed = gs_travel_step_counter + 197. Gemeint war vermutlich ein fester Wert (197 = 1,97 km/h).

**Im Netz bekannt:** nein

### Nr. 159: Gorahs Hütte: Silentium-Auswahl wird nie wiederholt

*Reise & Ereignisse · sicher, mittel*

Wählt man „Silentium“ mit einem Nicht-Magier oder misslingt der Zauber, nähert sich die Gruppe ohne Probe und ohne Hinterhalt, während ein gelungener Silentium den Hinterhalt auslöst.

**Warum:** approach_flag wird nach der Wahl immer 1, die Schleife läuft nicht erneut. silentium_flag wird nie gesetzt, FEATURE3.LTX 102 ist unerreichbar.

**Im Netz bekannt:** nein

### Nr. 162: Wegzoll 15 Dukaten insgesamt statt pro Person

*Reise & Ereignisse · sicher, mittel*

Der Wegelagerer verlangt 15 Dukaten pro Person, die Gruppe zahlt aber nur 15 Dukaten insgesamt.

**Warum:** p_money < 1500 und p_money −= 1500, FEATURE4.LTX 50/74 nennen „FÜNFZEHN DUKATEN PRO PERSON“.

**Im Netz bekannt:** nein

### Nr. 166: Harpyien: Fluchtoption ist kein Versuch

*Reise & Ereignisse · sicher, mittel*

„VERSUCHEN ZU FLIEHEN?“ gelingt immer ohne Probe, die Kriegskunst-Probe kommt erst nach einem verlorenen Kampf.

**Warum:** Das else gehört zum inneren if (do_fight). Antwort 2 tut nichts.

**Im Netz bekannt:** ja: „Harpyien-Kampf Thoss–Rybon fehlerhaft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=3)

### Nr. 170: Firun-Opfer: Schlafbonus bleibt für spätere Lager stehen

*Reise & Ereignisse · sicher, mittel*

Wer am Firun-Schrein opfert, schläft auch in späteren Wildnislagern besser (+1 bis +4), bis ein Rastplatz den Wert löscht.

**Warum:** g_wildcamp_sleep_quality wird nach do_location() nicht auf 0 gesetzt (anders als in TRV_found_camp_place() und tevent_078).

**Im Netz bekannt:** nein

### Nr. 191: Foramen aus dem Zaubermenü ignoriert den Türzuschlag

*Magie · sicher, mittel*

Wird Foramen frei aus dem Zaubermenü gesprochen, ist die Probe für jede Tür gleich leicht.

**Warum:** Der Zuschlag foramen_handicap der Tür wird nur im Türmenü berücksichtigt.

**Im Netz bekannt:** ja: „Foramen-Erschwernis umgehbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=7)

### Nr. 199: Hexenspeichel: AE-Prüfung greift nie

*Magie · sicher, mittel*

Mit Hexenspeichel kann eine Hexe mehr heilen, als ihre AE erlauben.

**Warum:** Im zweiten Zweig wird mit g_spell_special_aecost verglichen, der noch 0 ist. Der erste Zweig prüft die AE gar nicht und vergleicht fehlende LE mit le * 2: wer mehr als die Hälfte der fehlenden LE verlangt, wird ganz geheilt.

**Im Netz bekannt:** ja: „Hexenspeichel verbraucht zu viel AE“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=11)

### Nr. 207: Armatrutz: ein RS-Punkt mehr, als AE vorhanden

*Magie · sicher, mittel*

Armatrutz bietet eine Stufe mehr an, als die AE bezahlen können, bei Gegnern ebenso.

**Warum:** while (max_bonus * max_bonus < ae) liefert die kleinste Zahl mit Quadrat ≥ AE (AE 10: Bonus 4 für 16 AE). Bei Gegnern kappt MON_sub_ae() bei 0.

**Im Netz bekannt:** ja: „Armatrutz bietet zu hohen RS-Bonus an“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=23)

### Nr. 222: Tausch zweier Körperplätze ohne Prüfung und Boni

*Gegenstände & Inventar · sicher, kritisch*

Legt man einen Gegenstand zwischen zwei Körperplätzen um (z. B. Kraftgürtel linke → rechte Hand), werden seine Boni jedes Mal neu aufgeschlagen: KK +5 je Umlegen, ebenso CH (verfluchter Helm), Gefahrensinn (Kristallkugel), bis zum Überlauf. Der Schild-Trick senkt umgekehrt den RS.

**Warum:** Nur item_id_2 wird geprüft und angelegt. item_id_1 kommt ohne add_equip_boni() an inv_slot_2, beim späteren Ablegen wird unequip() erneut abgezogen. Vermutlich RS/BE-Drift, bei verfluchten Gegenständen auch Eigenschaften. Nicht im Spiel geprüft. Netz 77/78/135/136 bestätigt das im Spiel: beim Umlegen zwischen Körperplätzen wird unequip() für den Quellplatz nicht aufgerufen, add_equip_boni() für den Zielplatz schon. Damit sicher und ein großer Exploit.

**Im Netz bekannt:** ja: „Kraftgürtel-Bug: KK +5 durch Handwechsel“ (und 3 weitere Meldungen) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=29)

### Nr. 248: Stufenanstieg: ESC bei „WELCHE POSITIVE EIGENSCHAFT“ bringt rund 168 000 Dukaten

*Helden & Werte · sicher, kritisch*

Bricht man beim Stufenanstieg die Wahl der zu steigernden Eigenschaft mit ESC oder rechter Maustaste ab, bekommt der Held etwa 168 427 Dukaten und keine Eigenschaft.

**Warum:** GUI_radio() liefert bei Abbruch −1, minus 1 ergibt l_si = −2. hero->attrib[−2] liegt 6 Byte vor attrib (Offset 0x34), also auf den oberen beiden Bytes von hero->money (0x2e/0x2f). Sind sie < 20 (fast immer), endet die Schleife und normal++/current++ erhöhen das Geld um 0x01010000 Heller.

**Im Netz bekannt:** nein

### Nr. 312: Gefesselte Gegner bleiben in der Kampfliste und kämpfen beim nächsten Betreten erneut

*Kampf · sicher, egal*

Ein mit Band und Fessel gefesselter Gegner greift beim nächsten Betreten wieder an. AP gibt es jedes Mal, Beute nur einmal.

**Warum:** Gefesselte (flags.tied) zählen nicht als aktiv, der Kampf gilt als gewonnen. FIG_tidy_enemies() entfernt aber nur tote, versteinerte und verpilzte Gegner, write_fight_lst() schreibt den Gefesselten zurück.

**Im Netz bekannt:** ja: „Gegner greift nach „Band und Fessel“ erneut an“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)

### Nr. 322: Alte Nahkampfoption ohne Nachbarschaftsprüfung („Schattenboxen“)

*Kampf · sicher, mittel*

Mit „alte Optionen“ schlägt ein Held einen Gegner, der nicht mehr neben ihm steht.

**Warum:** Für die alte Nahkampfoption wird nur tot/verschreckt geprüft, die Nachbarschaft nur im Fern- und Zauberzweig (check_hero_range_attack).

**Im Netz bekannt:** ja: „„Schattenboxen“: Angriff ohne Nachbarschaft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=25)

### Nr. 325: Spinnendämon gilt als Tier und ist für Waffengift empfänglich

*Kampf · unsicher, mittel*

Mactans lässt sich mit Sanftmut oder Herr über das Tierreich bezwingen und vergiften.

**Warum:** MONSTER_ID_SPINNENDAEMON hat is_animal = 1 (Datenfehler), Waffengift wirkt ohne Immunitätsprüfung.

**Im Netz bekannt:** ja: „Dämonen durch Gift und Tierzauber bezwingbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=14)

### Nr. 327: Piratenangriff: Ergebnis von do_fight() wird nicht ausgewertet

*Kampf · sicher, egal*

Flucht aus dem Seekampf bleibt folgenlos. Nach „Verlassen/Laden“ erscheint trotzdem der Ausrüstungsbildschirm.

**Warum:** Rückgabe von do_fight(FIGHT_ID_S001) wird ignoriert, danach immer status_menu().

**Im Netz bekannt:** ja: „Seekampf „neu laden“ zeigt Ausrüstungsbildschirm“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)

### Nr. 331: Markt- und Hafenangebot stehen nicht im Spielstand

*Städte & Orte · sicher, egal*

Laden behält das tagesaktuelle Markt- und Hafenangebot, Abfahrten und Preise lassen sich so steuern.

**Warum:** Die Daten liegen in g_-Variablen, nicht im Spielstand.

**Im Netz bekannt:** ja: „Markt-/Hafenangebot beim Laden nicht zurückgesetzt“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=28)

### Nr. 339: Akademie: Bezahlung mit erstem gleichen Stück, auch kaputt

*Städte & Orte · unsicher, mittel*

Bezahlt man mit einem Gegenstand, wird auch ein kaputtes Stück genommen, Magierstäbe sind nicht ausgenommen.

**Warum:** Auswahl ohne kaputte, Bezahlung per get_first_hero_with_item().

**Im Netz bekannt:** ja: „Schule der Hellsicht: Bezahlung mit kaputtem Gegenstand/Zauberstab“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)

### Nr. 349: Transversalis im Dungeon auf Felder außerhalb des Plans

*Dungeons · sicher, mittel*

Auf dem Totenschiff gelangt man in nicht existente Schiffsteile.

**Warum:** Abgelehnt wird nur Wand (15).

**Im Netz bekannt:** ja: „Totenschiff: Transversalis in nicht existente Schiffsteile“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)

### Nr. 350: Verlässt eine Teilgruppe das Totenschiff, sinkt es für die übrigen nicht mehr

*Dungeons · sicher, mittel*

Eine Teilgruppe draußen rettet die Gruppe an Bord vor dem Untergang.

**Warum:** leave_dungeon() setzt gs_deathtrap global auf 0 (gleiche Ursache wie Nr. 95, Bright-Eyes 30).

**Im Netz bekannt:** ja: „Totenschiff: Teilgruppen beim Untergang“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=5)

### Nr. 358: Oberorken-Wasserraum: Türen beliebig oft knacken (je 1 AP)

*Dungeons · unsicher, egal*

Wiederholtes Knacken bringt AP.

**Warum:** Knacken prüft den Wasserzustand nicht. Wiederschließen nicht gefunden.

**Im Netz bekannt:** ja: „Oberorken: Wasserraum-Türen beliebig oft knackbar (AP)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=6)

### Nr. 359: Prem-Mine: AP für Deckenfelder beliebig oft

*Dungeons · sicher, mittel*

Am Ausgang gibt es je betretenem Deckenfeld 8 AP, wiederholtes Betreten zählt.

**Warum:** gs_dng13_collapsecount zählt jedes Betreten.

**Im Netz bekannt:** ja: „Prem-Mine: AP beliebig oft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=20)

### Nr. 361: Runin: Drachenhort und Platinschlüssel-Auftrag unabhängig

*Dungeons · sicher, egal*

Wurfdolch und Gold gibt es doppelt.

**Warum:** gs_dng10_hoard_plundered und der Auftrag sind getrennt.

**Im Netz bekannt:** ja: „Runin-Drachenhöhle: Drachenschatz/Wurfdolch doppelt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=5)

### Nr. 362: Fluch der Goldmünzen (Burgruine): jeder Kampf gilt sofort als gewonnen

*Dungeons · unsicher, kritisch*

Nimmt eine Gruppe die verfluchten Münzen, endet jeder weitere Kampf ohne Kampf als gewonnen, für alle Gruppen.

**Warum:** do_fight() liefert bei gs_dng15_took_cursed_money sofort 0, der Merker ist global und wird nie gelöscht. Die verfluchte Gruppe sitzt in der Ruine fest. Ob auch der Endkampf so übersprungen wird, vermutlich ja, nicht geprüft.

**Im Netz bekannt:** ja: „Burgruine: Goldmünzen-Fluch verhindert alle Kämpfe“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=28)

### Nr. 366: Orkhöhle: Ausgang setzt die Gruppe direkt aufs Reiseziel

*Reise & Ereignisse · sicher, mittel*

Die Orkhöhle ist eine Abkürzung, egal wie weit der Weg noch ist.

**Warum:** gs_travel_destination_* + 3 h.

**Im Netz bekannt:** ja: „Abkürzung durch Dungeon (Orkhöhle)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)

### Nr. 367: Weiterreisen ohne Nachtlager über unsichtbares Symbol

*Reise & Ereignisse · sicher, mittel*

Auch ohne guten Lagerplatz kann man weiterreisen.

**Warum:** Nur 6 Symbole gezeichnet, ACTION_ID_ICON_7 trotzdem ausgewertet.

**Im Netz bekannt:** ja: „Weiterreisen ohne Nachtlager“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=15)

### Nr. 372: Seereise: jeder Umweg-Wert führt ins Totenschiff

*Reise & Ereignisse · sicher, mittel*

Stirbt die reisende Gruppe oder wird der Seekampf verlassen, landen andere im Totenschiff („Boron-Opfer-Trick“).

**Warum:** gs_travel_detour != 0 führt zu DNG_enter_dungeon(TOTENSCHIFF), auch der Wert 99.

**Im Netz bekannt:** ja: „Totenschiff-Teleporter (Boron-Opfer-Trick)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=30)

### Nr. 374: Berghütte: Jagd zweimal möglich

*Reise & Ereignisse · sicher, egal*

Jagd und Kräutersuche wiederholbar.

**Warum:** do_wildcamp() wird zweimal aufgerufen, Status zurückgesetzt.

**Im Netz bekannt:** ja: „Wiederholbare Jagd an der Berghütte“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=10)

### Nr. 375: Jagd: Einzelheld schafft immer den Blattschuss

*Reise & Ereignisse · sicher, egal*

Ein einzelner Held erlegt nach gelungenem Schleichen immer die volle Beute.

**Warum:** count_heroes_in_group() / 2 = 0 <= passed. Speer als Jagdwaffe siehe Nr. 176.

**Im Netz bekannt:** ja: „Jagd: Blattschuss ohne Fernwaffe“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=26)

### Nr. 384: Herberge: Regenerationssperre nach Magie hängt am Heldenplatz

*Magie · sicher, egal*

Ein Magier, der in der Herberge gezaubert hat, regeneriert nach einem Platztausch doch.

**Warum:** magic_act[] ist nach Platz indiziert (vgl. Nr. 49). Die Sperre selbst ist gewollt.

**Im Netz bekannt:** ja: „Stabzauber-Regeneration“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=28)

### Nr. 397: Tausch zwischen Helden prüft nur die Traglast des Empfängers

*Gegenstände & Inventar · sicher, egal*

Über einen Tausch bekommt ein Held mehr Last, als er tragen kann.

**Warum:** Nur hero2 wird geprüft.

**Im Netz bekannt:** ja: „Tragegrenze durch Tausch umgehbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=25)

### Nr. 402: Aufgesparte Zaubersteigerungen laufen über 127

*Helden & Werte · unsicher, mittel*

Der Zähler springt von 127 auf −128, Steigern bleibt danach praktisch unbegrenzt möglich.

**Warum:** saved_spell_increases ist signed char ohne Obergrenze, Prüfungen mit != 0. Erreichbar vermutlich mit Nichtmagiern, die Versuche sammeln (Netz 119). Index 86 siehe Nr. 252.

**Im Netz bekannt:** ja: „Zaubersteigerungs-Zähler läuft über“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=14)

### Nr. 404: Elixiere stapeln sich ohne Obergrenze

*Helden & Werte · sicher, mittel*

Mehrere Elixiere heben eine Eigenschaft beliebig an, Werte können überlaufen.

**Warum:** Jedes setzt per set_mod_slot() +5 für 1 Stunde, ohne Grenze.

**Im Netz bekannt:** ja: „Attribute über Elixiere unbegrenzt steigerbar (Überlauf)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=27)

## Sicher falsch, kritisch (18)

### Helden & Werte

**Nr. 235: Zufallsheld aus Plätzen 0..n−1 statt aus den Gruppenmitgliedern**  
Der NSC wird nie als Zufallsheld getroffen. Lebt in der Gruppe nur noch der NSC, hängt das Spiel bei der nächsten Falle.  
*Im Netz bekannt:* nein  

**Nr. 244: Gruppenwechsel ohne Ausgang, wenn keine Gruppe frei handeln kann**  
Sind alle Gruppen im Kerker oder handlungsunfähig, hängt das Spiel nach Ereignissen, im Menü erscheint die Meldung endlos.  
*Im Netz bekannt:* nein  

**Nr. 248: Stufenanstieg: ESC bei „WELCHE POSITIVE EIGENSCHAFT“ bringt rund 168 000 Dukaten** · **Exploit**  
Bricht man beim Stufenanstieg die Wahl der zu steigernden Eigenschaft mit ESC oder rechter Maustaste ab, bekommt der Held etwa 168 427 Dukaten und keine Eigenschaft.  
*Im Netz bekannt:* nein  

**Nr. 260: Suche nach freier Gruppe ohne Grenze**  
Sind alle 6 Gruppen belegt und trennt man einen brauenden Helden ab, wird hinter die Gruppentabelle geschrieben, Helden werden ausgegraut, das Spiel kann abstürzen.  
*Im Netz bekannt:* ja: „Sieben Gruppen führen zu Speicherfehler“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=33)  

### Gegenstände & Inventar

**Nr. 214: Magisches Bastardschwert ist als „benutzbar“ markiert**  
Wer das magische Bastardschwert über BENUTZEN verwendet, bringt das Spiel zum Absturz.  
*Im Netz bekannt:* nein  

**Nr. 221: Zufälliger Gegenstandsverlust: Endlosschleife ohne ablegbaren Gegenstand**  
Hat ein Held keinen ablegbaren Gegenstand und trifft ihn ein Verlust (z. B. Bad in der Furt), hängt das Spiel.  
*Im Netz bekannt:* ja: „Endlosschleife beim Gegenstandsverlust“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=25)  

**Nr. 222: Tausch zweier Körperplätze ohne Prüfung und Boni** · **Exploit**  
Legt man einen Gegenstand zwischen zwei Körperplätzen um (z. B. Kraftgürtel linke → rechte Hand), werden seine Boni jedes Mal neu aufgeschlagen: KK +5 je Umlegen, ebenso CH (verfluchter Helm), Gefahrensinn (Kristallkugel), bis zum Überlauf. Der Schild-Trick senkt umgekehrt den RS.  
*Im Netz bekannt:* ja: „Kraftgürtel-Bug: KK +5 durch Handwechsel“ (und 3 weitere Meldungen) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=29)  

### Städte & Orte

**Nr. 40: Einbruch: „noch jemand frei?“ zählt die gerade erwischte Gruppe mit**  
Sitzen alle anderen Gruppen schon im Kerker und wird auch die letzte freie Gruppe beim Einbruch erwischt, hängt das Spiel.  
*Im Netz bekannt:* ja: „Gefängnis: beide Teilgruppen erwischt → Schleife“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=28)  

**Nr. 67: Zwingfeste: bei jedem Betreten 20 Dukaten** · **Exploit**  
Solange der Ugdalf-Auftrag läuft, bekommt die Gruppe bei jedem Betreten der Zwingfeste erneut 20 Dukaten.  
*Im Netz bekannt:* ja: „Zwingfeste: Dramosch-Vorauszahlung mehrfach“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=12)  

### Reise & Ereignisse

**Nr. 137: Schiffstempo in 16 Bit, Schnellsegler mit −106: Tempo 0 bringt das Spiel zum Absturz**  
Je nach Wetter wird eine Schnellsegler-Passage mit absurder Dauer angezeigt, beim Buchen stürzt das Spiel mit Division durch 0 ab. Andere Schiffe fahren mit falschem Tempo.  
*Im Netz bekannt:* nein  

**Nr. 164: Waten durch den Bach mit uninitialisiertem Helden**  
Beim Durchwaten des Bachs kann zufälliger Speicher verändert werden oder das Spiel hängen.  
*Im Netz bekannt:* ja: „Reiseereignisse 98/103/104 fehlerhaft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 180: Krakenmolch: Endlosschleife, wenn alle Helden über Bord oder tot sind**  
Werden alle Helden über Bord gerissen oder sterben, hängt das Spiel.  
*Im Netz bekannt:* ja: „Krake: Angriffsschwelle, Probe, Endlosschleife“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 184: Verirrt im Wald: alle sieben Heldenplätze sterben**  
Findet eine Teilgruppe nach fünf Tagen nicht aus dem Wald, sterben auch alle Helden der anderen Gruppen, das Spiel ist aus.  
*Im Netz bekannt:* nein  

### Dungeons

**Nr. 80: Nur noch der NSC handlungsfähig: das Spiel läuft weiter**  
Werden in der Verfallenen Herberge alle außer dem NSC versteinert (oder verschwinden bzw. sterben sie an der Steilwand), läuft das Spiel mit dem NSC allein weiter. Gruppenwechsel und Zufallsheld hängen dann.  
*Im Netz bekannt:* ja: „Verfallene Herberge: Versteinerung des Einzelhelden“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=12)  

**Nr. 117: Opferschale: ESC und große Eingaben (16-Bit-Überlauf) schenken Geld** · **Exploit**  
An der Opferschale bringt ESC 10 Heller. Gibt man eine große Zahl ein (z. B. 3300), läuft die Rechnung über und die Gruppe bekommt rund 3250 Dukaten, beliebig oft.  
*Im Netz bekannt:* ja: „Runin-Drachenhöhle: Opferstock-Überlauf“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=6)  

### Kampf

**Nr. 22: Übergelaufener Priester (Stufe 2) hängt das Spiel auf**  
Wird ein Priester der Stufe 2 mit Böser Blick auf die Seite der Helden gezogen, bleibt das Spiel in seinem Zug hängen.  
*Im Netz bekannt:* ja: „Absturz/Hänger durch Bösen Blick auf Zauberer bzw. Piraten“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=19)  

**Nr. 25: Fliehender Doppelgegner: Wegsuche hängt**  
Kämpfe mit zweiteiligen Gegnern (Drachen, Schlangen) können einfrieren, wenn ein solcher Gegner flieht.  
*Im Netz bekannt:* ja: „Kampf gegen Steppenhunde/Grimwölfe friert ein“ (und 2 weitere Meldungen) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=14)  

### Magie

**Nr. 196: Skelettarius belegt bei jedem Wirken 1288 Byte neu im Kampfobjektpuffer**  
Wird Skelettarius in einem Kampf oft gewirkt, läuft der Grafikpuffer über, das Spiel kann abstürzen.  
*Im Netz bekannt:* ja: „Skelettarius: Fehler bei Leichenstapeln und Hinterteilen“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=16)  

## Sicher falsch, mittel (115)

### Helden & Werte

**Nr. 229: h_index wird nur einmal vor der Schleife auf −1 gesetzt**  
Selten verschiebt sich ein abgelaufener Zauber- oder Fluchwert dauerhaft auf einen anderen Helden.  
*Im Netz bekannt:* nein  

**Nr. 232: Gift wirkt nur in game_loop(), bei Zeitsprüngen höchstens einmal**  
Wer vergiftet rastet oder reist, erleidet viel weniger Giftwirkung als vorgesehen.  
*Im Netz bekannt:* nein  

**Nr. 233: Heil-, Stab- und Lichttimer runden kleine Zeitsprünge weg**  
Beim Gehen in Stadt und Dungeon brennen Fackeln und Laternen nicht herunter, Heil- und Stabzaubersperren laufen nicht ab.  
*Im Netz bekannt:* ja: „Zeitzähler laufen nach Echtzeit statt Spielzeit“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=2)  

**Nr. 234: Stundenzählung für Kettenhemd-Schaden und Essen falsch**  
Bei langen Zeitsprüngen fehlen Schaden des verfluchten Kettenhemds und Hunger/Durst, bei genau 24 Stunden komplett.  
*Im Netz bekannt:* ja: „Verfluchtes Kettenhemd: Schaden nur beim Schlafen“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=2)  

**Nr. 240: Erneute Erkrankung löscht die Protokollbytes, Verluste werden dauerhaft**  
Erkrankt ein Held erneut an einer Krankheit, die er schon hat, bekommt er die verlorenen Eigenschaftspunkte nie zurück.  
*Im Netz bekannt:* ja: „Eigenschaftsverlust nach Krankheit bleibt (Neuinfektion)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=2)  

**Nr. 243: Gruppe teilen klappt nicht richtig, wenn der NSC in einer anderen Gruppe ist**  
Zwei Helden lassen sich nicht trennen, und man kann nicht alle bis auf einen abspalten, wenn der NSC woanders steht.  
*Im Netz bekannt:* ja: „Gruppenteilung mit NSC fehlerhaft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=29)  

**Nr. 249: Stufenanstieg: ESC bei „WELCHE NEGATIVE EIGENSCHAFT“ senkt die Intuition**  
Bricht man die Wahl der zu senkenden schlechten Eigenschaft ab, versucht das Spiel stattdessen die Intuition zu senken (fast immer mit Erfolg).  
*Im Netz bekannt:* nein  

**Nr. 250: MR-Neuberechnung beim Stufenanstieg verträgt sich nicht mit MR-Gegenständen**  
Mit Stirnreif, rotem Ring oder Amulett erscheint die MR-Meldung grundlos, und ein roter Ring in der rechten Hand kostet nach dem Ablegen dauerhaft 2 MR.  
*Im Netz bekannt:* ja: „MR sinkt auf -6“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=28)  

**Nr. 253: Spielende: Merker für bleibenden LE-Verlust wird gelöscht, LE-Maximum bleibt gesenkt**  
Exportierte Helden behalten ein gesenktes LE-Maximum, das sich später nicht mehr heilen lässt.  
*Im Netz bekannt:* ja: „Max-LE-Verlust durch Hunger/Durst“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=9)  

**Nr. 254: set_mod_slot() mit who = 0 für einen anderen Helden**  
Wird die Heldenreihenfolge innerhalb eines Tages geändert, bekommt beim Ablauf ein anderer Held den CH- bzw. GE-Abzug zurück, einer bleibt dauerhaft gesenkt.  
*Im Netz bekannt:* nein  

**Nr. 255: Harikas Abschiedsgeschenk kostet Steigerungsversuche**  
Verlässt Harika die Gruppe, bekommen die Helden Versuche auf Schleichen, die beim nächsten Stufenanstieg fehlen.  
*Im Netz bekannt:* ja: „Fehlende Steigerungsversuche beim Stufenaufstieg“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=17)  

**Nr. 256: Entlassene NSC kommen nie wieder**  
Ein NSC, der die Gruppe verlässt, ist für immer verschwunden, obwohl er nach 31 bzw. 64 Tagen wieder auftauchen sollte.  
*Im Netz bekannt:* ja: „NSCs nicht wieder anheuerbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=14)  

**Nr. 259: Brauen: Abtrennen des einzigen Helden einer Gruppe möglich**  
Hat die Gruppe nur einen handlungsfähigen Helden (plus NSC oder Bewusstlose), kann man ihn zum Brauen abtrennen. Zurück bleibt eine leere oder handlungsunfähige Gruppe.  
*Im Netz bekannt:* ja: „NSC allein in eigener Gruppe läuft frei“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 264: Verschwundener NSC: Datei und Timer um eins verschoben**  
Verschwindet ein NSC, bleibt er in seiner Taverne anwerbbar, dafür wird der nächste NSC gesperrt und dessen Datei überschrieben.  
*Im Netz bekannt:* nein  

**Nr. 269: Paralyse und Frostschäden: Erholung mit umgekehrter Bedingung**  
Nach Paralyse oder Frostschäden kommen KK und GE nie zurück, der Zustand bleibt für immer „Genesung“. Wer keinen Verlust hatte, bekommt sogar je einen Punkt geschenkt.  
*Im Netz bekannt:* ja: „Paralyse/Erfrierungen: Werte nicht wiederhergestellt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=2)  

**Nr. 270: Schlachtfeldfieber endet nie**  
Nach Schlachtfeldfieber erscheint jeden Tag „DIE GELBLICHE FÄRBUNG … GEHT ZURÜCK“, der Held wird nie gesund.  
*Im Netz bekannt:* ja: „Schlachtfeldfieber: Genesung endet nie“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=2)  

**Nr. 273: Tollwut: Beruhigen und Sanftmut werten misslungene Proben als Erfolg**  
Einen tollwütigen Helden zu beruhigen klappt fast immer, auch durch Helden anderer Gruppen oder Tote, und ein misslungener Sanftmut kostet 15 AE und gilt als Erfolg.  
*Im Netz bekannt:* nein  

**Nr. 274: Shurinknollengift und Bannstaub: Rückgabe der Werte fast nie**  
Nach Shurinknollengift kommen KK und GE, nach Bannstaub die AE nur alle gut 21 Stunden um einen Punkt zurück statt stündlich.  
*Im Netz bekannt:* ja: „Orkhöhlen-Truhengift senkt GE und KK dauerhaft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=11)  

**Nr. 404: Elixiere stapeln sich ohne Obergrenze** · **Exploit**  
Mehrere Elixiere heben eine Eigenschaft beliebig an, Werte können überlaufen.  
*Im Netz bekannt:* ja: „Attribute über Elixiere unbegrenzt steigerbar (Überlauf)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=27)  

### Gegenstände & Inventar

**Nr. 218: Analüs verspricht Wirkungen magischer Gegenstände, die es nicht gibt**  
Amulett gegen Ignifaxius, Feuerbann-Amulett, grüner Ring (Feuerschutz), magischer Speer (gegen Spinnen) und magisches Schwert (gegen Untote) haben keine der beschriebenen Wirkungen. Das magische Bastardschwert ist keine „hervorragende Waffe“ (AT −1, PA −2 gegenüber dem normalen).  
*Im Netz bekannt:* ja: „Runin-Drachenhöhle: grüner Ring wirkt nicht am Hebel“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=8)  

**Nr. 220: Zähler der belegten Inventarplätze driftet**  
Nach Stapeln und Tauschen kann ein Held keine Gegenstände mehr aufnehmen, obwohl Platz ist (oder umgekehrt).  
*Im Netz bekannt:* ja: „Gegenstandszähler bei Stapeln falsch“ (und 2 weitere Meldungen) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=23)  

**Nr. 225: Laterne nachfüllen: Öl beim letzten Zauberer gesucht, beim Benutzer entfernt**  
Füllt ein Held seine Laterne nach und hat zuletzt ein anderer Held gezaubert, verschwindet beim Benutzer ein falscher Gegenstand.  
*Im Netz bekannt:* ja: „Lampe auffüllen sucht Öl nicht beim Besitzer“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 393: Steinschleuder ist unbenutzbar**  
Mit der Schleuder kann man nicht schießen.  
*Im Netz bekannt:* ja: „Steinschleuder unbenutzbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=13)  

### Städte & Orte

**Nr. 39: Akademie identifiziert und bricht Flüche nur ein einziges Mal im Spiel**  
Nach der ersten Identifizierung in der Akademie heißt es „VERSUCHT ES MORGEN“ für immer.  
*Im Netz bekannt:* ja: „Schule der Hellsicht: nur ein Artefakt/Fluch pro Spiel“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=14)  

**Nr. 43: Statusleiste setzt 6 statt 4 Werte je Held, beim NSC wird die Krankheitstabelle überschrieben** · **Exploit**  
Mit NSC in der Gruppe zahlt der Heiler für die Heilung von Wundfieber 100 Heller drauf, und die Talentprobe auf Wundfieber wird leicht.  
*Im Netz bekannt:* ja: „Heiler verlangt negativen Betrag (Wundfieber)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=17)  

**Nr. 48: Brauender NSC bleibt in der Herberge stecken**  
Lässt man Curian in einer Herberge brauen, findet die Gruppe ihn dort nicht wieder, und seine Gruppe lässt sich nicht anwählen. Der NSC ist verloren.  
*Im Netz bekannt:* ja: „NSC allein in eigener Gruppe läuft frei“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 49: NSC kann in der Herberge nicht zaubern, magic_act[6] schreibt hinter das Feld**  
Curian kann in der Herberge keine Zauber anwenden, beim Übernachten wird eine fremde lokale Variable überschrieben.  
*Im Netz bekannt:* ja: „Curian: keine Magie in Herbergen“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=2)  

**Nr. 50: Brauende Helden: Einzelheld sieht sie nicht, nur der erste wird gefragt**  
Kommt ein einzelner Held in die Herberge, wird er nicht gefragt, ob er brauende Gefährten abholen will. Sind mehrere am Brauen, wird nur der erste geprüft.  
*Im Netz bekannt:* ja: „Alchemie: Brauen-Fehler“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 51: Nach dem Abholen gelten zusammengeführte Brauer weiter als „brauend“**  
Holt man einen von mehreren brauenden Helden ab, werden alle Gruppen vereint, die anderen bleiben im Zustand „braut“. Als Anführer einer Gruppe verursachen sie seltsames Verhalten.  
*Im Netz bekannt:* ja: „Alchemie: Brauen-Fehler“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 52: Herberge: Preisaufschlag für Einzelzimmer und Suite vom Schlafsaalpreis berechnet**  
In teuren Herbergen kosten Einzelzimmer und Suite fast den Grundpreis, in billigen fast keinen Rabatt.  
*Im Netz bekannt:* ja: „Herbergstexte widersprechen dem Ruf“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=4)  

**Nr. 53: „Besonders guter Preis“ beim Händler verschlechtert den Verkauf**  
Nach dem erfolgreichen Händlergespräch („ICH MACHE EUCH EINEN BESONDERS GUTEN PREIS“) bekommt man beim Verkaufen nur 75 %, beim Kaufen ändert sich nichts.  
*Im Netz bekannt:* nein  

**Nr. 54: Einkaufskorb: Platzbedarf falsch berechnet**  
Der Händler meldet „DIE GRUPPE KANN KEINE WEITEREN GEGENSTÄNDE AUFNEHMEN“, obwohl Platz ist, oder lässt zu viel in den Korb.  
*Im Netz bekannt:* nein  

**Nr. 59: Taverne im Gespräch: Runde und Essen werden in Heller statt Silber abgezogen** · **Exploit**  
Im Tavernengespräch kosten die Lokalrunde und das Essen nur ein Zehntel des genannten Preises.  
*Im Netz bekannt:* nein  

**Nr. 65: Straßenhändler überschreibt den Typ des gerade betretenen Gebäudes** · **Exploit**  
Erscheint der Straßenhändler beim Betreten eines Gebäudes, ist das Gebäude danach verdorben: ein Tempel wird zum Praiostempel, eine Taverne kann negative Essenspreise haben.  
*Im Netz bekannt:* nein  

**Nr. 68: Akademie erkennt einen Fluch beim Helden auf Platz 0 nicht**  
Ist der erste Held verflucht, sagt Aranda „KEINER VON EUCH HAT EINEN FLUCH AUF SICH“.  
*Im Netz bekannt:* ja: „Schule der Hellsicht: Gruppenerster und Totenschiff-Fluch“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=2)  

**Nr. 72: Spielhaus: Gewinn und Verlust vertauscht** · **Exploit**  
Im Spielhaus von Phexcaer verliert die Gruppe, wenn die Mehrheit beim Falschspiel Erfolg hat, und gewinnt, wenn sie versagt.  
*Im Netz bekannt:* ja: „Phexcaer-Spielhaus: Falschspiel-Ergebnis verkehrt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=29)  

**Nr. 76: Informanten werden von „erledigt“ (2) auf „bekannt“ (1) zurückgesetzt**  
Schon abgeschlossene Informanten (z. B. Eliane, Asgrimm, Umbrik) lassen sich nach einem Tavernengerücht erneut besuchen.  
*Im Netz bekannt:* ja: „Informanten durch Erwähnung reaktiviert“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=20)  

**Nr. 341: Treborn Kolberg erscheint nach Algrids Hinweis nie, Olvir/Treborn wiederholt sich**  
Hat Algrid von Kolberg erzählt, trifft man ihn nie (sein Kartenstück fehlt dann). Olvir und Kolberg kommen direkt hintereinander.  
*Im Netz bekannt:* ja: „Treborn Kolberg erscheint nach Algrid nie / nicht in Clanegh“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=1)  

### Reise & Ereignisse

**Nr. 127: Nur die erste Seeroute (Thorwal–Prem) wird zurückgesetzt**  
Außer auf Thorwal–Prem legen die Schiffe nicht um 9 Uhr ab, sondern liegen bis Mitternacht im Hafen.  
*Im Netz bekannt:* ja: „Hafen: nur die erste Route läuft um 9 Uhr aus“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=3)  

**Nr. 136: Passagepreis in 16 Bit: lange Routen werden spottbillig** · **Exploit**  
Auf langen Routen kostet eine Karracke oder Luxuspassage nur einen Bruchteil, z. B. 80 statt 735 Heller.  
*Im Netz bekannt:* nein  

**Nr. 139: Hafen Hjalsingor: Route nach Overthorn fehlt in der Liste**  
Nach der Schiffsreise von Overthorn nach Hjalsingor steht die Gruppe beim Verlassen des Hafens auf einem falschen Feld hinter den Häusern.  
*Im Netz bekannt:* ja: „Ankunft in Städten auf falschem Feld/verdreht“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=9)  

**Nr. 140: Hafen Leuchtturm Runin verweist auf eine Landweg-Liste**  
Nach der Schiffsreise zum Leuchtturm Runin steht die Gruppe auf einem entlegenen Feld des Ortes.  
*Im Netz bekannt:* ja: „Ankunft in Städten auf falschem Feld/verdreht“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=9)  

**Nr. 142: Reiseereignisse: Lage wird nach Richtungswechsel falsch korrigiert** · **Exploit**  
Einmalige Reiseereignisse lassen sich durch Umkehren wiederholen, gefährliche (z. B. Sümpfe mit Umkehr-Option) durch erneutes Umkehren umgehen, wiederholbare fallen nach dem Umkehren aus.  
*Im Netz bekannt:* ja: „Feste Reiseereignisse wiederholen sich“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=9)  

**Nr. 149: replenish_stocks() bekommt die Heldenposition als Versuchszähler**  
Steht der Anführer auf Platz 3 oder höher, passiert am Rastplatz nichts.  
*Im Netz bekannt:* ja: „Vorräte auffüllen: Heldenposition als Versuchsanzahl“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 150: „UMKEHREN“ auf der Rückreise behält die Rückwärtsrichtung (9 Stellen)**  
Wer auf dem Rückweg an Furt, Fähre, Sumpf, Hochmoor oder Brücke umkehren will, reist trotzdem weiter in dieselbe Richtung.  
*Im Netz bekannt:* nein  

**Nr. 151: Furt: GE-Probe gilt fast immer als bestanden**  
Beim Durchqueren einer Furt fällt fast nie jemand ins Wasser (95–100 %).  
*Im Netz bekannt:* ja: „Furt: gescheiterte Probe führt zum Erfolg“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=21)  

**Nr. 152: Fährpreis wächst bei jedem Versuch weiter**  
Hat die Gruppe zu wenig Geld oder wartet, steigt der Fährpreis bei jedem neuen Versuch um ein Vielfaches.  
*Im Netz bekannt:* nein  

**Nr. 153: Gruppenproben zählen alle 7 Plätze ohne Gruppen- und Todesprüfung**  
Beim Brückenbau liefern Helden anderer Gruppen oder Tote die nötigen Proben, beim Verstecken vor der alten Frau kann das Ergebnis kippen.  
*Im Netz bekannt:* nein  

**Nr. 158: Steilhang: Strickleiter macht den Sturz schlimmer**  
Wer eine Strickleiter dabeihat, stürzt am Hang schwer, nur mit Seil ohne Strickleiter leicht.  
*Im Netz bekannt:* ja: „Klettern: Fehlschlag löst Patzerfolgen aus; Schutthügel“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 159: Gorahs Hütte: Silentium-Auswahl wird nie wiederholt** · **Exploit**  
Wählt man „Silentium“ mit einem Nicht-Magier oder misslingt der Zauber, nähert sich die Gruppe ohne Probe und ohne Hinterhalt, während ein gelungener Silentium den Hinterhalt auslöst.  
*Im Netz bekannt:* nein  

**Nr. 161: Kräuterbonus für Gegenstände über 127 greift nie**  
An Fundstellen für Jorugawurzel, Thonnysblüte und Kairanhalm gibt es den versprochenen Fundbonus nicht.  
*Im Netz bekannt:* nein  

**Nr. 162: Wegzoll 15 Dukaten insgesamt statt pro Person** · **Exploit**  
Der Wegelagerer verlangt 15 Dukaten pro Person, die Gruppe zahlt aber nur 15 Dukaten insgesamt.  
*Im Netz bekannt:* nein  

**Nr. 165: Sturz in die Klamm: hero_disappear() mit der Antwortzahl als Position**  
Ein in die Klamm gestürzter Held lässt sich im Efferdtempel nicht wiederfinden.  
*Im Netz bekannt:* nein  

**Nr. 166: Harpyien: Fluchtoption ist kein Versuch** · **Exploit**  
„VERSUCHEN ZU FLIEHEN?“ gelingt immer ohne Probe, die Kriegskunst-Probe kommt erst nach einem verlorenen Kampf.  
*Im Netz bekannt:* ja: „Harpyien-Kampf Thoss–Rybon fehlerhaft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=3)  

**Nr. 167: Nebeltal: Funktionsadresse als Held**  
Die Orientierungsprobe im Nebeltal wird mit unsinnigen Werten gewürfelt.  
*Im Netz bekannt:* ja: „Reiseereignisse 98/103/104 fehlerhaft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 170: Firun-Opfer: Schlafbonus bleibt für spätere Lager stehen** · **Exploit**  
Wer am Firun-Schrein opfert, schläft auch in späteren Wildnislagern besser (+1 bis +4), bis ein Rastplatz den Wert löscht.  
*Im Netz bekannt:* nein  

**Nr. 172: Brücke: Sturz in die Tiefe ist unerreichbar**  
Beim Überqueren der Brücke kann niemand abstürzen, die gefährliche Variante fehlt.  
*Im Netz bekannt:* nein  

**Nr. 177: AT- und PA-Proben verkehrt herum (Höhlenbär, Krakenmolch)**  
Gute Kämpfer treffen den Krakenmolch und den Höhlenbären seltener, Helden mit guter Parade werden öfter verletzt.  
*Im Netz bekannt:* ja: „Krake: Angriffsschwelle, Probe, Endlosschleife“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 178: Kvillotter beißt nie**  
Die Begegnung mit der Kvillotter ist harmlos, die Schlange verschwindet immer.  
*Im Netz bekannt:* nein  

**Nr. 179: Krakenmolch: GE-Probe falsch ausgewertet**  
Der Krakenmolch packt fast nie zu, ab GE 19 nie.  
*Im Netz bekannt:* ja: „Krake: Angriffsschwelle, Probe, Endlosschleife“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 181: Zufallsbegegnungen: Vergleich verkehrt herum, fast alle unerreichbar**  
Auf Reisen gibt es nur eine Art Zufallsbegegnung (Kampf WILD1). Jagden, Bär, Schlange usw. kommen nie vor.  
*Im Netz bekannt:* nein  

**Nr. 185: Kletterpartie: tödlicher Sturz bei knapp verfehlter Probe statt beim Patzer**  
Wer die Kletterprobe um genau einen Punkt verfehlt, stürzt tödlich ab, ein Patzer ist dagegen harmlos.  
*Im Netz bekannt:* ja: „Klettern: Fehlschlag löst Patzerfolgen aus; Schutthügel“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 365: Reiseziel nicht getrennt je Teilgruppe**  
Reisen mehrere Gruppen, überschreibt eine das Ziel der anderen.  
*Im Netz bekannt:* ja: „Reiseziel nicht getrennt pro Teilgruppe“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 366: Orkhöhle: Ausgang setzt die Gruppe direkt aufs Reiseziel** · **Exploit**  
Die Orkhöhle ist eine Abkürzung, egal wie weit der Weg noch ist.  
*Im Netz bekannt:* ja: „Abkürzung durch Dungeon (Orkhöhle)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)  

**Nr. 367: Weiterreisen ohne Nachtlager über unsichtbares Symbol** · **Exploit**  
Auch ohne guten Lagerplatz kann man weiterreisen.  
*Im Netz bekannt:* ja: „Weiterreisen ohne Nachtlager“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=15)  

**Nr. 368: Tavernenhinweis schaltet Reiseereignisse 66 und 75 ab**  
Nach dem Gerücht in Ala bzw. Orkanger findet die Begegnung nicht statt.  
*Im Netz bekannt:* ja: „Reiseereignisse nach Tavernenhinweis abgeschaltet“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=6)  

**Nr. 370: Gorahs Haus nach Flucht der Helden unzugänglich**  
Nach Flucht startet das Ereignis wie mit lebendem Gorah.  
*Im Netz bekannt:* ja: „Gorahs Haus nach Flucht unzugänglich“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=11)  

**Nr. 372: Seereise: jeder Umweg-Wert führt ins Totenschiff** · **Exploit**  
Stirbt die reisende Gruppe oder wird der Seekampf verlassen, landen andere im Totenschiff („Boron-Opfer-Trick“).  
*Im Netz bekannt:* ja: „Totenschiff-Teleporter (Boron-Opfer-Trick)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=30)  

### Dungeons

**Nr. 79: Stündliche Sondertimer laufen bei Zeitsprüngen nicht**  
Die zweite Einhorn-Begegnung, der Sphärenriss in der Verfallenen Herberge und die Orkmist-Fässer verzögern sich stark, weil Rast und Reise sie nicht weiterzählen.  
*Im Netz bekannt:* ja: „Zeitzähler laufen nach Echtzeit statt Spielzeit“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=2)  

**Nr. 84: Todesfalle (Totenschiff, Spinnenhöhle) beendet das Spiel auch für Gruppen draußen**  
Sinkt das Totenschiff mit einer Teilgruppe an Bord, ist das Spiel aus, obwohl andere Gruppen woanders stehen.  
*Im Netz bekannt:* ja: „Totenschiff: Schlafen nach Heshthot unschädlich, Tod beim 30. Schritt“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=9)  

**Nr. 87: Dietrich-Zähler wird nach dem ersten Ebenenwechsel ständig zurückgesetzt** · **Exploit**  
Nach dem ersten Treppenwechsel kann man ein Schloss beliebig oft knacken, ohne dass die Dietriche beim 4. Versuch brechen.  
*Im Netz bekannt:* nein  

**Nr. 88: Klammerfehler: Feld (1,8,5) zeigt in jedem Dungeon den Spiegeltext**  
Auf Ebene 2, Feld (8,5) lässt sich in jedem Dungeon keine Gruppe vereinen, stattdessen kommt ein Text über Spiegel, und ein Merker der Ruine Hyggeliks wird gesetzt.  
*Im Netz bekannt:* ja: „Hyggeliks Höhle: Zusammenschluss auf Spiegelfeldern“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=9)  

**Nr. 91: Leeres Wasserfass füllt trotzdem alle Schläuche** · **Exploit**  
Aus einem leeren Fass kann man beliebig oft alle Wasserschläuche füllen, „DAS FASS IST NUN LEER“ kommt jedes Mal.  
*Im Netz bekannt:* nein  

**Nr. 94: Totenschiff: Deckenloch und Grube mit Zeilenbreite 32 statt 16 gesetzt**  
Nach dem Hochklettern durch den Spalt fehlt das Loch an der richtigen Stelle, dafür steht auf beiden Ebenen an (14,14) ein Loch bzw. eine Grube.  
*Im Netz bekannt:* ja: „Totenschiff: Falltüren falsch platziert“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)  

**Nr. 97: Rätseltruhe: 10 AP Zusatzbelohnung werden nie vergeben**  
Für die richtige Antwort MARBO gibt es nie die vorgesehenen 10 AP.  
*Im Netz bekannt:* ja: „Totenschiff: Abschlusstext und Goldschlüssel“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 102: weight_sum wird in der Gruppenschleife zurückgesetzt**  
Verlässt eine Gruppe die Platte (11,6), schließt sich die Wand (11,8), auch wenn eine andere Gruppe noch auf der Platte oder in der Wandlücke steht (außer es ist Gruppe 6).  
*Im Netz bekannt:* ja: „Verfallene Herberge: Mechanismen per Gruppenteilung umgehbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=18)  

**Nr. 105: Teleporter und Schilfmeer treffen auch Helden anderer Gruppen**  
Benutzt eine Gruppe einen der vier Teleporter der Spinnenhöhle, verlieren auch Helden anderer Gruppen (und Tote) 1W6 AE bzw. LE. Im Schilfmeer (Reiseereignis 29) können Helden anderer Gruppen Dumpfschädel bekommen.  
*Im Netz bekannt:* ja: „Spinnenhöhle: Teleporter-Schaden auch außerhalb der Gruppe“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 110: Zwei-Gruppen-Rätsel prüfen nur x/y der anderen Gruppe** · **Exploit**  
Eine zweite Gruppe, die irgendwo anders (andere Ebene, anderer Dungeon, Stadt) auf denselben Koordinaten steht, gilt als „am anderen Hebel“.  
*Im Netz bekannt:* nein  

**Nr. 114: Armbrustfalle 4 zählt nicht herunter**  
Die vierte Bolzenfalle löst bei jedem Betreten aus (2W6+4 Schaden), die anderen drei nur begrenzt oft.  
*Im Netz bekannt:* ja: „Gewölbe des Namenlosen: Bolzenfallen-Zähler / Hebel 8“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 119: Geheimtüren: test_result bei falscher Hebelstellung uninitialisiert**  
Bei falscher Hebelstellung kann eine der drei Geheimtüren trotzdem aufgehen.  
*Im Netz bekannt:* nein  

**Nr. 121: Verschütteter Gang (Zustand 3) blockiert nicht**  
In Hyggeliks Ruine lassen sich zwei verschüttete Gänge einfach durchschreiten, der Hinweis zum Freiräumen erscheint nie.  
*Im Netz bekannt:* nein  

**Nr. 122: Mit Werkzeug dauert das Freiräumen länger als ohne**  
Mit Schaufel, Hacke, Brecheisen und Wurfbeil braucht die Gruppe 1 Stunde statt 30 Minuten bzw. einen Tag statt 6 Stunden.  
*Im Netz bekannt:* ja: „Prem-Mine: Freischaufel-Zeitbonus vertauscht“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=29)  

**Nr. 126: Opfer für Efferd und Ingerimm zählen auch ohne Wegwerfen** · **Exploit**  
An den Opferstellen (Piratenhöhle, Zwergenfeste) gilt ein Gegenstand als geopfert, auch wenn man „WIRKLICH WEGWERFEN?“ verneint.  
*Im Netz bekannt:* nein  

**Nr. 345: Kampf auf Spezialfeldern startet erst nach Drehen oder Menü**  
Kommt die Gruppe per Falltür, Teleporter oder Treppe auf ein Kampffeld, beginnt der Kampf verspätet.  
*Im Netz bekannt:* ja: „Kampf auf Spezialfeldern verzögert“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=10)  

**Nr. 346: Ereignisfelder lösen nach Gruppenwechsel erneut aus**  
Nach einem Gruppenwechsel wiederholt sich das Ereignis des Feldes.  
*Im Netz bekannt:* ja: „Ereignisfeld bei Gruppenwechsel erneut ausgelöst“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=4)  

**Nr. 349: Transversalis im Dungeon auf Felder außerhalb des Plans** · **Exploit**  
Auf dem Totenschiff gelangt man in nicht existente Schiffsteile.  
*Im Netz bekannt:* ja: „Totenschiff: Transversalis in nicht existente Schiffsteile“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)  

**Nr. 350: Verlässt eine Teilgruppe das Totenschiff, sinkt es für die übrigen nicht mehr** · **Exploit**  
Eine Teilgruppe draußen rettet die Gruppe an Bord vor dem Untergang.  
*Im Netz bekannt:* ja: „Totenschiff: Teilgruppen beim Untergang“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=5)  

**Nr. 351: Speerfallgrube (Spinnenhöhle): Spielende auch für Gruppen draußen**  
Fällt die aktive Gruppe in die Grube, ist das Spiel aus.  
*Im Netz bekannt:* ja: „Spinnenhöhle: Speergrube beendet Spiel für alle“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=2)  

**Nr. 354: Gesperrte Türen ohne Meldung, Daspota-Falltür sperrt eine Tür für immer**  
Manche Türen lassen sich stumm nicht öffnen. Hat man in Daspota die Falltür gesichert, bleibt (0,11,1) zu.  
*Im Netz bekannt:* ja: „Daspota-Schatz: Türen blockiert“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=13)  

**Nr. 355: Schatzlager nur beim ersten Betreten plünderbar (Daspota-Schatz, Zwingfeste)**  
Was man liegen lässt, ist beim nächsten Besuch weg.  
*Im Netz bekannt:* ja: „Daspota-Schatz: Gegenstände nur beim ersten Betreten“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=4)  

**Nr. 357: Oberorken-Wasserfalle: Raum zu klein, Ertrinken nur an 5-Minuten-Marken**  
Das südlichste Feld ist sicher, Ertrinken nicht zuverlässig, Ertrunkene verschwinden.  
*Im Netz bekannt:* ja: „Oberorken: Wasserfalle fehlerhaft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 359: Prem-Mine: AP für Deckenfelder beliebig oft** · **Exploit**  
Am Ausgang gibt es je betretenem Deckenfeld 8 AP, wiederholtes Betreten zählt.  
*Im Netz bekannt:* ja: „Prem-Mine: AP beliebig oft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=20)  

### Kampf

**Nr. 2: Wurfwaffe Schneidzahn hat keine Entfernungstabelle**  
Der Schneidzahn wird geworfen, trifft aber ohne Wurfwaffen-Probe und ohne Entfernungsabzug.  
*Im Netz bekannt:* ja: „Schneidzahn ohne Fernwaffen-Index“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 3: MR von 19 Gegnertypen wird mit 15W4 statt 1W6 ausgewürfelt**  
Zombies, Piraten-Steuermann und -Kapitän, Folterknecht, Krieger, Höhlendrache, Zwergenveteran, Orkchampion u. a. haben eine riesige Magieresistenz, Beherrschungs- und Schadenszauber prallen fast immer ab.  
*Im Netz bekannt:* ja: „Unrealistisch hohe Magieresistenzen“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=1)  

**Nr. 18: Computerkampf: Helden greifen auch übergelaufene Gegner an**  
Im automatischen Kampf schlagen die Helden auch auf Gegner ein, die durch Böser Blick auf ihrer Seite kämpfen.  
*Im Netz bekannt:* ja: „NSC und Computerhelden greifen eigene Zombies/Beherrschte an“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 34: Wurfwaffe wird vor der Schadensberechnung abgelegt** · **Exploit**  
Wurfwaffen treffen immer, ohne Probe, und machen nur 1W6 wie waffenlos (außer Wurfsterne im Stapel).  
*Im Netz bekannt:* ja: „Wurfwaffen werden als waffenloser Kampf gewürfelt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=30)  

**Nr. 36: Gegner-Balsam heilt über das Maximum oder gar nicht**  
Ein Gegnerheiler heilt bei kleinen Wunden 7 LE (auch über das Maximum), bei großen Wunden nichts.  
*Im Netz bekannt:* nein  

**Nr. 313: Lagerüberfall: nur Heldenplatz 0 ist wach, eingeteilte Wachen zählen nicht**  
Bei einem nächtlichen Überfall steht nur der Held auf Platz 0 bereit, alle anderen schlafen die erste Runde, egal wer Wache hatte.  
*Im Netz bekannt:* ja: „Gruppenführer bei Nachtüberfall immer allein wach“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)  

**Nr. 319: Computerkampf zaubert auch mit Zauberwert unter −5**  
Computerhelden versuchen Schadenszauber, die sie nicht beherrschen, und verlieren Zug und halbe AE.  
*Im Netz bekannt:* ja: „Schadenszauber trotz Zauberwert unter -5 im Computerkampf“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=3)  

**Nr. 322: Alte Nahkampfoption ohne Nachbarschaftsprüfung („Schattenboxen“)** · **Exploit**  
Mit „alte Optionen“ schlägt ein Held einen Gegner, der nicht mehr neben ihm steht.  
*Im Netz bekannt:* ja: „„Schattenboxen“: Angriff ohne Nachbarschaft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=25)  

**Nr. 326: Waffengift: jedes Gift macht 10 × Stärke Zusatzschaden**  
Vergiftete Waffen richten sehr hohen Schaden an, lähmende Gifte wirken nur als Schaden.  
*Im Netz bekannt:* ja: „Gifte zu stark (Arax, Faktor 10)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

### Magie

**Nr. 189: Stabzauber: Abfrage „keine Stabzauber mehr“ auf Stufe 7 statt 4**  
Nach dem 4. Stabzauber versucht der Magier immer weiter, den 5. zu wirken: die Probe misslingt stets, jeder Versuch kostet halbe AE und 2 Stunden.  
*Im Netz bekannt:* nein  

**Nr. 190: Stabzauber-Probe prüft die zweite Eigenschaft doppelt**  
Die Probe für den Stabzauber benutzt die falsche Eigenschaft.  
*Im Netz bekannt:* ja: „Stabzauber-Probe prüft Eigenschaft 2 doppelt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=20)  

**Nr. 191: Foramen aus dem Zaubermenü ignoriert den Türzuschlag** · **Exploit**  
Wird Foramen frei aus dem Zaubermenü gesprochen, ist die Probe für jede Tür gleich leicht.  
*Im Netz bekannt:* ja: „Foramen-Erschwernis umgehbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=7)  

**Nr. 192: Gardianum kostet AE und schützt nicht**  
Der Schutzschirm Gardianum meldet „SCHUTZSCHIRM AKTIV“, wehrt aber nichts ab.  
*Im Netz bekannt:* nein  

**Nr. 197: Foramen aus dem Zaubermenü wirkt nur nach geöffnetem Türmenü**  
Foramen frei gesprochen öffnet eine Tür nur, wenn vorher das Türmenü (Einschlagen, Knacken, Zauber) offen war.  
*Im Netz bekannt:* nein  

**Nr. 199: Hexenspeichel: AE-Prüfung greift nie** · **Exploit**  
Mit Hexenspeichel kann eine Hexe mehr heilen, als ihre AE erlauben.  
*Im Netz bekannt:* ja: „Hexenspeichel verbraucht zu viel AE“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=11)  

**Nr. 207: Armatrutz: ein RS-Punkt mehr, als AE vorhanden** · **Exploit**  
Armatrutz bietet eine Stufe mehr an, als die AE bezahlen können, bei Gegnern ebenso.  
*Im Netz bekannt:* ja: „Armatrutz bietet zu hohen RS-Bonus an“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=23)  

**Nr. 211: g_ignite_mode wird nach dem Anzünden nicht zurückgesetzt**  
Nach dem Anzünden einer Fackel kann „Brenne toter Stoff“ keine Laterne mehr entzünden (und umgekehrt).  
*Im Netz bekannt:* nein  

**Nr. 212: Tollwut: Sanftmut ohne Prüfung des Zauberwerts**  
Jeder Zauberkundige kann den Tollwütigen mit Sanftmut beruhigen, auch wenn er den Zauber nicht kann. Magier mit 4. Stabzauber brauchen trotzdem 15 statt 13 AE, eine Meldung „beherrscht den Zauber nicht“ fehlt.  
*Im Netz bekannt:* nein  

**Nr. 213: Feuerbann wird gesetzt, aber nirgends ausgewertet**  
Der Zauber Feuerbann kostet AE und hat keine Wirkung.  
*Im Netz bekannt:* nein  

**Nr. 379: Axxeleratus: zweite Aktion wiederholt den Angriff ohne Zielwahl**  
Unter Axxeleratus führt der Held die zweite Aktion automatisch auf das alte Ziel aus, auch ohne Nachbarschaft.  
*Im Netz bekannt:* ja: „Axxeleratus-Zusatzattacke ohne Zielwahl / ins Leere“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=28)  

**Nr. 380: Böser Blick setzt dauerhaft zwei Attacken**  
Ein einmal beherrschter Gegner hat danach dauerhaft zwei Aktionen pro Runde.  
*Im Netz bekannt:* ja: „Böser Blick: Doppelangriff und Fehlverhalten“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=12)  

**Nr. 382: Große Gier ist nicht umgesetzt**  
Große Gier misslingt immer und kostet die halben AE.  
*Im Netz bekannt:* ja: „Große Gier wirkt nie“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=12)  

**Nr. 385: Curian zaubert im Kampf nur im Computerkampf mit Magie**  
Der NSC zaubert im normalen Kampf nie.  
*Im Netz bekannt:* ja: „Curian zaubert im Kampf nie“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=9)  

## Sicher falsch, egal (175)

### Helden & Werte

**Nr. 230: Aufräumen ungültiger Ziele setzt nur den aktuellen Eintrag zurück**  
Nichts Spürbares.  
*Im Netz bekannt:* nein  

**Nr. 231: Notbehelf bei 100 belegten Timern versagt im Kampf**  
Praktisch nichts, es braucht 100 gleichzeitig laufende Änderungen.  
*Im Netz bekannt:* nein  

**Nr. 237: BP-Anzeige bei Last ab 110 % falsch**  
Der Charakterbogen zeigt 3 BP, im Kampf hat der Held nur 1 BP.  
*Im Netz bekannt:* ja: „Überlast: Statusbildschirm zeigt 3 BP statt 1 BP“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=19)  

**Nr. 242: Ohne wählbaren Helden: Textbreite bleibt 3, geliefert wird Platz 0**  
Danach erscheinen Textfenster schmal, und Held 0 handelt, auch wenn er nicht wählbar war.  
*Im Netz bekannt:* nein  

**Nr. 245: Nachtwache folgt beim Tauschen nur einem der beiden Helden**  
Nach dem Umstellen der Helden steht eine Wache auf dem falschen Helden.  
*Im Netz bekannt:* nein  

**Nr. 246: Regeneration: Ergebnis 0 bleibt 0, −1 wird zu 1**  
In einem schlechten Lager regeneriert ein Held bei Wurf 0 nichts, bei noch schlechterem Wurf aber 1 Punkt.  
*Im Netz bekannt:* nein  

**Nr. 251: Zaubersteigerung: Zähler i wird je Runde nicht zurückgesetzt**  
Nichts Spürbares.  
*Im Netz bekannt:* nein  

**Nr. 252: Zufallszauber 1..86 statt 1..85**  
Nichts, gelegentlich verpufft ein Würfelversuch.  
*Im Netz bekannt:* ja: „Zaubersteigerungs-Zähler läuft über“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=14)  

**Nr. 258: Misslungene Wundheilung setzt Wundfieber ohne Prüfung**  
Hat der Patient schon Wundfieber oder ist in Genesung, beginnt die Krankheit von vorn.  
*Im Netz bekannt:* nein  

**Nr. 261: „DA SCHEINT ABER WAS ZU FEHLEN“ nennt die Zutat nicht**  
Beim Brauen erfährt man nicht, welche Zutat fehlt.  
*Im Netz bekannt:* nein  

**Nr. 263: Textdatei wird bei totem Patienten nicht zurückgeladen**  
Nach einem Heilversuch aus einem Ereignis auf einen toten oder versteinerten Helden erscheinen danach falsche Texte.  
*Im Netz bekannt:* nein  

**Nr. 265: Verschlimmerung zu Blauer Keuche mit 4 statt 5 %**  
Kaum spürbar.  
*Im Netz bekannt:* nein  

**Nr. 266: Nach dem Wechsel zu Blauer Keuche noch Dumpfschädel-Schaden am selben Tag**  
Am Tag der Verschlimmerung gibt es zusätzlich 1W6−1 LE Schaden vom eben beendeten Dumpfschädel.  
*Im Netz bekannt:* nein  

**Nr. 268: Blaue Keuche: „DAS LEBEN WIRD WOHL NIE WIEDER SEIN“ immer**  
Nach jeder Genesung von Blauer Keuche erscheint die Meldung über bleibende Schäden, auch ohne solche.  
*Im Netz bekannt:* ja: „Blaue Keuche: falscher Abschlusstext“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=3)  

**Nr. 403: Stufenaufstieg erst ab 101 statt 100 AP**  
Aufstieg einen AP später.  
*Im Netz bekannt:* ja: „Stufenaufstieg erst bei 101 statt 100 AP“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=5)  

**Nr. 407: Gruppengeld: Rest der Teilung verfällt**  
Ein paar Heller gehen beim Verteilen verloren.  
*Im Netz bekannt:* ja: „Gruppengeld-Rest verfällt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=21)  

### Gegenstände & Inventar

**Nr. 215: Magischer Brotbeutel wirkt wie der magische Beutel**  
Nichts, denn den magischen Brotbeutel gibt es im Spiel regulär nicht.  
*Im Netz bekannt:* nein  

**Nr. 217: Mengenabfrage lässt sich nicht abbrechen**  
„WIE VIELE MITNEHMEN?“ kann man nur mit 0 verlassen, ESC fragt erneut.  
*Im Netz bekannt:* nein  

**Nr. 223: Wasserschlauch in der Hand wird nicht gefunden**  
Ein Held, der seinen Wasserschlauch in der Hand trägt, trinkt nicht daraus.  
*Im Netz bekannt:* ja: „Wasserschlauch in der Hand wird nicht berücksichtigt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 226: Tarnele: Meldung ohne Text**  
Nach dem Essen einer Tarnele erscheint ein leeres oder altes Meldungsfenster.  
*Im Netz bekannt:* nein  

**Nr. 227: Antikrankheitselixier ohne Krankheit schreibt in disease[0]**  
Nichts.  
*Im Netz bekannt:* nein  

**Nr. 390: Unzerbrechliche Waffen können zerbrechen (Held gegen Held, Curians Stab)**  
Zauberstäbe und die Daspota-Orknase werden nach Paraden Held gegen Held zerbrechlich, Curians Stab ist es von Anfang an.  
*Im Netz bekannt:* ja: „Unzerbrechliche Waffen zerbrechen“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=12)  

**Nr. 391: Magier dürfen den verfluchten Lederhelm tragen**  
Der verfluchte Helm lässt sich von Magiern tragen.  
*Im Netz bekannt:* ja: „Verfluchter Lederhelm (Ardora) von Magiern tragbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=19)  

**Nr. 394: Wegwerfen: höchstens 99 Stück, Mengenabfrage auch bei einem Stück**  
Kosmetisch.  
*Im Netz bekannt:* ja: „Wegwerfen max. 99 Stück, Mengenabfrage bei einem Paket“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=10)  

**Nr. 395: Übergabe auf vollen 99er-Stapel: ohne Meldung nichts**  
Nach der Mengenabfrage passiert nichts.  
*Im Netz bekannt:* ja: „Stapel verschiedener Helden nicht kombinierbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=11)  

**Nr. 396: Kräuterfund bei vollem Rucksack verloren**  
Gefundene Kräuter sind weg, wenn der Sucher keinen Platz hat.  
*Im Netz bekannt:* ja: „Gefundene Gegenstände bei vollem Inventar verloren“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=7)  

**Nr. 397: Tausch zwischen Helden prüft nur die Traglast des Empfängers** · **Exploit**  
Über einen Tausch bekommt ein Held mehr Last, als er tragen kann.  
*Im Netz bekannt:* ja: „Tragegrenze durch Tausch umgehbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=25)  

**Nr. 398: Zweihänder von rechter in linke Hand: falsche Meldung**  
Es kommt Text 829 (Zweihänder und etwas anderes).  
*Im Netz bekannt:* ja: „Zweihänder-Handwechsel: falsche Meldung“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=6)  

**Nr. 400: Waffengift über BENUTZEN umgeht die Klassensperre**  
Ein Krieger kann seine Waffe vergiften.  
*Im Netz bekannt:* ja: „Krieger kann Waffen vergiften trotz Giftsperre“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)  

### Städte & Orte

**Nr. 41: Verhaftung nur ab Platz 0, solange die Plätze zur Gruppe gehören**  
Der NSC einer verhafteten Gruppe bleibt frei.  
*Im Netz bekannt:* nein  

**Nr. 42: Fehlende Haustypen 1 und 2: arr[1] bleibt ungesetzt**  
Nichts, keine Stadt hat diese Kombination.  
*Im Netz bekannt:* nein  

**Nr. 44: Antworten werden gemischt, das Ergebnis aber nie benutzt**  
Die Antworten stehen immer in derselben Reihenfolge.  
*Im Netz bekannt:* nein  

**Nr. 46: Tempel: bei 20, 30, … Helden eine leere Seite**  
Bei genau 20, 30, … gespeicherten Helden zeigt die letzte Seite nur „MEHR HELDEN“.  
*Im Netz bekannt:* nein  

**Nr. 47: leave_location() dreht die Gruppe auch dort um, wo sie nicht umdrehen soll**  
Nach dem Lager in Stadt oder Dungeon und nach dem Straßenhändler steht die Gruppe verkehrt herum. In Häusern mit Taverne und Herberge hängt die Blickrichtung davon ab, was man dort getan hat.  
*Im Netz bekannt:* ja: „Blickrichtung nach Dungeon-Rast verdreht“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 56: Schmied zeigt das Geld des Helden, bezahlt wird aus der Gruppenkasse**  
Die Geldanzeige beim Schmied passt nicht zum tatsächlich verfügbaren Geld.  
*Im Netz bekannt:* nein  

**Nr. 57: Bei genau passendem Geld Feilschprobe für 0 %**  
Hat die Gruppe genau den Reparaturpreis, fragt der Schmied „WER SOLL FEILSCHEN?“.  
*Im Netz bekannt:* nein  

**Nr. 60: Trinkprobe über Plätze 0..n−1 statt über die Gruppenmitglieder**  
Bei geteilter Gruppe werden fremde Helden betrunken, der NSC nie.  
*Im Netz bekannt:* nein  

**Nr. 61: Praios-Wunder: Prüfung auf Zauberkundige wirkungslos**  
Nichts im regulären Spiel, es gibt keinen Praiostempel (siehe Nr. 65).  
*Im Netz bekannt:* nein  

**Nr. 62: Praios und Hesinde heben Verwandlungen auch bei fremden Gruppen auf**  
Ein Wunder kann einen verwandelten Helden einer anderen Gruppe oder trotz Götterzorn heilen.  
*Im Netz bekannt:* nein  

**Nr. 64: Taschendieb: falsche Textteile, das Ende „Dieb entkommt“ erscheint nie**  
Bei der Verfolgung eines Taschendiebs erscheinen oft Teil 2 oder 3 der Mauer-Geschichte ohne Anfang.  
*Im Netz bekannt:* nein  

**Nr. 69: Fluchbrechung: Text 63 erscheint mit „%s“**  
Die Meldung zeigt wörtlich „BEMERKT %s, DASS …“.  
*Im Netz bekannt:* ja: „Schule der Hellsicht: Gruppenerster und Totenschiff-Fluch“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=2)  

**Nr. 70: Göttergunst über Index 0..13 statt 1..14**  
Ifirn bekommt Gunst vom Hetmann und von den Statuen nicht, dafür ein ungenutzter Eintrag.  
*Im Netz bekannt:* nein  

**Nr. 71: ESC ruft GUI_radio() mit −1 Optionen auf**  
Nach ESC erscheint das Auswahlfenster verstümmelt ohne Antworten, ENTER beendet es.  
*Im Netz bekannt:* ja: „Phexcaer-Stadthaus: Dialog nach ESC kaputt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=29)  

**Nr. 73: Spielhaus: „nach einigen Stunden“, aber die Zeit steht**  
Nach dem Glücksspiel ist keine Zeit vergangen.  
*Im Netz bekannt:* nein  

**Nr. 77: Rückgabewert bleibt oft uninitialisiert**  
Nach der Frage nach Neuigkeiten endet das Tavernengespräch zufällig vorzeitig oder geht weiter.  
*Im Netz bekannt:* nein  

**Nr. 78: Harikas Feilschbonus auch, wenn sie nicht dabei ist**  
Die Gruppe feilscht leichter, solange Harika irgendwo im Spiel ist, auch in einer anderen Gruppe oder tot.  
*Im Netz bekannt:* nein  

**Nr. 331: Markt- und Hafenangebot stehen nicht im Spielstand** · **Exploit**  
Laden behält das tagesaktuelle Markt- und Hafenangebot, Abfahrten und Preise lassen sich so steuern.  
*Im Netz bekannt:* ja: „Markt-/Hafenangebot beim Laden nicht zurückgesetzt“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=28)  

**Nr. 332: Öffnungszeit 8–19 Uhr gilt auch für Straßenhändler und Treborn Kolberg**  
Der Straßenhändler meldet zwischen 19 und 20 Uhr „geöffnet 8–19“.  
*Im Netz bekannt:* ja: „Straßenhändler/Kolberg ignorieren Öffnungszeiten“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=25)  

**Nr. 333: Tavernengerüchte in Serske: Lesen hinter dem Dateiende**  
In Serske erscheint Datenmüll als Gerücht.  
*Im Netz bekannt:* ja: „Tavernentexte in Serske voller Datenmüll“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=2)  

**Nr. 334: Beleidigter Wirt wirft nicht hinaus**  
Zimmer und Essen gehen weiter.  
*Im Netz bekannt:* ja: „Beleidigter Herbergsvater wirft nicht raus“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=10)  

**Nr. 336: Essen in der Taverne verbraucht zusätzlich Proviant**  
Nach dem Essen fehlt ein Proviantpaket.  
*Im Netz bekannt:* ja: „Taverne verbraucht zusätzlich Proviant“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=27)  

**Nr. 338: Treborn Kolbergs Laden: Ortszeile leer oder fremd**  
Leere Namensleiste, Zeichensalat.  
*Im Netz bekannt:* ja: „Kolberg-Krämer: leere Namensleiste und Zeichensalat“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=13)  

**Nr. 340: Informant Tiomar mit Manrin statt Brendhil eingetragen**  
Tiomar gilt in Manrin als Ortsinformant.  
*Im Netz bekannt:* ja: „Manriner Wirte: Tiomar „wohnt im Ort““ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=3)  

**Nr. 342: Jurge Torfinsson: „nur Thorwaler“ auch bei reinen Thorwalern**  
Der Satz passt nicht zur Gruppe.  
*Im Netz bekannt:* ja: „Jurge Torfinsson: „nur Thorwaler“ bei reinen Thorwalern“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=3)  

**Nr. 343: Taverne: Gewinn wird als unsinnige Ausgabe angezeigt**  
Nach Beorn erscheint eine Ausgabe von z. B. 20000.  
*Im Netz bekannt:* ja: „Dukaten-Abzug bei Beorn (Anzeige 20000)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=24)  

### Reise & Ereignisse

**Nr. 128: Sprung bis Mitternacht (Swafnild) überspringt feste Ereignisse und verbraucht 25 Stunden Licht**  
Nach der Nacht bei Swafnild sind Fackeln und Laternen leer, Rausch und Kettenhemd-Schaden der Stunden fehlen.  
*Im Netz bekannt:* ja: „Verfluchtes Kettenhemd: Schaden nur beim Schlafen“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=2)  

**Nr. 129: Wachenwahl: ESC liest g_wildcamp_herbstatus[−1]**  
Bei ESC erscheint „DU KANNST DOCH AUCH KEINE ZWEI DINGE GLEICHZEITIG MACHEN!“.  
*Im Netz bekannt:* ja: „Solo-Held als Wache: Fenster nicht verlassbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=14)  

**Nr. 130: Bei weniger als 3 Stunden Schlaf wechseln die Wachen nie**  
Bei kurzer Rast hält nur die erste Wache, Zwischenfälle gibt es nur in ihrer Wache.  
*Im Netz bekannt:* nein  

**Nr. 132: Hafenmeister schimpft auch beim Verlassen über fehlendes Geld**  
Wer mit weniger als 1 Silberling „DEN GELDHAI VERLASSEN“ wählt, wird trotzdem angebrüllt.  
*Im Netz bekannt:* nein  

**Nr. 133: Hunger-/Durst-Meldungssperre des NSC wird nicht zurückgesetzt**  
Der NSC meldet nach der Schiffsreise seinen Hunger womöglich nicht.  
*Im Netz bekannt:* nein  

**Nr. 135: Schiffsnamen werden bei jedem Blick neu ausgewürfelt**  
Fragt man im Hafen mehrmals nach, heißen dieselben Schiffe jedes Mal anders.  
*Im Netz bekannt:* ja: „Schiffsname wechselt bei erneuter Auswahl“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=2)  

**Nr. 138: Angezeigte Reisedauer nur in groben Stufen**  
Die Hafenliste zeigt für eine Route nur wenige typische Stundenwerte.  
*Im Netz bekannt:* nein  

**Nr. 145: Umweg-Linie wird um einen Punkt versetzt zurückgemalt**  
Nach einem Umweg bleibt auf der Reisekarte ein Pixel der Wegfarbe am Ort stehen.  
*Im Netz bekannt:* nein  

**Nr. 147: Rastplatz-Text ohne Platzhalter: Ortsbeschreibung und Kräuterhinweis fehlen**  
An Rastplätzen erscheint immer derselbe Bachlauf-Text, der Hinweis auf Kräuter kommt nie.  
*Im Netz bekannt:* nein  

**Nr. 148: Rastplatz: Status nur für den Anführer gelöscht**  
Ein anderer Held als der Anführer kann am Rastplatz womöglich nicht jagen oder sammeln, weil sein alter Status noch steht.  
*Im Netz bekannt:* nein  

**Nr. 155: Querverbindung: Herkunft wird nach Peilinen statt Kravik/Skelellen bestimmt**  
Nichts, der Wert wird nur mit Bit 0 ausgewertet.  
*Im Netz bekannt:* nein  

**Nr. 156: „AM UFER DES DES BODIR“**  
Doppelter Artikel im Furttext.  
*Im Netz bekannt:* nein  

**Nr. 157: Brückenbau Auplog–Varnheim: Pronomen im falschen Fall**  
„… KÖNNT IHR IHM VOR DEM ERTRINKEN RETTEN“ statt „IHN“.  
*Im Netz bekannt:* nein  

**Nr. 160: Hütte bei Orvil: Frage passt nicht zu den Antworten**  
Die Frage lautet „DIE SPUR AUFNEHMEN?“, die Antworten „AUF DEN WEG UMKEHREN?“/„HIER RASTEN?“.  
*Im Netz bekannt:* nein  

**Nr. 163: Tatzelwurm-Spuren: Name aus einem Nullzeiger**  
Beim zweiten Besuch erscheint ein Müllname im Text über die Tatzelwurm-Spuren.  
*Im Netz bekannt:* nein  

**Nr. 168: Dialogbild fehlt nach einem Lagerkampf**  
Nach einem Kampf im Lager fehlt das Kopfbild im Dialog.  
*Im Netz bekannt:* ja: „Reiseereignisse 98/103/104 fehlerhaft“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 171: Olimone: get_hero(−1), wenn niemand das Rezept nehmen konnte**  
Konnte niemand das Rezept aufnehmen, erscheint ein Müllname, oder ein Held, der das Rezept schon vorher hatte.  
*Im Netz bekannt:* nein  

**Nr. 175: Landschaft 7 liest hinter die 7 Spalten der Begegnungstabelle**  
Auf sieben Wegen (u. a. Ottarje–Orvil, Skjal–Orvil, Skelellen–Phexcaer) gibt es als Zufallsbegegnung nur die Karenjagd.  
*Im Netz bekannt:* nein  

**Nr. 182: Krankheitsprobe mit uninitialisiertem Helden**  
Die Chance auf Dumpfschädel auf dem Weg zur Ruine ist zufällig.  
*Im Netz bekannt:* nein  

**Nr. 183: Waldweg: Erfolgstext beim ersten Versuch unerreichbar**  
Findet die Gruppe den Weg gleich beim ersten Versuch, erscheint trotzdem der Text für die lange Suche.  
*Im Netz bekannt:* nein  

**Nr. 369: Nach Gorah dreht sich die Reiserichtung immer um**  
Auch auf dem Weg nach Orvil geht es zurück zu Umbrik.  
*Im Netz bekannt:* ja: „Rückkehr zu Umbrik nach Gorah“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=2)  

**Nr. 371: Grenz- und Straßentexte ohne Richtungsprüfung**  
„Orkland betreten“ und „hier endet die Straße“ auch auf dem Rückweg.  
*Im Netz bekannt:* ja: „„Orkland betreten“ auf dem Rückweg“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)  

**Nr. 374: Berghütte: Jagd zweimal möglich** · **Exploit**  
Jagd und Kräutersuche wiederholbar.  
*Im Netz bekannt:* ja: „Wiederholbare Jagd an der Berghütte“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=10)  

**Nr. 375: Jagd: Einzelheld schafft immer den Blattschuss** · **Exploit**  
Ein einzelner Held erlegt nach gelungenem Schleichen immer die volle Beute.  
*Im Netz bekannt:* ja: „Jagd: Blattschuss ohne Fernwaffe“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=26)  

### Dungeons

**Nr. 81: KK-Probe zählt Platz 0 ungeprüft**  
Beim Aufbrechen von Türen zählt die Kraft des ersten Heldenplatzes auch, wenn er tot ist oder zu einer anderen Gruppe gehört.  
*Im Netz bekannt:* nein  

**Nr. 82: Gelber Pfeil der aktiven Gruppe wird vom violetten verdeckt**  
Stehen zwei Gruppen auf einem Feld, zeigt die Karte den violetten Pfeil statt des gelben.  
*Im Netz bekannt:* ja: „Gruppe-vereinigen-Symbol bleibt aktiv“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 83: Sichtprüfung <= 16 lässt eine 17. Spalte neben der Karte zu**  
Eine Gruppe (oder das Kreuz) in Spalte 16 wird rechts neben die Karte gezeichnet.  
*Im Netz bekannt:* nein  

**Nr. 85: Kletter-Meldung nennt Plätze 0, 1, 2 … ohne Gruppenprüfung**  
Bei geteilter Gruppe nennt die Meldung falsche oder leere Namen.  
*Im Netz bekannt:* nein  

**Nr. 86: Proben und Hilfen zählen versteinerte und bewusstlose Helden mit (nur !dead geprüft)**  
Ein bewusstloser Magier hilft mit dem Stab beim Klettern, ein versteinerter Held schleicht beim Einbruch mit oder rettet die Gruppe mit Gefahrensinn.  
*Im Netz bekannt:* ja: „Daspota-Schatz: bewusstlose Helden springen über Speergrube“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=4)  

**Nr. 89: Symbol „Gruppen vereinen“ bleibt nach dem Vereinen aktiv**  
Nach dem Vereinen im Dungeon ist das Symbol nicht ausgegraut.  
*Im Netz bekannt:* ja: „Gruppe-vereinigen-Symbol bleibt aktiv“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 93: Proben und Wirkungen mit get_hero(0) statt dem ersten handlungsfähigen Helden**  
Kletterprobe am Totenschiff-Spalt, Sinnesschärfe an zwei Geheimtüren der Verfallenen Herberge und der CH-Abzug am Kothaufen treffen den ersten Heldenplatz, auch wenn er tot ist.  
*Im Netz bekannt:* nein  

**Nr. 95: Totenschiff: Meldung vom sinkenden Schiff kommt nie**  
Verlässt man das Totenschiff nach dem Dämon rechtzeitig, fehlt die Meldung, dass es hinter euch versinkt.  
*Im Netz bekannt:* ja: „Totenschiff: Abschlusstext und Goldschlüssel“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 96: sprintf mit der Zahl 10 statt eines Namens**  
Beim Goldschlüssel aus der Rätseltruhe erscheint statt des Heldennamens Datenmüll.  
*Im Netz bekannt:* ja: „Totenschiff: Abschlusstext und Goldschlüssel“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 98: Klebriger Boden: „GELINGT ES, SICH LOSZUREISSEN“ nur nach einem Fehlversuch**  
Die Befreiungsmeldung fehlt, wenn der Anführer es sofort schafft.  
*Im Netz bekannt:* nein  

**Nr. 99: Wegteleportieren von Druckplatte oder Hebel wird nicht erkannt**  
Teleportiert sich eine Gruppe mit Transversalis von einer Druckplatte oder einem Hebel weg, bleiben Wand bzw. Tür im alten Zustand.  
*Im Netz bekannt:* ja: „Verfallene Herberge: Mechanismen per Gruppenteilung umgehbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=18)  

**Nr. 100: Druckplatten der Verfallenen Herberge berücksichtigen andere Gruppen nur halb**  
Mit geteilter Gruppe öffnen und schließen sich die Wände an (10,11) und (11,8) nicht immer passend.  
*Im Netz bekannt:* ja: „Verfallene Herberge: Mechanismen per Gruppenteilung umgehbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=18)  

**Nr. 101: „IHR HÖRT EIN KNIRSCHEN“ auch bei schon offener Wand**  
Die Meldung kommt bei jedem Betreten der Platte.  
*Im Netz bekannt:* nein  

**Nr. 104: Beschwörungsszene erscheint auch nach dem Kampf**  
Die Meldung über die beiden Zauberer und Mumien kommt erneut, auch wenn der Kampf schon stattfand.  
*Im Netz bekannt:* nein  

**Nr. 107: Absturz in den Abgrund: hero_disappear() mit Position 0**  
Nichts Sichtbares, der Held ist ohnehin verloren.  
*Im Netz bekannt:* nein  

**Nr. 108: Ratten beißen auch tote Helden**  
„%s WIRD VON EINER RATTE GEBISSEN!“ erscheint für Tote.  
*Im Netz bekannt:* nein  

**Nr. 109: Armbrustfalle zählt nur Platz 0 und 1**  
Steht die Gruppe nicht auf Platz 0/1, nennt die Meldung einen Müll-Namen, und nur ein Held wird getroffen.  
*Im Netz bekannt:* nein  

**Nr. 112: Merker „Rätseltruhe geplündert“ wird nie gesetzt**  
Die orkische Rätseltruhe stellt ihre Frage immer wieder, „DIE TRUHE IST LEER“ kommt nie.  
*Im Netz bekannt:* nein  

**Nr. 113: Hilferuf aus der Grube jede Minute statt alle 20 Minuten**  
Solange ein Held in der Grube sitzt, erscheint „JEMAND RUFT UM HILFE“ sehr oft.  
*Im Netz bekannt:* nein  

**Nr. 115: Loser Stein: Zustand „entfernt“ wird überschrieben**  
Die Frage nach dem losen Stein kommt bei jedem Besuch wieder.  
*Im Netz bekannt:* ja: „Runin-Drachenhöhle: Stein wird nicht gemerkt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=6)  

**Nr. 123: Zähler der Grubenversuche steht nicht im Spielstand**  
Nach dem Laden beginnt die unsichtbare Sperre an der Speergrube wieder bei Versuch 1.  
*Im Netz bekannt:* nein  

**Nr. 347: Dungeonlicht: Lichtquellen anderer Gruppen zählen mit**  
Eine Teilgruppe ohne Licht sieht hell, sobald irgendein Held einer anderen Gruppe eine brennende Lichtquelle trägt.  
*Im Netz bekannt:* ja: „Lichtanzeige bei Teilgruppen falsch“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 352: Giftdorn-Truhe nach sechs Ladungen ohne Text**  
Die Truhe reagiert stumm.  
*Im Netz bekannt:* ja: „Spinnenhöhle: Giftdorn-Truhe ohne Text“ (https://www.crystals-dsa-foren.de/printthread.php?tid=502&page=4)  

**Nr. 353: Spinnenaltar: Auswahl ohne „später“, Text und Kampf wiederholen sich**  
Mit Kristallen lässt sich die Box nicht wegklicken, beim zweiten Kristall kommen Text und Kampf erneut.  
*Im Netz bekannt:* ja: „Spinnenaltar: Mactans-Text ohne Kampf“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=2)  

**Nr. 360: Orkhöhle: jedes Durchsuchen der Strohlager kostet CH −2**  
Wiederholtes Durchsuchen senkt CH weiter.  
*Im Netz bekannt:* ja: „Orkhöhle: CH-Verlust beim Durchsuchen“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=26)  

**Nr. 361: Runin: Drachenhort und Platinschlüssel-Auftrag unabhängig** · **Exploit**  
Wurfdolch und Gold gibt es doppelt.  
*Im Netz bekannt:* ja: „Runin-Drachenhöhle: Drachenschatz/Wurfdolch doppelt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=5)  

### Kampf

**Nr. 4: Doppelgegner überschreibt beim Zug über eine Leiche deren Feldinhalt**  
Nachdem ein zweiteiliger Gegner (z. B. Höhlendrache) über eine Leiche gelaufen ist, verschwindet sie vom Kampffeld.  
*Im Netz bekannt:* ja: „Leichen verschwinden unter zweifeldrigen Gegnern“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=17)  

**Nr. 5: Schwanzteil fliehender oder selbst getöteter Doppelgegner bleibt auf dem Feld**  
Nach Flucht oder Selbsttötung eines zweiteiligen Gegners bleibt ein unsichtbares Hindernis auf dem Schlachtfeld.  
*Im Netz bekannt:* ja: „Phantom-Hinterteile / unsichtbare Sperren“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=16)  

**Nr. 8: Schwanzteil eines durch eigenen Patzer getöteten Doppelgegners bleibt stehen**  
Siehe Nr. 5: unsichtbares Hindernis, Ziel für Skelettarius.  
*Im Netz bekannt:* ja: „Phantom-Hinterteile / unsichtbare Sperren“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=16)  

**Nr. 9: Abfrage g_max_enemies != 0 nach dem Sieg ist nie wahr**  
Geflohene Gegner eines gekürzten Kampfes bleiben in der Kampfliste stehen.  
*Im Netz bekannt:* nein  

**Nr. 10: Fluchtverteilung übergeht den NSC auf Platz 6**  
Flieht die Gruppe im Dungeon in verschiedene Richtungen, landet der NSC immer bei der Restgruppe.  
*Im Netz bekannt:* nein  

**Nr. 12: Flucht über den oberen und unteren Rand kostet ein BP mehr**  
Über den linken Rand kann ein Held mit einem BP weniger fliehen als über oben oder unten.  
*Im Netz bekannt:* nein  

**Nr. 13: Fluchtfeld wird mit dem Inhalt einer früheren Cursorposition geprüft**  
Ein freies Fluchtfeld kann als „ZIEL: BLOCKIERT“ erscheinen, nach Cursorbewegung klappt es.  
*Im Netz bekannt:* ja: „Fluchtfelder als „Blockiert“ angezeigt“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=13)  

**Nr. 14: Verschachtelte ifs: tote Figur auf dem Zielfeld überspringt die Weg- und BP-Prüfung**  
Zeigt der Cursor auf eine Leiche, die zu weit oder unerreichbar ist, erscheint „ZIEL: 99 BP“.  
*Im Netz bekannt:* ja: „Zielanzeige „99 BP“ statt „zu weit““ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=2)  

**Nr. 16: Knochen mit Rune beendet die Beuteliste**  
Gegenstände, die in der Beute hinter dem Knochen mit Rune liegen (Kampf THOR8), werden nicht angeboten.  
*Im Netz bekannt:* nein  

**Nr. 17: Meldung „ZIEL IST NICHT MEHR IN GÜLTIGER SCHUSSPOSITION“ kann doppelt kommen**  
Die Meldung erscheint zweimal hintereinander.  
*Im Netz bekannt:* nein  

**Nr. 19: Fluchtfelder (negative Ids) werden als Held gelesen**  
Nichts Sichtbares, höchstens wird eine Schusslinie falsch als blockiert oder frei gewertet.  
*Im Netz bekannt:* nein  

**Nr. 20: Umzingelt-Prüfung mit Grenze 25 statt 23**  
Nichts Spürbares.  
*Im Netz bekannt:* nein  

**Nr. 26: memcpy 20 Byte in g_fig_move_pathdir[10]**  
Nichts.  
*Im Netz bekannt:* ja: „FIG_backtrack: Wege im 20er-Abstand / found_dir uninitialisiert“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=14)  

**Nr. 27: Schwanzteile toter Doppelgegner sind nicht begehbar**  
Auf dem Feld eines toten Drachenschwanzes kann ein Held stehen bleiben, aber nicht hindurchgehen, für Gegner ist es gesperrt.  
*Im Netz bekannt:* nein  

**Nr. 28: Grenzprüfung: new_y > 23 doppelt, new_x > 23 fehlt**  
Gegner mit Fernangriff suchen sich am rechten Rand gelegentlich ein falsches Feld zum Schießen.  
*Im Netz bekannt:* ja: „seg038: doppelte Bereichsprüfung in der Wegfindung“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=14)  

**Nr. 29: Geworfene Waffen werden nur als Gegenstandsnummer gemerkt** · **Exploit**  
Nach dem Kampf bekommt man geworfene Waffen als neue Stücke zurück: zerbrochene sind wieder heil, Gift und „magisch erkannt“ sind weg.  
*Im Netz bekannt:* nein  

**Nr. 31: Held gegen Held: Werte des Verteidigers falsch berechnet**  
Greift ein verhexter Held einen Gefährten an, stimmen AT und PA des Angegriffenen nicht.  
*Im Netz bekannt:* nein  

**Nr. 32: Paradepatzer setzt backfire_1 statt backfire_2**  
Bei einem Paradepatzer wird die falsche Figur animiert.  
*Im Netz bekannt:* nein  

**Nr. 311: Geflohener Gegner gilt als getötet (Gorah)**  
Flieht Gorah (z. B. durch Horriphobus), bekommt die Gruppe trotzdem Knochenrune, Hütte und Truhe, der Text spricht von seiner Leiche.  
*Im Netz bekannt:* ja: „Gegner gilt nach Flucht als getötet (Gorah)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)  

**Nr. 312: Gefesselte Gegner bleiben in der Kampfliste und kämpfen beim nächsten Betreten erneut** · **Exploit**  
Ein mit Band und Fessel gefesselter Gegner greift beim nächsten Betreten wieder an. AP gibt es jedes Mal, Beute nur einmal.  
*Im Netz bekannt:* ja: „Gegner greift nach „Band und Fessel“ erneut an“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)  

**Nr. 317: Computerkampf: Held mit 1 BP oder weniger als 3 Rest-BP bleibt untätig**  
Ein Computerheld mit nur 1 BP oder unter 3 Rest-BP nach einer Bewegung rückt nicht weiter vor.  
*Im Netz bekannt:* ja: „Held mit 1 BP bleibt untätig“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=7)  

**Nr. 320: Gegner fliehen auch über den rechten Rand, Helden nicht**  
Gegner verlassen die Karte rechts, wo Helden nicht fliehen können.  
*Im Netz bekannt:* ja: „Gegner flieht in Richtung ohne Fluchtfeld“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=13)  

**Nr. 327: Piratenangriff: Ergebnis von do_fight() wird nicht ausgewertet** · **Exploit**  
Flucht aus dem Seekampf bleibt folgenlos. Nach „Verlassen/Laden“ erscheint trotzdem der Ausrüstungsbildschirm.  
*Im Netz bekannt:* ja: „Seekampf „neu laden“ zeigt Ausrüstungsbildschirm“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

### Magie

**Nr. 187: Transversalis-Kreuz darf eine Zeile unter die Karte**  
Das Zielkreuz kann unter die Karte wandern.  
*Im Netz bekannt:* nein  

**Nr. 193: Spieltext „AUF %S RUHT …“ mit großem S**  
Statt des Heldennamens erscheint Unsinn oder „%S“.  
*Im Netz bekannt:* nein  

**Nr. 194: „IST DOCH GARNICHT TOT“ ohne Subjekt**  
Skelettarius auf einen lebenden Gegner meldet nur „IST DOCH GARNICHT TOT.“  
*Im Netz bekannt:* nein  

**Nr. 195: Skelettarius: Leiche darunter verschwindet aus der Auswahl**  
Liegt die Zielleiche auf einer anderen, wird diese noch angezeigt, lässt sich aber nicht mehr wählen.  
*Im Netz bekannt:* ja: „Skelettarius: Fehler bei Leichenstapeln und Hinterteilen“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=16)  

**Nr. 198: Balsam: Mindestkosten 7 AE nicht einheitlich**  
Wer weniger als 7 LE heilen will, zahlt 7 AE, wer mehr verlangt als fehlen, zahlt nur die fehlenden LE, auch unter 7.  
*Im Netz bekannt:* nein  

**Nr. 200: Abbruch der Gegenstandswahl: inventory[−1]**  
Nach Abbruch untersuchen die Zauber ein Phantom vor dem Inventar, Analüs kann dabei ein Bit im Heldendatensatz setzen.  
*Im Netz bekannt:* nein  

**Nr. 201: Analüs: volle AE-Kosten, wenn nur der Zuschlag die Probe kippt**  
Gelingt die Grundprobe, scheitert aber am gegenstandseigenen Zuschlag, kostet Analüs die vollen AE statt der Hälfte.  
*Im Netz bekannt:* nein  

**Nr. 202: „Eigenschaften seid gelesen“ zeigt 80 % des Nahkampfschadens**  
Der Zauber zeigt einen kleineren Schaden an, als der Gegner im Nahkampf macht.  
*Im Netz bekannt:* nein  

**Nr. 203: Exposami vergleicht mit einem noch leeren Listeneintrag**  
Gelegentlich fehlt eine Gegnerart in der Exposami-Liste.  
*Im Netz bekannt:* nein  

**Nr. 204: Hexenknoten: Grafikfehler beim zweiten Wirken**  
Wird Hexenknoten mehrmals gewirkt, wird die Barriere falsch gezeichnet, in späteren Kämpfen mit fremden Daten.  
*Im Netz bekannt:* nein  

**Nr. 206: Spieltext „VERWANDLET“**  
Tippfehler im Spieltext.  
*Im Netz bekannt:* nein  

**Nr. 208: Paralue auf einen Helden prüft das alte Zauberziel**  
War das letzte Zauberziel der Zauberer selbst, scheitert Paralue auf einen Helden still und kostet nichts.  
*Im Netz bekannt:* nein  

**Nr. 210: Brenne entzündet immer die erste Fackel oder Laterne**  
Wählt man die zweite unentzündete Fackel zum Anzünden, brennt die erste.  
*Im Netz bekannt:* nein  

**Nr. 381: Böser Blick auf Leichen möglich**  
Böser Blick gelingt auf tote Gegner und kostet AE.  
*Im Netz bekannt:* ja: „Böser Blick auf Tote anwendbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 384: Herberge: Regenerationssperre nach Magie hängt am Heldenplatz** · **Exploit**  
Ein Magier, der in der Herberge gezaubert hat, regeneriert nach einem Platztausch doch.  
*Im Netz bekannt:* ja: „Stabzauber-Regeneration“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=28)  

### Technik & Oberfläche

**Nr. 275: Übertrag der Subtraktion fehlt im High-Word der Titellänge**  
Nichts bei normalen CD-Titeln, ein Titel könnte sonst zu lange laufen.  
*Im Netz bekannt:* nein  

**Nr. 277: Rückgabewert bei RLE-Bildern nicht gesetzt**  
Nichts.  
*Im Netz bekannt:* nein  

**Nr. 278: Heldenbild-Abfrage reicht ein Feld zu weit (ACTION_ID_HERO_NPC + 1)**  
Nichts.  
*Im Netz bekannt:* nein  

**Nr. 279: Lesen hinter icons_bak[9]**  
Höchstens ein unnötiges Neuzeichnen der Symbolleiste.  
*Im Netz bekannt:* nein  

**Nr. 280: Bright-Eyes-Fix für die AE-Leiste prüft den falschen Merker**  
Im 3D-Modus wird die AE-Leiste nach AE-Verlust nicht neu gezeichnet (nur Anzeige).  
*Im Netz bekannt:* nein  

**Nr. 281: Anschnitt oben vergrößert die Bildhöhe, statt sie zu verkleinern (3 Stellen)**  
Höchstens Grafikmüll am oberen Rand von Dungeonbildern, die beiden anderen Stellen werden nie erreicht.  
*Im Netz bekannt:* nein  

**Nr. 282: Wanduhr: Position läuft bis 84, Tabelle hat nur 81 Werte**  
In der letzten knappen halben Stunde vor 7 bzw. 19 Uhr springen Sonne und Mond auf der Uhr.  
*Im Netz bekannt:* nein  

**Nr. 284: Zufallszahl kann 0 oder negativ werden**  
Sehr selten (etwa 1 zu 65536) fällt ein Würfelwurf 0 oder −1 statt 1..n aus.  
*Im Netz bekannt:* nein  

**Nr. 286: Toter OPL-Code: Timer 1 startet nie, wirkungslose Befehle**  
Nichts, der Code wird nie aufgerufen.  
*Im Netz bekannt:* nein  

**Nr. 287: Leeres Tagebuch: Vergleich mit gs_diary_entries[−1]**  
Höchstens fehlt einmal der allererste Tagebucheintrag.  
*Im Netz bekannt:* nein  

**Nr. 288: Kopierhöhe line * 7 statt (line − startline) * 7**  
Nichts, die zusätzlichen Zeilen sind leer und durchsichtig.  
*Im Netz bekannt:* nein  

**Nr. 289: Speichern: do-while ohne Prüfung von findfirst**  
Nichts, es gibt immer CHR-Dateien.  
*Im Netz bekannt:* nein  

**Nr. 292: Stadtdatei: liest 100 Byte mehr, als g_locations_tab fasst**  
Nichts, die Stadtdateien sind kurz genug.  
*Im Netz bekannt:* nein  

**Nr. 293: Kein break nach dem Fund**  
Nichts, die Funktion wird nie aufgerufen.  
*Im Netz bekannt:* nein  

**Nr. 294: Bereichsprüfung clip_num > max statt >= max**  
Nichts.  
*Im Netz bekannt:* nein  

**Nr. 295: Toter Zweig bei der Tiefe für Objekte 58..61**  
Nichts.  
*Im Netz bekannt:* nein  

**Nr. 296: Dukaten-Anzeige läuft ab 32768 D über**  
Ab 32768 Dukaten zeigt das Spiel einen negativen oder falschen Betrag.  
*Im Netz bekannt:* nein  

**Nr. 297: Bedingung state || mode != 2 ist immer wahr**  
Nichts.  
*Im Netz bekannt:* nein  

**Nr. 298: Lesen vor g_hero_pic_posx bei Maus-x < 3**  
Nichts.  
*Im Netz bekannt:* nein  

**Nr. 299: Kartenwerte ab 0xf0 lesen hinter die Tabelle**  
Nichts, solche Werte kommen in Stadtkarten nicht vor.  
*Im Netz bekannt:* nein  

**Nr. 303: Pronomentabelle hat 7 statt 8 Einträge**  
Nichts, der einzige Aufrufer nutzt den 4. Fall Singular.  
*Im Netz bekannt:* nein  

**Nr. 305: Feldzugriff vor der Grenzprüfung (3 Stellen)**  
Nichts.  
*Im Netz bekannt:* nein  

**Nr. 306: Abfrage „WIE VIELE DER … SOLLEN …“ im falschen Fall**  
Grammatikfehler in der Mengenabfrage beim Übergeben.  
*Im Netz bekannt:* nein  

**Nr. 308: Speichermeldung nennt 5000 Byte zu wenig**  
Bei zu wenig Speicher nennt die Startmeldung einen zu kleinen oder negativen Fehlbetrag.  
*Im Netz bekannt:* nein  

**Nr. 310: sprintf mit mehr Argumenten als Platzhaltern**  
Nichts.  
*Im Netz bekannt:* nein  

**Nr. 409: Namensbeugung: Eigennamen mit Artikel, Adjektive ungebeugt**  
„DER GORAH“, „DER GOLDENER SCHILD IST NICHT MAGISCH“.  
*Im Netz bekannt:* ja: „Eigennamen in Kampfmeldungen mit Artikel“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=12)  

**Nr. 410: Kampfmusik läuft nach direkt gestarteten Dungeonkämpfen weiter**  
Nach manchen Dungeonkämpfen bleibt die Kampfmusik.  
*Im Netz bekannt:* ja: „Kampfmusik hört nach dem Kampf nicht auf“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=1)  

**Nr. 411: Umlaute im Spielstandnamen ergeben „Diskette voll“**  
Speichern scheitert mit falscher Meldung.  
*Im Netz bekannt:* ja: „Umlaute im Spielstandnamen → „Festplatte voll““ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=25)  

**Nr. 412: Namenlose Tage im Tagebuch als −5. bis −1. Swafnir**  
Falsches Datum im Tagebuch.  
*Im Netz bekannt:* ja: „Namenlose Tage im Tagebuch als -5. bis -1. Swafnir“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 413: Zeitvorspulen ohne Bremse**  
Auf schnellen Rechnern rast die Uhr.  
*Im Netz bekannt:* ja: „Zeitvorspulen auf schnellen Rechnern blitzartig“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=3)  

**Nr. 415: Schiffsreise mit Händlermusik**  
Auf See läuft TERMS.XMI.  
*Im Netz bekannt:* ja: „Schiffsreisemusik fehlt (CD)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=8)  

**Nr. 416: Zufallswurf vor der Erledigt-Prüfung**  
Nichts.  
*Im Netz bekannt:* ja: „Zufallswurf vor Prüfung“ (https://www.crystals-dsa-foren.de/printthread.php?tid=1471&page=1)  

## Unsicher falsch, kritisch (7)

### Helden & Werte

**Nr. 401: Entlassene Helden hinterlassen Lücken in den Heldenplätzen**  
Sind die ersten Plätze nach dem Entlassen leer, kann das Spiel beim Reisen oder bei Zufallsbegegnungen einfrieren.  
*Warum:* char_letgo() nullt nur den Platz, GRP_sort_heroes() fehlt. Viele Stellen setzen lückenlose Plätze ab 0 voraus (vgl. Nr. 41, 235). Absturzpfad vermutlich, nicht verfolgt.  
*Im Netz bekannt:* ja: „Absturz bei Zufallsbegegnung mit leeren ersten Heldenplätzen“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=16)  

### Reise & Ereignisse

**Nr. 174: Zustand 33 ist eine Endlosschleife**  
Erreicht das Gespräch mit der alten Frau je Zustand 33, hängt das Spiel.  
*Warum:* do {} while (1). Ob F092.TLK diesen Zustand ansteuert, ist nicht geprüft (vermutlich nie).  
*Im Netz bekannt:* nein  

### Dungeons

**Nr. 362: Fluch der Goldmünzen (Burgruine): jeder Kampf gilt sofort als gewonnen** · **Exploit**  
Nimmt eine Gruppe die verfluchten Münzen, endet jeder weitere Kampf ohne Kampf als gewonnen, für alle Gruppen.  
*Warum:* do_fight() liefert bei gs_dng15_took_cursed_money sofort 0, der Merker ist global und wird nie gelöscht. Die verfluchte Gruppe sitzt in der Ruine fest. Ob auch der Endkampf so übersprungen wird, vermutlich ja, nicht geprüft.  
*Im Netz bekannt:* ja: „Burgruine: Goldmünzen-Fluch verhindert alle Kämpfe“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=28)  

### Kampf

**Nr. 7: Heldenzug ohne verbliebene Heldenphasen: Endlosschleife möglich**  
Unter seltenen Umständen bleibt der Kampf hängen, wenn die Helden keine Aktion mehr haben, die Gegner aber schon.  
*Warum:* Der Zweig nr_hero_act_phases_left_in_round == 0 steht hinter „<= nr_enemy…“ und ist tot. Beginnt eine Runde mit 0 Heldenphasen (z. B. einziger handlungsfähiger Held mit liegengebliebener Fluchtaktion) und kommen die Helden zuerst, bekommen sie eine Phase, FIG_choose_next_hero() findet keinen Helden und würfelt endlos. Nicht im Spiel nachgestellt.  
*Im Netz bekannt:* nein  

**Nr. 15: Bei genau 30 Beutestücken schreibt „MEHR“ über einen nicht gesetzten Zeiger**  
Bei sehr viel Beute (genau 30 Stück) kann das Blättern auf Seite 2 Speicher zerstören oder abstürzen.  
*Warum:* loot_names[31], belegt werden nur 0..29. Auf Seite 2 ist more_entry_pos = item_cnt = 30, strcpy(loot_names[30], „MEHR“) schreibt über einen Zeiger vom Stack. Was dort liegt, hängt vom Stack ab.  
*Im Netz bekannt:* nein  

**Nr. 30: Angstgift: Schreiben über nicht gesetzten Zeiger bei Held gegen Held**  
Schlägt ein verhexter Held mit Angstgift-Waffe einen anderen Helden, wird zufälliger Speicher verändert.  
*Warum:* enemy_p wird nur bei attack_hero == 0 gesetzt. Im Zweig Angstgift werden flags.scared und flags.renegade über den uninitialisierten Zeiger geschrieben. Was getroffen wird, hängt vom Stack ab.  
*Im Netz bekannt:* nein  

### Technik & Oberfläche

**Nr. 309: Fehlgeschlagener Start von GEN.EXE: Spiel läuft ohne Puffer weiter**  
Lässt sich die Heldenerschaffung nicht starten, läuft das Spiel ohne globalen Puffer, Maus und Timer weiter und stürzt vermutlich ab.  
*Warum:* Nach cleanup_game() fehlt exit(). Nur wenn spawnl() scheitert (fehlende Datei, zu wenig Speicher).  
*Im Netz bekannt:* nein  

## Unsicher falsch, mittel (32)

### Helden & Werte

**Nr. 238: Erneute Vergiftung löscht die Protokollbytes**  
Wird ein bereits vergifteter Held erneut mit demselben Gift vergiftet, bleiben die schon erlittenen Verluste dauerhaft.  
*Warum:* status, time_counter und log_1..3 werden ohne Prüfung auf 0 gesetzt. poison_effect() gibt KK/GE (Shurin) bzw. AE (Bannstaub) nach den Protokollbytes zurück. Gleiches Muster wie Bright-Eyes 46.  
*Im Netz bekannt:* nein  

**Nr. 262: Heilen Krankheiten: Wundfieber und Blaue Keuche nie heilbar, Erfolg wird als Misserfolg gemeldet**  
Gelingt die Probe auf Wundfieber oder Blaue Keuche, meldet das Spiel trotzdem „DAS HEILEN MISSLINGT“ mit 3 Schaden.  
*Warum:* Die Erfolgsbedingung schließt beide Krankheiten aus, die Meldung landet im Misserfolgszweig, retval > 0 wird trotzdem zurückgegeben. Ob die Krankheiten per Talent unheilbar sein sollten, ist offen, die Meldung ist aber falsch.  
*Im Netz bekannt:* nein  

**Nr. 402: Aufgesparte Zaubersteigerungen laufen über 127** · **Exploit**  
Der Zähler springt von 127 auf −128, Steigern bleibt danach praktisch unbegrenzt möglich.  
*Warum:* saved_spell_increases ist signed char ohne Obergrenze, Prüfungen mit != 0. Erreichbar vermutlich mit Nichtmagiern, die Versuche sammeln (Netz 119). Index 86 siehe Nr. 252.  
*Im Netz bekannt:* ja: „Zaubersteigerungs-Zähler läuft über“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=14)  

**Nr. 406: Rüstungsbehinderung wirkt auf keine Talentprobe**  
Schwimmen und Klettern sind in schwerer Rüstung nicht schwerer.  
*Warum:* test_talent() berücksichtigt rs_be nicht.  
*Im Netz bekannt:* ja: „Rüstungsbehinderung wirkt nicht auf körperliche Talente“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=4)  

### Städte & Orte

**Nr. 45: Nicht vorhandene Antworten werden mitgemischt**  
In Zufallsgesprächen kann eine alte oder leere Antwort sichtbar werden, die in den falschen Zustand führt.  
*Warum:* opt0_txt..opt2_txt werden nur für vorhandene Antworten gesetzt, sonst bleibt der Wert eines früheren Zustands (oder Stackmüll). Die Schutzprüfung != txt_offset greift auch bei richtiger Belegung nur bei Zufall 0. Ob die Dialogdaten solche Zustände enthalten, ist nicht geprüft.  
*Im Netz bekannt:* nein  

**Nr. 55: Volle Korbsumme wird bezahlt, auch wenn nicht alles verteilt werden konnte**  
Passt nicht alles in die Rucksäcke, zahlt die Gruppe trotzdem den ganzen Korb.  
*Warum:* Nur wenn gar nichts verteilt wurde (nothing_given), entfällt die Zahlung. Die Meldung „mehr gekauft, als ihr tragen könnt“ (Text 566) erscheint, könnte also Absicht sein. Folge des Platzfehlers Nr. 54.  
*Im Netz bekannt:* nein  

**Nr. 58: Tavernenmerker firstinfo, infopromise, round werden nie zurückgesetzt**  
Nach der ersten Information in irgendeiner Taverne verlaufen spätere Tavernengespräche dauerhaft anders.  
*Warum:* Die drei Merker stehen im Spielstand und werden nur gesetzt (infopromise in Zustand 71 gelöscht). fullness, testdrunk, drinkcount werden dagegen bei Gesprächsbeginn zurückgesetzt. Dialogwirkung nicht im Spiel geprüft.  
*Im Netz bekannt:* nein  

**Nr. 63: Firun-Wunder „keinen Hunger und Durst mehr“ wirkt nur auf einen Helden**  
Der Text spricht alle an, satt bleibt aber nur ein zufälliger Held.  
*Warum:* set_mod_slot(…, get_random_hero() …). sprintf übergibt einen Namen, den der Text nicht nutzt. miracle_modify() behandelt sonst alle Helden der Gruppe.  
*Im Netz bekannt:* nein  

**Nr. 66: Zeughaus: Störrebrandt-Schulden werden aus dem Staatsbudget bezahlt** · **Exploit**  
Wer bei Störrebrandt Schulden hat, zahlt im Zeughaus 50 D davon mit dem Geld des Hetmanns. Der Rest des Budgets wird auf ganze Dukaten abgerundet.  
*Warum:* Das Budget ersetzt während do_merchant() das Gruppengeld. Die Schuldenabfrage für Thorwal ab 100 D greift dort weiter und zieht 50 D ab.  
*Im Netz bekannt:* nein  

**Nr. 337: Schmied Gurthag nach dem Kellerausgang dauerhaft unbenutzbar**  
Nach dem Weg durch Gurthags Keller kann man dort nie wieder reparieren.  
*Warum:* gs_dng14_cellarexit_flag wird nie zurückgesetzt, do_smith ruft nur talk_smith(). Kann gewollt sein.  
*Im Netz bekannt:* ja: „Gurthag nach Zwingfeste unbenutzbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=13)  

**Nr. 339: Akademie: Bezahlung mit erstem gleichen Stück, auch kaputt** · **Exploit**  
Bezahlt man mit einem Gegenstand, wird auch ein kaputtes Stück genommen, Magierstäbe sind nicht ausgenommen.  
*Warum:* Auswahl ohne kaputte, Bezahlung per get_first_hero_with_item().  
*Im Netz bekannt:* ja: „Schule der Hellsicht: Bezahlung mit kaputtem Gegenstand/Zauberstab“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

### Reise & Ereignisse

**Nr. 131: Seeweg Prem–Manrin nur 54 km lang**  
Die längste Seeroute der Karte ist im Spiel eine der kürzesten.  
*Warum:* Vermutlich 310 gemeint, im Byte bleibt 310 − 256 = 54. Bright-Eyes setzt 255 als Notbehelf.  
*Im Netz bekannt:* ja: „Extrem kurze Schiffsreise Prem–Manrin“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=12)  

**Nr. 134: Efferds Schutz verhindert den Krakenmolch, nicht die Piraten**  
Trotz „EFFERD GEWÄHRT EUCH SEINEN SCHUTZ AUF WASSER“ greifen Piraten an.  
*Warum:* Der Timer INGAME_TIMER_EFFERD_SAFE_PASSAGE wird nur beim Krakenmolch geprüft. Ob Piraten gemeint waren, ist Auslegung.  
*Im Netz bekannt:* nein  

**Nr. 143: Gewaltmarsch-Tempo hängt von der schon gegangenen Strecke ab** · **Exploit**  
Wer den Gewaltmarsch spät auf einem Weg beginnt, ist um ein Vielfaches schneller.  
*Warum:* gs_travel_speed = gs_travel_step_counter + 197. Gemeint war vermutlich ein fester Wert (197 = 1,97 km/h).  
*Im Netz bekannt:* nein  

**Nr. 146: Axt-Liste für Reiseereignisse**  
Wurfaxt und Wurfbeil gelten beim Brückenbau und am umgestürzten Baum als Axt, Orknase und magische Orknase (Daspota) nicht.  
*Warum:* Abgleich Netz 93: Orknase und magische Orknase sind in ITEMS.DAT WEAPON_TYPE_AXT, die Liste nimmt aber leichte Wurfäxte und lässt sie weg. Bright-Eyes nennt die Liste selbst „a bit arbitrary“. Früher als Design gewertet, jetzt als Fehler (unsicher).  
*Im Netz bekannt:* ja: „Magische Axt aus Daspota für Konstruktionen unbrauchbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=6)  

### Dungeons

**Nr. 90: Rückwärtsschritt prüft halbdurchlässige Wände nicht** · **Exploit**  
Einbahn-Wände lassen sich rückwärts gehend immer durchqueren.  
*Warum:* Vorwärts wird bei DNG_TILE_SEMIPERMEABLE_WALL das Richtungsflag geprüft, rückwärts fehlt der Fall. Ob die Dungeonkarten solche Wände haben, ist nicht geprüft (seg085 setzt eine).  
*Im Netz bekannt:* ja: „Einseitige Geheimtüren/Illusionswände rückwärts passierbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)  

**Nr. 92: Totenschiff-Spalt: nur Seil oder Strickleiter, kein Magierstab**  
Am Spalt im Totenschiff hilft der verwandelte Magierstab nicht, anders als an der Grube.  
*Warum:* Geprüft werden nur Seil und Strickleiter. DNG_check_climb_tools() würde den Stab ab Stufe 3 zulassen. Eher eine Designlücke. Netz 94: Ohne Seil ist die Bilge eine Sackgasse, der Magierstab hilft nicht.  
*Im Netz bekannt:* ja: „Zauberstab nicht als Seil/Wurfhaken (Totenschiff-Bilge)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=5)  

**Nr. 106: Hebel und Geheimtür verlangen Betreten UND Drehen im selben Schritt**  
Der Hebel an (0,9,11) in der Spinnenhöhle und die Geheimtür an (4,8) im Orkdungeon sind praktisch nicht zu benutzen bzw. zu finden.  
*Warum:* target_pos != pos_bak && viewdir != viewdir_bak. Beim Gehen bleibt die Blickrichtung gleich, beim Drehen bleibt die Position gleich. Die Schwesterstellen (Hebel (1,3,11), Geheimtüren seg078) haben ||. Nicht im Spiel geprüft.  
*Im Netz bekannt:* ja: „Spinnenhöhle: Speergruben-Truhe ohne Schlossprobe / Hebel nicht findbar“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=6)  

**Nr. 111: Schleifen über 7 Plätze beginnen beim ersten verfügbaren Helden statt bei Platz 0**  
Ist Platz 0 nicht handlungsfähig, werden Helden übersehen und Daten hinter dem Heldenfeld gelesen und verändert, z. B. MU +3 durch das Elixier, Essen am Kessel.  
*Warum:* hero bleibt auf dem ersten verfügbaren Helden stehen, die Schleife läuft trotzdem 7 Plätze. Liegt dieser auf Platz k > 0, werden k Einträge hinter dem Heldenfeld geprüft und ggf. beschrieben. Beim Verlassen von Dungeon 7 auf anderem Weg bleibt MU +3 dauerhaft. Was hinter dem Feld liegt, ist nicht geprüft.  
*Im Netz bekannt:* nein  

**Nr. 120: Zeitabfragen „day_timer % X == 0“ greifen mehrmals hintereinander**  
Im Giftgasraum gibt es vermutlich mehr als 1 LE Schaden je 10 Minuten, die einsturzgefährdeten Stollen der Zwergenmine stürzen schneller ein.  
*Warum:* gs_day_timer läuft im Timer-Interrupt. Die Dungeonschleife sieht dieselbe Marke in mehreren Durchläufen hintereinander und zählt jedes Mal. Nicht im Spiel gemessen.  
*Im Netz bekannt:* ja: „Oberorken: Gangstücke stürzen zu früh ein“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=4)  

**Nr. 124: Texte 31 und 32 fehlen in der deutschen OBER.DTX**  
Ab dem dritten Sprung über die Speergrube und beim Durchbrechen erscheint ein falscher oder unsinniger Text.  
*Warum:* OBER.DTX hat nur die Texte 0..30. get_tx(31) und get_tx(32) lesen Zeiger hinter der Indextabelle (Reste eines früher geladenen Textes). sprintf() benutzt den Text als Format. Was dort steht, ist nicht geprüft.  
*Im Netz bekannt:* ja: „Oberorken: Datenmüll beim Rennen gegen das Naturgemälde“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=1)  

**Nr. 348: Lager in Dungeons: immer einer von vier allgemeinen Lagerkämpfen**  
Auf dem Totenschiff überfallen Orks in Burgkulisse.  
*Warum:* g_dcampfights hat nur 4 Kämpfe für alle Dungeons.  
*Im Netz bekannt:* ja: „Falsche Lagerkämpfe in Dungeons“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=1)  

### Kampf

**Nr. 1: Speer wird als Fernkampfwaffe behandelt**  
Wer im Nahkampf mit dem Speer zusticht, macht nur Schaden, wenn eine Probe auf Schusswaffen gelingt. Geworfen wird der Speer abgelegt und trifft wie waffenlos.  
*Warum:* Speer und magischer Speer haben in der Waffentabelle keinen KK-Zuschlag, aber eine Fernkampftabelle. FIG_weapon_gfx_id_melee() schließt WEAPON_TYPE_SPEER aus, also läuft jeder Speerangriff durch den Fernkampfzweig: Probe auf TA_SCHUSSWAFFEN, Entfernungsabzug „extrem nah“. range_attack_check_ammo() wirft den (nicht magischen) Speer wie ein Wurfbeil. Die KI nimmt ihn nie als Ersatz-Nahkampfwaffe. Nach DSA ist der Speer Nahkampfwaffe (Talent Speere). Ob Attic einen Wurfspeer meinte, ist offen, daher unsicher.  
*Im Netz bekannt:* ja: „Fernkämpfer wechseln im Nahkampf nicht auf Nahkampfwaffe“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=4)  

**Nr. 11: Zauberziel: begehbare Objekte (Id ≥ 50) werden nicht abgewiesen**  
Ein Zauber auf ein begehbares Deko-Objekt wird angenommen und wirkt auf Speicher hinter der Gegnerliste.  
*Warum:* Anders als bei Angriff und Gegenstand prüft der Zauberzweig nur < 0. FIG_do_hero_action() zieht 20 ab und greift auf g_enemy_sheets[20..46] zu (u. a. flags.asleep = 0). Welche Daten dahinter liegen, ist nicht geprüft.  
*Im Netz bekannt:* nein  

**Nr. 21: Gegner schießen auch auf direkt benachbarte Helden**  
Gegner mit Bogen oder Fernzauber schießen auch Helden auf dem Nachbarfeld an.  
*Warum:* Die Suche entlang der Blickrichtung beginnt beim Nachbarfeld, ein Mindestabstand wird nicht geprüft. Nach DSA ist Fernkampf im Handgemenge nicht möglich, ob Attic das so wollte, ist offen.  
*Im Netz bekannt:* ja: „Kein Fernkampf auf Nachbarfeld (nur für Helden)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 37: Gegner-Plumbumbarum wirkt eine Stunde statt 5 Kampfrunden**  
Trifft ein Gegner einen Helden mit Plumbumbarum, bleibt dessen AT noch lange nach dem Kampf um 3 gesenkt.  
*Warum:* set_mod_slot(…, HOURS(1), …). Der Heldenzauber wirkt nur 5 Kampfrunden. Ob Absicht, ist offen.  
*Im Netz bekannt:* nein  

**Nr. 38: Endkampf: ohne Grimring kämpft Heldenplatz 0**  
Legt man Grimring im Statusmenü vor dem Endkampf ab, wird der Held auf Platz 0 zum Hauptakteur, auch wenn er nicht dabei oder kampfunfähig ist.  
*Warum:* grimring_hero_pos = 0, wenn get_first_hero_with_item() −1 liefert. Keine Prüfung von Gruppe und Zustand.  
*Im Netz bekannt:* nein  

**Nr. 314: Kampf: Waffe oder Gegenstand lässt sich nur tauschen, nicht ablegen**  
Im Kampf gibt es keinen Wechsel auf die bloße Hand. Waffe und Schild gegen einen Zweihänder (etwa nach Waffenbruch) zu tauschen, ist blockiert.  
*Warum:* Beide Menüpunkte bieten nur belegte Rucksackplätze an und tauschen per move_item(). Ablegen fehlt, die Zweihandprüfung blockiert dann. Ob Absicht, ist offen.  
*Im Netz bekannt:* ja: „Gegenstand im Kampf nicht ablegbar / Wechsel blockiert“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=2)  

**Nr. 316: Zerbrochene Waffe bleibt in der Hand, Computerkampf wechselt nicht**  
Bricht eine Waffe, kämpft der Held mit ihr weiter (wie waffenlos). Computerhelden und NSC greifen nicht zur Ersatzwaffe.  
*Warum:* Bei Waffenbruch wird nur flags.broken gesetzt. AFIG_switch_to_melee_weapon() läuft nur bei unbrauchbarer Fernwaffe.  
*Im Netz bekannt:* ja: „Zerbrochene Waffe bleibt in der Hand“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=4)  

**Nr. 325: Spinnendämon gilt als Tier und ist für Waffengift empfänglich** · **Exploit**  
Mactans lässt sich mit Sanftmut oder Herr über das Tierreich bezwingen und vergiften.  
*Warum:* MONSTER_ID_SPINNENDAEMON hat is_animal = 1 (Datenfehler), Waffengift wirkt ohne Immunitätsprüfung.  
*Im Netz bekannt:* ja: „Dämonen durch Gift und Tierzauber bezwingbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=14)  

### Magie

**Nr. 383: Transversalis auf der Landreise kostet AE ohne Wirkung**  
Auf der Reise lässt sich Transversalis wirken, AE gehen weg, es passiert nichts.  
*Warum:* Geprüft werden nur gs_dungeon_id/gs_town_id, gs_town_id steht auf der Reise vermutlich noch auf der Abreisestadt.  
*Im Netz bekannt:* ja: „Transversalis außerhalb von Stadt/Dungeon“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=29)  

### Technik & Oberfläche

**Nr. 414: Einhorn: Held als Zeiger im Spielstand**  
Nach Laden oder Platzwechsel prüft das Einhorn-Gespräch einen falschen Helden.  
*Warum:* gs_unicorn_hero_ptr ist ein Speicherzeiger.  
*Im Netz bekannt:* ja: „Einhorn: Held als Speicheradresse gespeichert“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=1)  

## Unsicher falsch, egal (34)

### Helden & Werte

**Nr. 271: Zeitzähler von Krankheiten und Giften können überlaufen**  
Bei sehr langen Krankheiten (z. B. dem nie endenden Schlachtfeldfieber) wird der Zähler nach 127 Tagen negativ.  
*Warum:* time_counter ist signed char und wird ohne Grenze erhöht. Folgen hängen von der jeweiligen Krankheit ab, nicht geprüft.  
*Im Netz bekannt:* nein  

**Nr. 405: Angriffsart bleibt nach dem Kampf im Helden stehen**  
Beim Export nach Sternenschweif bleibt die AT/PA-Verschiebung (aggressiv/vorsichtig) erhalten.  
*Warum:* fight_atpa_mod wird nicht zurückgesetzt. In Schick folgenlos, im Export sichtbar.  
*Im Netz bekannt:* ja: „AT/PA-Verschiebung bleibt beim Export“ (https://www.crystals-dsa-foren.de/printthread.php?tid=1471&page=1)  

### Gegenstände & Inventar

**Nr. 216: Truhenliste: 20 Byte je Name zu kurz**  
Lange Gegenstandsnamen laufen in den nächsten Eintrag, am Listenende über das Feld.  
*Warum:* char names[20][20]. „MAGISCHER BROTBEUTEL“ braucht 21, „REZEPT FÜR STARKEN HEILTRANK“ 29 Byte. loot_simple_chest() nutzt 30 Byte. Ob eine Truhe so einen Namen enthält, ist nicht geprüft.  
*Im Netz bekannt:* nein  

**Nr. 224: Waffengift: Schleuder wird nicht ausgeschlossen**  
Gift lässt sich auf eine Schleuder auftragen.  
*Warum:* Ausgeschlossen werden nur Kurzbogen, Langbogen und Armbrust.  
*Im Netz bekannt:* nein  

**Nr. 392: Zwei Kriegsbeile mit verschiedenen Werten**  
Zwei gleich aussehende Kriegsbeile haben verschiedene AT/PA, das unhandlichere macht mehr Schaden.  
*Warum:* KRIEGSBEIL__SPECIAL und KRIEGSBEIL. Datenfehler oder Sonderstück.  
*Im Netz bekannt:* ja: „Zwei Kriegsbeil-Sorten mit verschiedenen Werten“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=6)  

**Nr. 399: Schild mit Zauberstab oder Hexenbesen kombinierbar**  
Schild und Stab gleichzeitig möglich.  
*Warum:* Zweihändig ist nur WEAPON_TYPE_ZWEIHAENDER, Stäbe sind WEAPON_TYPE_SPEER.  
*Im Netz bekannt:* ja: „Schild und Zauberstab/Hexenbesen kombinierbar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=14)  

### Städte & Orte

**Nr. 74: Phextempel: gs_ptemple_closed wird nie gesetzt**  
Nach der Beleidigung der Geweihten passiert nichts, der verschlossene Betraum (Text 60) erscheint nie.  
*Warum:* Der Zweig für den geschlossenen Tempel ist unerreichbar. Vermutlich sollte das Flag nach Text 56 gesetzt werden.  
*Im Netz bekannt:* nein  

**Nr. 335: Wein und Schnaps ohne Zechenprobe**  
Wein oder Schnaps machen sofort betrunken.  
*Warum:* hero_get_drunken() ohne Probe. Nüchtern fest um 10 Uhr (gewollt).  
*Im Netz bekannt:* ja: „Alkohol ohne Zechenprobe / zu schnell nüchtern“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=21)  

### Reise & Ereignisse

**Nr. 141: Ork-Monolith (Ereignis 71) steht auch auf dem Weg Oberorken–Felsteyn**  
Auf dem Weg Oberorken–Felsteyn kann der Monolith „AM ANDEREN UFER DES THOSSEL“ erscheinen, nur einmal im Spiel.  
*Warum:* Ereignis 71 gehört zu Thoss–Liskor. Andere Mehrfacheinträge liegen nur auf Wegen mit gemeinsamem Ort (67/68 Ala, 62–64 Orvil, 87/88 Einsiedlersee). Wahrscheinlich ein Datenfehler. gs_tevent071_flag ist gemeinsam.  
*Im Netz bekannt:* nein  

**Nr. 169: „NACH GUT ACHT STUNDEN“, vergehen aber 12**  
Text und verstrichene Zeit passen nicht zusammen.  
*Warum:* timewarp(HOURS(12)) zu Text 68. Möglich, dass Umweg plus Rückweg gemeint sind.  
*Im Netz bekannt:* nein  

**Nr. 173: Einsiedlersee: nur der erste Wasserschlauch wird gefüllt**  
Am Seeufer wird nur ein Wasserschlauch je Held gefüllt.  
*Warum:* inv_slot_of_item() findet den ersten Schlauch. Anderswo (Fass) werden alle gefüllt. Ob Absicht, ist offen.  
*Im Netz bekannt:* nein  

**Nr. 373: Floß ohne Werkzeug baubar**  
Ein Floß kostet nur eine Stunde.  
*Warum:* Kein Werkzeug geprüft, Brücken prüfen g_travel_event_axes. Kann Absicht sein.  
*Im Netz bekannt:* ja: „Floß ohne Seil/Axt baubar“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=6)  

### Dungeons

**Nr. 116: Trittplatte wirkt nur beim Entschärfen**  
Unentdeckt oder in Ruhe gelassen tut die Trittplatte nichts, nur ein misslungenes Entschärfen löst die Bolzen aus.  
*Warum:* Text 13 fragt nach dem Entschärfen, der Schaden folgt nur dort. Seltsam, aber als Absicht lesbar. Abgleich Netz 292: Eine Trittplatte, die unentdeckt nie auslöst, ist eher ein fehlender Zweig als Absicht. Jetzt Fehler (unsicher). Das Entschärfen wird zudem nicht gemerkt.  
*Im Netz bekannt:* ja: „Runin-Drachenhöhle: Trittplatte/Bolzenfalle“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=6)  

**Nr. 356: Zwingfeste: Anseilen erst ab fünf Seilen, kein Seil verbraucht**  
Unabhängig von der Gruppengröße.  
*Warum:* Feste Grenze 5.  
*Im Netz bekannt:* ja: „Zwingfeste: Abstieg verlangt immer 5 Seile“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=11)  

**Nr. 358: Oberorken-Wasserraum: Türen beliebig oft knacken (je 1 AP)** · **Exploit**  
Wiederholtes Knacken bringt AP.  
*Warum:* Knacken prüft den Wasserzustand nicht. Wiederschließen nicht gefunden.  
*Im Netz bekannt:* ja: „Oberorken: Wasserraum-Türen beliebig oft knackbar (AP)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=6)  

### Kampf

**Nr. 24: best_viewdir möglicherweise uninitialisiert**  
Nichts Beobachtetes.  
*Warum:* Bleibt uninitialisiert, wenn Start und Ziel gleich sind. Ob das vorkommt, ist offen. Die Initialisierung schadet nicht.  
*Im Netz bekannt:* ja: „FIG_backtrack: Wege im 20er-Abstand / found_dir uninitialisiert“ (https://www.crystals-dsa-foren.de/printthread.php?tid=5191&page=14)  

**Nr. 33: Waffenbruch bei gleichen Würfen trifft nie die Waffe eines Gegners**  
Gegnerwaffen gehen nie kaputt, eine unzerbrechliche Angreiferwaffe schützt auch die Waffe des Verteidigers.  
*Warum:* Gegner haben keinen Bruchfaktor, darum vermutlich bewusst so. Die Kopplung an bf != −99 des Angreifers ist aber seltsam.  
*Im Netz bekannt:* nein  

**Nr. 35: double_size bleibt beim Rückverwandeln eines Doppelgegners ungesetzt**  
Vermutlich nichts.  
*Warum:* Im Zweig für doppelt große Figuren wird g_fig_list_elem.double_size nicht gesetzt, der alte Wert bleibt.  
*Im Netz bekannt:* nein  

**Nr. 315: Schusslinie: bewusstlose und schlafende Figuren blockieren, Leichen nicht**  
Liegende (bewusstlose, schlafende) Helden versperren die Schusslinie, Tote und Büsche nicht.  
*Warum:* Freigegeben werden leere Felder, Objekte aus g_cb_obj_nonobstacle und Figuren mit flags.dead, nicht andere liegende Figuren.  
*Im Netz bekannt:* ja: „Schuss über Büsche, aber nicht über liegende Figuren“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=2)  

**Nr. 318: Computerkampf zaubert ohne Blick auf das Ziel**  
Computerhelden wirken Horriphobus auf umzingelte oder fliehende Gegner und Balsam auf Helden mit vollen LE.  
*Warum:* Zauber zufällig aus g_af_spell_list, Zustand und fehlende LE des Ziels werden nicht geprüft. Balsam bei vollen LE kostet trotzdem 7 AE (vgl. Nr. 198). Eher KI-Schwäche.  
*Im Netz bekannt:* ja: „KI zaubert sinnlos (Horriphobus, Balsam)“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=8)  

**Nr. 321: Verschreckter Gegner ohne Fluchtweg tut nichts mehr**  
Ein umstellter fliehender Gegner bleibt stehen und wehrt sich nicht mehr.  
*Warum:* Bei erreichtem le_flee ohne Fluchtweg bp = 0, kein Angriff. Aufgeben ist nicht vorgesehen. Kann Absicht sein.  
*Im Netz bekannt:* ja: „Umstellter Gegner kann nicht fliehen“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=27)  

**Nr. 323: Schwacher Heshthot (Runin) hat nach der LE-Kürzung 0 LE**  
Der Heshthot in der Runiner Drachenhöhle ist sofort erledigt.  
*Warum:* MONSTER_ID_HESHTOT__WEAK hat LE 1, fill_enemy_sheet() kürzt auf 5/6, also 0. Datenfehler. Kann als „verschreckter“ Heshthot gewollt sein.  
*Im Netz bekannt:* ja: „Heshthot (Runin) mit 0 LE“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=2)  

**Nr. 324: Keine Waffenresistenzen für Skelette und Mumien**  
Untote nehmen von Stich- und Fernwaffen vollen Schaden.  
*Warum:* Abweichung vom Handbuch, nur Einzelboni (magischer Säbel, Silberstreitkolben). Kann Vereinfachung sein.  
*Im Netz bekannt:* ja: „Skelette/Mumien ohne Waffenresistenzen“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=26)  

### Magie

**Nr. 186: Visibili verhindert den Kampf nur bei einem Einzelhelden**  
Sind alle Helden einer Gruppe mit Visibili unsichtbar, kommt es trotzdem zum Kampf.  
*Warum:* Geprüft wird nur get_hero(0) bei Gruppengröße 1. Ob das Absicht ist, ist offen.  
*Im Netz bekannt:* ja: „Visibili umgeht Kampf nur bei Einzelheld“ (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=34)  

**Nr. 188: Meditation bei LE 6 oder 7 kostet LE und bringt keine AE**  
Ein Magier mit 6 oder 7 LE verliert beim Meditieren noch LE bis 5, ohne AE zu bekommen.  
*Warum:* ae = le − 8 wird ≤ 0, add_hero_ae() gibt nichts, sub_hero_le(ae + 3) zieht noch 1–2 LE ab. Grenzfall, vermutlich in Kauf genommen.  
*Im Netz bekannt:* nein  

**Nr. 205: Eisenrost auf ein Tier kostet volle AE**  
„TIERE HABEN KEINE WAFFEN“, die AE sind trotzdem weg.  
*Warum:* g_spell_special_aecost bleibt −1 (volle Kosten). Bannbaladin setzt im gleichen Fall 0.  
*Im Netz bekannt:* nein  

**Nr. 209: Salander auf zweiteilige Gegner: „strange effect“**  
Ein in einen Pilz verwandelter Drache sieht vermutlich seltsam aus (Schwanzteil bleibt).  
*Warum:* Nur der Kopfteil bekommt das Pilz-Flag, für den Schwanzteil gibt es keine Behandlung. Im Spiel nicht nachgeprüft.  
*Im Netz bekannt:* nein  

**Nr. 386: Analüs erkennt nur 28 Gegenstände, Grimring nicht**  
Analüs meldet bei Gegenständen außerhalb seiner Liste „nichts Besonderes“, auch beim Grimring.  
*Warum:* Nur die 28 Einträge in g_analues_items werden erkannt. Der Kraftgürtel-Text „verleiht einmalig“ steht so in SPELLTXT.LTX (Datenfehler).  
*Im Netz bekannt:* ja: „Grimring nicht als magisch erkannt“ (und 1 weitere Meldung) (https://www.crystals-dsa-foren.de/printthread.php?tid=98&page=6)  

### Technik & Oberfläche

**Nr. 276: NVF-Typ 1: Versatz mit nicht gelesenen Breiten und Höhen**  
Nichts, solange kein Bild vom Typ 1 mit Index > 0 geladen wird.  
*Warum:* In der Borland-Fassung fehlen in der Schleife die Lesezugriffe auf width/height (Bright-Eyes hat sie nur außerhalb von Borland eingefügt, ohne Vermerk). Ob Typ 1 im Spiel vorkommt, ist nicht geprüft.  
*Im Netz bekannt:* nein  

**Nr. 285: Modus 3: Anschnitt oben rückt die Quelle um die Bildbreite statt um 320 vor**  
Vermutlich nichts, sonst ein verschobener Bildausschnitt im Kampfbildschirm.  
*Warum:* Bei Anschnitt oben wird SI um skip_top * width erhöht. Modus 3 liest aus einer 320 breiten Quelle (5 Aufrufer: seg005, seg042, seg098). Wirkt nur, wenn g_pic_copy_rect dort oben schneidet, das ist nicht geprüft.  
*Im Netz bekannt:* nein  

**Nr. 291: Letzte Figur liest das Ende hinter der TAB-Tabelle**  
Nichts, das Bild wird trotzdem geladen.  
*Warum:* p_tab[fig] für fig == max_entries liegt hinter der Tabelle. read_archive_file() begrenzt auf das Dateiende, nur das Puffern kann fehlschlagen.  
*Im Netz bekannt:* nein  

**Nr. 300: Gespiegelte Bilder werden am Rand von der falschen Seite beschnitten**  
Angeschnittene, gespiegelte Dungeonbilder zeigen vermutlich den falschen Ausschnitt.  
*Warum:* copy_solid_permuted() liest ab src + width2 − 1 rückwärts, der Beschnitt überspringt aber die linken Quellspalten. Nicht im Spiel geprüft.  
*Im Netz bekannt:* nein  

**Nr. 302: Beugung der Namen: Bits mit + gesetzt, 2. und 3. Fall verwechselt**  
Bei manchen Gegenständen und Gegnern stimmen Artikel oder Genitiv-S im 3. Fall nicht.  
*Warum:* + statt | setzt Geschlecht und Plural (bei Namen ohne Plural kippt das Pluralbit). Die Abfrage == 1 trifft 2. und 3. Fall Singular, angehängt wird das Genitiv-S auch im 3. Fall. Bright-Eyes selbst nennt den +-Teil eher Unschönheit.  
*Im Netz bekannt:* nein  

**Nr. 304: Spielstandname bearbeiten: Schreibstelle hinter dem Namen**  
Hängt man an einen vorhandenen Spielstandnamen Zeichen an, gehen sie verloren, solange man nicht erst die Rücktaste drückt.  
*Warum:* length >= si gibt auch das Endezeichen aus und setzt pos und dst eine Stelle hinter das Namensende. Nicht im Spiel geprüft.  
*Im Netz bekannt:* nein  

## Geprüft: kein Fehler (36)

Markierungen von Bright-Eyes und Meldungen aus dem Netz, die sich am Code als Absicht oder als harmlos erwiesen haben.

**Nr. 6: „Orig_BUG: I had this loop running infinitely“**  
Nichts nachweisbar.  
*Warum:* Die Schleife endet, solange die Summe der attacks_left dem Zähler in FIG_do_round() entspricht. Skelettarius setzt attacks_left, Böser Blick ändert nur attacks. Eine Ursache für die beobachtete Endlosschleife ist im Code nicht zu finden.  

**Nr. 23: „Why unset 'tied' and not 'illusion'?“**  
Nichts, ein gefesselter Gegner kommt mit 4 % je Zug frei.  
*Warum:* Die Deutung „Illusion verschwindet“ stammte aus einem falschen Kommentar. Das Lösen der Fessel (Band und Fessel) ist als Befreiungschance plausibel.  

**Nr. 75: „this would reactivate algrid“**  
Nichts Fehlerhaftes: nach Treborns Hinweis kann man Algrid erneut ansprechen.  
*Warum:* Algrids Dialog prüft gs_algrid_treborns_hint in Zustand 3 und 4. Das Zurücksetzen auf 1 macht gerade dieses Gespräch möglich, vermutlich gewollt.  

**Nr. 103: „VORSICHT NACH DREI SCHRITT LINKS“ – müsste es „rechts“ heißen?**  
Nichts, der Hinweis stimmt.  
*Warum:* Blick nach Norden auf (12,5): links ist Westen, zur Geheimtür (8,4). Blick nach Süden auf (4,5): links ist Osten, ebenfalls zu x = 8. Beide Inschriften zeigen richtig, nach drei Schritten folgt das Feld der Tür.  

**Nr. 118: „possibly a bug: position (8,12) might be (8,11)“**  
Nichts.  
*Warum:* Von (8,12) nach Norden blickt die Gruppe auf die Geheimtür (8,11), die geöffnet wird. Die beiden anderen Türen funktionieren genauso.  

**Nr. 125: Deckeneinsturz (schwer): mit Werkzeug 18 statt 12 Stunden?**  
Nichts, mit Werkzeug geht es schneller (12 statt 18 Stunden).  
*Warum:* Fehlgelesen: has_items == 0 ergibt 18 h, mit Werkzeug 12 h. Der Bright-Eyes-Fix dreht es falsch herum und erzeugt erst den Fehler. Der echte Fehler dieser Art steckt eine Funktion weiter oben (Nr. 122).  

**Nr. 144: Gewaltmarsch zieht den LE-Verlust auf einmal ab**  
Ein schwacher Held kann beim Gewaltmarsch sofort sterben.  
*Warum:* Bright-Eyes nennt es „brutal und nah an einem Fehler“. Es ist eine Designentscheidung.  

**Nr. 154: „Original-Glitch“: gs_travel_detour = 1 ist zugleich die Nummer des Totenschiffs**  
Nichts.  
*Warum:* gs_travel_detour != 0 beendet nur trv_do_journey(), danach startet do_travel_mode() die Querverbindung. Ein Dungeon wird nicht betreten.  

**Nr. 176: Speer als Jagdwaffe**  
Mit einem Speer darf man auf die Jagd.  
*Warum:* Jagd mit dem Speer ist plausibel. Bright-Eyes nimmt ihn im Rahmen von 60 heraus, für die Jagd ist das eine Designänderung, kein Fehlerfix.  

**Nr. 219: Kraftgürtel aktualisiert abhängige Werte nicht**  
Nichts erkennbar.  
*Warum:* Der Gürtel ändert attrib[KK].current. update_atpa() rechnet mit .normal, AT/PA-Grundwert hängt also nie vom Gürtel ab. Werte wie die Tragkraft lesen .current ohnehin direkt. Abgleich Netz 77/137: Der bekannte Kraftgürtel-Fehler (KK +5 je Umlegen) liegt nicht hier, sondern in move_item(), siehe Nr. 222. Für atpa_base bleibt es kein Fehler.  

**Nr. 228: „What about herokeeping()?“ im stündlichen Timer**  
Nichts.  
*Warum:* herokeeping() läuft bei jeder Eingabeabfrage. magical_chainmail_damage() setzt stündlich g_herokeeping_flag, das löst dort Essen und Trinken aus. Der stündliche Ablauf ist also vollständig.  

**Nr. 236: Rote Anzeige des bleibenden LE-Verlusts (Bright-Eyes-Komfort)**  
Nichts, der Spieler sieht nur nicht, dass sein LE-Maximum dauerhaft gesenkt ist.  
*Warum:* Kein Fehler, sondern eine Komfortfunktion von Bright-Eyes (Formatstring auf %s umbiegen, Wert rot).  

**Nr. 239: „not a real BUG“: keine Prüfung auf leeren Heldenplatz**  
Nichts.  
*Warum:* Bright-Eyes nennt es selbst keinen echten Fehler. Ein leerer Platz bekommt höchstens Krankheitswerte, die nie gelesen werden.  

**Nr. 241: AT-Basis nicht nach DSA 3 (MU + KK + GE) / 5**  
Nichts, der AT/PA-Grundwert folgt den DSA-1-Regeln.  
*Warum:* DSA 1 kennt nur einen AT/PA-Grundwert (IN + KK + GE) / 5. Das ist eine Regelabweichung zu DSA 3, kein Programmfehler.  

**Nr. 247: Brauen endet nur ohne Gift, Krankheit und Stabzaubersperre**  
Ein kranker oder vergifteter Held wird mit dem Brauen erst fertig, wenn er gesund ist.  
*Warum:* Die Bedingungen sind gewollt streng (staffspell_timer wirkt als Regenerationssperre). Ein Fehler ist nicht zu erkennen.  

**Nr. 257: „Why STAFFSPELL?? BUG!“ – staffspell_timer als Regenerationssperre**  
Nach misslungener Wundheilung bzw. beim Brauen regeneriert der Held in der nächsten Nacht nicht.  
*Warum:* staffspell_timer wird nur in GRP_hero_sleep() gelesen und wirkt dort als Sperre der Regeneration. heal_timer ist schon gesetzt. Vermutlich Absicht, nur der Name ist irreführend.  

**Nr. 267: „Original-Bug? forgot? time_counter = 0“**  
Nichts, außer bei Bannstaub (dort verzögert sich die ohnehin fehlerhafte AE-Rückgabe, siehe Nr. 274).  
*Warum:* Bei den Krankheiten setzt der Genesungsblock direkt darunter time_counter = 0. Bei den Giften setzt hero_gets_poisoned() den Zähler beim nächsten Vergiften auf 0, im Zustand HEALTHY wird nicht gezählt.  

**Nr. 272: „Not a bug, but useless“: Markierung sex = 50**  
Nichts.  
*Warum:* Bright-Eyes nennt es selbst keinen Fehler. Der Umweg über sex = 50 findet den Helden nach dem Gruppenwechsel (der die Plätze umsortiert) wieder.  

**Nr. 283: „potential bug“: nur dst von g_pic_copy wird gesichert**  
Nichts erkennbar.  
*Warum:* Gesichert wird nur der Zielzeiger (bzw. zusätzlich das Rechteck). Die Aufrufer von do_pic_copy() setzen x1..y2 und src vor jeder Benutzung neu, ein Fehlbild ist nicht nachweisbar.  

**Nr. 290: „when using EMS something strange happens“**  
Nichts nachweisbar.  
*Warum:* Im Code ist kein Grund für ein Fehlverhalten mit EMS zu erkennen. Die Behauptung ist unbelegt.  

**Nr. 301: „not a bug, but useless“**  
Nichts.  
*Warum:* signpost_typeindex = 1 und gleich danach die Abfrage darauf: überflüssiger Code ohne Wirkung.  

**Nr. 307: Lichtquellen-Suche auch auf leeren Heldenplätzen**  
Nichts.  
*Warum:* Leere Plätze sind genullt (remove_npc, Entlassen), eine brennende Fackel findet sich dort nicht. Netz 156: Die Schleife prüft aber die Gruppe nicht, Lichtquellen anderer Gruppen zählen mit (eigener Eintrag aus dem Netz).  

**Nr. 328: Computerkampf im Endkampf ohne Meldung gesperrt**  
Im Endkampf passiert bei COMPUTERKAMPF nichts.  
*Warum:* Bei FIGHT_ID_F144 absichtlich übergangen, nur ohne Rückmeldung.  

**Nr. 329: „Alte Optionen“ fehlt im zweiten Menü eines Zuges**  
„Alte Optionen“ erscheint nur im ersten Menü eines Zuges.  
*Warum:* Absicht. Ein Verschwinden bis zum nächsten Kampf gibt der Code nicht her.  

**Nr. 330: Zufällige Zugreihenfolge statt Initiative**  
Die Reihenfolge der Akteure ist zufällig.  
*Warum:* Designentscheidung.  

**Nr. 344: Akrobatik und Falschspiel je nach Ort verschieden**  
Akrobatik geht in Taverne und Markt, Falschspiel ist dort gesperrt.  
*Warum:* Gewollter Code (g_forbid_ta_falschspiel).  

**Nr. 363: Spinnenhöhle: Teleport überspringt Feldereignisse**  
Ereignisse auf übersprungenen Feldern kommen nicht.  
*Warum:* Designfolge der feldgebundenen Ereignisse.  

**Nr. 364: Säuregruben treffen nur die ersten beiden Helden**  
Rüstungen verlieren 3 RS.  
*Warum:* Absicht (Proben der ersten beiden Helden).  

**Nr. 376: Gorah-Versteck ohne Weiterreise**  
Mit Umbriks Auftrag nur Silentium, Anschleichen, Fordern oder Umkehren.  
*Warum:* Design.  

**Nr. 377: Gorahs Truhe: immer der erste Held öffnet**  
Keine Auswahl.  
*Warum:* Design.  

**Nr. 378: Nachtrast immer bis 8 Uhr**  
Nach 8 Uhr wird fast ein ganzer Tag gerastet.  
*Warum:* Gewolltes „bis 8 Uhr“.  

**Nr. 387: Horriphobus: verschreckter Gegner gilt nicht als besiegt**  
Verschreckte Gegner bleiben bis zur Flucht im Kampf.  
*Warum:* Gewollte Fluchtlogik.  

**Nr. 388: Zauberproben als Summenprobe (Böser Blick gelingt oft)**  
Zauber gelingen auch mit niedrigem Wert recht oft.  
*Warum:* Bewusste Vereinfachung (Feature-Mod 6).  

**Nr. 389: Kampfzauber außerhalb des Kampfes wählbar**  
Grau gezeichnete Kampfzauber lassen sich wählen, dann kommt „nur im Kampf“.  
*Warum:* Kosmetisch, Meldung folgt.  

**Nr. 408: Zaubersteigerungen bei Nichtmagiern („Waldelf-Jäger-Fehler“)**  
Nichtmagier sammeln Zaubersteigerungen.  
*Warum:* Ursache in GEN.EXE (Typwechsel löscht sie nicht). BLADEM übernimmt die Werte nur. Folge siehe Überlauf-Eintrag (Netz 65).  

**Nr. 417: Textfenster schließen bei jeder Taste**  
Jede Taste schließt.  
*Warum:* Verhalten, kein Fehler.  

## Anhang: Durchgesehene Quellen im Netz

| Kürzel | Titel | URL | Status | Seiten |
|---|---|---|---|---|
| S1 | Crystals-DSA-Foren, Thread „Fehler und Bugs“ (tid=98) | https://www.crystals-dsa-foren.de/showthread.php?tid=98 | gelesen | 34 von 34 (2006–2025) |
| S2 | Crystals-DSA-Foren, Thread „Schicksalsklinge: Umfassender Bugfix-Patch“ von NRS (tid=5191), inkl. Änderungsliste vom 05.09.2016 (130 behobene + 10 bekannte Fehler) | https://www.crystals-dsa-foren.de/showthread.php?tid=5191 | gelesen | 23 von 23 (2016–2026) |
| S3 | Crystals-DSA-Foren, Thread „Schicksalsklinge (original) Modding“ (tid=5047) | https://www.crystals-dsa-foren.de/showthread.php?tid=5047 | gelesen (fast nur Modding, 1 Eintrag) | 7 von 7 |
| S4 | Crystals-DSA-Foren, Thread „Probleme in der Spinnenhöhle“ (tid=502) | https://www.crystals-dsa-foren.de/showthread.php?tid=502&page=3 | gelesen | Seiten 3 und 4 von 6 (1, 2, 5, 6 nicht gelesen) |
| S5 | Crystals-DSA-Foren, Thread „DOSBox-Patch für Schicksalsklinge“ (Hendriks Logger, tid=1471) | https://www.crystals-dsa-foren.de/showthread.php?tid=1471 | gelesen (fast nur Logger-Technik) | 4 von 4 |
| S6 | Steam-Diskussion „Realms of Arkania: Blade of Destiny“ (Kraftgürtel-Bug, Premer Mine) | https://steamcommunity.com/app/237550/discussions/0/5019775982161608143 | gelesen | 1 |
| S7 | Steam-Diskussion, Bugsammlung von Unywien (2013) | https://steamcommunity.com/app/237550/discussions/0/846960628409957221 | gelesen, nicht verwendet (nur HD-Remake) | 1 |
| S8 | VOGONS, Thread zu RoA 1 unter DOSBox | https://www.vogons.org/viewtopic.php?p=22598 | gelesen (nur Emulator-Probleme) | 1 |
| S9 | VOGONS, Thread zu Blade of Destiny | https://www.vogons.org/viewtopic.php?p=316607 | gelesen, nicht verwendet (nur HD-Remake) | 1 |
| – | WebSearch „Schicksalsklinge Bug Fehlerliste“, „…Absturz Exploit Liste“, „Blade of Destiny bug list original DOS“, „Blade of Destiny bug crash … gog“ | – | durchsucht, keine neuen Sammlungen | – |

## Technische Details

Codestelle, genaue Beschreibung, Begründung der Einordnung und Fix-Vorschlag je Nummer: `Analyse/Fehler/Originalfehler.md` und `Analyse/Fehler/Fehler_Raster.tsv` im Quelltext-Paket. Die Fehler sind im rekonstruierten Quelltext bewusst so nachgebaut, damit er Byte für Byte das Original ergibt. Behoben werden sie später über Schalter, wenn auch Schatten über Riva fertig ist.
