#!/usr/bin/env python3
"""
abdeckung.py (Werkzeug W3) - Auswertung der Laufzeit-Abdeckung je Modul (seg*.c).

Liest:
  - Port/src/abdeckung_liste.tsv   (Katalog aller Funktionen aus seg001.c..seg122.c)
  - Port/build/abdeckung.tsv       (Zur Laufzeit mit DSA_ABDECKUNG=1 erreichte Funktionen)

Ausgabe:
  - Tabelle je Modul (seg*.c): Gesamtzahl, erreicht, Prozent, Liste der noch nicht gelaufenen Funktionen
  - Optional: Schreibt Port/ABDECKUNG.md (mit --markdown)
"""

from __future__ import annotations

import sys
from collections import defaultdict
from pathlib import Path

PORT_DIR = Path(__file__).resolve().parent.parent
CATALOG_FILE = PORT_DIR / "src" / "abdeckung_liste.tsv"
HITS_FILE = PORT_DIR / "build" / "abdeckung.tsv"
MD_OUT = PORT_DIR / "ABDECKUNG.md"


def main() -> int:
    if not CATALOG_FILE.exists():
        print(f"Katalog fehlt: {CATALOG_FILE} (bitte zuerst prepare_sources.py ausfuehren)", file=sys.stderr)
        return 1

    all_funcs: dict[str, list[str]] = defaultdict(list)
    for line in CATALOG_FILE.read_text(encoding="utf-8").splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        parts = line.split("\t")
        if len(parts) >= 3:
            _, seg, fn = parts[0], parts[1], parts[2]
            all_funcs[seg].append(fn)

    hits_file = HITS_FILE
    for arg in sys.argv[1:]:
        if not arg.startswith("-"):
            hits_file = Path(arg)
            break

    hit_funcs: dict[str, set[str]] = defaultdict(set)
    if hits_file.exists():
        for line in hits_file.read_text(encoding="utf-8", errors="replace").splitlines():
            if not line.strip() or line.startswith("#"):
                continue
            parts = line.split("\t")
            if len(parts) >= 2:
                hit_funcs[parts[0]].add(parts[1])
        if hits_file.resolve() != HITS_FILE.resolve():
            HITS_FILE.parent.mkdir(parents=True, exist_ok=True)
            HITS_FILE.write_bytes(hits_file.read_bytes())

    total_all = 0
    total_hit = 0
    rows: list[tuple[str, int, int, float, list[str]]] = []

    for seg in sorted(all_funcs.keys()):
        fns = all_funcs[seg]
        hits = hit_funcs.get(seg, set())
        missing = [f for f in fns if f not in hits]
        n_all = len(fns)
        n_hit = n_all - len(missing)
        pct = (100.0 * n_hit / n_all) if n_all > 0 else 100.0
        total_all += n_all
        total_hit += n_hit
        rows.append((seg, n_all, n_hit, pct, missing))

    total_pct = (100.0 * total_hit / total_all) if total_all > 0 else 0.0
    print(f"=== ABDECKUNG (W3): {total_hit}/{total_all} Funktionen ({total_pct:.1f} %) in {len(rows)} Modulen ===")
    print(f"{'Modul':<10} {'Erreicht':>10} {'Quote':>8}  {'Nicht gelaufene Funktionen'}")
    print("-" * 90)
    for seg, n_all, n_hit, pct, missing in rows:
        miss_str = ", ".join(missing[:5]) + (f" (+{len(missing)-5} weitere)" if len(missing) > 5 else "")
        if not missing:
            miss_str = "(alle erreicht)"
        print(f"{seg:<10} {f'{n_hit}/{n_all}':>10} {pct:>6.1f} %  {miss_str}")

    if "--markdown" in sys.argv:
        md = [
            "# Laufzeit-Abdeckungsbericht (`DSA_ABDECKUNG=1`)",
            "",
            f"- **Gesamt:** {total_hit} / {total_all} Funktionen erreicht ({total_pct:.1f} %)",
            f"- **Module (`seg*.c`):** {sum(1 for _, _, h, _, _ in rows if h > 0)} / {len(rows)} Module aktiv durchlaufen",
            "",
            "| Modul | Funktionen | Erreicht | Quote | Noch nicht erreichte Funktionen |",
            "| :--- | :---: | :---: | :---: | :--- |",
        ]
        for seg, n_all, n_hit, pct, missing in rows:
            miss_str = ", ".join(f"`{m}`" for m in missing) if missing else "*alle erreicht*"
            md.append(f"| `{seg}.c` | {n_all} | {n_hit} | {pct:.1f} % | {miss_str} |")
        MD_OUT.write_text("\n".join(md) + "\n", encoding="utf-8")
        print(f"\nMarkdown-Bericht geschrieben: {MD_OUT}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
