#!/usr/bin/env python3
"""bauen.py - baut BLADEM.EXE (DSA 1 Die Schicksalsklinge, Attic 1992) aus dem rekonstruierten Quelltext
und vergleicht sie mit dem Original (SCHICKM.EXE, CD-Fassung 3.02 deutsch).

Laeuft unter Windows und Linux, nur Python-Standardbibliothek. DOSBox: unter Windows die aus dem GOG-Spiel (0.74),
unter Linux DOSBox Staging, DOSBox 0.74 oder DOSBox-X (auch als Flatpak io.github.dosbox-staging).
Ablauf: Bauordner leeren -> Compiler und Quelle hineinkopieren -> Datum 12.01.1994 setzen (DATUM.COM, das Datum
steht in der EXE) -> alle Module in der Reihenfolge von Quelle\\build.bat uebersetzen (BCC/TASM) -> in der
Entwicklungsumgebung linken (BC.EXE /b BLADEM.PRJ, wie Attic) -> Vergleich -> Build\\BLADEM.EXE.

Wichtig: Die Module werden immer in derselben Reihenfolge und im selben DOSBox-Lauf uebersetzt. BCC 3.1 liest
bei der Registerwahl fuer switch nicht vorbelegten Speicher; das Ergebnis kann von der vorher uebersetzten Datei
abhaengen (so bei Sternenschweif O030 beobachtet).

Aufruf:  python3 bauen.py [wichtig|alles] [--exploits-lassen] [--fixflags "..."]
                         [--projekt ORDNER] [--compiler ORDNER] [--dosbox DOSBox.exe] [--baudir ORDNER] [--sichtbar]

Fix-Modus (python bauen.py wichtig | alles): nach dem Standardbau (FIX_BUGS=0, muss bytegleich sein) zusaetzlich die
Fehlerbehebungen einschalten (Quelle\\s_fix.h: FIX_BUGS=1 "wichtig" = kritisch + mittel, 2 "alles"), alle Module mit
den Schaltern neu uebersetzen, residente Module auf ihre Originallaenge auffuellen (Werkzeuge\\segpad.py), linken und
gegen den Standardbau pruefen (Werkzeuge\\fixcheck.py) -> Build\\BLADEM_FIX_WICHTIG.EXE bzw. BLADEM_FIX_ALLES.EXE.
ALLE BEHEBUNGEN SIND IM SPIEL UNGETESTET (siehe Bauen\\FIXES.md)."""
import os, sys, re, shutil, stat, hashlib, subprocess, time, argparse, tempfile

HIER = os.path.dirname(os.path.abspath(__file__))
WINDOWS = os.name == 'nt'
ORIGINAL_MD5 = '4ce035f95ead01a8b2553b4487544e5d'
DATUM = (1994, 1, 12)          # Linkdatum von BLADEM.EXE 3.02
# DATUM.COM: mov ah,2Bh / mov cx,jahr / mov dx,monat:tag / int 21h / mov ax,4C00h / int 21h
DATUM_COM = bytes([0xB4, 0x2B, 0xB9, DATUM[0] & 255, DATUM[0] >> 8, 0xBA, DATUM[2], DATUM[1], 0xCD, 0x21,
                   0xB8, 0x00, 0x4C, 0xCD, 0x21])
GOG = [r'C:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania',
       r'C:\Program Files\GOG Galaxy\Games\Realms of Arkania',
       r'C:\GOG Games\Realms of Arkania', r'D:\GOG Games\Realms of Arkania',
       r'C:\GOG Games\Realms of Arkania - Blade of Destiny']
LOG = None
DOSLOG = None                  # Build/dosbox.log: Ausgabe aller DOSBox-Laeufe
HINWEIS_FENSTER = False
_ART = {}


def pfad(*teile):
    """Pfadangabe fuer Meldungen, mit dem Trenner des Systems (Build\\bau.log bzw. Build/bau.log)"""
    return os.sep.join(teile)


