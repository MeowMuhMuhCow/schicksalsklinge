#!/usr/bin/env python3
"""dcmp.py ORIG.EXE NEU.OBJ DATEIOFFSET [kontext] - Disassembly-Diff: neu (OBJ) gegen Original ab DATEIOFFSET.
DATEIOFFSET = '@0x...' aus omfcmp.py fuer die bytegleiche Fassung desselben Moduls. Fixup-Bytes werden genullt."""
import sys, subprocess, difflib, re, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import omfcmp
exe, obj, off = sys.argv[1], sys.argv[2], int(sys.argv[3], 16)
ctx = int(sys.argv[4]) if len(sys.argv) > 4 else 3
segs = omfcmp.parse_obj(obj)
code = [s for s in segs if s[1] == 'CODE'][0]
data, mask = bytearray(code[2]), code[3]
orig = bytearray(open(exe, 'rb').read()[off:off + len(data) + 64])
for i in range(len(data) - 4):   # TLINK: far call im selben Segment -> push cs; call near
    if data[i] == 0x9A and orig[i:i + 3] == b'\x90\x0e\xe8':
        orig[i:i + 5] = data[i:i + 5]
for i, m in enumerate(mask):
    if not m and i < len(orig): data[i] = 0; orig[i] = 0
def dis(b):
    p = '/tmp/claude-0/dcmp.bin'; open(p, 'wb').write(b)
    out = subprocess.run(['ndisasm', '-b16', p], capture_output=True, text=True).stdout.splitlines()
    return [re.sub(r'^([0-9A-F]{8})\s+\S+\s+', r'\1 ', l) for l in out]
A = dis(bytes(orig[:len(data)])); B = dis(bytes(data))
strip = lambda L: [l.split(' ', 1)[1] if ' ' in l else l for l in L]
for l in difflib.unified_diff(strip(A), strip(B), 'original', 'neu', lineterm='', n=ctx):
    print(l)
