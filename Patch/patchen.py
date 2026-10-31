#!/usr/bin/env python3
"""patchen.py - erzeugt die Bugfix-Fassungen aus der eigenen Original-Datei (GOG) mit den
Differenz-Patches (*.ndp) neben diesem Skript. Kein Compiler noetig, nur Python 3.
Die Original-Datei wird nur gelesen, nie veraendert. Gleiches Ergebnis wie patchen.ps1 (Windows).

Aufruf: python3 patchen.py SPIELORDNER NAME ZIELORDNER [MUSTER]
   z. B. python3 patchen.py ~/Spiele/Schicksalsklinge SCHICKM.EXE ../Build "BLADEM_FIX_*.ndp"

Format NDP1 (gzip): 'NDP1', u32 Groesse + 16 Byte MD5 der Quelle, u32 Groesse + 16 Byte MD5 des Ziels,
dann Befehle 0x01 COPY (u32 Quell-Offset, u32 Laenge) | 0x02 ADD (u32 Laenge, Bytes) | 0x00 Ende."""
import fnmatch, gzip, hashlib, os, struct, sys


def lies_patch(pfad):
    p = gzip.open(pfad, 'rb').read()
    if p[:4] != b'NDP1':
        sys.exit(f"{os.path.basename(pfad)} ist kein NDP1-Patch.")
    return p


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    spiel, name, ziel = sys.argv[1], sys.argv[2], sys.argv[3]
    muster = sys.argv[4] if len(sys.argv) > 4 else '*.ndp'
    hier = os.path.dirname(os.path.abspath(__file__))
    patches = sorted(f for f in os.listdir(hier) if fnmatch.fnmatchcase(f, muster))
    if not patches:
        sys.exit(f"Keine Patches ({muster}) neben patchen.py gefunden.")

    p0 = lies_patch(os.path.join(hier, patches[0]))
    src_len = struct.unpack_from('<I', p0, 4)[0]
    src_md5 = p0[8:24].hex()

    # Original suchen: im Spielordner und darunter, Gross-/Kleinschreibung egal (Linux)
    kand = []
    for wurzel, ordner, dateien in os.walk(spiel):
        ordner.sort()
        for f in sorted(dateien):
            if f.upper() == name.upper():
                kand.append(os.path.join(wurzel, f))
    src = gefunden = None
    gesehen = []
    for k in kand:
        if os.path.getsize(k) != src_len:
            gesehen.append(f"{k} (Groesse {os.path.getsize(k)})")
            continue
        b = open(k, 'rb').read()
        h = hashlib.md5(b).hexdigest()
        if h == src_md5:
            src, gefunden = b, k
            break
        gesehen.append(f"{k} (MD5 {h})")
    if src is None:
        print(f"\nKeine passende {name} gefunden.")
        print(f"Der Patch braucht genau die deutsche GOG-Fassung (MD5 {src_md5}, {src_len} Byte).")
        if gesehen:
            print("Gefunden, aber passt nicht:")
            for g in gesehen:
                print("  " + g)
        sys.exit(1)
    print(f"Original: {gefunden}")
    print(f"          MD5 {src_md5} - passt.")
    os.makedirs(ziel, exist_ok=True)

    for pf in patches:
        p = lies_patch(os.path.join(hier, pf))
        if p[8:24].hex() != src_md5:
            sys.exit(f"{pf} gehoert zu einer anderen Original-Datei.")
        dst_len = struct.unpack_from('<I', p, 24)[0]
        dst_md5 = p[28:44].hex()
        out = bytearray()
        o = 44
        while True:
            t = p[o]; o += 1
            if t == 0:
                break
            if t == 1:
                so, l = struct.unpack_from('<II', p, o); o += 8
                out += src[so:so + l]
            elif t == 2:
                l, = struct.unpack_from('<I', p, o); o += 4
                out += p[o:o + l]; o += l
            else:
                sys.exit(f"{pf}: unbekannter Befehl {t}")
        h = hashlib.md5(out).hexdigest()
        if len(out) != dst_len or h != dst_md5:
            sys.exit(f"{pf}: Ergebnis stimmt nicht (MD5 {h} statt {dst_md5}).")
        basis = pf[:-4]                      # BLADEM_FIX_WICHTIG.ndp -> .EXE, SCHICK.DAT.ndp -> SCHICK.DAT
        if '.' not in basis:
            basis += '.EXE'
        aus = os.path.join(ziel, basis)
        open(aus, 'wb').write(out)
        print(f"Erzeugt:  {aus}")
        print(f"          MD5 {h} - geprueft.")
    print(f"\nFertig. Das Original ({name}) wurde nicht veraendert.")


if __name__ == '__main__':
    main()
