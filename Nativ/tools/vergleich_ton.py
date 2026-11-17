#!/usr/bin/env python3
"""
vergleich_ton.py - W5 Audio-Referenzvergleich (Etappe 3)
Nimmt TEMPLE.XMI und THORWAL.XMI im Original unter DOSBox (Strg+F6) sowie im
Port (General MIDI ueber Windows midiOut "Microsoft GS Wavetable Synth" und
AdLib/OPL2 ueber den DOSBox-DBOPL-Emulator) als WAV-Hoerproben auf, berechnet
Pegel- und Frequenzspektren (DFT ueber 16 Frequenzbaender 63 Hz .. 8 kHz) und
erzeugt Vergleichsgrafiken (PNG) in Port/build/ton_vergleich/.
"""

from __future__ import annotations

import math
import os
import shutil
import struct
import subprocess
import wave
import zlib
from pathlib import Path

PORT_DIR = Path(__file__).resolve().parent.parent


def _default_win_root() -> Path:
    if os.name == "nt":
        return Path(os.environ.get("SystemDrive", "C:") + "\\")
    parts = PORT_DIR.parts
    if len(parts) >= 3 and parts[1] == "mnt" and len(parts[2]) == 1:
        return Path("/") / parts[1] / parts[2]
    return Path("/tmp")


_WIN_ROOT = _default_win_root()
WDIR = Path(
    os.environ.get(
        "SCHICK_BUILD_DIR",
        str(_WIN_ROOT / "Windows" / "Temp" / "schick_port_build"),
    )
)
OUT_DIR = PORT_DIR / "build" / "ton_vergleich"
MINGW_BIN = os.environ.get(
    "SCHICK_MINGW_BIN",
    str(_WIN_ROOT / "Program Files (x86)" / "Embarcadero" / "Dev-Cpp" / "TDM-GCC-64" / "bin"),
)
GOG_DIR_WIN = os.environ.get(
    "SCHICK_GOG_WIN",
    r"C:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania",
)
GOG_DIR_WSL = Path(
    os.environ.get(
        "SPIELPFAD",
        str(_WIN_ROOT / "Program Files (x86)" / "GOG Galaxy" / "Games" / "Realms of Arkania"),
    )
)