def say(*a):
    t = ' '.join(str(x) for x in a)
    print(t, flush=True)
    if LOG: LOG.write(t + '\n'); LOG.flush()


def abbruch(msg):
    say(''); say('ABBRUCH: ' + msg)
    raise SystemExit(2)


def md5(p):
    return hashlib.md5(open(p, 'rb').read()).hexdigest()


def _rw(func, path, _):
    os.chmod(path, stat.S_IWRITE | stat.S_IREAD); func(path)


def rmtree(p):
    if os.path.isdir(p):
        if sys.version_info >= (3, 12): shutil.rmtree(p, onexc=lambda f, q, e: _rw(f, q, e))
        else: shutil.rmtree(p, onerror=_rw)


def copytree(a, b):
    for d, _, fs in os.walk(a):
        z = os.path.join(b, os.path.relpath(d, a)); os.makedirs(z, exist_ok=True)
        for f in fs:
            q, t = os.path.join(d, f), os.path.join(z, f)
            if os.path.exists(t): os.chmod(t, stat.S_IWRITE | stat.S_IREAD)
            shutil.copyfile(q, t)


def finde_ci(d, name):
    if not d or not os.path.isdir(d): return None
    for f in os.listdir(d):
        if f.upper() == name.upper(): return os.path.join(d, f)
    return None


def finde_projekt(arg):
    if arg: return os.path.abspath(arg)
    d = HIER
    for _ in range(4):
        if os.path.isdir(os.path.join(d, 'Quelle')) and os.path.isdir(os.path.join(d, 'Original')): return d
        d = os.path.dirname(d)
    abbruch(f'Projektordner (mit {pfad("Quelle", "")} und {pfad("Original", "")}) nicht gefunden. {pfad("Bauen", "")} '
            'gehoert in den Projektordner - oder --projekt ORDNER angeben.')


def finde_compiler(arg, projekt):
    kand = [arg] if arg else []
    kand += [os.path.join(projekt, 'Compiler')]
    for k in kand:
        if k and finde_ci(finde_ci(k, 'BIN'), 'BCC.EXE') and finde_ci(finde_ci(k, 'BIN'), 'BC.EXE'): return k
    abbruch(f'Borland C++ 3.1 nicht gefunden. Bitte nach {pfad("Compiler", "")} kopieren (BIN, INCLUDE, LIB), '
            'zu finden auf archive.org: https://archive.org/details/bcpp31')


def finde_dosbox(arg):
    """liefert den Aufruf als Liste: [Programm] oder [flatpak, run, io.github.dosbox-staging].
    Linux: dieselbe Reihenfolge wie Linux/gemeinsam.sh (dosbox-staging, dosbox, dosbox-x, Flatpak)"""
    kand = [arg] if arg else []
    kand += [os.path.join(HIER, 'DOSBox', 'DOSBox.exe')]
    if WINDOWS:
        kand += [os.path.join(g, 'DOSBOX', 'DOSBox.exe') for g in GOG]
    else:
        kand += [shutil.which(n) or '' for n in ('dosbox-staging', 'dosbox', 'dosbox-x')]
    for k in kand:
        if k and os.path.isfile(k): return [os.path.abspath(k)]
    if not WINDOWS and shutil.which('flatpak'):
        if subprocess.run(['flatpak', 'info', 'io.github.dosbox-staging'], stdout=subprocess.DEVNULL,
                          stderr=subprocess.DEVNULL).returncode == 0:
            return ['flatpak', 'run', 'io.github.dosbox-staging']
    if WINDOWS:
        abbruch('DOSBox nicht gefunden (GOG-Ordner Realms of Arkania\\DOSBOX\\DOSBox.exe) - oder --dosbox PFAD angeben.')
    abbruch('DOSBox nicht gefunden. Bitte DOSBox Staging oder DOSBox installieren (z. B. sudo dnf install dosbox, '
            'sudo apt install dosbox oder flatpak install flathub io.github.dosbox-staging) '
            '- oder --dosbox PFAD angeben.')


