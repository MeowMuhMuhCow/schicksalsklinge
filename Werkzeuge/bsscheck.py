#!/usr/bin/env python3
"""bsscheck.py MAP QUELLDATEIEN... - BSS je Modul: Soll-Adresse (// ds:0x...) gegen MAP.
Meldet je Modul mit BSS, wie viele Publics an der falschen Stelle liegen."""
import re, sys, glob
mp = open(sys.argv[1], encoding='latin1').read()
want = {}
for f in sys.argv[2:]:
    for l in open(f, encoding='latin1'):
        m = re.search(r'\b(g[s]?_\w+)\s*(\[[^\]]*\])*\s*(=.*)?;\s*//\s*ds:0x([0-9a-fA-F]+)', l)
        if m and not l.lstrip().startswith(('/*', 'extern')):
            want[m[1].upper()] = int(m[4], 16)
dg = None; mods = []
for l in mp.splitlines():
    m = re.match(r'\s*([0-9A-F]{4}):([0-9A-F]{4}) ([0-9A-F]{4}) C=BSS\s+S=_BSS\s+G=DGROUP\s+M=(\S+)', l)
    if m and int(m[3], 16):
        mods.append((int(m[2], 16), int(m[2], 16) + int(m[3], 16), m[4]))
have = {}
for l in mp.split('Publics by Value')[-1].splitlines():
    m = re.match(r'\s*([0-9A-F]{4}):([0-9A-F]{4})\s+(?:idle\s+)?_(\S+)\s*$', l)
    if m: have[m[3].upper()] = int(m[2], 16)
tot = 0
for a, b, mod in mods:
    vs = [(n, have[n], want.get(n)) for n in have if a <= have[n] < b]
    bad = [v for v in vs if v[2] is not None and v[1] != v[2]]
    tot += len(bad)
    print(f'{mod:12s} {a:04x}-{b:04x} {len(vs):4d} Publics, {len(bad):4d} falsch')
print('falsch gesamt', tot)
