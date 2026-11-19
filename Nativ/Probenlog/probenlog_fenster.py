#!/usr/bin/env python3
"""probenlog_fenster.py - Live-Probenlog-Fenster fuer Linux und macOS (Ersatz fuer probenlog_fenster.ps1).

Oeffnet ein eigenes Fenster (Tkinter-GUI, oder bei fehlendem Tkinter ein Terminal-Fenster
bzw. Live-Konsolenausgabe) und zeigt alle Wuerfelproben aus PROBEN.CSV laufend als Klartext an.
"""
import os
import shutil
import subprocess
import sys
import time

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, SCRIPT_DIR)
import probenlog  # noqa: E402


def run_tk_window(csv_path: str) -> bool:
    if os.environ.get("DSA_NO_TK", "0") == "1":
        return False
    if not os.environ.get("DISPLAY") and not os.environ.get("WAYLAND_DISPLAY") and sys.platform != "darwin":
        return False
    try:
        import tkinter as tk
        from tkinter import scrolledtext
    except Exception:
        return False

    try:
        root = tk.Tk()
    except Exception:
        return False

    spiel = getattr(probenlog, "SPIEL", "DSA")
    root.title(f"Probenlog – {spiel} ({os.path.basename(csv_path)})")
    root.geometry("920x420")
    root.configure(bg="#141414")

    header = tk.Label(
        root,
        text=f"Probenlog {spiel} — liest {csv_path} live mit",
        bg="#1f1f1f",
        fg="#e0c068",
        font=("Monospace", 10, "bold"),
        anchor="w",
        padx=8,
        pady=4,
    )
    header.pack(fill=tk.X)

    txt = scrolledtext.ScrolledText(
        root,
        bg="#101010",
        fg="#d8d8d8",
        insertbackground="#ffffff",
        font=("Monospace", 10),
        wrap=tk.WORD,
        state=tk.DISABLED,
    )
    txt.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)
    txt.tag_config("ok", foreground="#7cd97c")
    txt.tag_config("fail", foreground="#f07878")
    txt.tag_config("crit", foreground="#ff4040")
    txt.tag_config("info", foreground="#d8d8d8")

    leser = probenlog.Leser()
    state = {"fp": None, "rest": ""}

    def append_line(line: str):
        tag = "info"
        if "PATZER" in line:
            tag = "crit"
        elif "gelungen" in line or "getroffen" in line or "trifft" in line or "pariert (" in line or " → pariert" in line:
            tag = "ok"
        elif "misslungen" in line or "daneben" in line or "verfehlt" in line or "pariert nicht" in line:
            tag = "fail"
        txt.configure(state=tk.NORMAL)
        txt.insert(tk.END, line + "\n", tag)
        txt.see(tk.END)
        txt.configure(state=tk.DISABLED)

    def poll_csv():
        if state["fp"] is None:
            if os.path.exists(csv_path):
                try:
                    state["fp"] = open(csv_path, encoding="latin-1")
                except Exception:
                    pass
        if state["fp"] is not None:
            while True:
                s = state["fp"].readline()
                if not s:
                    break
                state["rest"] += s
                if not state["rest"].endswith("\n"):
                    continue
                out = leser.zeile(state["rest"])
                state["rest"] = ""
                if out:
                    append_line(out)
        root.after(200, poll_csv)

    root.after(100, poll_csv)
    root.mainloop()
    if state["fp"] is not None:
        state["fp"].close()
    return True


def launch_terminal_fallback(csv_path: str) -> bool:
    if os.environ.get("DSA_PROBENLOG_IN_TERM", "0") == "1":
        return False
    if not os.environ.get("DISPLAY") and not os.environ.get("WAYLAND_DISPLAY"):
        return False
    py_script = os.path.join(SCRIPT_DIR, "probenlog.py")
    env = os.environ.copy()
    env["DSA_PROBENLOG_IN_TERM"] = "1"
    terms = [
        ("x-terminal-emulator", ["-e", sys.executable, py_script, "--live", csv_path]),
        ("gnome-terminal", ["--", sys.executable, py_script, "--live", csv_path]),
        ("konsole", ["-e", sys.executable, py_script, "--live", csv_path]),
        ("xfce4-terminal", ["-x", sys.executable, py_script, "--live", csv_path]),
        ("mate-terminal", ["-x", sys.executable, py_script, "--live", csv_path]),
        ("xterm", ["-hold", "-e", sys.executable, py_script, "--live", csv_path]),
    ]
    for exe, args in terms:
        if shutil.which(exe):
            try:
                proc = subprocess.Popen([exe] + args, env=env)
                proc.wait()
                return True
            except Exception:
                continue
    return False


def main():
    args = [a for a in sys.argv[1:] if a not in ("--live", "--fenster", "--gui")]
    if not args:
        print(__doc__)
        sys.exit(1)
    csv_path = args[0]
    if "--headless" not in sys.argv:
        if run_tk_window(csv_path):
            return
        if launch_terminal_fallback(csv_path):
            return
    # Direkter Live-Mitlesemodus auf stdout
    sys.argv = [sys.argv[0], "--live", csv_path]
    probenlog.main()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