def dosbox_art(cmd):
    """'staging', 'x' oder 'klassisch' (DOSBox 0.74): bestimmt Bildausgabe und Startoptionen.
    Unter Windows ist es die DOSBox aus dem GOG-Spiel (0.74), dort wird nicht nachgefragt."""
    k = ' '.join(cmd)
    if k in _ART: return _ART[k]
    if WINDOWS:
        _ART[k] = 'klassisch'; return _ART[k]
    try:
        r = subprocess.run(cmd + ['--version'], capture_output=True, text=True, timeout=20)
        v = (r.stdout + r.stderr).lower()
    except Exception:
        v = ''
    _ART[k] = 'staging' if 'staging' in v else ('x' if ('dosbox-x' in v or 'dosbox x' in v) else 'klassisch')
    return _ART[k]


def baudir(arg, projekt):
    if arg: return os.path.abspath(arg)
    if WINDOWS:
        d = os.path.join(tempfile.gettempdir(), 'schicksalsklinge_bau')
        if not d.isascii():          # DOSBox 0.74 vertraegt keine Umlaute im mount-Pfad
            d = os.environ.get('SystemDrive', 'C:') + '\\schicksalsklinge_bau'
        return d
    # Linux: nicht in /tmp (eine DOSBox aus Flatpak hat ein eigenes /tmp), sondern in Build/ des Projekts,
    # bei Umlauten im Pfad unter ~/.cache
    d = os.path.join(projekt, 'Build', '_bau')
    if not d.isascii():
        d = os.path.join(os.environ.get('XDG_CACHE_HOME') or os.path.expanduser('~/.cache'), 'schicksalsklinge_bau')
    return d


class Dos:
    def __init__(self, cmd, root, sichtbar):
        self.cmd, self.root, self.sichtbar = cmd, root, sichtbar
        self.art = dosbox_art(cmd)
        if self.art != 'klassisch' and not sichtbar:
            # DOSBox Staging und DOSBox-X brauchen eine Bildausgabe (OpenGL), unsichtbar laufen sie nicht
            # (Issue #1): das Fenster geht waehrend des Baus kurz auf
            self.sichtbar = True
            global HINWEIS_FENSTER
            if not HINWEIS_FENSTER:
                say(f'      Hinweis: DOSBox {"Staging" if self.art == "staging" else "-X"} braucht ein Fenster, '
                    'es geht waehrend des Baus kurz auf (das ist normal).')
                HINWEIS_FENSTER = True
        self.run_dir = os.path.join(root, '_dosbox'); os.makedirs(self.run_dir, exist_ok=True)

    def _lauf(self, cp, unsichtbar, timeout, name):
        env = dict(os.environ)
        if unsichtbar:
            # nur die klassische DOSBox 0.74 (SDL 1.2) laeuft mit dem Dummy-Treiber ohne Fenster
            env['SDL_VIDEODRIVER'] = 'dummy'
        env.setdefault('SDL_AUDIODRIVER', 'dummy')
        arg = self.cmd + ['-conf', cp] + (['-noconsole'] if WINDOWS else []) + ['-exit']
        if DOSLOG: DOSLOG.write(f'\n===== {name} ({"unsichtbar" if unsichtbar else "sichtbar"}, {self.art}): {" ".join(arg)}\n'); DOSLOG.flush()
        t0 = time.time()
        try:
            r = subprocess.run(arg, cwd=self.run_dir, env=env, stdout=DOSLOG or subprocess.DEVNULL,
                               stderr=subprocess.STDOUT, timeout=timeout)
        except subprocess.TimeoutExpired:
            abbruch(f'DOSBox-Lauf "{name}" nach {timeout} s nicht fertig (Ausgabe: {pfad("Build", "dosbox.log")}).')
        return r.returncode, time.time() - t0

    def run(self, wd, befehle, timeout, name):
        """fuehrt DOS-Befehle im Ordner C:\\wd aus; Ausgaben leiten die Befehle selbst in Dateien um"""
        sdl = ['[sdl]', 'fullscreen=false', 'autolock=false']
        if self.art == 'klassisch': sdl.insert(1, 'output=surface')
        conf = '\n'.join(sdl + ['[dosbox]', 'machine=svga_s3', 'memsize=16',
                          '[cpu]', 'core=normal', 'cputype=auto', 'cycles=max',
                          '[mixer]', 'nosound=true', '[speaker]', 'pcspeaker=false',
                          '[sblaster]', 'sbtype=none', '[gus]', 'gus=false', '[midi]', 'mpu401=none',
                          '[dos]', 'keyboardlayout=none',
                          '[autoexec]', f'mount c "{self.root}"', 'c:', 'C:\\DATUM.COM', f'cd \\{wd}']
                         + befehle + ['exit', ''])
        cp = os.path.join(self.run_dir, 'lauf.conf')
        open(cp, 'w', encoding='mbcs' if WINDOWS else 'utf-8').write(conf)
        rc, t = self._lauf(cp, not self.sichtbar, timeout, name)
        if rc != 0 and not self.sichtbar:
            say(f'      Hinweis: DOSBox ist unsichtbar abgebrochen (Rueckgabe {rc}), neuer Versuch mit Fenster ...')
            self.sichtbar = True
            rc, t = self._lauf(cp, False, timeout, name)
        if rc != 0:
            abbruch(f'DOSBox ist beim Lauf "{name}" abgestuerzt (Rueckgabe {rc}). Die Meldungen von DOSBox stehen in '
                    f'{pfad("Build", "dosbox.log")}.')
        return t


