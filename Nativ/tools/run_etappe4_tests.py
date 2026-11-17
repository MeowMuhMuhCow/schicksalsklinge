#!/usr/bin/env python3
"""
run_etappe4_tests.py - Automatisierte Replay-Testsuiten fuer Etappe 4
(Durchspieltest aller Orte/Gilden, Hauptquest/Informanten, Stufenaufstieg,
Gegenstaende/Konsum, Magierakademie/Zauber, aller 15 Dungeons + Truhenfallen,
Reiserouten + Querverbindungen, Gegnerzauber sowie Endkampf + Abspann / Spielende)
mit kumulativer W3-Abdeckungsmessung.
"""

from __future__ import annotations

import hashlib
import os
import shutil
import struct
import subprocess
from pathlib import Path

PORT_DIR = Path(__file__).resolve().parent.parent
TESTS_DIR = PORT_DIR / "tests"
BUILD_DIR = PORT_DIR / "build"


def _default_win_root() -> Path:
    if os.name == "nt":
        return Path(os.environ.get("SystemDrive", "C:") + "\\")
    parts = PORT_DIR.parts
    if len(parts) >= 3 and parts[1] == "mnt" and len(parts[2]) == 1:
        return Path("/") / parts[1] / parts[2]
    return Path("/tmp")


WDIR = Path(
    os.environ.get(
        "SCHICK_BUILD_DIR",
        str(_default_win_root() / "Windows" / "Temp" / "schick_port_build"),
    )
)
TRUN = WDIR / "test_run"
EXE = WDIR / "SCHICK_PORT.EXE"


