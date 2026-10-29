# Handverlesene Kommentare fuer berechnete/besondere Stellen
# LINE[(datei, zeile)] oder LINE[(datei, zeile, arg)] = Kommentartext (ohne /* */)
LINE = {
    ('seg002.c', 5793): 'FIGHTTXT.LTX 41..44: Himmelsrichtung',
    ('seg025.c', 479): 'MAPTEXT.LTX 0..51: Ortsname',
    ('seg030.c', 362): 'TEXT.LTX 236..287: Ortsname',
    ('seg031.c', 351): 'TAVERN.TLK 147..166 zufällig, z.B. "ICH WEISS ETWAS, WAS EUCH BESTIMMT INTERESSIERT:@%s"',
    ('seg048.c', 479): 'CHARTEXT.LTX 26..: Krankheitsbeschreibung',
    ('seg048.c', 587): 'CHARTEXT.LTX 26..: Krankheitsbeschreibung',
    ('seg048.c', 622): 'CHARTEXT.LTX 26..: Krankheitsbeschreibung',
    ('seg066.c', 1249): 'Stadt-LTX[0]: Ortsname',
    ('seg067.c', 112): '24..28 zufällig, z.B. "DEN SEID IHR WOHL LOS."',
    ('seg067.c', 238): '"EIN KIND BLEIBT VOR EUCH STEHEN UND S..." (randval == 1)',
    ('seg067.c', 242): '99/101/102 zufällig, z.B. "<JEDOCH, UMSONST IST DER TOD, SAGTE M..."',
    ('seg068.c', 84): 'Sieger 57..60, Verlierer 61..67, z.B. "POTTWAL PREM" / "RUHMREICH NOSTRIA"',
    ('seg068.c', 402): '38..47 zufällig: Losungswort, z.B. "GRÜNKOHL MIT SPECK"',
    ('seg073.c', 570, 'name'): 'TAVERN.TLK 167..206: Vor-/Nachname',
    ('seg073.c', 570, 'surname'): 'TAVERN.TLK 167..206: Vor-/Nachname',
    ('seg075.c', 913): 'SHIP.DTX: "URPLÖTZLICH BEGINNT DAS GESAMTE SCHIFF ZU BEBEN..."',
    ('seg076.c', 137): 'FINAL.DTX: "IHR STEHT VOR EINER TÜR, AN DIE MIT SCHWEREN KRAMPEN..."',
    ('seg076.c', 463): 'DTX[0]: Name des Dungeons',
    ('seg076.c', 544): 'FINAL.DTX: "IHR KÖNNT EUCH DURCH DIE BEIDEN SPIEGEL DIE HÄNDE REICHEN..."',
    ('seg090.c', 354): 'Text 31 fehlt in der dt. OBER.DTX (nur 0..30)',
    ('seg090.c', 413): 'Text 32 fehlt in der dt. OBER.DTX (nur 0..30)',
    ('seg093.c', 859): 'TEXT.LTX 236..287: Ortsname',
    ('seg093.c', 983): 'MAPTEXT.LTX: Ortsname und Beschreibung',
    ('seg094.c', 780): 'TEXT.LTX 236..287: Ortsname',
    ('seg109.c', 201, 'randval'): 'FEATURE.LTX 3..7: Ort',
    ('seg109.c', 248, 'randval'): 'FEATURE.LTX 11..15: Ort',
}
# SKIP[(datei, zeile)] = Grund
SKIP = {}
for _l in (618, 633, 639, 646, 652, 665, 678, 681, 698):
    SKIP[('seg109.c', _l)] = 'TRV_hunt_generic: Textbeginn kommt als Parameter vom aufrufenden tevent (FEATURE1..8)'
for _l in (750, 751, 759, 794, 823, 827, 829, 841):
    SKIP[('seg109.c', _l)] = 'TRV_barrier: Textbeginn kommt als Parameter vom aufrufenden tevent (FEATURE1/2)'