def fix_bau(projekt, root, dosbox, sichtbar, flags, name):
    """Fix-Modus (Portierung von Werkzeuge/build_fix.sh): alle Module mit den Schaltern neu uebersetzen, residente
    Module (SEG001..SEG013) auf die Laenge des Standardbaus auffuellen, linken, mit fixcheck pruefen."""
    werkz = os.path.join(projekt, 'Werkzeuge')
    segpad, fixcheck = finde_ci(werkz, 'segpad.py'), finde_ci(werkz, 'fixcheck.py')
    if not segpad or not fixcheck: abbruch(f'Fix-Modus: {pfad("Werkzeuge", "segpad.py")} oder fixcheck.py fehlt')
    src = os.path.join(root, 'SRC'); fix = os.path.join(root, 'FIX')
    rmtree(fix); copytree(os.path.join(projekt, 'Quelle'), fix)
    for f in os.listdir(fix):
        if f.upper().endswith(('.OBJ', '.EXE', '.MAP', '.LOG')): os.remove(os.path.join(fix, f))
    aflags = re.sub(r'-D', '/d', flags)
    bat = open(finde_ci(fix, 'build.bat'), encoding='cp437').read().replace('\r', '').split('\n')
    cmds = []
    for l in bat:
        t = l.strip()
        if not re.search(r'\\(BCC|TASM)\.EXE', t, re.I) or t.upper().startswith('@REM') or 'SETDATE' in t.upper(): continue
        t = re.sub(r'\.\.\\BIN\\', r'C:\\BC\\BIN\\', t, flags=re.I)
        if re.search(r'BCC\.EXE', t, re.I):
            t = re.sub(r'(BCC\.EXE)\s', lambda m: m.group(1) + ' ' + flags + ' ', t, count=1, flags=re.I)
        else:
            m = re.search(r'(\w+\.ASM)', t, re.I)
            a_ = finde_ci(fix, m.group(1)) if m else None
            if a_ and re.search(r'^\s*IF\b.*\bFIX_\d{4}', open(a_, encoding='cp437').read(), re.I | re.M):
                t = re.sub(r'(TASM\.EXE)\s', lambda m: m.group(1) + ' ' + aflags + ' ', t, count=1, flags=re.I)
        cmds.append(t + ' >> UEBERS.LOG')
    say(f'[5/7] Fix-Modus ({flags}): {len(cmds)} Module mit Schaltern uebersetzen')
    open(os.path.join(fix, 'UEBERS.BAT'), 'w', newline='\r\n').write('@ECHO OFF\n' + '\n'.join(cmds) + '\n')
    t = Dos(dosbox, root, sichtbar).run('FIX', ['CALL UEBERS.BAT'], 1800, 'Fix-Uebersetzen')
    log = open(os.path.join(fix, 'UEBERS.LOG'), 'rb').read().decode('cp437', 'replace') if os.path.exists(os.path.join(fix, 'UEBERS.LOG')) else ''
    fehler = [l for l in log.replace('\r', '').split('\n') if re.match(r'(Error \S+ \d+:|Fatal |\*\*Error|\*\*Fatal)', l)]
    objs = sorted(f.upper() for f in os.listdir(fix) if f.upper().endswith('.OBJ'))
    ref = sorted(f.upper() for f in os.listdir(src) if f.upper().endswith('.OBJ'))
    say(f'      {len(objs)} OBJ-Dateien, {t:.0f} s')
    for l in fehler[:30]: say('      ' + l)
    if fehler: abbruch(f'Fix-Modus: Uebersetzungsfehler (siehe oben bzw. {pfad("Build", "bau.log")})')
    if objs != ref: abbruch(f'Fix-Modus: {len(objs)} statt {len(ref)} OBJ-Dateien')
    say('[6/7] residente Module auf ihre Originallaenge auffuellen (segpad)')
    n = 0
    for o in objs:
        if not re.match(r'SEG0(0[1-9]|1[0-3])\.OBJ$', o): continue
        r = subprocess.run([sys.executable, '-I', segpad, finde_ci(fix, o), finde_ci(src, o)], capture_output=True, text=True)
        if r.returncode: say(r.stdout + r.stderr); abbruch(f'Fix-Modus: {o} passt nicht in sein residentes Segment')
        if 'Nullbytes' in r.stdout: n += 1
    say(f'      {n} residente Module aufgefuellt')
    say('[7/7] Fix-EXE linken und pruefen (fixcheck gegen den Standardbau)')
    Dos(dosbox, root, sichtbar).run('FIX', ['C:\\BC\\BIN\\BC.EXE /b BLADEM.PRJ > LINK.LOG'], 600, 'Fix-Linken')
    exe = finde_ci(fix, 'BLADEM.EXE')
    if not exe: abbruch(f'Fix-Modus: keine EXE erzeugt (Linkfehler, siehe {pfad("FIX", "LINK.LOG")} im Bauordner)')
    r = subprocess.run([sys.executable, '-I', fixcheck, exe, finde_ci(fix, 'BLADEM.MAP'),
                        finde_ci(src, 'BLADEM.EXE'), finde_ci(src, 'BLADEM.MAP')], capture_output=True, text=True)
    for l in (r.stdout + r.stderr).strip().split('\n'): say('      ' + l)
    if r.returncode: abbruch('Fix-Modus: fixcheck meldet Fehler - die Fix-EXE wird nicht ausgegeben')
    ziel = os.path.join(projekt, 'Build', f'BLADEM_FIX_{name}.EXE')
    shutil.copyfile(exe, ziel)
    shutil.copyfile(finde_ci(fix, 'BLADEM.MAP'), os.path.join(projekt, 'Build', f'BLADEM_FIX_{name}.MAP'))
    say(f'      {pfad("Build", f"BLADEM_FIX_{name}.EXE")}  {os.path.getsize(ziel)} Byte  MD5 {md5(ziel)}')
    say(f'      ALLE BEHEBUNGEN SIND IM SPIEL UNGETESTET - Rueckmeldungen willkommen ({pfad("Bauen", "FIXES.md")})')
    return ziel