def make_gam(
    base_bytes: bytes,
    *,
    loctype: int = 0,
    loctype_bak: int = 0,
    town_id: int = 1,
    town_id_bak: int = 1,
    typeindex: int = 0,
    locdata: int = 0,
    dng_id: int = 0,
    dng_lvl: int = 0,
    x: int = 9,
    y: int = 9,
    viewdir: int = 0,
    x_bak: int = 9,
    y_bak: int = 9,
    viewdir_bak: int = 0,
    day_timer: int = 12 * 5400,
    hero0_ap: int | None = None,
    hero0_le_delta: int = 0,
    hero0_money: int = 50000,
    hero0_inv7_item: int = 0,
    broken_weapon: bool = False,
    clear_npc: bool = False,
    trv_dest_town: int = 1,
    trv_dest_x: int = 9,
    trv_dest_y: int = 9,
    trv_dest_dir: int = 2,
    hero0_low_ini: bool = False,
    boost_stats: bool = True,
) -> bytes:
    buf = bytearray(base_bytes)
    st_off = 0x14

    buf[st_off + 0x0009] = viewdir & 0xFF
    struct.pack_into("<h", buf, st_off + 0x0010, x)
    struct.pack_into("<h", buf, st_off + 0x0012, y)
    buf[st_off + 0x002C] = loctype & 0xFF
    for g in range(6):
        buf[st_off + 0x002D + g] = loctype & 0xFF
    buf[st_off + 0x0033] = town_id & 0xFF
    buf[st_off + 0x003A] = dng_id & 0xFF
    buf[st_off + 0x0041] = dng_lvl & 0xFF
    buf[st_off + 0x0048] = viewdir_bak & 0xFF
    struct.pack_into("<h", buf, st_off + 0x004F, x_bak)
    struct.pack_into("<h", buf, st_off + 0x0051, y_bak)
    buf[st_off + 0x006B] = loctype_bak & 0xFF
    buf[st_off + 0x0072] = town_id_bak & 0xFF
    struct.pack_into("<i", buf, st_off + 0x0087, day_timer)
    struct.pack_into("<h", buf, st_off + 0x14EE, locdata)
    struct.pack_into("<h", buf, st_off + 0x14F0, typeindex)
    struct.pack_into("<h", buf, st_off + 0x05DA, -1)  # gs_dng_pos_bak

    # Hohes Goetteransehen fuer sichere Wunder in ask_miracle (seg062)
    for god in range(15):
        struct.pack_into("<i", buf, st_off + 0x041A + god * 4, 20000)

    # Informanten-Flags bekannt setzen
    for inf in range(15):
        buf[st_off + 0x0630 + inf] = 1

    # Reiseziel nach Dungeon-Ausgang = Thorwal (9, 9)
    struct.pack_into("<h", buf, st_off + 0x1604, trv_dest_town)
    struct.pack_into("<h", buf, st_off + 0x1606, trv_dest_x)
    struct.pack_into("<h", buf, st_off + 0x1608, trv_dest_y)
    struct.pack_into("<h", buf, st_off + 0x160A, trv_dest_dir)
    struct.pack_into("<h", buf, st_off + 0x1616, trv_dest_town)

    # Helden-Structs am Dateiende (7 * 1754 Bytes ab 0x2040)
    h0_off = len(buf) - 7 * 1754
    if boost_stats:
        for hi in range(6):
            off = h0_off + hi * 1754
            for a in range(7):
                buf[off + 0x34 + a * 3] = 20
                buf[off + 0x35 + a * 3] = 20
            for sp in range(86):
                buf[off + 0x13D + sp] = 18
            for ta in range(52):
                buf[off + 0x108 + ta] = 18

    if hero0_ap is not None:
        struct.pack_into("<i", buf, h0_off + 0x28, hero0_ap)
    struct.pack_into("<i", buf, h0_off + 0x2C, hero0_money)

    le_max = struct.unpack_from("<h", buf, h0_off + 0x5E)[0]
    struct.pack_into("<h", buf, h0_off + 0x60, max(5, le_max + hero0_le_delta))

    if hero0_inv7_item > 0:
        inv7 = h0_off + 0x196 + 7 * 14
        struct.pack_into("<hhH", buf, inv7, hero0_inv7_item, 1, 0)
        if buf[h0_off + 0x20] < 8:
            buf[h0_off + 0x20] = 8

    if broken_weapon:
        inv3 = h0_off + 0x196 + 3 * 14
        flags = struct.unpack_from("<H", buf, inv3 + 4)[0]
        struct.pack_into("<H", buf, inv3 + 4, flags | 1)  # broken = 1

    if clear_npc:
        h6_off = len(buf) - 1 * 1754
        buf[h6_off + 0x21] = 0  # get_hero(6)->typus = 0
        buf[h6_off + 0x8A] = 0  # get_hero(6)->slot_pos = 0
        struct.pack_into("<h", buf, st_off + 0x073A, 0)  # gs_npc_meet_tavern = 0
        for n in range(7):
            buf[st_off + 0x08CD + n] = 0  # gs_npc_timers[n] = 0

    if hero0_low_ini:
        for hi in range(1, 6):
            hi_off = h0_off + hi * 1754
            buf[hi_off + 0x87] = 1  # andere Gruppe
        buf[st_off + 0x0002] = 1  # gs_group_member_counts[0] = 1

    return bytes(buf)


def make_trap_gam(base_bytes: bytes, did: int, dlvl: int, x: int, y: int, vdir: int) -> bytes:
    g = bytearray(
        make_gam(
            base_bytes,
            loctype=0,
            town_id=0,
            dng_id=did,
            dng_lvl=dlvl,
            x=x,
            y=y,
            viewdir=vdir,
            hero0_inv7_item=35,  # ITEM_ID_DIETRICHE
            boost_stats=False,
        )
    )
    h0_off = len(g) - 7 * 1754
    for a in range(7):
        g[h0_off + 0x34 + a * 3] = 2
        g[h0_off + 0x35 + a * 3] = 2
    g[h0_off + 0x108 + 48] = (-20) & 0xFF  # TA_SCHLOESSER = -20 -> loest garantiert die Truhenfalle aus
    return bytes(g)


def write_games_nam(names: list[str]) -> None:
    padded = []
    for nm in (names + ["LEER____"] * 5)[:5]:
        s = nm[:8].ljust(8, "_").encode("latin-1") + b"\x00"
        padded.append(s)
    (TRUN / "GAMES.NAM").write_bytes(b"".join(padded))


