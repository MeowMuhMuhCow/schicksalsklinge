#!/usr/bin/env python3
"""omfcmp.py - vergleicht Borland-OBJ-Dateien (OMF) mit einer Original-EXE.

Liest die Code-Bytes jedes Segments aus der OBJ, maskiert alle Stellen,
die der Linker erst spaeter fuellt (Fixups), und sucht das Muster in der
Original-EXE. Ergebnis pro Segment: GLEICH / ABWEICHEND / NICHT GEFUNDEN.

Aufruf: omfcmp.py ORIGINAL.EXE A.OBJ [B.OBJ ...]
"""
import sys, struct

FIXSIZE = {0: 1, 1: 2, 2: 2, 3: 4, 4: 1, 5: 2, 9: 4, 11: 6, 13: 4}


def read_index(d, p):
    b = d[p]
    if b & 0x80:
        return ((b & 0x7F) << 8) | d[p + 1], p + 2
    return b, p + 1


def parse_obj(path):
    d = open(path, 'rb').read()
    p = 0
    lnames = ['']
    segdefs = [None]          # (name, class, length)
    seg = {}                  # segidx -> {'data': bytearray, 'mask': bytearray}
    last = None               # (segidx, offset) of last LEDATA
    while p < len(d):
        typ = d[p]; ln = struct.unpack_from('<H', d, p + 1)[0]
        body = d[p + 3:p + 3 + ln - 1]
        p += 3 + ln
        if typ == 0x96:  # LNAMES
            q = 0
            while q < len(body):
                n = body[q]; lnames.append(body[q + 1:q + 1 + n].decode('latin1')); q += 1 + n
        elif typ in (0x98, 0x99):  # SEGDEF
            acbp = body[0]; q = 1
            if (acbp >> 5) == 0:
                q += 3
            if typ == 0x98:
                length = struct.unpack_from('<H', body, q)[0]; q += 2
            else:
                length = struct.unpack_from('<I', body, q)[0]; q += 4
            if acbp & 2 and typ == 0x98:
                length = 0x10000
            ni, q = read_index(body, q); ci, q = read_index(body, q)
            segdefs.append((lnames[ni], lnames[ci], length))
        elif typ in (0xA0, 0xA1):  # LEDATA
            si, q = read_index(body, 0)
            if typ == 0xA0:
                off = struct.unpack_from('<H', body, q)[0]; q += 2
            else:
                off = struct.unpack_from('<I', body, q)[0]; q += 4
            data = body[q:]
            s = seg.setdefault(si, {'data': bytearray(), 'mask': bytearray()})
            need = off + len(data)
            if len(s['data']) < need:
                s['data'].extend(b'\0' * (need - len(s['data'])))
                s['mask'].extend(b'\0' * (need - len(s['mask'])))
            s['data'][off:need] = data
            for i in range(off, need):
                s['mask'][i] = 1
            last = (si, off)
        elif typ in (0xA2, 0xA3):
            print(f'{path}: LIDATA uebersprungen', file=sys.stderr); return []
        elif typ in (0x9C, 0x9D):  # FIXUPP
            q = 0
            while q < len(body):
                b = body[q]
                if b & 0x80:  # FIXUP
                    locat = (b << 8) | body[q + 1]; q += 2
                    loc = (locat >> 10) & 0xF
                    drec = locat & 0x3FF
                    fd = body[q]; q += 1
                    F = fd & 0x80; frame = (fd >> 4) & 7; T = fd & 8; P = fd & 4
                    if not F and frame < 3:
                        _, q = read_index(body, q)
                    if not T:
                        _, q = read_index(body, q)
                    if not P:
                        q += 2 if typ == 0x9C else 4
                    si, off = last
                    s = seg[si]
                    for i in range(FIXSIZE.get(loc, 2)):
                        s['mask'][off + drec + i] = 0
                else:  # THREAD
                    D = b & 0x40; method = (b >> 2) & 7; q += 1
                    if (D and method < 3) or (not D and method < 4):
                        _, q = read_index(body, q)
        elif typ == 0x8A or typ == 0x8B:
            break
    out = []
    for si, s in seg.items():
        name, cls, length = segdefs[si]
        out.append((name, cls, bytes(s['data']), bytes(s['mask'])))
    return out