def main():
    global LOG, DOSLOG
    ap = argparse.ArgumentParser(description='baut BLADEM.EXE aus dem Quelltext')
    ap.add_argument('fix', nargs='?', choices=['wichtig', 'alles'], type=str.lower,
                    help='zusaetzlich die Fix-EXE bauen: wichtig (FIX_BUGS=1) oder alles (FIX_BUGS=2)')
    ap.add_argument('--exploits-lassen', action='store_true',
                    help='Fix-Modus: Fehler, die dem Spieler nuetzen, nicht beheben (FIX_EXPLOITS=0)')
    ap.add_argument('--fixflags', default='', help='Fix-Modus: weitere Schalter fuer BCC, z. B. "-DFIX_0040=0"')
    ap.add_argument('--projekt'); ap.add_argument('--compiler'); ap.add_argument('--dosbox')
    ap.add_argument('--baudir'); ap.add_argument('--sichtbar', action='store_true')
    a = ap.parse_args()
    projekt = finde_projekt(a.projekt)
    os.makedirs(os.path.join(projekt, 'Build'), exist_ok=True)
    LOG = open(os.path.join(projekt, 'Build', 'bau.log'), 'w', encoding='utf-8')
    DOSLOG = open(os.path.join(projekt, 'Build', 'dosbox.log'), 'w', encoding='utf-8', errors='replace')
    say('Schicksalsklinge - Bau von BLADEM.EXE aus dem Quelltext')
    say('Projekt:  ', projekt)
    comp = finde_compiler(a.compiler, projekt); say('Compiler: ', comp)
    dosbox = finde_dosbox(a.dosbox); say('DOSBox:   ', ' '.join(dosbox), f'({dosbox_art(dosbox)})')
    orig = finde_ci(os.path.join(projekt, 'Original'), 'SCHICKM.EXE')
    if orig and md5(orig) != ORIGINAL_MD5:
        say(f'Hinweis: {pfad("Original", "SCHICKM.EXE")} ist nicht die CD-Fassung 3.02 (andere MD5). Verglichen wird trotzdem.')
    root = baudir(a.baudir, projekt); say('Bauordner:', root)

    N = 7 if a.fix else 4
    say(f'[1/{N}] Bauordner vorbereiten')
    rmtree(root); os.makedirs(root)
    for t in ('BIN', 'INCLUDE', 'LIB'):
        copytree(finde_ci(comp, t), os.path.join(root, 'BC', t))
    open(os.path.join(root, 'BC', 'BIN', 'TURBOC.CFG'), 'w', newline='\r\n').write('-IC:\\BC\\INCLUDE\n-LC:\\BC\\LIB\n')
    open(os.path.join(root, 'BC', 'BIN', 'TLINK.CFG'), 'w', newline='\r\n').write('-LC:\\BC\\LIB\n')
    open(os.path.join(root, 'DATUM.COM'), 'wb').write(DATUM_COM)
    src = os.path.join(root, 'SRC'); copytree(os.path.join(projekt, 'Quelle'), src)

    # Uebersetzungsbefehle aus build.bat, gleiche Reihenfolge, Ausgabe je Befehl ins Protokoll
    bat = open(finde_ci(src, 'build.bat'), encoding='cp437').read().replace('\r', '').split('\n')
    cmds = []
    for l in bat:
        s = l.strip()
        if not re.search(r'\\(BCC|TASM)\.EXE', s, re.I) or s.upper().startswith('@REM') or 'SETDATE' in s.upper(): continue
        cmds.append(re.sub(r'\.\.\\BIN\\', r'C:\\BC\\BIN\\', s, flags=re.I) + ' >> UEBERS.LOG')
    say(f'[2/{N}] {len(cmds)} Module uebersetzen (BCC 3.1 / TASM 3.1)')
    open(os.path.join(src, 'UEBERS.BAT'), 'w', newline='\r\n').write('@ECHO OFF\n' + '\n'.join(cmds) + '\n')
    t = Dos(dosbox, root, a.sichtbar).run('SRC', ['CALL UEBERS.BAT'], 1800, 'Uebersetzen')
    objs = [f for f in os.listdir(src) if f.upper().endswith('.OBJ')]
    log = open(os.path.join(src, 'UEBERS.LOG'), 'rb').read().decode('cp437', 'replace') if os.path.exists(os.path.join(src, 'UEBERS.LOG')) else ''
    fehler = [l for l in log.replace('\r', '').split('\n') if re.match(r'(Error \S+ \d+:|Fatal |\*\*Error)', l)]
    say(f'      {len(objs)} OBJ-Dateien, {t:.0f} s')
    for l in fehler[:30]: say('      ' + l)
    if fehler: abbruch(f'Uebersetzungsfehler (siehe oben bzw. {pfad("Build", "bau.log")})')
    if not objs:
        abbruch(f'Keine einzige OBJ-Datei: der Compiler ist in DOSBox nicht gelaufen. Die Meldungen von DOSBox stehen in '
                f'{pfad("Build", "dosbox.log")}; mit --sichtbar sieht man das DOSBox-Fenster.')

    say(f'[3/{N}] in der Entwicklungsumgebung linken (BC.EXE /b BLADEM.PRJ)')
    exe = os.path.join(src, 'BLADEM.EXE')
    if os.path.exists(exe): os.remove(exe)
    Dos(dosbox, root, a.sichtbar).run('SRC', ['C:\\BC\\BIN\\BC.EXE /b BLADEM.PRJ > LINK.LOG'], 600, 'Linken')
    if not os.path.exists(exe):
        say(open(os.path.join(src, 'LINK.LOG'), 'rb').read().decode('cp437', 'replace')[-2000:] if os.path.exists(os.path.join(src, 'LINK.LOG')) else '')
        abbruch(f'BLADEM.EXE wurde nicht erzeugt (Linkfehler, Protokoll: {pfad("SRC", "LINK.LOG")} im Bauordner)')
    ziel = os.path.join(projekt, 'Build', 'BLADEM.EXE')
    shutil.copyfile(exe, ziel)
    for f in ('BLADEM.MAP',):
        if os.path.exists(os.path.join(src, f)): shutil.copyfile(os.path.join(src, f), os.path.join(projekt, 'Build', f))

    say(f'[4/{N}] Vergleich mit dem Original' + (' (Standardbau, Vergleichsfassung fuer den Fix-Modus)' if a.fix else ''))
    m = md5(ziel); say('      gebaut:   ', m)
    gleich = False
    if orig:
        say('      Original: ', md5(orig))
        a_, b_ = open(ziel, 'rb').read(), open(orig, 'rb').read()
        gleich = a_ == b_
        if not gleich:
            d = next((i for i in range(min(len(a_), len(b_))) if a_[i] != b_[i]), min(len(a_), len(b_)))
            say(''); say(f'ABWEICHUNG vom Original: erste Differenz bei Byte {d} (0x{d:x}), Groesse {len(a_)} / {len(b_)}')
    else:
        gleich = m == ORIGINAL_MD5
        if not gleich: say(f'{pfad("Original", "SCHICKM.EXE")} fehlt - nur MD5-Vergleich, und der weicht ab.')
    if gleich: say(''); say('BYTEGLEICH mit dem Original')
    if not a.fix:
        return 0 if gleich else 1
    if not gleich:
        abbruch('Fix-Modus nur nach bytegleichem Standardbau (er ist die Vergleichsfassung fuer fixcheck)')
    flags = ('-DFIX_BUGS=1' if a.fix == 'wichtig' else '-DFIX_BUGS=2') + (' -DFIX_EXPLOITS=0' if a.exploits_lassen else '')
    flags = (flags + ' ' + a.fixflags).strip()
    say('')
    fix_bau(projekt, root, dosbox, a.sichtbar, flags, a.fix.upper())
    start = f'BLADEM_FIX_{a.fix.upper()}_STARTEN.bat' if WINDOWS else f'bash Linux/starten.sh {a.fix}'
    say(''); say(f'Fix-EXE fertig: {pfad("Build", f"BLADEM_FIX_{a.fix.upper()}.EXE")} - starten mit {start}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