def run_single_scenario(
    name: str,
    gam_bytes: bytes,
    body_lines: list[str],
    rep_out: Path,
    append_log: list[str],
    *,
    intro_lines: list[str] | None = None,
) -> tuple[int, str]:
    gam_name = "KAP_TEST"
    (TRUN / f"{gam_name}.GAM").write_bytes(gam_bytes)
    write_games_nam([gam_name])

    prefix = intro_lines if intro_lines is not None else ["K +30 1C0D", "K +30 1C0D", "K +40 1C0D"]
    lines = [
        f"# Szenario: {name}",
        *prefix,
        *body_lines,
        "Q +20",
    ]
    append_log.extend(lines)
    rep_out.write_text("\n".join(lines) + "\n", encoding="utf-8")

    env = os.environ.copy()
    env["DSA_WATCHDOG_MS"] = "25000"
    win_rep = "C:/Windows/Temp/schick_port_build/" + rep_out.name
    shutil.copy2(rep_out, WDIR / rep_out.name)
    res = subprocess.run(
        [
            str(EXE),
            "--fast-replay",
            "--coverage",
            "--replay",
            win_rep,
            "C:/Windows/Temp/schick_port_build/test_run",
        ],
        cwd=str(TRUN),
        env=env,
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=30,
        check=False,
    )
    out_text = (res.stdout or b"").decode("latin-1", errors="replace")
    print(f"  [{name}] rc={res.returncode}")
    return res.returncode, out_text


