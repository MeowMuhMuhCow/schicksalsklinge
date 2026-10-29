#!/usr/bin/env python3
"""segpad.py OBJ REF_OBJ - Fix-Modus: Codesegmente eines residenten Moduls auf die Laenge der Originalfassung bringen.

Residente Segmente liegen im Original byteweise dicht hintereinander vor DGROUP. Wuerde ein residentes Modul durch
eine Fehlerbehebung kuerzer oder laenger, verschoeben sich alle folgenden residenten Adressen (und Programmteile
mit festen Offsets, z. B. die Pruefsumme ueber SEG003 in O011). Darum:
  - jedes *_TEXT-Segment der OBJ, das kuerzer ist als in REF_OBJ, bekommt im SEGDEF die Laenge aus REF_OBJ
    (TLINK fuellt den Rest mit Nullbytes - sie liegen hinter dem letzten Befehl und werden nie ausgefuehrt);
  - ist ein Segment laenger als in REF_OBJ: Fehler (Exit 1), die Behebung passt nicht in das residente Segment.
Die OBJ wird an Ort und Stelle geaendert. Ausgabe: je Segment alte/neue Laenge."""
import struct, sys


def idx(b, q):
    v = b[q]
    return (((v & 0x7f) << 8) | b[q + 1], q + 2) if v & 0x80 else (v, q + 1)


def segdefs(d):
    """Liste (Satzanfang, Lage des Laengenfelds im Satz, Laenge, Name) aller SEGDEF (nur 16-Bit-Saetze 98h)."""
    p = 0; ln = ['']; out = []
    while p < len(d):
        t = d[p]; n = struct.unpack_from('<H', d, p + 1)[0]; b = d[p + 3:p + 2 + n]
        if t == 0x96:
            q = 0
            while q < len(b):
                l = b[q]; ln.append(b[q + 1:q + 1 + l].decode('latin1')); q += 1 + l
        elif t == 0x98:
            a = b[0]; q = 1 + (3 if a >> 5 == 0 else 0)
            big = a & 2
            L = 0x10000 if big else struct.unpack_from('<H', b, q)[0]
            ni, _ = idx(b, q + 2)
            out.append((p, q, L, ln[ni]))
        elif t == 0x99:
            raise SystemExit('segpad: 32-Bit-SEGDEF (99h) nicht unterstuetzt')
        p += 3 + n
    return out


def main():
    obj, ref = sys.argv[1:3]
    d = bytearray(open(obj, 'rb').read())
    R = {nm: L for _, _, L, nm in segdefs(open(ref, 'rb').read())}
    err = 0; changed = 0
    for p, q, L, nm in segdefs(d):
        if not nm.endswith('_TEXT') or nm not in R:
            continue
        r = R[nm]
        if L > r:
            print(f'segpad {obj}: {nm} {L:#x} > Original {r:#x} - Fehlerbehebung passt nicht in das residente Segment')
            err = 1
        elif L < r:
            struct.pack_into('<H', d, p + 3 + q, r)
            n = struct.unpack_from('<H', d, p + 1)[0]
            d[p + 2 + n] = (-sum(d[p:p + 2 + n])) & 0xff     # Pruefbyte des Satzes neu
            print(f'segpad {obj}: {nm} {L:#x} -> {r:#x} (+{r - L} Nullbytes)')
            changed += 1
        else:
            print(f'segpad {obj}: {nm} {L:#x} (gleich lang)')
    if err:
        sys.exit(1)
    if changed:
        open(obj, 'wb').write(d)


if __name__ == '__main__':
    main()
