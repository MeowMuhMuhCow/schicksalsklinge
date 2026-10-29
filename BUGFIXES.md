# Die Schicksalsklinge – Bugfix-Fassung: Änderungsliste

**Version 1.0 vom 29.10.2026** · inoffiziell · zur deutschen GOG-Fassung 3.02 · **im Spiel noch ungetestet, Tester willkommen**

## 248 Fehler behoben

## So geht's

Nötig: Windows und **„Realms of Arkania 1 - Blade of Destiny Classic“ von GOG** (deutsche Fassung). Kein Python, kein Compiler.

| | |
|---|---|
| 1 | Das Bugfix-Paket herunterladen (bei GitHub unter **Releases**, oder **Code → Download ZIP**) und irgendwohin entpacken |
| 2 | **`PATCHEN.bat`** doppelklicken. Es findet den GOG-Ordner (oder fragt), prüft die eigene `SCHICKM.EXE` per Prüfsumme und baut daraus die Bugfix-EXEs. **Das Original wird nicht verändert** |
| 3 | **`BLADEM_FIX_WICHTIG_STARTEN.bat`** doppelklicken und spielen. Wer wirklich alles behoben haben will: `BLADEM_FIX_ALLES_STARTEN.bat` |

**Funktionieren meine Spielstände weiter?** Ja. Die Bugfix-Fassung ändert das Speicherformat nicht, kein Feld, keine Größe. Eure alten Spielstände aus dem GOG-Ordner erscheinen beim Laden automatisch und lassen sich weiterspielen. Was ihr mit der Bugfix-Fassung neu speichert, landet in einem eigenen Ordner unter `Build\`. Eure Original-Spielstände werden dabei nie überschrieben. Und weil das Format gleich bleibt, kann man jederzeit zurück zum Original.

| Fassung | behebt |
|---|---|
| **WICHTIG** (empfohlen) | kritische und mittlere Fehler: Abstürze, Hänger, verlorene Ausrüstung, falsche Preise, Fehler, die man im Spiel merkt |
| **ALLES** | zusätzlich viele kleine Fehler ohne große Spielwirkung |

## Was ist neu

- Auf Landreisen trifft man zum ersten Mal Höhlenbären und Kvillottern und kann Karene, Wildschweine, Auerochsen und Nashörner jagen. Attic hatte sie eingebaut, ein verkehrter Vergleich machte sie unerreichbar – Nr. 181
- Stufenanstieg: ESC bei der Eigenschaftswahl bringt keine 168.000 Dukaten mehr (Exploit) – Nr. 248
- Spielhänger behoben: übergelaufener Priester, fliehender Doppelgegner, Einbruch (auch Nr. 25, 40) – Nr. 22
- Eine Todesfalle (Totenschiff, Spinnenhöhle) beendet das Spiel nicht mehr auch für die anderen Gruppen – Nr. 84
- Schnellsegler: Schiffstempo 0 bringt das Spiel nicht mehr zum Stehen – Nr. 137
- Seereise: nicht mehr jeder Umweg führt ins Totenschiff – Nr. 372
- Die Kvillotter beißt jetzt wirklich, Bär und Krakenmolch würfeln Attacke und Parade richtig herum (auch Nr. 177) – Nr. 178
- Opferschale: ESC und große Eingaben schenken kein Geld mehr (Exploit) – Nr. 117

## Überblick

| | ganz behoben | teilweise | Summe |
|---|---:|---:|---:|
| WICHTIG (steckt auch in ALLES) | 120 | 5 | 125 |
| nur in ALLES | 118 | 5 | 123 |
| **ALLES gesamt** | **238** | **10** | **248** |

Davon 34 **Exploits** (Fehler, die dem Spieler nützen, z. B. Geld oder Werte aus dem Nichts). Sie sind standardmäßig mit behoben. Wer sie behalten will, baut selbst mit `FIX_EXPLOITS 0` (siehe README).

## Alle Änderungen nach Spielbereich

*Stufe W = schon in WICHTIG, A = nur in ALLES. 💰 = Exploit.*

### Reise & Ereignisse (45)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 127 | **Nur die erste Seeroute (Thorwal–Prem) wird zurückgesetzt** – Außer auf Thorwal–Prem legen die Schiffe nicht um 9 Uhr ab, sondern liegen bis Mitternacht im Hafen. | W |
| 136 | **Passagepreis in 16 Bit: lange Routen werden spottbillig** 💰 – Auf langen Routen kostet eine Karracke oder Luxuspassage nur einen Bruchteil, z. B. 80 statt 735 Heller. | W |
| 137 | **Schiffstempo in 16 Bit, Schnellsegler mit −106: Tempo 0 bringt das Spiel zum Absturz** – Je nach Wetter wird eine Schnellsegler-Passage mit absurder Dauer angezeigt, beim Buchen stürzt das Spiel mit Division durch 0 ab. Andere Schiffe fahren mit falschem Tempo. | W |
| 142 | **Reiseereignisse: Lage wird nach Richtungswechsel falsch korrigiert** 💰 – Einmalige Reiseereignisse lassen sich durch Umkehren wiederholen, gefährliche (z. B. Sümpfe mit Umkehr-Option) durch erneutes Umkehren umgehen, wiederholbare fallen nach dem Umkehren aus. | W |
| 149 | **replenish_stocks() bekommt die Heldenposition als Versuchszähler** – Steht der Anführer auf Platz 3 oder höher, passiert am Rastplatz nichts. | W |
| 150 | **„UMKEHREN“ auf der Rückreise behält die Rückwärtsrichtung (9 Stellen)** – Wer auf dem Rückweg an Furt, Fähre, Sumpf, Hochmoor oder Brücke umkehren will, reist trotzdem weiter in dieselbe Richtung. | W |
| 151 | **Furt: GE-Probe gilt fast immer als bestanden** – Beim Durchqueren einer Furt fällt fast nie jemand ins Wasser (95–100 %). | W |
| 152 | **Fährpreis wächst bei jedem Versuch weiter** – Hat die Gruppe zu wenig Geld oder wartet, steigt der Fährpreis bei jedem neuen Versuch um ein Vielfaches. | W |
| 153 | **Gruppenproben zählen alle 7 Plätze ohne Gruppen- und Todesprüfung** – Beim Brückenbau liefern Helden anderer Gruppen oder Tote die nötigen Proben, beim Verstecken vor der alten Frau kann das Ergebnis kippen. | W |
| 158 | **Steilhang: Strickleiter macht den Sturz schlimmer** – Wer eine Strickleiter dabeihat, stürzt am Hang schwer, nur mit Seil ohne Strickleiter leicht. | W |
| 159 | **Gorahs Hütte: Silentium-Auswahl wird nie wiederholt** 💰 – Wählt man „Silentium“ mit einem Nicht-Magier oder misslingt der Zauber, nähert sich die Gruppe ohne Probe und ohne Hinterhalt, während ein gelungener Silentium den Hinterhalt auslöst. | W |
| 161 | **Kräuterbonus für Gegenstände über 127 greift nie** – An Fundstellen für Jorugawurzel, Thonnysblüte und Kairanhalm gibt es den versprochenen Fundbonus nicht. | W |
| 162 | **Wegzoll 15 Dukaten insgesamt statt pro Person** 💰 – Der Wegelagerer verlangt 15 Dukaten pro Person, die Gruppe zahlt aber nur 15 Dukaten insgesamt. | W |
| 164 | **Waten durch den Bach mit uninitialisiertem Helden** – Beim Durchwaten des Bachs kann zufälliger Speicher verändert werden oder das Spiel hängen. | W |
| 165 | **Sturz in die Klamm: hero_disappear() mit der Antwortzahl als Position** – Ein in die Klamm gestürzter Held lässt sich im Efferdtempel nicht wiederfinden. | W |
| 166 | **Harpyien: Fluchtoption ist kein Versuch** 💰 – „VERSUCHEN ZU FLIEHEN?“ gelingt immer ohne Probe, die Kriegskunst-Probe kommt erst nach einem verlorenen Kampf. | W |
| 167 | **Nebeltal: Funktionsadresse als Held** – Die Orientierungsprobe im Nebeltal wird mit unsinnigen Werten gewürfelt. | W |
| 170 | **Firun-Opfer: Schlafbonus bleibt für spätere Lager stehen** 💰 – Wer am Firun-Schrein opfert, schläft auch in späteren Wildnislagern besser (+1 bis +4), bis ein Rastplatz den Wert löscht. | W |
| 174 | **Zustand 33 ist eine Endlosschleife** – Erreicht das Gespräch mit der alten Frau je Zustand 33, hängt das Spiel. | W |
| 177 | **AT- und PA-Proben verkehrt herum (Höhlenbär, Krakenmolch)** – Gute Kämpfer treffen den Krakenmolch und den Höhlenbären seltener, Helden mit guter Parade werden öfter verletzt. | W |
| 178 | **Kvillotter beißt nie** – Die Begegnung mit der Kvillotter ist harmlos, die Schlange verschwindet immer. | W |
| 179 | **Krakenmolch: GE-Probe falsch ausgewertet** – Der Krakenmolch packt fast nie zu, ab GE 19 nie. | W |
| 180 | **Krakenmolch: Endlosschleife, wenn alle Helden über Bord oder tot sind** – Werden alle Helden über Bord gerissen oder sterben, hängt das Spiel. | W |
| 181 | **Zufallsbegegnungen: Vergleich verkehrt herum, fast alle unerreichbar** – Auf Reisen gibt es nur eine Art Zufallsbegegnung (Kampf WILD1). Jagden, Bär, Schlange usw. kommen nie vor. | W |
| 184 | **Verirrt im Wald: alle sieben Heldenplätze sterben** – Findet eine Teilgruppe nach fünf Tagen nicht aus dem Wald, sterben auch alle Helden der anderen Gruppen, das Spiel ist aus. | W |
| 185 | **Kletterpartie: tödlicher Sturz bei knapp verfehlter Probe statt beim Patzer** – Wer die Kletterprobe um genau einen Punkt verfehlt, stürzt tödlich ab, ein Patzer ist dagegen harmlos. | W |
| 367 | **Weiterreisen ohne Nachtlager über unsichtbares Symbol** 💰 – Auch ohne guten Lagerplatz kann man weiterreisen. | W |
| 372 | **Seereise: jeder Umweg-Wert führt ins Totenschiff** 💰 – Stirbt die reisende Gruppe oder wird der Seekampf verlassen, landen andere im Totenschiff („Boron-Opfer-Trick“). | W |
| 128 | **Sprung bis Mitternacht (Swafnild) überspringt feste Ereignisse und verbraucht 25 Stunden Licht** – Nach der Nacht bei Swafnild sind Fackeln und Laternen leer, Rausch und Kettenhemd-Schaden der Stunden fehlen. | A |
| 129 | **Wachenwahl: ESC liest g_wildcamp_herbstatus[−1]** – Bei ESC erscheint „DU KANNST DOCH AUCH KEINE ZWEI DINGE GLEICHZEITIG MACHEN!“. | A |
| 130 | **Bei weniger als 3 Stunden Schlaf wechseln die Wachen nie** – Bei kurzer Rast hält nur die erste Wache, Zwischenfälle gibt es nur in ihrer Wache. | A |
| 132 | **Hafenmeister schimpft auch beim Verlassen über fehlendes Geld** – Wer mit weniger als 1 Silberling „DEN GELDHAI VERLASSEN“ wählt, wird trotzdem angebrüllt. | A |
| 133 | **Hunger-/Durst-Meldungssperre des NSC wird nicht zurückgesetzt** – Der NSC meldet nach der Schiffsreise seinen Hunger womöglich nicht. | A |
| 135 | **Schiffsnamen werden bei jedem Blick neu ausgewürfelt** – Fragt man im Hafen mehrmals nach, heißen dieselben Schiffe jedes Mal anders. | A |
| 138 | **Angezeigte Reisedauer nur in groben Stufen** – Die Hafenliste zeigt für eine Route nur wenige typische Stundenwerte. | A |
| 145 | **Umweg-Linie wird um einen Punkt versetzt zurückgemalt** – Nach einem Umweg bleibt auf der Reisekarte ein Pixel der Wegfarbe am Ort stehen. | A |
| 148 | **Rastplatz: Status nur für den Anführer gelöscht** – Ein anderer Held als der Anführer kann am Rastplatz womöglich nicht jagen oder sammeln, weil sein alter Status noch steht. | A |
| 156 | **„AM UFER DES DES BODIR“** – Doppelter Artikel im Furttext. | A |
| 160 | **Hütte bei Orvil: Frage passt nicht zu den Antworten** – Die Frage lautet „DIE SPUR AUFNEHMEN?“, die Antworten „AUF DEN WEG UMKEHREN?“/„HIER RASTEN?“. | A |
| 163 | **Tatzelwurm-Spuren: Name aus einem Nullzeiger** – Beim zweiten Besuch erscheint ein Müllname im Text über die Tatzelwurm-Spuren. | A |
| 168 | **Dialogbild fehlt nach einem Lagerkampf** – Nach einem Kampf im Lager fehlt das Kopfbild im Dialog. | A |
| 171 | **Olimone: get_hero(−1), wenn niemand das Rezept nehmen konnte** – Konnte niemand das Rezept aufnehmen, erscheint ein Müllname, oder ein Held, der das Rezept schon vorher hatte. | A |
| 182 | **Krankheitsprobe mit uninitialisiertem Helden** – Die Chance auf Dumpfschädel auf dem Weg zur Ruine ist zufällig. | A |
| 183 | **Waldweg: Erfolgstext beim ersten Versuch unerreichbar** – Findet die Gruppe den Weg gleich beim ersten Versuch, erscheint trotzdem der Text für die lange Suche. | A |
| 375 | **Jagd: Einzelheld schafft immer den Blattschuss** 💰 – Ein einzelner Held erlegt nach gelungenem Schleichen immer die volle Beute. | A |

### Helden & Werte (40)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 229 | **h_index wird nur einmal vor der Schleife auf −1 gesetzt** – Selten verschiebt sich ein abgelaufener Zauber- oder Fluchwert dauerhaft auf einen anderen Helden. | W |
| 233 | **Heil-, Stab- und Lichttimer runden kleine Zeitsprünge weg** – Beim Gehen in Stadt und Dungeon brennen Fackeln und Laternen nicht herunter, Heil- und Stabzaubersperren laufen nicht ab. | W |
| 234 | **Stundenzählung für Kettenhemd-Schaden und Essen falsch** – Bei langen Zeitsprüngen fehlen Schaden des verfluchten Kettenhemds und Hunger/Durst, bei genau 24 Stunden komplett. | W |
| 235 | **Zufallsheld aus Plätzen 0..n−1 statt aus den Gruppenmitgliedern** – Der NSC wird nie als Zufallsheld getroffen. Lebt in der Gruppe nur noch der NSC, hängt das Spiel bei der nächsten Falle. | W |
| 238 | **Erneute Vergiftung löscht die Protokollbytes** – Wird ein bereits vergifteter Held erneut mit demselben Gift vergiftet, bleiben die schon erlittenen Verluste dauerhaft. | W |
| 240 | **Erneute Erkrankung löscht die Protokollbytes, Verluste werden dauerhaft** – Erkrankt ein Held erneut an einer Krankheit, die er schon hat, bekommt er die verlorenen Eigenschaftspunkte nie zurück. | W |
| 243 | **Gruppe teilen klappt nicht richtig, wenn der NSC in einer anderen Gruppe ist** – Zwei Helden lassen sich nicht trennen, und man kann nicht alle bis auf einen abspalten, wenn der NSC woanders steht. | W |
| 244 | **Gruppenwechsel ohne Ausgang, wenn keine Gruppe frei handeln kann** – Sind alle Gruppen im Kerker oder handlungsunfähig, hängt das Spiel nach Ereignissen, im Menü erscheint die Meldung endlos. | W |
| 248 | **Stufenanstieg: ESC bei „WELCHE POSITIVE EIGENSCHAFT“ bringt rund 168 000 Dukaten** 💰 – Bricht man beim Stufenanstieg die Wahl der zu steigernden Eigenschaft mit ESC oder rechter Maustaste ab, bekommt der Held etwa 168 427 Dukaten und keine Eigenschaft. | W |
| 249 | **Stufenanstieg: ESC bei „WELCHE NEGATIVE EIGENSCHAFT“ senkt die Intuition** – Bricht man die Wahl der zu senkenden schlechten Eigenschaft ab, versucht das Spiel stattdessen die Intuition zu senken (fast immer mit Erfolg). | W |
| 250 | **MR-Neuberechnung beim Stufenanstieg verträgt sich nicht mit MR-Gegenständen** – Mit Stirnreif, rotem Ring oder Amulett erscheint die MR-Meldung grundlos, und ein roter Ring in der rechten Hand kostet nach dem Ablegen dauerhaft 2 MR. | W |
| 253 | **Spielende: Merker für bleibenden LE-Verlust wird gelöscht, LE-Maximum bleibt gesenkt** – Exportierte Helden behalten ein gesenktes LE-Maximum, das sich später nicht mehr heilen lässt. | W |
| 254 | **set_mod_slot() mit who = 0 für einen anderen Helden** – Wird die Heldenreihenfolge innerhalb eines Tages geändert, bekommt beim Ablauf ein anderer Held den CH- bzw. GE-Abzug zurück, einer bleibt dauerhaft gesenkt. | W |
| 255 | **Harikas Abschiedsgeschenk kostet Steigerungsversuche** – Verlässt Harika die Gruppe, bekommen die Helden Versuche auf Schleichen, die beim nächsten Stufenanstieg fehlen. | W |
| 256 | **Entlassene NSC kommen nie wieder** – Ein NSC, der die Gruppe verlässt, ist für immer verschwunden, obwohl er nach 31 bzw. 64 Tagen wieder auftauchen sollte. | W |
| 259 | **Brauen: Abtrennen des einzigen Helden einer Gruppe möglich** – Hat die Gruppe nur einen handlungsfähigen Helden (plus NSC oder Bewusstlose), kann man ihn zum Brauen abtrennen. Zurück bleibt eine leere oder handlungsunfähige Gruppe. | W |
| 260 | **Suche nach freier Gruppe ohne Grenze** – Sind alle 6 Gruppen belegt und trennt man einen brauenden Helden ab, wird hinter die Gruppentabelle geschrieben, Helden werden ausgegraut, das Spiel kann abstürzen. | W |
| 264 | **Verschwundener NSC: Datei und Timer um eins verschoben** – Verschwindet ein NSC, bleibt er in seiner Taverne anwerbbar, dafür wird der nächste NSC gesperrt und dessen Datei überschrieben. | W |
| 269 | **Paralyse und Frostschäden: Erholung mit umgekehrter Bedingung** – Nach Paralyse oder Frostschäden kommen KK und GE nie zurück, der Zustand bleibt für immer „Genesung“. Wer keinen Verlust hatte, bekommt sogar je einen Punkt geschenkt. | W |
| 270 | **Schlachtfeldfieber endet nie** – Nach Schlachtfeldfieber erscheint jeden Tag „DIE GELBLICHE FÄRBUNG … GEHT ZURÜCK“, der Held wird nie gesund. | W |
| 273 | **Tollwut: Beruhigen und Sanftmut werten misslungene Proben als Erfolg** – Einen tollwütigen Helden zu beruhigen klappt fast immer, auch durch Helden anderer Gruppen oder Tote, und ein misslungener Sanftmut kostet 15 AE und gilt als Erfolg. | W |
| 274 | **Shurinknollengift und Bannstaub: Rückgabe der Werte fast nie** – Nach Shurinknollengift kommen KK und GE, nach Bannstaub die AE nur alle gut 21 Stunden um einen Punkt zurück statt stündlich. | W |
| 401 | **Entlassene Helden hinterlassen Lücken in den Heldenplätzen** – Sind die ersten Plätze nach dem Entlassen leer, kann das Spiel beim Reisen oder bei Zufallsbegegnungen einfrieren. | W |
| 402 | **Aufgesparte Zaubersteigerungen laufen über 127** 💰 – Der Zähler springt von 127 auf −128, Steigern bleibt danach praktisch unbegrenzt möglich. | W |
| 404 | **Elixiere stapeln sich ohne Obergrenze** 💰 – Mehrere Elixiere heben eine Eigenschaft beliebig an, Werte können überlaufen. | W |
| 230 | **Aufräumen ungültiger Ziele setzt nur den aktuellen Eintrag zurück** – Nichts Spürbares. | A |
| 231 | **Notbehelf bei 100 belegten Timern versagt im Kampf** – Praktisch nichts, es braucht 100 gleichzeitig laufende Änderungen. | A |
| 237 | **BP-Anzeige bei Last ab 110 % falsch** – Der Charakterbogen zeigt 3 BP, im Kampf hat der Held nur 1 BP. | A |
| 242 | **Ohne wählbaren Helden: Textbreite bleibt 3, geliefert wird Platz 0** *(teilweise)* – Danach erscheinen Textfenster schmal, und Held 0 handelt, auch wenn er nicht wählbar war. | A |
| 245 | **Nachtwache folgt beim Tauschen nur einem der beiden Helden** – Nach dem Umstellen der Helden steht eine Wache auf dem falschen Helden. | A |
| 246 | **Regeneration: Ergebnis 0 bleibt 0, −1 wird zu 1** – In einem schlechten Lager regeneriert ein Held bei Wurf 0 nichts, bei noch schlechterem Wurf aber 1 Punkt. | A |
| 251 | **Zaubersteigerung: Zähler i wird je Runde nicht zurückgesetzt** – Nichts Spürbares. | A |
| 252 | **Zufallszauber 1..86 statt 1..85** – Nichts, gelegentlich verpufft ein Würfelversuch. | A |
| 258 | **Misslungene Wundheilung setzt Wundfieber ohne Prüfung** – Hat der Patient schon Wundfieber oder ist in Genesung, beginnt die Krankheit von vorn. | A |
| 263 | **Textdatei wird bei totem Patienten nicht zurückgeladen** – Nach einem Heilversuch aus einem Ereignis auf einen toten oder versteinerten Helden erscheinen danach falsche Texte. | A |
| 265 | **Verschlimmerung zu Blauer Keuche mit 4 statt 5 %** – Kaum spürbar. | A |
| 266 | **Nach dem Wechsel zu Blauer Keuche noch Dumpfschädel-Schaden am selben Tag** – Am Tag der Verschlimmerung gibt es zusätzlich 1W6−1 LE Schaden vom eben beendeten Dumpfschädel. | A |
| 403 | **Stufenaufstieg erst ab 101 statt 100 AP** – Aufstieg einen AP später. | A |
| 405 | **Angriffsart bleibt nach dem Kampf im Helden stehen** – Beim Export nach Sternenschweif bleibt die AT/PA-Verschiebung (aggressiv/vorsichtig) erhalten. | A |
| 407 | **Gruppengeld: Rest der Teilung verfällt** – Ein paar Heller gehen beim Verteilen verloren. | A |

### Städte & Orte (38)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 39 | **Akademie identifiziert und bricht Flüche nur ein einziges Mal im Spiel** – Nach der ersten Identifizierung in der Akademie heißt es „VERSUCHT ES MORGEN“ für immer. | W |
| 40 | **Einbruch: „noch jemand frei?“ zählt die gerade erwischte Gruppe mit** – Sitzen alle anderen Gruppen schon im Kerker und wird auch die letzte freie Gruppe beim Einbruch erwischt, hängt das Spiel. | W |
| 43 | **Statusleiste setzt 6 statt 4 Werte je Held, beim NSC wird die Krankheitstabelle überschrieben** 💰 – Mit NSC in der Gruppe zahlt der Heiler für die Heilung von Wundfieber 100 Heller drauf, und die Talentprobe auf Wundfieber wird leicht. | W |
| 45 | **Nicht vorhandene Antworten werden mitgemischt** – In Zufallsgesprächen kann eine alte oder leere Antwort sichtbar werden, die in den falschen Zustand führt. | W |
| 48 | **Brauender NSC bleibt in der Herberge stecken** – Lässt man Curian in einer Herberge brauen, findet die Gruppe ihn dort nicht wieder, und seine Gruppe lässt sich nicht anwählen. Der NSC ist verloren. | W |
| 49 | **NSC kann in der Herberge nicht zaubern, magic_act[6] schreibt hinter das Feld** – Curian kann in der Herberge keine Zauber anwenden, beim Übernachten wird eine fremde lokale Variable überschrieben. | W |
| 50 | **Brauende Helden: Einzelheld sieht sie nicht, nur der erste wird gefragt** – Kommt ein einzelner Held in die Herberge, wird er nicht gefragt, ob er brauende Gefährten abholen will. Sind mehrere am Brauen, wird nur der erste geprüft. | W |
| 51 | **Nach dem Abholen gelten zusammengeführte Brauer weiter als „brauend“** – Holt man einen von mehreren brauenden Helden ab, werden alle Gruppen vereint, die anderen bleiben im Zustand „braut“. Als Anführer einer Gruppe verursachen sie seltsames Verhalten. | W |
| 52 | **Herberge: Preisaufschlag für Einzelzimmer und Suite vom Schlafsaalpreis berechnet** – In teuren Herbergen kosten Einzelzimmer und Suite fast den Grundpreis, in billigen fast keinen Rabatt. | W |
| 53 | **„Besonders guter Preis“ beim Händler verschlechtert den Verkauf** *(teilweise)* – Nach dem erfolgreichen Händlergespräch („ICH MACHE EUCH EINEN BESONDERS GUTEN PREIS“) bekommt man beim Verkaufen nur 75 %, beim Kaufen ändert sich nichts. | W |
| 54 | **Einkaufskorb: Platzbedarf falsch berechnet** – Der Händler meldet „DIE GRUPPE KANN KEINE WEITEREN GEGENSTÄNDE AUFNEHMEN“, obwohl Platz ist, oder lässt zu viel in den Korb. | W |
| 59 | **Taverne im Gespräch: Runde und Essen werden in Heller statt Silber abgezogen** 💰 – Im Tavernengespräch kosten die Lokalrunde und das Essen nur ein Zehntel des genannten Preises. | W |
| 63 | **Firun-Wunder „keinen Hunger und Durst mehr“ wirkt nur auf einen Helden** – Der Text spricht alle an, satt bleibt aber nur ein zufälliger Held. | W |
| 65 | **Straßenhändler überschreibt den Typ des gerade betretenen Gebäudes** 💰 – Erscheint der Straßenhändler beim Betreten eines Gebäudes, ist das Gebäude danach verdorben: ein Tempel wird zum Praiostempel, eine Taverne kann negative Essenspreise haben. | W |
| 66 | **Zeughaus: Störrebrandt-Schulden werden aus dem Staatsbudget bezahlt** *(teilweise)* 💰 – Wer bei Störrebrandt Schulden hat, zahlt im Zeughaus 50 D davon mit dem Geld des Hetmanns. Der Rest des Budgets wird auf ganze Dukaten abgerundet. | W |
| 67 | **Zwingfeste: bei jedem Betreten 20 Dukaten** 💰 – Solange der Ugdalf-Auftrag läuft, bekommt die Gruppe bei jedem Betreten der Zwingfeste erneut 20 Dukaten. | W |
| 68 | **Akademie erkennt einen Fluch beim Helden auf Platz 0 nicht** – Ist der erste Held verflucht, sagt Aranda „KEINER VON EUCH HAT EINEN FLUCH AUF SICH“. | W |
| 72 | **Spielhaus: Gewinn und Verlust vertauscht** 💰 – Im Spielhaus von Phexcaer verliert die Gruppe, wenn die Mehrheit beim Falschspiel Erfolg hat, und gewinnt, wenn sie versagt. | W |
| 76 | **Informanten werden von „erledigt“ (2) auf „bekannt“ (1) zurückgesetzt** – Schon abgeschlossene Informanten (z. B. Eliane, Asgrimm, Umbrik) lassen sich nach einem Tavernengerücht erneut besuchen. | W |
| 339 | **Akademie: Bezahlung mit erstem gleichen Stück, auch kaputt** 💰 – Bezahlt man mit einem Gegenstand, wird auch ein kaputtes Stück genommen, Magierstäbe sind nicht ausgenommen. | W |
| 341 | **Treborn Kolberg erscheint nach Algrids Hinweis nie, Olvir/Treborn wiederholt sich** *(teilweise)* – Hat Algrid von Kolberg erzählt, trifft man ihn nie (sein Kartenstück fehlt dann). Olvir und Kolberg kommen direkt hintereinander. | W |
| 41 | **Verhaftung nur ab Platz 0, solange die Plätze zur Gruppe gehören** – Der NSC einer verhafteten Gruppe bleibt frei. | A |
| 46 | **Tempel: bei 20, 30, … Helden eine leere Seite** – Bei genau 20, 30, … gespeicherten Helden zeigt die letzte Seite nur „MEHR HELDEN“. | A |
| 47 | **leave_location() dreht die Gruppe auch dort um, wo sie nicht umdrehen soll** *(teilweise)* – Nach dem Lager in Stadt oder Dungeon und nach dem Straßenhändler steht die Gruppe verkehrt herum. In Häusern mit Taverne und Herberge hängt die Blickrichtung davon ab, was man dort getan hat. | A |
| 56 | **Schmied zeigt das Geld des Helden, bezahlt wird aus der Gruppenkasse** – Die Geldanzeige beim Schmied passt nicht zum tatsächlich verfügbaren Geld. | A |
| 57 | **Bei genau passendem Geld Feilschprobe für 0 %** – Hat die Gruppe genau den Reparaturpreis, fragt der Schmied „WER SOLL FEILSCHEN?“. | A |
| 60 | **Trinkprobe über Plätze 0..n−1 statt über die Gruppenmitglieder** – Bei geteilter Gruppe werden fremde Helden betrunken, der NSC nie. | A |
| 61 | **Praios-Wunder: Prüfung auf Zauberkundige wirkungslos** – Nichts im regulären Spiel, es gibt keinen Praiostempel (siehe Nr. 65). | A |
| 62 | **Praios und Hesinde heben Verwandlungen auch bei fremden Gruppen auf** – Ein Wunder kann einen verwandelten Helden einer anderen Gruppe oder trotz Götterzorn heilen. | A |
| 64 | **Taschendieb: falsche Textteile, das Ende „Dieb entkommt“ erscheint nie** – Bei der Verfolgung eines Taschendiebs erscheinen oft Teil 2 oder 3 der Mauer-Geschichte ohne Anfang. | A |
| 69 | **Fluchbrechung: Text 63 erscheint mit „%s“** – Die Meldung zeigt wörtlich „BEMERKT %s, DASS …“. | A |
| 70 | **Göttergunst über Index 0..13 statt 1..14** – Ifirn bekommt Gunst vom Hetmann und von den Statuen nicht, dafür ein ungenutzter Eintrag. | A |
| 71 | **ESC ruft GUI_radio() mit −1 Optionen auf** – Nach ESC erscheint das Auswahlfenster verstümmelt ohne Antworten, ENTER beendet es. | A |
| 77 | **Rückgabewert bleibt oft uninitialisiert** – Nach der Frage nach Neuigkeiten endet das Tavernengespräch zufällig vorzeitig oder geht weiter. | A |
| 78 | **Harikas Feilschbonus auch, wenn sie nicht dabei ist** – Die Gruppe feilscht leichter, solange Harika irgendwo im Spiel ist, auch in einer anderen Gruppe oder tot. | A |
| 332 | **Öffnungszeit 8–19 Uhr gilt auch für Straßenhändler und Treborn Kolberg** – Der Straßenhändler meldet zwischen 19 und 20 Uhr „geöffnet 8–19“. | A |
| 336 | **Essen in der Taverne verbraucht zusätzlich Proviant** – Nach dem Essen fehlt ein Proviantpaket. | A |
| 343 | **Taverne: Gewinn wird als unsinnige Ausgabe angezeigt** – Nach Beorn erscheint eine Ausgabe von z. B. 20000. | A |

### Dungeons (38)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 79 | **Stündliche Sondertimer laufen bei Zeitsprüngen nicht** – Die zweite Einhorn-Begegnung, der Sphärenriss in der Verfallenen Herberge und die Orkmist-Fässer verzögern sich stark, weil Rast und Reise sie nicht weiterzählen. | W |
| 80 | **Nur noch der NSC handlungsfähig: das Spiel läuft weiter** – Werden in der Verfallenen Herberge alle außer dem NSC versteinert (oder verschwinden bzw. sterben sie an der Steilwand), läuft das Spiel mit dem NSC allein weiter. Gruppenwechsel und Zufallsheld hängen dann. | W |
| 84 | **Todesfalle (Totenschiff, Spinnenhöhle) beendet das Spiel auch für Gruppen draußen** – Sinkt das Totenschiff mit einer Teilgruppe an Bord, ist das Spiel aus, obwohl andere Gruppen woanders stehen. | W |
| 87 | **Dietrich-Zähler wird nach dem ersten Ebenenwechsel ständig zurückgesetzt** 💰 – Nach dem ersten Treppenwechsel kann man ein Schloss beliebig oft knacken, ohne dass die Dietriche beim 4. Versuch brechen. | W |
| 88 | **Klammerfehler: Feld (1,8,5) zeigt in jedem Dungeon den Spiegeltext** – Auf Ebene 2, Feld (8,5) lässt sich in jedem Dungeon keine Gruppe vereinen, stattdessen kommt ein Text über Spiegel, und ein Merker der Ruine Hyggeliks wird gesetzt. | W |
| 90 | **Rückwärtsschritt prüft halbdurchlässige Wände nicht** 💰 – Einbahn-Wände lassen sich rückwärts gehend immer durchqueren. | W |
| 91 | **Leeres Wasserfass füllt trotzdem alle Schläuche** 💰 – Aus einem leeren Fass kann man beliebig oft alle Wasserschläuche füllen, „DAS FASS IST NUN LEER“ kommt jedes Mal. | W |
| 94 | **Totenschiff: Deckenloch und Grube mit Zeilenbreite 32 statt 16 gesetzt** – Nach dem Hochklettern durch den Spalt fehlt das Loch an der richtigen Stelle, dafür steht auf beiden Ebenen an (14,14) ein Loch bzw. eine Grube. | W |
| 97 | **Rätseltruhe: 10 AP Zusatzbelohnung werden nie vergeben** – Für die richtige Antwort MARBO gibt es nie die vorgesehenen 10 AP. | W |
| 102 | **weight_sum wird in der Gruppenschleife zurückgesetzt** – Verlässt eine Gruppe die Platte (11,6), schließt sich die Wand (11,8), auch wenn eine andere Gruppe noch auf der Platte oder in der Wandlücke steht (außer es ist Gruppe 6). | W |
| 105 | **Teleporter und Schilfmeer treffen auch Helden anderer Gruppen** – Benutzt eine Gruppe einen der vier Teleporter der Spinnenhöhle, verlieren auch Helden anderer Gruppen (und Tote) 1W6 AE bzw. LE. Im Schilfmeer (Reiseereignis 29) können Helden anderer Gruppen Dumpfschädel bekommen. | W |
| 106 | **Hebel und Geheimtür verlangen Betreten UND Drehen im selben Schritt** – Der Hebel an (0,9,11) in der Spinnenhöhle und die Geheimtür an (4,8) im Orkdungeon sind praktisch nicht zu benutzen bzw. zu finden. | W |
| 110 | **Zwei-Gruppen-Rätsel prüfen nur x/y der anderen Gruppe** 💰 – Eine zweite Gruppe, die irgendwo anders (andere Ebene, anderer Dungeon, Stadt) auf denselben Koordinaten steht, gilt als „am anderen Hebel“. | W |
| 111 | **Schleifen über 7 Plätze beginnen beim ersten verfügbaren Helden statt bei Platz 0** – Ist Platz 0 nicht handlungsfähig, werden Helden übersehen und Daten hinter dem Heldenfeld gelesen und verändert, z. B. MU +3 durch das Elixier, Essen am Kessel. | W |
| 114 | **Armbrustfalle 4 zählt nicht herunter** – Die vierte Bolzenfalle löst bei jedem Betreten aus (2W6+4 Schaden), die anderen drei nur begrenzt oft. | W |
| 117 | **Opferschale: ESC und große Eingaben (16-Bit-Überlauf) schenken Geld** 💰 – An der Opferschale bringt ESC 10 Heller. Gibt man eine große Zahl ein (z. B. 3300), läuft die Rechnung über und die Gruppe bekommt rund 3250 Dukaten, beliebig oft. | W |
| 119 | **Geheimtüren: test_result bei falscher Hebelstellung uninitialisiert** – Bei falscher Hebelstellung kann eine der drei Geheimtüren trotzdem aufgehen. | W |
| 121 | **Verschütteter Gang (Zustand 3) blockiert nicht** – In Hyggeliks Ruine lassen sich zwei verschüttete Gänge einfach durchschreiten, der Hinweis zum Freiräumen erscheint nie. | W |
| 122 | **Mit Werkzeug dauert das Freiräumen länger als ohne** – Mit Schaufel, Hacke, Brecheisen und Wurfbeil braucht die Gruppe 1 Stunde statt 30 Minuten bzw. einen Tag statt 6 Stunden. | W |
| 126 | **Opfer für Efferd und Ingerimm zählen auch ohne Wegwerfen** 💰 – An den Opferstellen (Piratenhöhle, Zwergenfeste) gilt ein Gegenstand als geopfert, auch wenn man „WIRKLICH WEGWERFEN?“ verneint. | W |
| 351 | **Speerfallgrube (Spinnenhöhle): Spielende auch für Gruppen draußen** – Fällt die aktive Gruppe in die Grube, ist das Spiel aus. | W |
| 81 | **KK-Probe zählt Platz 0 ungeprüft** – Beim Aufbrechen von Türen zählt die Kraft des ersten Heldenplatzes auch, wenn er tot ist oder zu einer anderen Gruppe gehört. | A |
| 82 | **Gelber Pfeil der aktiven Gruppe wird vom violetten verdeckt** – Stehen zwei Gruppen auf einem Feld, zeigt die Karte den violetten Pfeil statt des gelben. | A |
| 83 | **Sichtprüfung <= 16 lässt eine 17. Spalte neben der Karte zu** – Eine Gruppe (oder das Kreuz) in Spalte 16 wird rechts neben die Karte gezeichnet. | A |
| 85 | **Kletter-Meldung nennt Plätze 0, 1, 2 … ohne Gruppenprüfung** – Bei geteilter Gruppe nennt die Meldung falsche oder leere Namen. | A |
| 86 | **Proben und Hilfen zählen versteinerte und bewusstlose Helden mit (nur !dead geprüft)** – Ein bewusstloser Magier hilft mit dem Stab beim Klettern, ein versteinerter Held schleicht beim Einbruch mit oder rettet die Gruppe mit Gefahrensinn. | A |
| 89 | **Symbol „Gruppen vereinen“ bleibt nach dem Vereinen aktiv** – Nach dem Vereinen im Dungeon ist das Symbol nicht ausgegraut. | A |
| 93 | **Proben und Wirkungen mit get_hero(0) statt dem ersten handlungsfähigen Helden** – Kletterprobe am Totenschiff-Spalt, Sinnesschärfe an zwei Geheimtüren der Verfallenen Herberge und der CH-Abzug am Kothaufen treffen den ersten Heldenplatz, auch wenn er tot ist. | A |
| 95 | **Totenschiff: Meldung vom sinkenden Schiff kommt nie** – Verlässt man das Totenschiff nach dem Dämon rechtzeitig, fehlt die Meldung, dass es hinter euch versinkt. | A |
| 96 | **sprintf mit der Zahl 10 statt eines Namens** – Beim Goldschlüssel aus der Rätseltruhe erscheint statt des Heldennamens Datenmüll. | A |
| 98 | **Klebriger Boden: „GELINGT ES, SICH LOSZUREISSEN“ nur nach einem Fehlversuch** – Die Befreiungsmeldung fehlt, wenn der Anführer es sofort schafft. | A |
| 101 | **„IHR HÖRT EIN KNIRSCHEN“ auch bei schon offener Wand** – Die Meldung kommt bei jedem Betreten der Platte. | A |
| 107 | **Absturz in den Abgrund: hero_disappear() mit Position 0** – Nichts Sichtbares, der Held ist ohnehin verloren. | A |
| 108 | **Ratten beißen auch tote Helden** – „%s WIRD VON EINER RATTE GEBISSEN!“ erscheint für Tote. | A |
| 109 | **Armbrustfalle zählt nur Platz 0 und 1** – Steht die Gruppe nicht auf Platz 0/1, nennt die Meldung einen Müll-Namen, und nur ein Held wird getroffen. | A |
| 113 | **Hilferuf aus der Grube jede Minute statt alle 20 Minuten** – Solange ein Held in der Grube sitzt, erscheint „JEMAND RUFT UM HILFE“ sehr oft. | A |
| 115 | **Loser Stein: Zustand „entfernt“ wird überschrieben** – Die Frage nach dem losen Stein kommt bei jedem Besuch wieder. | A |
| 347 | **Dungeonlicht: Lichtquellen anderer Gruppen zählen mit** – Eine Teilgruppe ohne Licht sieht hell, sobald irgendein Held einer anderen Gruppe eine brennende Lichtquelle trägt. | A |

### Kampf (30)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 7 | **Heldenzug ohne verbliebene Heldenphasen: Endlosschleife möglich** – Unter seltenen Umständen bleibt der Kampf hängen, wenn die Helden keine Aktion mehr haben, die Gegner aber schon. | W |
| 11 | **Zauberziel: begehbare Objekte (Id ≥ 50) werden nicht abgewiesen** – Ein Zauber auf ein begehbares Deko-Objekt wird angenommen und wirkt auf Speicher hinter der Gegnerliste. | W |
| 15 | **Bei genau 30 Beutestücken schreibt „MEHR“ über einen nicht gesetzten Zeiger** – Bei sehr viel Beute (genau 30 Stück) kann das Blättern auf Seite 2 Speicher zerstören oder abstürzen. | W |
| 18 | **Computerkampf: Helden greifen auch übergelaufene Gegner an** – Im automatischen Kampf schlagen die Helden auch auf Gegner ein, die durch Böser Blick auf ihrer Seite kämpfen. | W |
| 22 | **Übergelaufener Priester (Stufe 2) hängt das Spiel auf** – Wird ein Priester der Stufe 2 mit Böser Blick auf die Seite der Helden gezogen, bleibt das Spiel in seinem Zug hängen. | W |
| 25 | **Fliehender Doppelgegner: Wegsuche hängt** – Kämpfe mit zweiteiligen Gegnern (Drachen, Schlangen) können einfrieren, wenn ein solcher Gegner flieht. | W |
| 30 | **Angstgift: Schreiben über nicht gesetzten Zeiger bei Held gegen Held** – Schlägt ein verhexter Held mit Angstgift-Waffe einen anderen Helden, wird zufälliger Speicher verändert. | W |
| 34 | **Wurfwaffe wird vor der Schadensberechnung abgelegt** 💰 – Wurfwaffen treffen immer, ohne Probe, und machen nur 1W6 wie waffenlos (außer Wurfsterne im Stapel). | W |
| 36 | **Gegner-Balsam heilt über das Maximum oder gar nicht** – Ein Gegnerheiler heilt bei kleinen Wunden 7 LE (auch über das Maximum), bei großen Wunden nichts. | W |
| 38 | **Endkampf: ohne Grimring kämpft Heldenplatz 0** – Legt man Grimring im Statusmenü vor dem Endkampf ab, wird der Held auf Platz 0 zum Hauptakteur, auch wenn er nicht dabei oder kampfunfähig ist. | W |
| 313 | **Lagerüberfall: nur Heldenplatz 0 ist wach, eingeteilte Wachen zählen nicht** – Bei einem nächtlichen Überfall steht nur der Held auf Platz 0 bereit, alle anderen schlafen die erste Runde, egal wer Wache hatte. | W |
| 316 | **Zerbrochene Waffe bleibt in der Hand, Computerkampf wechselt nicht** – Bricht eine Waffe, kämpft der Held mit ihr weiter (wie waffenlos). Computerhelden und NSC greifen nicht zur Ersatzwaffe. | W |
| 319 | **Computerkampf zaubert auch mit Zauberwert unter −5** – Computerhelden versuchen Schadenszauber, die sie nicht beherrschen, und verlieren Zug und halbe AE. | W |
| 322 | **Alte Nahkampfoption ohne Nachbarschaftsprüfung („Schattenboxen“)** 💰 – Mit „alte Optionen“ schlägt ein Held einen Gegner, der nicht mehr neben ihm steht. | W |
| 8 | **Schwanzteil eines durch eigenen Patzer getöteten Doppelgegners bleibt stehen** – Siehe Nr. 5: unsichtbares Hindernis, Ziel für Skelettarius. | A |
| 9 | **Abfrage g_max_enemies != 0 nach dem Sieg ist nie wahr** – Geflohene Gegner eines gekürzten Kampfes bleiben in der Kampfliste stehen. | A |
| 12 | **Flucht über den oberen und unteren Rand kostet ein BP mehr** – Über den linken Rand kann ein Held mit einem BP weniger fliehen als über oben oder unten. | A |
| 13 | **Fluchtfeld wird mit dem Inhalt einer früheren Cursorposition geprüft** – Ein freies Fluchtfeld kann als „ZIEL: BLOCKIERT“ erscheinen, nach Cursorbewegung klappt es. | A |
| 14 | **Verschachtelte ifs: tote Figur auf dem Zielfeld überspringt die Weg- und BP-Prüfung** – Zeigt der Cursor auf eine Leiche, die zu weit oder unerreichbar ist, erscheint „ZIEL: 99 BP“. | A |
| 16 | **Knochen mit Rune beendet die Beuteliste** – Gegenstände, die in der Beute hinter dem Knochen mit Rune liegen (Kampf THOR8), werden nicht angeboten. | A |
| 17 | **Meldung „ZIEL IST NICHT MEHR IN GÜLTIGER SCHUSSPOSITION“ kann doppelt kommen** – Die Meldung erscheint zweimal hintereinander. | A |
| 19 | **Fluchtfelder (negative Ids) werden als Held gelesen** – Nichts Sichtbares, höchstens wird eine Schusslinie falsch als blockiert oder frei gewertet. | A |
| 20 | **Umzingelt-Prüfung mit Grenze 25 statt 23** – Nichts Spürbares. | A |
| 24 | **best_viewdir möglicherweise uninitialisiert** – Nichts Beobachtetes. | A |
| 27 | **Schwanzteile toter Doppelgegner sind nicht begehbar** – Auf dem Feld eines toten Drachenschwanzes kann ein Held stehen bleiben, aber nicht hindurchgehen, für Gegner ist es gesperrt. | A |
| 28 | **Grenzprüfung: new_y > 23 doppelt, new_x > 23 fehlt** – Gegner mit Fernangriff suchen sich am rechten Rand gelegentlich ein falsches Feld zum Schießen. | A |
| 31 | **Held gegen Held: Werte des Verteidigers falsch berechnet** – Greift ein verhexter Held einen Gefährten an, stimmen AT und PA des Angegriffenen nicht. | A |
| 32 | **Paradepatzer setzt backfire_1 statt backfire_2** – Bei einem Paradepatzer wird die falsche Figur animiert. | A |
| 312 | **Gefesselte Gegner bleiben in der Kampfliste und kämpfen beim nächsten Betreten erneut** 💰 – Ein mit Band und Fessel gefesselter Gegner greift beim nächsten Betreten wieder an. AP gibt es jedes Mal, Beute nur einmal. | A |
| 327 | **Piratenangriff: Ergebnis von do_fight() wird nicht ausgewertet** *(teilweise)* 💰 – Flucht aus dem Seekampf bleibt folgenlos. Nach „Verlassen/Laden“ erscheint trotzdem der Ausrüstungsbildschirm. | A |

### Magie (22)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 189 | **Stabzauber: Abfrage „keine Stabzauber mehr“ auf Stufe 7 statt 4** – Nach dem 4. Stabzauber versucht der Magier immer weiter, den 5. zu wirken: die Probe misslingt stets, jeder Versuch kostet halbe AE und 2 Stunden. | W |
| 190 | **Stabzauber-Probe prüft die zweite Eigenschaft doppelt** – Die Probe für den Stabzauber benutzt die falsche Eigenschaft. | W |
| 191 | **Foramen aus dem Zaubermenü ignoriert den Türzuschlag** 💰 – Wird Foramen frei aus dem Zaubermenü gesprochen, ist die Probe für jede Tür gleich leicht. | W |
| 196 | **Skelettarius belegt bei jedem Wirken 1288 Byte neu im Kampfobjektpuffer** – Wird Skelettarius in einem Kampf oft gewirkt, läuft der Grafikpuffer über, das Spiel kann abstürzen. | W |
| 197 | **Foramen aus dem Zaubermenü wirkt nur nach geöffnetem Türmenü** – Foramen frei gesprochen öffnet eine Tür nur, wenn vorher das Türmenü (Einschlagen, Knacken, Zauber) offen war. | W |
| 199 | **Hexenspeichel: AE-Prüfung greift nie** 💰 – Mit Hexenspeichel kann eine Hexe mehr heilen, als ihre AE erlauben. | W |
| 207 | **Armatrutz: ein RS-Punkt mehr, als AE vorhanden** 💰 – Armatrutz bietet eine Stufe mehr an, als die AE bezahlen können, bei Gegnern ebenso. | W |
| 211 | **g_ignite_mode wird nach dem Anzünden nicht zurückgesetzt** – Nach dem Anzünden einer Fackel kann „Brenne toter Stoff“ keine Laterne mehr entzünden (und umgekehrt). | W |
| 212 | **Tollwut: Sanftmut ohne Prüfung des Zauberwerts** *(teilweise)* – Jeder Zauberkundige kann den Tollwütigen mit Sanftmut beruhigen, auch wenn er den Zauber nicht kann. Magier mit 4. Stabzauber brauchen trotzdem 15 statt 13 AE, eine Meldung „beherrscht den Zauber nicht“ fehlt. | W |
| 385 | **Curian zaubert im Kampf nur im Computerkampf mit Magie** – Der NSC zaubert im normalen Kampf nie. | W |
| 187 | **Transversalis-Kreuz darf eine Zeile unter die Karte** – Das Zielkreuz kann unter die Karte wandern. | A |
| 188 | **Meditation bei LE 6 oder 7 kostet LE und bringt keine AE** – Ein Magier mit 6 oder 7 LE verliert beim Meditieren noch LE bis 5, ohne AE zu bekommen. | A |
| 193 | **Spieltext „AUF %S RUHT …“ mit großem S** – Statt des Heldennamens erscheint Unsinn oder „%S“. | A |
| 195 | **Skelettarius: Leiche darunter verschwindet aus der Auswahl** – Liegt die Zielleiche auf einer anderen, wird diese noch angezeigt, lässt sich aber nicht mehr wählen. | A |
| 200 | **Abbruch der Gegenstandswahl: inventory[−1]** – Nach Abbruch untersuchen die Zauber ein Phantom vor dem Inventar, Analüs kann dabei ein Bit im Heldendatensatz setzen. | A |
| 201 | **Analüs: volle AE-Kosten, wenn nur der Zuschlag die Probe kippt** – Gelingt die Grundprobe, scheitert aber am gegenstandseigenen Zuschlag, kostet Analüs die vollen AE statt der Hälfte. | A |
| 202 | **„Eigenschaften seid gelesen“ zeigt 80 % des Nahkampfschadens** – Der Zauber zeigt einen kleineren Schaden an, als der Gegner im Nahkampf macht. | A |
| 203 | **Exposami vergleicht mit einem noch leeren Listeneintrag** – Gelegentlich fehlt eine Gegnerart in der Exposami-Liste. | A |
| 205 | **Eisenrost auf ein Tier kostet volle AE** – „TIERE HABEN KEINE WAFFEN“, die AE sind trotzdem weg. | A |
| 206 | **Spieltext „VERWANDLET“** – Tippfehler im Spieltext. | A |
| 208 | **Paralue auf einen Helden prüft das alte Zauberziel** – War das letzte Zauberziel der Zauberer selbst, scheitert Paralue auf einen Helden still und kostet nichts. | A |
| 210 | **Brenne entzündet immer die erste Fackel oder Laterne** – Wählt man die zweite unentzündete Fackel zum Anzünden, brennt die erste. | A |

### Technik & Oberfläche (19)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 309 | **Fehlgeschlagener Start von GEN.EXE: Spiel läuft ohne Puffer weiter** – Lässt sich die Heldenerschaffung nicht starten, läuft das Spiel ohne globalen Puffer, Maus und Timer weiter und stürzt vermutlich ab. | W |
| 275 | **Übertrag der Subtraktion fehlt im High-Word der Titellänge** – Nichts bei normalen CD-Titeln, ein Titel könnte sonst zu lange laufen. | A |
| 280 | **Bright-Eyes-Fix für die AE-Leiste prüft den falschen Merker** – Im 3D-Modus wird die AE-Leiste nach AE-Verlust nicht neu gezeichnet (nur Anzeige). | A |
| 281 | **Anschnitt oben vergrößert die Bildhöhe, statt sie zu verkleinern (3 Stellen)** – Höchstens Grafikmüll am oberen Rand von Dungeonbildern, die beiden anderen Stellen werden nie erreicht. | A |
| 282 | **Wanduhr: Position läuft bis 84, Tabelle hat nur 81 Werte** – In der letzten knappen halben Stunde vor 7 bzw. 19 Uhr springen Sonne und Mond auf der Uhr. | A |
| 284 | **Zufallszahl kann 0 oder negativ werden** – Sehr selten (etwa 1 zu 65536) fällt ein Würfelwurf 0 oder −1 statt 1..n aus. | A |
| 287 | **Leeres Tagebuch: Vergleich mit gs_diary_entries[−1]** – Höchstens fehlt einmal der allererste Tagebucheintrag. | A |
| 288 | **Kopierhöhe line * 7 statt (line − startline) * 7** – Nichts, die zusätzlichen Zeilen sind leer und durchsichtig. | A |
| 289 | **Speichern: do-while ohne Prüfung von findfirst** – Nichts, es gibt immer CHR-Dateien. | A |
| 292 | **Stadtdatei: liest 100 Byte mehr, als g_locations_tab fasst** – Nichts, die Stadtdateien sind kurz genug. | A |
| 293 | **Kein break nach dem Fund** – Nichts, die Funktion wird nie aufgerufen. | A |
| 294 | **Bereichsprüfung clip_num > max statt >= max** – Nichts. | A |
| 296 | **Dukaten-Anzeige läuft ab 32768 D über** – Ab 32768 Dukaten zeigt das Spiel einen negativen oder falschen Betrag. | A |
| 298 | **Lesen vor g_hero_pic_posx bei Maus-x < 3** – Nichts. | A |
| 299 | **Kartenwerte ab 0xf0 lesen hinter die Tabelle** – Nichts, solche Werte kommen in Stadtkarten nicht vor. | A |
| 304 | **Spielstandname bearbeiten: Schreibstelle hinter dem Namen** – Hängt man an einen vorhandenen Spielstandnamen Zeichen an, gehen sie verloren, solange man nicht erst die Rücktaste drückt. | A |
| 306 | **Abfrage „WIE VIELE DER … SOLLEN …“ im falschen Fall** – Grammatikfehler in der Mengenabfrage beim Übergeben. | A |
| 308 | **Speichermeldung nennt 5000 Byte zu wenig** – Bei zu wenig Speicher nennt die Startmeldung einen zu kleinen oder negativen Fehlbetrag. | A |
| 410 | **Kampfmusik läuft nach direkt gestarteten Dungeonkämpfen weiter** – Nach manchen Dungeonkämpfen bleibt die Kampfmusik. | A |

### Gegenstände & Inventar (16)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 214 | **Magisches Bastardschwert ist als „benutzbar“ markiert** *(teilweise)* – Wer das magische Bastardschwert über BENUTZEN verwendet, bringt das Spiel zum Absturz. | W |
| 220 | **Zähler der belegten Inventarplätze driftet** – Nach Stapeln und Tauschen kann ein Held keine Gegenstände mehr aufnehmen, obwohl Platz ist (oder umgekehrt). | W |
| 221 | **Zufälliger Gegenstandsverlust: Endlosschleife ohne ablegbaren Gegenstand** – Hat ein Held keinen ablegbaren Gegenstand und trifft ihn ein Verlust (z. B. Bad in der Furt), hängt das Spiel. | W |
| 222 | **Tausch zweier Körperplätze ohne Prüfung und Boni** 💰 – Legt man einen Gegenstand zwischen zwei Körperplätzen um (z. B. Kraftgürtel linke → rechte Hand), werden seine Boni jedes Mal neu aufgeschlagen: KK +5 je Umlegen, ebenso CH (verfluchter Helm), Gefahrensinn (Kristallkugel), bis zum Überlauf. Der Schild-Trick senkt umgekehrt den RS. | W |
| 225 | **Laterne nachfüllen: Öl beim letzten Zauberer gesucht, beim Benutzer entfernt** – Füllt ein Held seine Laterne nach und hat zuletzt ein anderer Held gezaubert, verschwindet beim Benutzer ein falscher Gegenstand. | W |
| 216 | **Truhenliste: 20 Byte je Name zu kurz** – Lange Gegenstandsnamen laufen in den nächsten Eintrag, am Listenende über das Feld. | A |
| 217 | **Mengenabfrage lässt sich nicht abbrechen** – „WIE VIELE MITNEHMEN?“ kann man nur mit 0 verlassen, ESC fragt erneut. | A |
| 223 | **Wasserschlauch in der Hand wird nicht gefunden** – Ein Held, der seinen Wasserschlauch in der Hand trägt, trinkt nicht daraus. | A |
| 224 | **Waffengift: Schleuder wird nicht ausgeschlossen** – Gift lässt sich auf eine Schleuder auftragen. | A |
| 226 | **Tarnele: Meldung ohne Text** *(teilweise)* – Nach dem Essen einer Tarnele erscheint ein leeres oder altes Meldungsfenster. | A |
| 227 | **Antikrankheitselixier ohne Krankheit schreibt in disease[0]** – Nichts. | A |
| 390 | **Unzerbrechliche Waffen können zerbrechen (Held gegen Held, Curians Stab)** *(teilweise)* – Zauberstäbe und die Daspota-Orknase werden nach Paraden Held gegen Held zerbrechlich, Curians Stab ist es von Anfang an. | A |
| 394 | **Wegwerfen: höchstens 99 Stück, Mengenabfrage auch bei einem Stück** – Kosmetisch. | A |
| 397 | **Tausch zwischen Helden prüft nur die Traglast des Empfängers** 💰 – Über einen Tausch bekommt ein Held mehr Last, als er tragen kann. | A |
| 398 | **Zweihänder von rechter in linke Hand: falsche Meldung** – Es kommt Text 829 (Zweihänder und etwas anderes). | A |
| 400 | **Waffengift über BENUTZEN umgeht die Klassensperre** – Ein Krieger kann seine Waffe vergiften. | A |

## Bewusst nicht geändert

133 gemeldete Fehler bleiben, wie sie sind. Die Gründe: Die Absicht von Attic ist unklar (Spielbalance), die Behebung würde Spielstände oder Spieldaten verändern, sie hätte keine Wirkung, oder sie wäre eine neue Spielmechanik statt einer Fehlerbehebung. Jeder einzelne Grund steht in [Bauen/FIXES.md](Bauen/FIXES.md).

## Etwas gefunden?

Alle Behebungen sind am Quelltext entworfen und technisch geprüft, aber **noch nicht ausgiebig gespielt**. Wenn sich etwas falsch anfühlt oder abstürzt: bitte mit der **Nummer** aus dieser Liste melden (GitHub Issues oder im Crystals-DSA-Forum). Mit `FIX_nnnn=0` lässt sich jede einzelne Behebung abschalten, so lässt sich ein Problem schnell eingrenzen.