WASAPI_REC_CPP = r"""
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static const CLSID CLSID_MMDeviceEnumerator_Local = {0xBCDE0395, 0xE52F, 0x467C, {0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E}};
static const IID   IID_IMMDeviceEnumerator_Local  = {0xA95664D2, 0x9614, 0x4F35, {0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6}};
static const IID   IID_IAudioClient_Local         = {0x1CB9AD4C, 0xDBFA, 0x4C32, {0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2}};
static const IID   IID_IAudioCaptureClient_Local  = {0xC8ADBD64, 0xE71E, 0x48A0, {0xA4,0xDE,0x18,0x5C,0x39,0x5C,0xD3,0x17}};

int main(int argc, char **argv) {
    if (argc < 3) return 1;
    const char *out_wav = argv[1];
    int dur_ms = atoi(argv[2]);
    if (dur_ms <= 0) dur_ms = 4000;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    IMMDeviceEnumerator *pEnum = NULL;
    IMMDevice *pDev = NULL;
    IAudioClient *pClient = NULL;
    IAudioCaptureClient *pCap = NULL;
    WAVEFORMATEX *pwfx = NULL;

    if (FAILED(CoCreateInstance(CLSID_MMDeviceEnumerator_Local, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator_Local, (void**)&pEnum))) return 2;
    if (FAILED(pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDev))) return 3;
    if (FAILED(pDev->Activate(IID_IAudioClient_Local, CLSCTX_ALL, NULL, (void**)&pClient))) return 4;
    if (FAILED(pClient->GetMixFormat(&pwfx))) return 5;
    if (FAILED(pClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 10000000, 0, pwfx, NULL))) return 6;
    if (FAILED(pClient->GetService(IID_IAudioCaptureClient_Local, (void**)&pCap))) return 7;

    int rate = (int)pwfx->nSamplesPerSec;
    int ch   = (int)pwfx->nChannels;
    int bits = (int)pwfx->wBitsPerSample;
    int is_float = (pwfx->wFormatTag == 3) || (pwfx->wFormatTag == 0xFFFE && bits == 32);

    uint32_t max_frames = (uint32_t)((rate * (dur_ms + 1000)) / 1000);
    int16_t *pcm = (int16_t *)calloc(max_frames * 2, sizeof(int16_t));
    uint32_t total_frames = 0;

    pClient->Start();
    DWORD t0 = GetTickCount();
    while ((int)(GetTickCount() - t0) < dur_ms && total_frames < max_frames) {
        Sleep(10);
        UINT32 pkt = 0;
        while (SUCCEEDED(pCap->GetNextPacketSize(&pkt)) && pkt > 0) {
            BYTE *data = NULL;
            UINT32 nframes = 0;
            DWORD flags = 0;
            if (SUCCEEDED(pCap->GetBuffer(&data, &nframes, &flags, NULL, NULL))) {
                for (UINT32 i = 0; i < nframes && total_frames < max_frames; ++i) {
                    int16_t l = 0, r = 0;
                    if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT) && data) {
                        if (is_float) {
                            const float *fp = (const float *)(data + i * pwfx->nBlockAlign);
                            float fl = fp[0];
                            float fr = (ch > 1) ? fp[1] : fl;
                            int il = (int)(fl * 32767.0f);
                            int ir = (int)(fr * 32767.0f);
                            if (il > 32767) il = 32767; if (il < -32768) il = -32768;
                            if (ir > 32767) ir = 32767; if (ir < -32768) ir = -32768;
                            l = (int16_t)il; r = (int16_t)ir;
                        } else if (bits == 16) {
                            const int16_t *sp = (const int16_t *)(data + i * pwfx->nBlockAlign);
                            l = sp[0];
                            r = (ch > 1) ? sp[1] : l;
                        }
                    }
                    pcm[total_frames * 2 + 0] = l;
                    pcm[total_frames * 2 + 1] = r;
                    total_frames++;
                }
                pCap->ReleaseBuffer(nframes);
            } else {
                break;
            }
        }
    }
    pClient->Stop();

    FILE *fp = fopen(out_wav, "wb");
    if (fp) {
        uint32_t data_bytes = total_frames * 4;
        uint32_t riff_len = data_bytes + 36;
        uint32_t fmt_len = 16;
        uint16_t fmt_tag = 1, out_ch = 2, blk = 4, out_bits = 16;
        uint32_t out_rate = (uint32_t)rate, byte_rate = out_rate * 4;
        fwrite("RIFF", 1, 4, fp); fwrite(&riff_len, 4, 1, fp);
        fwrite("WAVEfmt ", 1, 8, fp); fwrite(&fmt_len, 4, 1, fp);
        fwrite(&fmt_tag, 2, 1, fp); fwrite(&out_ch, 2, 1, fp);
        fwrite(&out_rate, 4, 1, fp); fwrite(&byte_rate, 4, 1, fp);
        fwrite(&blk, 2, 1, fp); fwrite(&out_bits, 2, 1, fp);
        fwrite("data", 1, 4, fp); fwrite(&data_bytes, 4, 1, fp);
        fwrite(pcm, 1, data_bytes, fp);
        fclose(fp);
    }
    free(pcm);
    return 0;
}
"""


