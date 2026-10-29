/*
 * seg122 - Leerer Orts-Handler
 *
 * Inhalt:    do_location_unkn1() ist eine leere Funktion. Sie steht in der Tabelle der
 *            Orts-Handler (seg025) an Index 1, dem Ortstyp LOCTYPE_UNKN1.
 * Aufrufer:  Orts-Handler-Tabelle in seg025.
 * Daten:     keine.
 * Besonders: reiner Platzhalter; ein eigenes Codesegment fuer eine leere Funktion.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg122.c
 */

#include "v302de.h"

void do_location_unkn1(void)
{

}
