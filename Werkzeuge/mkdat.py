import struct, sys, hashlib
o = open(sys.argv[1], 'rb').read(); n = open(sys.argv[2], 'rb').read(); out = sys.argv[3]
def tab(d):
    t = []; i = 0
    while True:
        t.append(struct.unpack_from('<I', d, i*4)[0]); i += 1
        if i*4 >= t[0]: break
    return t
to, tn = tab(o), tab(n); assert len(to) == len(tn)
# Endmarke der Tabelle steht im Original auf 0 -> FX18.VOC (306) nicht lesbar; Ende = Dateiende
if to[-1] == 0: to[-1] = len(o)
if tn[-1] == 0: tn[-1] = len(n)
NIMM = set([14, 19, 79, 130, 133, 190, 197, 208, 209, 210, 211, 218, 220, 221, 222, 223, 224, 225, 238, 239, 241, 242,
            272, 273, 274, 277, 280, 281, 283, 288] + list(range(78, 128)) + list(range(156, 176)))
NIMM -= {48, 50, 53, 61, 72, 74}   # Stadtplaene (gehoeren zu NRS-Code)
parts = []
for i in range(len(to) - 1):
    a = o[to[i]:to[i+1]]; b = n[tn[i]:tn[i+1]]
    parts.append(b if (i in NIMM and a != b) else a)
hdr = len(to) * 4; offs = []; p = hdr
for x in parts: offs.append(p); p += len(x)
offs.append(p)
d = struct.pack('<%dI' % len(offs), *offs) + b''.join(parts)
assert len(offs) == len(to)
open(out, 'wb').write(d)
print('neu', len(d), hashlib.md5(d).hexdigest(), 'uebernommen', sum(1 for i in NIMM if o[to[i]:to[i+1]] != n[tn[i]:tn[i+1]]))