DOSBOX_WAV_CPP = r"""
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

struct FindCtx {
    DWORD pid;
    HWND  hwnd;
    int   w, h;
};

static BOOL CALLBACK enum_cb(HWND hwnd, LPARAM lparam) {
    FindCtx *ctx = (FindCtx *)lparam;
    if (IsWindowVisible(hwnd)) {
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid == ctx->pid) {
            RECT r;
            GetClientRect(hwnd, &r);
            int w = r.right - r.left;
            int h = r.bottom - r.top;
            if (w >= 320 && h >= 200) {
                ctx->hwnd = hwnd;
                ctx->w = w;
                ctx->h = h;
                return FALSE;
            }
        }
    }
    return TRUE;
}

static void tap_key(HWND hwnd, BYTE vk, BYTE scan) {
    if (hwnd) { SetForegroundWindow(hwnd); SetFocus(hwnd); }
    Sleep(40);
    keybd_event(vk, scan, 0, 0);
    Sleep(50);
    keybd_event(vk, scan, KEYEVENTF_KEYUP, 0);
}

static void toggle_ctrl_f6(HWND hwnd) {
    if (hwnd) { SetForegroundWindow(hwnd); SetFocus(hwnd); }
    Sleep(40);
    keybd_event(VK_CONTROL, 0x1D, 0, 0);
    Sleep(30);
    keybd_event(VK_F6, 0x40, 0, 0);
    Sleep(50);
    keybd_event(VK_F6, 0x40, KEYEVENTF_KEYUP, 0);
    Sleep(30);
    keybd_event(VK_CONTROL, 0x1D, KEYEVENTF_KEYUP, 0);
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    const char *conf_path = argv[1];
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "\"C:\\Program Files (x86)\\GOG Galaxy\\Games\\Realms of Arkania\\DOSBOX\\DOSBox.exe\" -conf \"%s\" -noconsole",
        conf_path);

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL,
                        "C:\\Program Files (x86)\\GOG Galaxy\\Games\\Realms of Arkania",
                        &si, &pi)) {
        return 1;
    }

    FindCtx ctx = { pi.dwProcessId, NULL, 0, 0 };
    for (int i = 0; i < 30; ++i) {
        Sleep(150);
        EnumWindows(enum_cb, (LPARAM)&ctx);
        if (ctx.hwnd && ctx.w >= 320) break;
    }
    /* Warte auf GEN.EXE b n Start -> CD_play_track(4) laeuft sofort an */
    Sleep(1800);
    ctx.hwnd = NULL; EnumWindows(enum_cb, (LPARAM)&ctx);
    toggle_ctrl_f6(ctx.hwnd);
    Sleep(4800);
    ctx.hwnd = NULL; EnumWindows(enum_cb, (LPARAM)&ctx);
    toggle_ctrl_f6(ctx.hwnd);
    Sleep(400);
    /* Strg+F9 beendet DOSBox sauber, sodass die WAV-Datei geflusht und geschlossen wird */
    if (ctx.hwnd) { SetForegroundWindow(ctx.hwnd); SetFocus(ctx.hwnd); }
    Sleep(40);
    keybd_event(VK_CONTROL, 0x1D, 0, 0);
    Sleep(30);
    keybd_event(VK_F9, 0x43, 0, 0);
    Sleep(50);
    keybd_event(VK_F9, 0x43, KEYEVENTF_KEYUP, 0);
    Sleep(30);
    keybd_event(VK_CONTROL, 0x1D, KEYEVENTF_KEYUP, 0);
    if (WaitForSingleObject(pi.hProcess, 1500) != WAIT_OBJECT_0) {
        TerminateProcess(pi.hProcess, 0);
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
"""


