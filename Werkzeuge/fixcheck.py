#!/usr/bin/env python3
"""fixcheck.py NEU.EXE NEU.MAP REF.EXE REF.MAP - statische Pruefung einer Fix-EXE (FIX_BUGS > 0) gegen den
Standardbau (FIX_BUGS = 0, bytegleich mit dem Original). Nach dem Vorbild Sternenschweif (tools/fixcheck.py).

Die Fix-EXE kann nicht bytegleich sein. Geprueft wird, was fuer ein lauffaehiges Programm und alte Spielstaende gleich
bleiben muss:
  1. Segmenttabelle der MAP: dieselben Segmente in derselben Reihenfolge, jedes (ausser dem Overlay-Code, Klasse
     :OVY) an derselben Stelle mit derselben Laenge -> residenter Code, Stub-Segmente der Overlays, DGROUP und alle residenten Adressen unveraendert
     (geaenderte residente Module fuellt build_fix.sh bzw. bauen.py auf ihre Originallaenge auf).
  2. Datensegment (initialisierte Daten von DGROUP bis zum Ende des Ladebilds): Byte fuer Byte gleich.
  3. Overlays: gleich viele, je Overlay gleich viele Einspruenge; jeder Einsprung zeigt in beiden EXEs auf dieselbe
     oeffentliche Funktion (Namen aus der MAP), ohne Namen auf einen Funktionsanfang mit gleichem Prolog; kein
     Einsprung hinter dem Codeende.
  4. MZ-Kopf: Stack (SS:SP) und Einsprung (CS:IP) gleich; die Zahl der Relocations darf sich aendern (Hinweis).
Ausgabe: Abweichungen und eine Zusammenfassung; Exit 1 bei Fehlern."""
import re, struct, sys


def load_map(p):
    segs = []; pubs = {}; dgroup = None
    sec = 0
    for l in open(p, encoding='latin1'):
        if 'Publics by Value' in l: sec = 2; continue
        if 'Publics by Name' in l: sec = 1; continue
        m = re.match(r'\s*([0-9A-F]{5})H ([0-9A-F]{5})H ([0-9A-F]{5})H (\S+)\s+(\S+)', l)
        if m and sec == 0:
            segs.append((m[4], m[5], int(m[1], 16), int(m[3], 16)))
            continue
        m = re.match(r'\s*([0-9A-F]{4}):0000\s+[0-9A-F]{4}\s+C=DATA\s+S=_DATA\s+G=DGROUP', l)
        if m and dgroup is None: dgroup = int(m[1], 16)
        m = re.match(r'\s*([0-9A-F]{4}):([0-9A-F]{4})\s+(?:idle\s+|Abs\s+)?(\S+)\s*$', l)
        if m and sec == 1:
            pubs[(int(m[1], 16), int(m[2], 16))] = m[3]
    return segs, pubs, dgroup


def load_exe(p):
    d = open(p, 'rb').read()
    l, n = struct.unpack_from('<HH', d, 2)
    e = (n - 1) * 512 + l if l else n * 512
    h = struct.unpack_from('<H', d, 8)[0] * 16
    return d, h, e


def stubs(d, h, e, segs):
    """je Stub-Segment: (Name, Paragraph, Einspruenge, Overlay-Code)"""
    out = []
    for name, cls, start, ln in segs:
        if cls != 'STUBSEG' or ln == 0: continue
        o = h + start
        _, _, fo, cs, rs, ne, _ = struct.unpack_from('<HHIHHHH', d, o)
        ent = [struct.unpack_from('<H', d, o + 0x20 + 5 * i + 2)[0] for i in range(ne)]
        code = d[e + 0x10 + fo:e + 0x10 + fo + cs]
        out.append((name, start >> 4, ent, code))
    return out


