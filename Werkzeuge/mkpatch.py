#!/usr/bin/env python3
"""mkpatch.py ORIGINAL ZIEL AUSGABE.ndp - Differenz-Patch (Format NDP1) erzeugen.
Format (gzip-komprimiert): 'NDP1', u32 Groesse+16 Byte MD5 der Quelle, u32 Groesse+16 Byte MD5 des Ziels,
dann Befehle: 0x01 COPY (u32 Quell-Offset, u32 Laenge) | 0x02 ADD (u32 Laenge, Bytes) | 0x00 Ende.
Der Patch enthaelt nur die neuen Bytes; alles andere wird aus der Original-EXE kopiert."""
import sys, struct, hashlib, gzip
K = 12
def mk(src, dst):
    idx = {}
    for i in range(len(src) - K + 1):
        l = idx.setdefault(src[i:i+K], [])
        if len(l) < 16: l.append(i)
    ops = []; lit = bytearray(); i = 0; nxt = -1
    def flush():
        if lit: ops.append((2, bytes(lit))); lit.clear()
    while i < len(dst):
        best_o, best_l = -1, 0
        cands = idx.get(dst[i:i+K], []) if i + K <= len(dst) else []
        if nxt >= 0 and nxt < len(src) and src[nxt] == dst[i]: cands = [nxt] + cands
        for o in cands:
            l = 0
            while i + l < len(dst) and o + l < len(src) and src[o+l] == dst[i+l]: l += 1
            if l > best_l: best_o, best_l = o, l
        if best_l >= K or (best_o == nxt and best_l >= 4):
            flush(); ops.append((1, best_o, best_l)); i += best_l; nxt = best_o + best_l
        else:
            lit.append(dst[i]); i += 1; nxt = nxt + 1 if nxt >= 0 else -1
    flush()
    out = bytearray(b'NDP1' + struct.pack('<I', len(src)) + hashlib.md5(src).digest()
                    + struct.pack('<I', len(dst)) + hashlib.md5(dst).digest())
    for op in ops:
        if op[0] == 1: out += struct.pack('<BII', 1, op[1], op[2])
        else: out += struct.pack('<BI', 2, len(op[1])) + op[1]
    out += b'\0'
    return bytes(out), ops
def apply(src, p):
    assert p[:4] == b'NDP1'; o = 4 + 20 + 20; out = bytearray()
    while True:
        t = p[o]; o += 1
        if t == 0: break
        if t == 1: so, l = struct.unpack_from('<II', p, o); o += 8; out += src[so:so+l]
        else: l, = struct.unpack_from('<I', p, o); o += 4; out += p[o:o+l]; o += l
    return bytes(out)
if __name__ == '__main__':
    src = open(sys.argv[1], 'rb').read(); dst = open(sys.argv[2], 'rb').read()
    raw, ops = mk(src, dst)
    assert apply(src, raw) == dst
    open(sys.argv[3], 'wb').write(gzip.compress(raw, 9, mtime=0))
    lit = sum(len(o[1]) for o in ops if o[0] == 2)
    print(f"{sys.argv[3]}: {len(ops)} Befehle, {lit} neue Bytes, Patch {len(gzip.compress(raw,9,mtime=0))} Byte")