def convert_xmi_to_mid(xmi_bytes: bytes, out_mid_path: Path, max_ticks: int = 2400) -> int:
    """Konvertiert einen XMIDI-EVNT-Strom exakt nach den Regeln des Ports in eine Standard-MIDI-Datei (.MID, 120 Hz)."""
    pos = xmi_bytes.find(b"EVNT")
    if pos < 0:
        return 0
    ln = struct.unpack_from(">I", xmi_bytes, pos + 4)[0]
    ev = xmi_bytes[pos + 8 : pos + 8 + ln]

    # Sammle (tick, status, d1, d2) Ereignisse
    events: list[tuple[int, bytes]] = []
    # Tempo 500000 us/quarter bei 60 ticks/quarter = 120 Hz
    events.append((0, b"\xFF\x51\x03\x07\xA1\x20"))

    i = 0
    cur_tick = 0
    note_Count = 0
    while i < len(ev) and cur_tick < max_ticks:
        if ev[i] < 0x80:
            cur_tick += ev[i]
            i += 1
            continue
        st = ev[i]
        i += 1
        if st == 0xFF:
            mt = ev[i]
            i += 1
            ml = 0
            while True:
                c = ev[i]
                i += 1
                ml = (ml << 7) | (c & 0x7F)
                if not (c & 0x80):
                    break
            if mt == 0x2F:
                break
            i += ml
        elif st in (0xF0, 0xF7):
            ml = 0
            while True:
                c = ev[i]
                i += 1
                ml = (ml << 7) | (c & 0x7F)
                if not (c & 0x80):
                    break
            i += ml
        else:
            hi = st & 0xF0
            ch = st & 0x0F
            if hi == 0x90:
                nt = ev[i] & 0x7F
                vel = ev[i + 1] & 0x7F
                i += 2
                dur = 0
                while i < len(ev) and (ev[i] & 0x80):
                    dur = (dur << 7) | (ev[i] & 0x7F)
                    i += 1
                if i < len(ev):
                    dur = (dur << 7) | (ev[i] & 0x7F)
                    i += 1
                if vel > 0:
                    events.append((cur_tick, bytes([0x90 | ch, nt, vel])))
                    events.append((cur_tick + max(1, dur), bytes([0x80 | ch, nt, 0])))
                    note_Count += 1
                else:
                    events.append((cur_tick, bytes([0x80 | ch, nt, 0])))
            elif hi == 0x80:
                nt = ev[i] & 0x7F
                i += 2
                events.append((cur_tick, bytes([0x80 | ch, nt, 0])))
            elif hi in (0xA0, 0xB0, 0xE0):
                d1 = ev[i] & 0x7F
                d2 = ev[i + 1] & 0x7F
                i += 2
                if hi != 0xB0 or d1 < 110 or d1 >= 120:
                    events.append((cur_tick, bytes([st, d1, d2])))
            elif hi in (0xC0, 0xD0):
                d1 = ev[i] & 0x7F
                i += 1
                events.append((cur_tick, bytes([st, d1])))

    events.sort(key=lambda x: x[0])

    def vlq(v: int) -> bytes:
        out = [v & 0x7F]
        v >>= 7
        while v > 0:
            out.append((v & 0x7F) | 0x80)
            v >>= 7
        return bytes(reversed(out))

    trk = bytearray()
    last_t = 0
    for t, msg in events:
        dt = max(0, t - last_t)
        last_t = t
        trk.extend(vlq(dt))
        trk.extend(msg)
    trk.extend(b"\x00\xFF\x2F\x00")

    hdr = b"MThd" + struct.pack(">IHHH", 6, 0, 1, 60) + b"MTrk" + struct.pack(">I", len(trk))
    out_mid_path.write_bytes(hdr + bytes(trk))
    return note_Count