def find(exe, data, mask):
    # unmaskierte Bloecke als Anker; mehrere probieren, falls einer abweicht
    blocks = []; i = 0; n = len(data)
    while i < n:
        if mask[i]:
            j = i
            while j < n and mask[j]:
                j += 1
            if j - i >= 12:
                blocks.append((j - i, i, j))
            i = j
        else:
            i += 1
    blocks.sort(reverse=True)
    bases = set()
    for _, a, b in blocks[:12]:
        anchor = data[a:min(b, a + 64)]
        start = 0
        while True:
            k = exe.find(anchor, start)
            if k < 0:
                break
            if k - a >= 0:
                bases.add(k - a)
            start = k + 1
    hits = []
    for base in bases:
        seg = exe[base:base + n]
        if len(seg) < n:
            continue
        bad = sum(1 for t in range(n) if mask[t] and seg[t] != data[t])
        hits.append((bad, base))
    if not hits:
        return None, 0
    hits.sort()
    return hits[0][1], hits[0][0]


REGREG = {0x00, 0x01, 0x08, 0x09, 0x10, 0x11, 0x18, 0x19, 0x20, 0x21,
          0x28, 0x29, 0x30, 0x31, 0x38, 0x39, 0x88, 0x89}


def classify(exe, pos, data, mask):
    """Zaehlt Abweichungen; erkennt zwei harmlose Faelle:
    farcall:  OBJ 9A ofs seg  -> EXE 90 0E E8 rel  (TLINK macht aus einem
              far call im selben Segment push cs + near call)
    encoding: gleicher Register-Register-Befehl, andere gueltige Kodierung
              (z. B. 89 F3 / 8B DE = mov bx,si) - Inline-Assembler vs. TASM
    """
    far = enc = 0; real = []
    t = 0; n = len(data)
    while t < n:
        e = exe[pos + t]
        if not mask[t] or e == data[t]:
            t += 1; continue
        d = data[t]
        if d == 0x9A and exe[pos + t:pos + t + 3] == b'\x90\x0e\xe8':
            far += 1; t += 5; continue
        lo = min(d, e)
        if t + 1 < n and (d ^ e) == 2 and lo in REGREG:
            m1, m2 = data[t + 1], exe[pos + t + 1]
            if m1 >= 0xC0 and m2 >= 0xC0 and \
               ((m1 >> 3) & 7) == (m2 & 7) and (m1 & 7) == ((m2 >> 3) & 7):
                enc += 1; t += 2; continue
        real.append(t); t += 1
    return far, enc, real


def main():
    exe = open(sys.argv[1], 'rb').read()
    for obj in sys.argv[2:]:
        for name, cls, data, mask in parse_obj(obj):
            if not data or cls not in ('CODE', ''):
                continue
            pos, bad = find(exe, data, mask)
            fixed = len(data) - sum(mask)
            if pos is None:
                print(f'{obj:12} {name:14} {len(data):6} Bytes  NICHT GEFUNDEN')
                continue
            far, enc, real = classify(exe, pos, data, mask)
            extra = []
            if far: extra.append(f'{far} far->near')
            if enc: extra.append(f'{enc} Kodierung')
            extra = ('  [' + ', '.join(extra) + ']') if extra else ''
            if not real:
                print(f'{obj:12} {name:14} {len(data):6} Bytes  GLEICH     @0x{pos:06x}{extra}')
            else:
                print(f'{obj:12} {name:14} {len(data):6} Bytes  ABWEICHEND @0x{pos:06x}  {len(real)} Bytes echt anders{extra}  erste bei +0x{real[0]:x}')


if __name__ == '__main__':
    main()
