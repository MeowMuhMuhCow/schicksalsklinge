#!/usr/bin/env python3
"""
vergleich.py (Werkzeug W5) - Bild-fuer-Bild-Referenzvergleich zwischen Original (DOSBox)
und nativem Windows-Port (SCHICK_PORT.EXE).

Verwendung:
    python3 Port/tools/vergleich.py <referenz.bmp/.png> <port.bmp/.png> [diff_out.png]

Unterstuetzt 24-Bit-BMP und 24/32-Bit-PNG (Standardbibliothek ohne externe Pakete).
Berechnet:
  - Exakte Pixel-Uebereinstimmung (%)
  - Farbaehnlichkeit (100 % - mittlerer absoluter RGB-Abstand / 255)
  - Erzeugt ein Differenzbild (320x200 PNG), in dem abweichende Pixel rot/magenta hervorgehoben sind.
"""

from __future__ import annotations

import struct
import sys
import zlib
from pathlib import Path


def read_bmp_rgb(path: Path) -> tuple[int, int, list[tuple[int, int, int]]]:
    data = path.read_bytes()
    if data[:2] != b"BM":
        raise ValueError(f"{path} ist keine BMP-Datei")
    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    w = struct.unpack_from("<i", data, 18)[0]
    h_raw = struct.unpack_from("<i", data, 22)[0]
    bpp = struct.unpack_from("<H", data, 28)[0]
    top_down = h_raw < 0
    h = abs(h_raw)
    stride = ((w * (bpp // 8) + 3) // 4) * 4
    pixels: list[tuple[int, int, int]] = [(0, 0, 0)] * (w * h)
    for y in range(h):
        src_y = y if top_down else (h - 1 - y)
        row_off = pixel_offset + src_y * stride
        for x in range(w):
            off = row_off + x * (bpp // 8)
            b, g, r = data[off], data[off + 1], data[off + 2]
            pixels[y * w + x] = (r, g, b)
    return w, h, pixels


def read_png_rgb(path: Path) -> tuple[int, int, list[tuple[int, int, int]]]:
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path} ist keine PNG-Datei")
    pos = 8
    w = h = color_type = 0
    idat_chunks: list[bytes] = []
    plte: list[tuple[int, int, int]] = []
    while pos < len(data):
        length = struct.unpack_from(">I", data, pos)[0]
        tag = data[pos + 4 : pos + 8]
        body = data[pos + 8 : pos + 8 + length]
        pos += 12 + length
        if tag == b"IHDR":
            w, h, bit_depth, color_type, _, _, _ = struct.unpack(">IIBBBBB", body)
        elif tag == b"PLTE":
            for i in range(0, len(body), 3):
                plte.append((body[i], body[i + 1], body[i + 2]))
        elif tag == b"IDAT":
            idat_chunks.append(body)
        elif tag == b"IEND":
            break
    raw = zlib.decompress(b"".join(idat_chunks))
    bpp = {2: 3, 6: 4, 3: 1}[color_type]
    stride = w * bpp
    pixels: list[tuple[int, int, int]] = []
    prev_row = bytearray(stride)
    off = 0
    for _ in range(h):
        ftype = raw[off]
        row = bytearray(raw[off + 1 : off + 1 + stride])
        off += 1 + stride
        if ftype == 1:
            for i in range(bpp, stride):
                row[i] = (row[i] + row[i - bpp]) & 0xFF
        elif ftype == 2:
            for i in range(stride):
                row[i] = (row[i] + prev_row[i]) & 0xFF
        elif ftype == 3:
            for i in range(stride):
                a = row[i - bpp] if i >= bpp else 0
                b = prev_row[i]
                row[i] = (row[i] + ((a + b) >> 1)) & 0xFF
        elif ftype == 4:
            for i in range(stride):
                a = row[i - bpp] if i >= bpp else 0
                b = prev_row[i]
                c = prev_row[i - bpp] if i >= bpp else 0
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                row[i] = (row[i] + pr) & 0xFF
        prev_row = row
        for x in range(w):
            if color_type == 3:
                pixels.append(plte[row[x]])
            else:
                px_off = x * bpp
                pixels.append((row[px_off], row[px_off + 1], row[px_off + 2]))
    return w, h, pixels


def load_image(path: Path) -> tuple[int, int, list[tuple[int, int, int]]]:
    magic = path.read_bytes()[:8]
    if magic[:2] == b"BM":
        return read_bmp_rgb(path)
    if magic == b"\x89PNG\r\n\x1a\n":
        return read_png_rgb(path)
    if path.suffix.lower() == ".bmp":
        return read_bmp_rgb(path)
    return read_png_rgb(path)


def write_png_rgb(path: Path, w: int, h: int, pixels: list[tuple[int, int, int]]) -> None:
    raw_rows = []
    for y in range(h):
        row = bytearray([0])
        for x in range(w):
            r, g, b = pixels[y * w + x]
            row.extend((r, g, b))
        raw_rows.append(bytes(row))
    raw = b"".join(raw_rows)

    def chunk(tag: bytes, body: bytes) -> bytes:
        return struct.pack(">I", len(body)) + tag + body + struct.pack(">I", zlib.crc32(tag + body) & 0xFFFFFFFF)

    ihdr = struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b"")
    path.write_bytes(png)


def scale_nearest(w: int, h: int, pixels: list[tuple[int, int, int]], tw: int, th: int) -> list[tuple[int, int, int]]:
    if w == tw and h == th:
        return pixels
    out: list[tuple[int, int, int]] = []
    for y in range(th):
        sy = (y * h) // th
        for x in range(tw):
            sx = (x * w) // tw
            out.append(pixels[sy * w + sx])
    return out


def main() -> int:
    if len(sys.argv) < 3:
        print("Verwendung: python3 Port/tools/vergleich.py <referenz.bmp/.png> <port.bmp/.png> [diff_out.png]", file=sys.stderr)
        return 2

    ref_p = Path(sys.argv[1])
    port_p = Path(sys.argv[2])
    diff_p = Path(sys.argv[3]) if len(sys.argv) >= 4 else None

    rw, rh, rpix = load_image(ref_p)
    pw, ph, ppix = load_image(port_p)

    rpix = scale_nearest(rw, rh, rpix, 320, 200)
    ppix = scale_nearest(pw, ph, ppix, 320, 200)

    total = 320 * 200
    exact = 0
    close = 0
    rgb_diff_sum = 0
    diff_pixels: list[tuple[int, int, int]] = []

    for (r1, g1, b1), (r2, g2, b2) in zip(rpix, ppix):
        d = abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2)
        rgb_diff_sum += d
        if d == 0:
            exact += 1
            close += 1
            diff_pixels.append((r2 // 3, g2 // 3, b2 // 3))
        elif d <= 12:
            close += 1
            diff_pixels.append((r2 // 2, g2 // 2, b2 // 2))
        else:
            diff_pixels.append((255, min(255, d), 255))

    exact_pct = 100.0 * exact / total
    close_pct = 100.0 * close / total
    sim_pct = 100.0 * (1.0 - (rgb_diff_sum / (total * 3.0 * 255.0)))
    abw_pct = 100.0 - exact_pct

    print(f"Referenz:          {ref_p.name} ({rw}x{rh})")
    print(f"Port:              {port_p.name} ({pw}x{ph})")
    print(f"Exakt identisch:   {exact}/{total} Pixel ({exact_pct:.2f} %, Abweichung: {abw_pct:.2f} %)")
    print(f"Toleranz <=4/Ch:   {close}/{total} Pixel ({close_pct:.2f} %)")
    print(f"RGB-Aehnlichkeit:  {sim_pct:.2f} %")

    if diff_p is not None:
        write_png_rgb(diff_p, 320, 200, diff_pixels)
        print(f"Differenzbild:     {diff_p}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