def analyze_wav(wav_path: Path) -> dict:
    raw_file = wav_path.read_bytes()
    if len(raw_file) >= 44 and raw_file[:4] == b"RIFF" and struct.unpack_from("<I", raw_file, 24)[0] == 0:
        pcm_bytes = raw_file[44:]
        hdr = struct.pack(
            "<4sI4s4sIHHIIHH4sI",
            b"RIFF",
            len(pcm_bytes) + 36,
            b"WAVE",
            b"fmt ",
            16,
            1,
            2,
            44100,
            176400,
            4,
            16,
            b"data",
            len(pcm_bytes),
        )
        raw_file = hdr + pcm_bytes
        wav_path.write_bytes(raw_file)
    with wave.open(str(wav_path), "rb") as w:
        n = w.getnframes()
        ch = w.getnchannels()
        rate = w.getframerate()
        raw = w.readframes(n)
    samples = struct.unpack(f"<{n * ch}h", raw)
    mono = [samples[i * ch] for i in range(n)] if ch > 1 else list(samples)
    peak = max((abs(s) for s in mono), default=0)
    rms = math.sqrt(sum(s * s for s in mono) / max(1, len(mono)))

    # Berechne Spektrum ueber 12 Frequenzbaender (Goertzel / DFT ueber aktive Fenster)
    bands_hz = [80, 125, 180, 250, 350, 500, 700, 1000, 1400, 2000, 2800, 4000]
    band_e = [0.0] * len(bands_hz)
    win_size = 1024
    # Nimm bis zu 30 gleichmaessig verteilte nicht-stille Fenster
    active_starts = []
    for st in range(0, max(1, len(mono) - win_size), max(win_size, len(mono) // 40)):
        seg = mono[st : st + win_size]
        if max((abs(x) for x in seg), default=0) > 200:
            active_starts.append(st)
        if len(active_starts) >= 25:
            break

    for st in active_starts:
        seg = mono[st : st + win_size]
        for bi, f_hz in enumerate(bands_hz):
            omega = 2.0 * math.pi * f_hz / rate
            coeff = 2.0 * math.cos(omega)
            s0 = s1 = s2 = 0.0
            for x in seg:
                s0 = (x / 32768.0) + coeff * s1 - s2
                s2 = s1
                s1 = s0
            mag = math.sqrt(max(0.0, s1 * s1 + s2 * s2 - coeff * s1 * s2)) / win_size
            band_e[bi] += mag

    if active_starts:
        band_e = [e / len(active_starts) for e in band_e]
    max_b = max(band_e + [1e-6])
    band_norm = [e / max_b for e in band_e]
    return {
        "frames": n,
        "duration_s": n / max(1, rate),
        "rate": rate,
        "peak": peak,
        "rms": rms,
        "bands_hz": bands_hz,
        "band_norm": band_norm,
        "mono": mono,
    }


def save_spectrum_png(title: str, stats_list: list[tuple[str, dict, tuple[int, int, int]]], out_png: Path) -> None:
    w, h = 640, 320
    img = bytearray(w * h * 3)
    for i in range(w * h):
        img[i * 3 : i * 3 + 3] = b"\x18\x1c\x24"

    def rect(x0: int, y0: int, x1: int, y1: int, rgb: tuple[int, int, int]) -> None:
        r, g, b = rgb
        for y in range(max(0, y0), min(h, y1)):
            row = y * w * 3
            for x in range(max(0, x0), min(w, x1)):
                idx = row + x * 3
                img[idx] = r
                img[idx + 1] = g
                img[idx + 2] = b

    # Gitterlinien
    for gy in range(40, 270, 45):
        rect(40, gy, w - 30, gy + 1, (55, 62, 75))
    rect(40, 265, w - 30, 267, (160, 170, 185))

    n_bands = 12
    n_series = len(stats_list)
    group_w = (w - 80) // n_bands
    bar_w = max(6, (group_w - 8) // max(1, n_series))

    for bi in range(n_bands):
        gx = 45 + bi * group_w
        for si, (_, st, color) in enumerate(stats_list):
            val = st["band_norm"][bi]
            bh = int(val * 210)
            bx0 = gx + si * (bar_w + 2)
            rect(bx0, 265 - bh, bx0 + bar_w, 265, color)

    raw_rows = bytearray()
    for y in range(h):
        raw_rows.append(0)
        raw_rows.extend(img[y * w * 3 : (y + 1) * w * 3])

    def chunk(tag: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    png = (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(bytes(raw_rows), 6))
        + chunk(b"IEND", b"")
    )
    out_png.write_bytes(png)


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    dat = (WDIR / "test_run" / "SCHICK.DAT").read_bytes()
    offs = [struct.unpack_from("<I", dat, i * 4)[0] for i in range(309)]

    # 1. XMIDI -> Standard-MIDI (.MID) fuer TEMPLE.XMI (144) und THORWAL.XMI (142)
    n_temple = convert_xmi_to_mid(dat[offs[144] : offs[145]], OUT_DIR / "temple_gm.mid")
    n_thorwal = convert_xmi_to_mid(dat[offs[142] : offs[143]], OUT_DIR / "thorwal_gm.mid")
    print(f"[W5-Ton] MIDI-Export: temple_gm.mid ({n_temple} Noten), thorwal_gm.mid ({n_thorwal} Noten)")

    # 2. Port-Replays fuer TEMPLE (im Tempel) und THORWAL (in 3D-Stadt)
    (WDIR / "ton_temple.rep").write_text(
        "K +25 1C0D\nK +25 1C0D\nK +30 1C0D\nQ +220\n", encoding="utf-8"
    )
    (WDIR / "ton_thorwal.rep").write_text(
        "K +25 1C0D\nK +25 1C0D\nK +30 1C0D\nK +25 4300\nQ +220\n", encoding="utf-8"
    )

    # Stelle sicher, dass SPIEL_4B an Position 0 von GAMES.NAM steht
    nam_path = WDIR / "test_run" / "GAMES.NAM"
    nam_bytes = b"SPIEL_4B\x00" + b"LEER____\x00" * 4
    nam_path.write_bytes(nam_bytes)

    # 3. Port CD-Audio-Aufnahme (Standard wie MobileEyes/GOG: MUSIC/TrackNN.OGG) und OPL2-Aufnahme
    for track, rep_name in [("temple", "ton_temple.rep"), ("thorwal", "ton_thorwal.rep")]:
        wav_cd = WDIR / f"port_{track}_cd.wav"
        if wav_cd.exists():
            wav_cd.unlink()
        subprocess.run(
            [
                str(WDIR / "SCHICK_PORT.EXE"),
                "--music=cd",
                "--dump-wav",
                f"C:/Windows/Temp/schick_port_build/port_{track}_cd.wav",
                "--replay",
                f"C:/Windows/Temp/schick_port_build/{rep_name}",
                "C:/Windows/Temp/schick_port_build/test_run",
            ],
            cwd=str(WDIR / "test_run"),
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=20,
            check=True,
        )
        shutil.copy2(wav_cd, OUT_DIR / f"port_{track}_cd.wav")

        out_opl = OUT_DIR / f"port_{track}_opl.wav"
        if out_opl.exists() and out_opl.stat().st_size > 100000:
            continue
        wav_out = WDIR / f"port_{track}_opl.wav"
        if wav_out.exists():
            wav_out.unlink()
        subprocess.run(
            [
                str(WDIR / "SCHICK_PORT.EXE"),
                "--music=opl",
                "--dump-wav",
                f"C:/Windows/Temp/schick_port_build/port_{track}_opl.wav",
                "--replay",
                f"C:/Windows/Temp/schick_port_build/{rep_name}",
                "C:/Windows/Temp/schick_port_build/test_run",
            ],
            cwd=str(WDIR / "test_run"),
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=20,
            check=True,
        )
        shutil.copy2(wav_out, out_opl)

    # 4. Baue wasapi_rec.exe und nimm General MIDI ("Microsoft GS Wavetable Synth") live auf
    env = os.environ.copy()
    env["PATH"] = f"{MINGW_BIN}:{env.get('PATH', '')}"
    env["WSLENV"] = "PATH/l"
    if not (OUT_DIR / "port_temple_gm.wav").exists() or not (OUT_DIR / "port_thorwal_gm.wav").exists():
        (WDIR / "wasapi_rec.cpp").write_text(WASAPI_REC_CPP, encoding="utf-8")
        subprocess.run(
            ["bash", "-c", "g++.exe -m32 -O2 wasapi_rec.cpp -o wasapi_rec.exe -lole32 -luuid"],
            cwd=str(WDIR),
            env=env,
            stdin=subprocess.DEVNULL,
            check=True,
        )
        for track, rep_name in [("temple", "ton_temple.rep"), ("thorwal", "ton_thorwal.rep")]:
            gm_wav = WDIR / f"port_{track}_gm.wav"
            if gm_wav.exists():
                gm_wav.unlink()
            rec_proc = subprocess.Popen(
                [str(WDIR / "wasapi_rec.exe"), f"C:/Windows/Temp/schick_port_build/port_{track}_gm.wav", "5200"],
                cwd=str(WDIR),
                stdin=subprocess.DEVNULL,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            subprocess.run(
                [
                    str(WDIR / "SCHICK_PORT.EXE"),
                    "--music=gm",
                    "--replay",
                    f"C:/Windows/Temp/schick_port_build/{rep_name}",
                    "C:/Windows/Temp/schick_port_build/test_run",
                ],
                cwd=str(WDIR / "test_run"),
                stdin=subprocess.DEVNULL,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=20,
                check=True,
            )
            rec_proc.wait(timeout=10)
            if gm_wav.exists():
                shutil.copy2(gm_wav, OUT_DIR / f"port_{track}_gm.wav")

    # 5. DOSBox-Referenzaufnahme (Strg+F6 WAV-Aufnahme im Original unter DOSBox)
    if not (OUT_DIR / "dosbox_temple_ref.wav").exists() or not (OUT_DIR / "dosbox_thorwal_ref.wav").exists():
        cap_dir = WDIR / "dosbox_wav_cap"
        cap_dir.mkdir(parents=True, exist_ok=True)
        (WDIR / "dosbox_wav.cpp").write_text(DOSBOX_WAV_CPP, encoding="utf-8")
        subprocess.run(
            ["bash", "-c", "g++.exe -m32 -O2 dosbox_wav.cpp -o dosbox_wav.exe -lgdi32 -luser32"],
            cwd=str(WDIR),
            env=env,
            stdin=subprocess.DEVNULL,
            check=True,
        )
        orig_inst = (GOG_DIR_WSL / "game.inst").read_text(encoding="latin-1")
        for track, ogg_num in [("temple", "Track07.OGG"), ("thorwal", "Track06.OGG")]:
            for old_w in cap_dir.glob("*.wav"):
                old_w.unlink()
            custom_inst = orig_inst.replace('FILE "Music\\Track04.OGG" MP3', f'FILE "Music\\{ogg_num}" MP3')
            inst_path = WDIR / f"game_{track}.inst"
            inst_path.write_text(custom_inst, encoding="latin-1")
            conf_txt = f"""[sdl]
fullscreen=false
output=surface
autolock=false

[dosbox]
machine=svga_s3
captures=C:\\Windows\\Temp\\schick_port_build\\dosbox_wav_cap
memsize=16

[cpu]
core=auto
cputype=auto
cycles=fixed 12000

[mixer]
nosound=false
rate=44100
blocksize=1024
prebuffer=80

[midi]
mpu401=intelligent
mididevice=default

[sblaster]
sbtype=sb16
sbbase=220
irq=7
dma=1
hdma=5
sbmixer=true
oplmode=auto
oplemu=default
oplrate=44100

[autoexec]
@echo off
mount C "."
imgmount D "C:\\Windows\\Temp\\schick_port_build\\game_{track}.inst" -t iso
c:
gen.exe b n
exit
"""
            conf_path = WDIR / f"dosbox_ton_{track}.conf"
            conf_path.write_text(conf_txt, encoding="latin-1")
            subprocess.run(
                [str(WDIR / "dosbox_wav.exe"), f"C:\\Windows\\Temp\\schick_port_build\\dosbox_ton_{track}.conf"],
                cwd=str(WDIR),
                stdin=subprocess.DEVNULL,
                timeout=20,
                check=False,
            )
            cap_wavs = sorted(cap_dir.glob("*.wav"))
            if cap_wavs:
                shutil.copy2(cap_wavs[0], OUT_DIR / f"dosbox_{track}_ref.wav")

    # 6. Auswertung & Spektrum-Vergleichsbilder
    for track in ["temple", "thorwal"]:
        series = []
        p_cd  = OUT_DIR / f"port_{track}_cd.wav"
        p_opl = OUT_DIR / f"port_{track}_opl.wav"
        p_gm  = OUT_DIR / f"port_{track}_gm.wav"
        p_db  = OUT_DIR / f"dosbox_{track}_ref.wav"
        if p_cd.exists():
            st_cd = analyze_wav(p_cd)
            series.append(("Port Standard (CD-Audio OGG / MobileEyes)", st_cd, (60, 215, 110)))
            print(f"[{track.upper()} - Port CD]    dauer={st_cd['duration_s']:.2f}s peak={st_cd['peak']} rms={st_cd['rms']:.1f}")
        if p_db.exists():
            st_db = analyze_wav(p_db)
            series.append(("DOSBox Original (Strg+F6)", st_db, (235, 165, 75)))
            print(f"[{track.upper()} - DOSBox]     dauer={st_db['duration_s']:.2f}s peak={st_db['peak']} rms={st_db['rms']:.1f}")
        if p_gm.exists():
            st_gm = analyze_wav(p_gm)
            series.append(("Port GM (midiOut)", st_gm, (175, 130, 240)))
            print(f"[{track.upper()} - Port GM]    dauer={st_gm['duration_s']:.2f}s peak={st_gm['peak']} rms={st_gm['rms']:.1f}")
        if p_opl.exists():
            st_opl = analyze_wav(p_opl)
            series.append(("Port OPL2 (DBOPL)", st_opl, (90, 160, 240)))
            print(f"[{track.upper()} - Port OPL2]  dauer={st_opl['duration_s']:.2f}s peak={st_opl['peak']} rms={st_opl['rms']:.1f}")
        if series:
            save_spectrum_png(track.upper(), series, OUT_DIR / f"{track}_spektrum_vergleich.png")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
