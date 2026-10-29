#!/usr/bin/env python3
"""pubs.py OBJ - PUBDEF-Namen mit Segment und Offset (OMF)"""
import sys, struct
d = open(sys.argv[1], 'rb').read(); p = 0; ln = ['']; segs = ['']
def idx(p):
    b = d[p]
    return (((b & 0x7f) << 8) | d[p+1], p+2) if b & 0x80 else (b, p+1)
while p < len(d):
    t = d[p]; L = struct.unpack_from('<H', d, p+1)[0]; r = d[p+3:p+3+L-1]
    if t == 0x96:
        q = 0
        while q < len(r): n = r[q]; ln.append(r[q+1:q+1+n].decode('latin1')); q += 1+n
    elif t in (0x98, 0x99):
        q = 1 + (4 if t == 0x99 else 2)
        b = r[q]; sn = r[q] | ((r[q+1] & 0x7f) << 8) if b & 0x80 else b
        segs.append(ln[sn] if sn < len(ln) else '?')
    elif t in (0x90, 0x91):
        q = 0; g, q = (r[0], 1) if not r[0] & 0x80 else (((r[0]&0x7f)<<8)|r[1], 2)
        s = r[q]; q += 1
        if s == 0: q += 2
        while q < len(r):
            n = r[q]; name = r[q+1:q+1+n].decode('latin1'); q += 1+n
            off = struct.unpack_from('<H', r, q)[0]; q += 2; q += 1 if not r[q] & 0x80 else 2
            print(f'{segs[s] if s < len(segs) else s:12s} {off:04x} {name}')
    p += 3 + L
