#!/usr/bin/env python3
"""show.py <seg.c> <access|all> [TEXTFILE] [funcregex]
prints every call with function name; with TEXTFILE resolves literal indices."""
import sys, re
sys.path.insert(0, '/root/sternenschweif/agT/tools_kommentar')
from lib import *

fn = sys.argv[1]
acc = sys.argv[2]
tf = sys.argv[3] if len(sys.argv) > 3 and sys.argv[3] != '-' else None
fre = sys.argv[4] if len(sys.argv) > 4 else None
L = read_src(fn)
F = functions(L)
T = load_texts(tf) if tf else None
for i, l in enumerate(L):
    for (a, arg, s, e) in find_calls(l):
        if acc != 'all' and a != acc:
            continue
        if fre and not re.search(fre, F[i] or ''):
            continue
        n = parse_int(arg) if arg else None
        txt = ''
        if T is not None and n is not None:
            txt = short(T.get(n, '<<FEHLT>>'), 90)
        print(f'{i+1:5} {F[i]:28.28} {a}({arg}) {txt}' + ('' if T is not None and n is not None else '   | ' + l.strip()[:110]))