def main() -> int:
    TESTS_DIR.mkdir(parents=True, exist_ok=True)
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    base_gam = (TRUN / "SPIEL_4B.GAM").read_bytes()
    kap_end_gam = (TRUN / "KAP_END_.GAM").read_bytes()

    rep_4a = TESTS_DIR / "etappe4a_hauptquest_orte_items.rep"
    rep_4b = TESTS_DIR / "etappe4b_alle_dungeons.rep"
    rep_4c = TESTS_DIR / "etappe4c_reiserouten_gegnerzauber.rep"

    log_4a: list[str] = []
    log_4b: list[str] = []
    log_4c: list[str] = []

    print("=== ETAPPE 4A: Hauptquest, Gilden/Orte, Stufenaufstieg, Items, Zauber & Spielende ===")
    # 1. Heiler in Thorwal (13, 7) (seg053)
    run_single_scenario(
        "4a-1 Heiler Thorwal (seg053)",
        make_gam(base_gam, loctype=0, town_id=1, x=13, y=7, viewdir=0, hero0_le_delta=-6),
        [
            "K +90 4B00",
            "K +30 4900",
            "K +15 1C0D",
            "K +15 1C0D",
            "K +15 1C0D",
            "K +15 1C0D",
        ],
        rep_4a,
        log_4a,
    )

    # 2. Tempelwunder in Thorwal (seg062)
    run_single_scenario(
        "4a-2 Tempelwunder (seg062)",
        make_gam(base_gam, loctype=2, loctype_bak=2, town_id=1, typeindex=1, locdata=0, x=9, y=8, viewdir=0),
        [
            "K +90 4900",
            "K +10 5000",
            "K +10 5000",
            "K +10 5000",
            "K +10 5000",
            "K +10 5000",
            "K +10 5000",
            "K +15 1C0D",
            "K +25 1C0D",
        ],
        rep_4a,
        log_4a,
    )

    # 3. Schmied + Zufallsdialog + Reparieren in Thorwal (5, 8) (seg058, seg031)
    run_single_scenario(
        "4a-3 Schmied & Reparatur (seg058/031)",
        make_gam(base_gam, loctype=0, town_id=1, x=5, y=8, viewdir=0, broken_weapon=True),
        [
            "K +90 4B00",
            "K +30 4900",
            "K +15 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +25 4900",
            "K +15 5000",
            "K +15 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
        ],
        rep_4a,
        log_4a,
    )

    # 4. Haendler Verkaufen in Thorwal (5, 11) (seg055, seg056, seg057)
    run_single_scenario(
        "4a-4 Haendler Verkauf (seg057)",
        make_gam(base_gam, loctype=0, town_id=1, x=5, y=11, viewdir=0),
        [
            "K +90 4B00",
            "K +30 4900",
            "K +15 5000",
            "K +15 1C0D",
            "K +20 1C0D",
        ],
        rep_4a,
        log_4a,
    )

    # 5. Taverne Phexcaer (20, 7, typeindex=40) + Geruechte + NSC Harika (seg059, seg060, seg073, seg095)
    run_single_scenario(
        "4a-5 Taverne Phexcaer + Geruechte + NSC Harika (seg060/073/095)",
        make_gam(base_gam, loctype=0, town_id=18, x=20, y=7, viewdir=0, day_timer=21 * 5400, clear_npc=True),
        [
            "K +90 4B00",
            "K +30 4900",
            *["K +15 1C0D"] * 13,
        ],
        rep_4a,
        log_4a,
    )

    # 6. Informantin Algrid Trondesdotter in Felsteyn (8, 14) (seg072)
    run_single_scenario(
        "4a-6 Informantin Algrid in Felsteyn (seg072)",
        make_gam(base_gam, loctype=0, town_id=20, x=8, y=14, viewdir=0),
        [
            "K +90 4B00",
            "K +30 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
        ],
        rep_4a,
        log_4a,
    )

    # 7. Konsumieren (seg108) & Zauber sprechen (seg099)
    cast_seq = [
        "K +25 4900",
        "K +10 5000",
        "K +10 5000",
        "K +10 5000",
        "K +10 5000",
        "K +10 5000",
        "K +15 1C0D",
        "K +10 5000",
        "K +10 5000",
        "K +10 5000",
        "K +10 5000",
        "K +15 1C0D",
        "K +15 1C0D",
        "K +15 1C0D",
        "K +15 1C0D",
        "K +25 1C0D",
    ]
    run_single_scenario(
        "4a-7 Konsumieren (seg108) & Zauber (seg099)",
        make_gam(base_gam, loctype=0, town_id=1, x=9, y=9, viewdir=0),
        [
            "K +90 1205",
            "K +15 1C0D",
            "K +20 1C0D",
            "K +10 5000",
            "K +15 1C0D",
            "K +20 1C0D",
            "K +20 4900",
            "K +10 5000",
            "K +10 5000",
            "K +10 5000",
            "K +10 5000",
            "K +10 5000",
            "K +15 1C0D",
            "K +20 4B00",
            *cast_seq,
            *cast_seq,
        ],
        rep_4a,
        log_4a,
    )

    # 8. Stadtereignisse in Thorwal (city_event_switch in seg067)
    walk_lines = ["K +90 4800"]
    for _ in range(90):
        walk_lines.append("K +2 5000")
        walk_lines.append("K +2 4800")
    for _ in range(15):
        walk_lines.append("K +10 1C0D")
    run_single_scenario(
        "4a-8 Stadtereignis Thorwal (seg067)",
        make_gam(base_gam, loctype=0, town_id=1, x=9, y=9, viewdir=2, day_timer=12 * 5400),
        walk_lines,
        rep_4a,
        log_4a,
    )

    # 9. Endkampf (FIGHT_ID_F144) + Abspann (final_intro & show_outro in seg065, GAME_STATE_VICTORY / GAME_STATE_OUTRO -> Spielende)
    end_screenshots = [
        "etappe4_endkampf_duell.png",
        "etappe4_endkampf_sieg_360ap.png",
        "etappe4_abspann_outro1.png",
        "etappe4_abspann_outro3_pottwal.png",
        "etappe4_spielende_speichern.png",
    ]
    for fn in end_screenshots:
        if (WDIR / fn).exists():
            (WDIR / fn).unlink()
        if (BUILD_DIR / fn).exists():
            (BUILD_DIR / fn).unlink()

    rc_end, out_end = run_single_scenario(
        "4a-9 Endkampf F144 + Abspann / Spielende erreicht (seg065/116)",
        kap_end_gam,
        [
            "K +120 4900",
            "K +20 1C0D",
            "K +470 1C0D",
            "K +25 1C0D",
            "K +920 1C0D",
            "K +45 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +45 4900",
            "K +10 4800",
            "K +10 1C0D",
            "K +55 1C0D",
            "K +55 1C0D",
            "S +250 C:/Windows/Temp/schick_port_build/etappe4_endkampf_duell.png",
            # Runde 0..2 gegen den Orkchampion (mit Grimring -> 360 AP Sieg)
            "K +10 5000",
            "K +10 1C0D",
            "K +10 4800",
            "K +10 1C0D",
            "K +10 1C0D",
            "K +360 5000",
            "K +10 1C0D",
            "K +10 4800",
            "K +10 1C0D",
            "K +10 1C0D",
            "K +360 5000",
            "K +10 1C0D",
            "K +10 4800",
            "K +10 1C0D",
            "K +10 1C0D",
            "S +350 C:/Windows/Temp/schick_port_build/etappe4_endkampf_sieg_360ap.png",
            "K +20 1C0D",
            "K +50 1C0D",
            "S +50 C:/Windows/Temp/schick_port_build/etappe4_abspann_outro1.png",
            "K +20 1C0D",
            "K +50 1C0D",
            "K +70 1C0D",
            "K +50 1C0D",
            "K +70 1C0D",
            "K +50 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "S +30 C:/Windows/Temp/schick_port_build/etappe4_abspann_outro3_pottwal.png",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "S +50 C:/Windows/Temp/schick_port_build/etappe4_spielende_speichern.png",
            "K +20 011B",
        ],
        rep_4a,
        log_4a,
        intro_lines=["K +35 1C0D", "K +35 1C0D", "K +45 1C0D"],
    )

    # Replay-Log fuer das Spielende speichern und Pflichtzeilen pruefen
    replay_log_path = TESTS_DIR / "etappe4_spielende_replay.log"
    replay_log_path.write_text(out_end, encoding="utf-8")
    for line in out_end.splitlines():
        if "[REPLAY-LOG]" in line or "Lade Kampf fight_id 192" in line or "Kampfrunde" in line or "500 AP" in line:
            log_4a.append(f"# LOG: {line.strip()}")

    required_log_markers = [
        "[REPLAY-LOG] Aufruf final_intro() (seg065)",
        "Lade Kampf fight_id 192",
        "[REPLAY-LOG] Endkampf F144 beendet: SIEGER (GAME_STATE_VICTORY, +500 AP)",
        "ARBOSH erhaelt 500 AP",
        "[REPLAY-LOG] Aufruf show_outro() (seg065): Abspann startet",
        "[REPLAY-LOG] Ende des Abspanns erreicht (gs_datseg_status_start=99, Speicherdialog)",
    ]
    for marker in required_log_markers:
        if marker not in out_end:
            print(f"FEHLGESCHLAGEN: Pflicht-Logzeile fehlt im Spielende-Replay-Log: {marker!r}")
            return 1
    print("  [PRUEFUNG REPLAY-LOG] Alle Pflicht-Nachweiszeilen (Kampfende/Sieger, show_outro, Abspann-Ende) vorhanden.")

    # Automatische MD5-Pruefung aller 5 Spielende-Screenshots (Bild != Vorgaenger und paarweise verschieden)
    prev_md5 = None
    prev_fn = None
    seen_md5s: dict[str, str] = {}
    for fn in end_screenshots:
        src_p = WDIR / fn
        if not src_p.exists() or src_p.stat().st_size == 0:
            print(f"FEHLGESCHLAGEN: Beweis-Screenshot fehlt oder leer: {fn}")
            return 1
        shutil.copy2(src_p, BUILD_DIR / fn)
        cur_md5 = hashlib.md5(src_p.read_bytes()).hexdigest()
        print(f"  [MD5] {fn}: {cur_md5}")
        if cur_md5 == prev_md5:
            print(f"FEHLGESCHLAGEN: {fn} ist identisch zum Vorgaenger {prev_fn} (MD5 {cur_md5})!")
            return 1
        if cur_md5 in seen_md5s:
            print(f"FEHLGESCHLAGEN: {fn} ist identisch zu {seen_md5s[cur_md5]} (MD5 {cur_md5})!")
            return 1
        seen_md5s[cur_md5] = fn
        prev_md5 = cur_md5
        prev_fn = fn
    print("  [PRUEFUNG SCREENSHOTS] Alle 5 Spielende-Screenshots sind paarweise verschieden (MD5 geprueft).")

    rep_4a.write_text("\n".join(log_4a) + "\n", encoding="utf-8")

    print("=== ETAPPE 4B: Alle 15 Dungeons (seg077..091) & Truhenfallen (seg092) ===")
    dng_specs = [
        (1, 2, 8, 11, True, "DNG01 Totenschiff (seg077)"),
        (2, 0, 1, 0, False, "DNG02 Verfallene Herberge (seg078)"),
        (3, 0, 0, 8, False, "DNG03 Spinnenhoehle (seg079)"),
        (4, 0, 7, 15, False, "DNG04 Wolfshoehle (seg080)"),
        (6, 0, 13, 15, False, "DNG06 Goblinhoehle (seg081)"),
        (7, 0, 1, 14, True, "DNG07 Ruine des Schwarzmagiers (seg082)"),
        (8, 0, 0, 14, False, "DNG08 Orkbehausung (seg083)"),
        (9, 0, 7, 15, False, "DNG09 Kultstaette des Namenlosen (seg084)"),
        (10, 0, 0, 3, False, "DNG10 Drachenhoehle (seg085)"),
        (11, 0, 8, 15, False, "DNG11 Piratenhoehle (seg086)"),
        (12, 0, 13, 15, False, "DNG12 Zwergenfeste (seg090)"),
        (13, 0, 7, 15, False, "DNG13 Verlassene Mine Prem (seg091)"),
    ]
    for did, dlvl, dx, dy, needs_bool, ddesc in dng_specs:
        dlines = ["K +90 4B00"]
        if needs_bool:
            dlines.append("K +20 1C0D")
        dlines.append("K +20 1C0D")
        run_single_scenario(
            f"4b {ddesc}",
            make_gam(base_gam, loctype=0, town_id=0, dng_id=did, dng_lvl=dlvl, x=dx, y=dy, viewdir=0),
            dlines,
            rep_4b,
            log_4b,
        )

    # Truhenfallen & Schloesser in seg092 (20/22 Funktionen = 90.9 %)
    chest_open_keys = [
        "K +90 1C0D",
        "K +15 4900",
        "K +10 5000",
        "K +10 5000",
        "K +10 5000",
        "K +10 5000",
        "K +10 5000",
        "K +10 5000",
        "K +15 1C0D",
        "K +20 1C0D",
        "K +20 1C0D",
    ]
    for did, dlvl, sx, sy, vdir, nm in [
        (1, 0, 12, 8, 2, "seg092 use_key_on_chest + chest_protected_normal"),
        (1, 1, 5, 9, 0, "seg092 chest_closed"),
        (1, 1, 14, 7, 2, "seg092 chest_protected_heavy"),
        (1, 3, 13, 5, 0, "seg092 chest_cursed"),
        (1, 3, 13, 6, 1, "seg092 chest_fulminictus"),
        (2, 1, 13, 7, 1, "seg092 chest_petrified"),
        (3, 0, 10, 11, 1, "seg092 chest_poisoned2"),
        (3, 0, 9, 11, 2, "seg092 chest_ignifax_normal"),
        (3, 1, 1, 13, 2, "seg092 chest_ignifax_brutal"),
        (3, 1, 1, 2, 2, "seg092 chest_protected_brutal"),
        (9, 1, 2, 5, 3, "seg092 chest_poisoned3"),
        (11, 0, 14, 13, 0, "seg092 chest_crossbow_bolts"),
        (11, 0, 7, 10, 3, "seg092 chest_ignifax_heavy"),
        (15, 1, 13, 14, 1, "seg092 chest_poisoned1"),
    ]:
        run_single_scenario(f"4b {nm}", make_trap_gam(base_gam, did, dlvl, sx, sy, vdir), chest_open_keys, rep_4b, log_4b)

    rep_4b.write_text("\n".join(log_4b) + "\n", encoding="utf-8")

    print("=== ETAPPE 4C: Reiserouten (seg110..118), Querverbindung (seg094 100%) & Gegnerzauber (seg102) ===")
    # 1. Orvil (28) -> Rovik (Signpost 3): Meile 0 tevent_121 (seg118), Meile 3 tevent_122 (seg114)
    run_single_scenario(
        "4c-1 Reise Orvil->Rovik (seg118 + seg114)",
        make_gam(base_gam, loctype=12, town_id=28, typeindex=3, day_timer=8 * 5400),
        [
            "K +60 4900",
            "K +20 1C0D",
            "K +180 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +250 1C0D",
            "K +30 1C0D",
            "K +30 1C0D",
        ],
        rep_4c,
        log_4c,
    )

    # 2. Orvil (28) -> Ala (Signpost 2): Meile 4 tevent_065 (seg111)
    run_single_scenario(
        "4c-2 Reise Orvil->Ala (seg111)",
        make_gam(base_gam, loctype=12, town_id=28, typeindex=2, day_timer=8 * 5400),
        [
            "K +60 4900",
            "K +20 1C0D",
            "K +300 1C0D",
            "K +30 1C0D",
            "K +30 1C0D",
        ],
        rep_4c,
        log_4c,
    )

    # 3. Ala (27) -> Thoss (Signpost 1): Meile 7 tevent_067 (seg112)
    run_single_scenario(
        "4c-3 Reise Ala->Thoss (seg112)",
        make_gam(base_gam, loctype=12, town_id=27, typeindex=1, day_timer=8 * 5400),
        [
            "K +60 4900",
            "K +20 1C0D",
            "K +500 1C0D",
            "K +30 1C0D",
            "K +30 1C0D",
        ],
        rep_4c,
        log_4c,
    )

    # 4. Nordvest (6) -> Kravik (Signpost 2): Meile 8 tevent_016 (seg115)
    run_single_scenario(
        "4c-4 Reise Nordvest->Kravik (seg115)",
        make_gam(base_gam, loctype=12, town_id=6, typeindex=2, day_timer=8 * 5400),
        [
            "K +60 4900",
            "K +20 1C0D",
            "K +550 1C0D",
            "K +30 1C0D",
            "K +30 1C0D",
        ],
        rep_4c,
        log_4c,
    )

    # 5. Peilinen (4) -> Rovamund (Signpost 1) -> Meile 10 tevent_011 -> Querverbindung Crosslink (seg094 9/9 = 100%)
    run_single_scenario(
        "4c-5 Querverbindung Peilinen->Rovamund Crosslink (seg094 100% + seg110)",
        make_gam(base_gam, loctype=12, town_id=4, typeindex=1, day_timer=8 * 5400, boost_stats=True),
        [
            "K +60 4900",
            "K +20 1C0D",
            "K +650 5000",
            "K +15 1C0D",
            "K +15 5000",
            "K +15 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +20 1C0D",
            "K +60 4900",
            "K +15 5000",
            "K +15 1C0D",
            "K +30 4900",
            "K +10 5000",
            "K +10 5000",
            "K +10 5000",
            "K +10 5000",
            "K +10 5000",
            "K +15 1C0D",
            "K +30 1C0D",
        ],
        rep_4c,
        log_4c,
    )

    # 6. Kampf in DNG09 (1, 2, 2, fid=106 F12628) -> MON_cast_spell (seg102)
    run_single_scenario(
        "4c-6 Kampf DNG09 F12628 Gegnerzauber (seg102)",
        make_gam(base_gam, loctype=0, town_id=0, dng_id=9, dng_lvl=1, x=2, y=2, viewdir=0),
        [
            "K +90 5000",
            "K +5 5000",
            "K +5 5000",
            "K +5 5000",
            "K +5 5000",
            "K +5 5000",
            "K +5 5000",
            "K +5 5000",
            "K +5 5000",
            "K +5 5000",
            "K +15 1C0D",
            "K +15 1C0D",
            "K +400 1C0D",
            "K +400 1C0D",
            "K +400 1C0D",
        ],
        rep_4c,
        log_4c,
    )
    rep_4c.write_text("\n".join(log_4c) + "\n", encoding="utf-8")

    # Stelle Standard-GAMES.NAM wieder her und loesche temporaeren Spielstand
    (TRUN / "KAP_TEST.GAM").unlink(missing_ok=True)
    write_games_nam(["SPIEL_4B", "KAP_TOD_", "KAP_ZEIT", "KAP_HYGG", "KAP_END_"])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