def main():
    nexe, nmap, rexe, rmap = sys.argv[1:5]
    err = []; warn = []
    sa, pa, dga = load_map(rmap); sb, pb, dgb = load_map(nmap)
    A, ha, ea = load_exe(rexe); B, hb, eb = load_exe(nexe)
    # 1. Segmenttabelle
    if [(s[0], s[1]) for s in sa] != [(s[0], s[1]) for s in sb]:
        err.append(f'Segmenttabelle: andere Segmente ({len(sa)} -> {len(sb)})')
    nres = 0
    for x, y in zip(sa, sb):
        if x[1] == ':OVY':
            continue        # Overlay-Code (liegt hinter dem Ladebild, darf wachsen); Pruefung unter 3.
        if (x[2], x[3]) != (y[2], y[3]):
            err.append(f'Segment {x[0]} ({x[1]}): Lage/Laenge {x[2]:#x}/{x[3]:#x} -> {y[2]:#x}/{y[3]:#x}')
        elif x[1] == 'CODE' and A[ha + x[2]:ha + x[2] + x[3]] != B[hb + y[2]:hb + y[2] + y[3]]:
            nres += 1
    # 2. Datensegment
    if dga != dgb: err.append(f'DGROUP {dga:#x} -> {dgb:#x}')
    da = A[ha + dga * 16:ea]; db = B[hb + dgb * 16:eb]
    if da != db:
        bad = [i for i in range(min(len(da), len(db))) if da[i] != db[i]]
        err.append(f'Datensegment: Laenge {len(da):#x} -> {len(db):#x}, {len(bad)} Bytes anders'
                   + (f', erstes ds:{bad[0]:#x}' if bad else ''))
    # 3. Overlays
    ta = stubs(A, ha, ea, sa); tb = stubs(B, hb, eb, sb)
    if len(ta) != len(tb): err.append(f'Overlays: {len(ta)} -> {len(tb)}')
    nov = 0; nent = 0
    for (na, pa_, ea_, ca), (nb, pb_, eb_, cb) in zip(ta, tb):
        if len(ea_) != len(eb_):
            err.append(f'{na}: Einspruenge {len(ea_)} -> {len(eb_)}'); continue
        if ca != cb: nov += 1
        for k, (x, y) in enumerate(zip(ea_, eb_)):
            nent += 1
            if y >= len(cb): err.append(f'{na} Einsprung {k}: {y:#x} hinter dem Codeende {len(cb):#x}'); continue
            fa = pa.get((pa_, x)); fb = pb.get((pb_, y))
            if fa or fb:
                if fa != fb: err.append(f'{na} Einsprung {k}: {fa} -> {fb}')
                continue
            if ca[x:x + 3] != cb[y:y + 3]:
                err.append(f'{na} Einsprung {k} (ohne Namen): Prolog {ca[x:x + 4].hex()} -> {cb[y:y + 4].hex()}')
    # 4. MZ-Kopf
    for nm, o in (('Relocations', 6), ('SS', 0x0e), ('SP', 0x10), ('IP', 0x14), ('CS', 0x16)):
        va = struct.unpack_from('<H', A, o)[0]; vb = struct.unpack_from('<H', B, o)[0]
        if va != vb: (err if nm != 'Relocations' else warn).append(f'MZ {nm}: {va:#x} -> {vb:#x}')
    for w in warn: print('HINWEIS', w)
    for e in err: print('FEHLER', e)
    print(f'fixcheck {nexe}: {len(sb)} Segmente (Lage gleich, {nres} residente Codesegmente geaendert), Datensegment '
          f'{"gleich" if not any(e.startswith("Datensegment") for e in err) else "ANDERS"}, {len(tb)} Overlays '
          f'({nov} geaendert), {nent} Einspruenge geprueft, Groesse {len(A)} -> {len(B)} Bytes: '
          f'{"IN ORDNUNG" if not err else f"{len(err)} FEHLER"}')
    sys.exit(1 if err else 0)


if __name__ == '__main__':
    main()
