#!/usr/bin/env python3
"""bauen.py - baut BLADEM.EXE (DSA 1 Die Schicksalsklinge, Attic 1992) aus dem rekonstruierten Quelltext
und vergleicht sie mit dem Original (SCHICKM.EXE, CD-Fassung 3.02 deutsch).

Laeuft unter Windows und Linux, nur Python-Standardbibliothek, Standard-DOSBox 0.74 (die aus dem GOG-Spiel).
Ablauf: Bauordner leeren -> Compiler und Quelle hineinkopieren -> Datum 12.01.1994 setzen (DATUM.COM, das Datum
steht in der EXE) -> alle Module in der Reihenfolge von Quelle\\build.bat uebersetzen (BCC/TASM) -> in der
Entwicklungsumgebung linken (BC.EXE /b BLADEM.PRJ, wie Attic) -> Vergleich -> Build\\BLADEM.EXE.

Wichtig: Die Module werden immer in derselben Reihenfolge und im selben DOSBox-Lauf uebersetzt. BCC 3.1 liest
bei der Registerwahl fuer switch nicht vorbelegten Speicher; das Ergebnis kann von der vorher uebersetzten Datei
abhaengen (so bei Sternenschweif O030 beobachtet).

Aufruf:  python bauen.py [wichtig|alles] [--exploits-lassen] [--fixflags "..."]
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
    abbruch('Projektordner (mit Quelle\\ und Original\\) nicht gefunden. Bauen\\ gehoert in den Projektordner '
            '- oder --projekt ORDNER angeben.')


def finde_compiler(arg, projekt):
    kand = [arg] if arg else []
    kand += [os.path.join(projekt, 'Compiler')]
    # MooMoos Rechner: der Compiler liegt schon im Sternenschweif-Projekt
    kand += [os.path.join(os.path.dirname(projekt), 'Q 004 DSA Sternenschweif', 'Compiler')]
    for k in kand:
        if k and finde_ci(finde_ci(k, 'BIN'), 'BCC.EXE') and finde_ci(finde_ci(k, 'BIN'), 'BC.EXE'): return k
    abbruch('Borland C++ 3.1 nicht gefunden. Bitte nach Compiler\\ kopieren (BIN, INCLUDE, LIB), '
            'zu finden auf archive.org: https://archive.org/details/bcpp31')


def finde_dosbox(arg):
    kand = [arg] if arg else []
    kand += [os.path.join(HIER, 'DOSBox', 'DOSBox.exe')]
    if WINDOWS: kand += [os.path.join(g, 'DOSBOX', 'DOSBox.exe') for g in GOG]
    else: kand += [shutil.which('dosbox') or '']
    for k in kand:
        if k and os.path.isfile(k): return os.path.abspath(k)
    abbruch('DOSBox nicht gefunden (GOG-Ordner Realms of Arkania\\DOSBOX\\DOSBox.exe) - oder --dosbox PFAD angeben.')


def baudir(arg):
    if arg: return os.path.abspath(arg)
    d = os.path.join(tempfile.gettempdir(), 'schicksalsklinge_bau')
    if not d.isascii():          # DOSBox 0.74 vertraegt keine Umlaute im mount-Pfad
        d = (os.environ.get('SystemDrive', 'C:') + '\\schicksalsklinge_bau') if WINDOWS else '/tmp/schicksalsklinge_bau'
    return d


class Dos:
    def __init__(self, exe, root, sichtbar):
        self.exe, self.root, self.dummy = exe, root, not sichtbar
        self.run_dir = os.path.join(root, '_dosbox'); os.makedirs(self.run_dir, exist_ok=True)

    def run(self, wd, befehle, timeout, name):
        """fuehrt DOS-Befehle im Ordner C:\\wd aus; Ausgaben leiten die Befehle selbst in Dateien um"""
        conf = '\n'.join(['[sdl]', 'output=surface', 'fullscreen=false', 'autolock=false',
                          '[dosbox]', 'machine=svga_s3', 'memsize=16',
                          '[cpu]', 'core=normal', 'cputype=auto', 'cycles=max',
                          '[mixer]', 'nosound=true', '[speaker]', 'pcspeaker=false',
                          '[sblaster]', 'sbtype=none', '[gus]', 'gus=false', '[midi]', 'mpu401=none',
                          '[dos]', 'keyboardlayout=none',
                          '[autoexec]', f'mount c "{self.root}"', 'c:', 'C:\\DATUM.COM', f'cd \\{wd}']
                         + befehle + ['exit', ''])
        cp = os.path.join(self.run_dir, 'lauf.conf')
        open(cp, 'w', encoding='mbcs' if WINDOWS else 'utf-8').write(conf)
        env = dict(os.environ)
        if self.dummy: env['SDL_VIDEODRIVER'] = 'dummy'
        env.setdefault('SDL_AUDIODRIVER', 'dummy')
        t0 = time.time()
        try:
            subprocess.run([self.exe, '-conf', cp, '-noconsole', '-exit'], cwd=self.run_dir, env=env,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=timeout)
        except subprocess.TimeoutExpired:
            abbruch(f'DOSBox-Lauf "{name}" nach {timeout} s nicht fertig.')
        return time.time() - t0


def fix_bau(projekt, root, dosbox, sichtbar, flags, name):
    """Fix-Modus (Portierung von Werkzeuge/build_fix.sh): alle Module mit den Schaltern neu uebersetzen, residente
    Module (SEG001..SEG013) auf die Laenge des Standardbaus auffuellen, linken, mit fixcheck pruefen."""
    werkz = os.path.join(projekt, 'Werkzeuge')
    segpad, fixcheck = finde_ci(werkz, 'segpad.py'), finde_ci(werkz, 'fixcheck.py')
    if not segpad or not fixcheck: abbruch('Fix-Modus: Werkzeuge\\segpad.py oder fixcheck.py fehlt')
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
    if fehler: abbruch('Fix-Modus: Uebersetzungsfehler (siehe oben bzw. Build\\bau.log)')
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
    if not exe: abbruch('Fix-Modus: keine EXE erzeugt (Linkfehler, siehe FIX\\LINK.LOG im Bauordner)')
    r = subprocess.run([sys.executable, '-I', fixcheck, exe, finde_ci(fix, 'BLADEM.MAP'),
                        finde_ci(src, 'BLADEM.EXE'), finde_ci(src, 'BLADEM.MAP')], capture_output=True, text=True)
    for l in (r.stdout + r.stderr).strip().split('\n'): say('      ' + l)
    if r.returncode: abbruch('Fix-Modus: fixcheck meldet Fehler - die Fix-EXE wird nicht ausgegeben')
    ziel = os.path.join(projekt, 'Build', f'BLADEM_FIX_{name}.EXE')
    shutil.copyfile(exe, ziel)
    shutil.copyfile(finde_ci(fix, 'BLADEM.MAP'), os.path.join(projekt, 'Build', f'BLADEM_FIX_{name}.MAP'))
    say(f'      Build\\BLADEM_FIX_{name}.EXE  {os.path.getsize(ziel)} Byte  MD5 {md5(ziel)}')
    say('      ALLE BEHEBUNGEN SIND IM SPIEL UNGETESTET - Rueckmeldungen willkommen (Bauen\\FIXES.md)')
    return ziel


def main():
    global LOG
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
    say('Schicksalsklinge - Bau von BLADEM.EXE aus dem Quelltext')
    say('Projekt:  ', projekt)
    comp = finde_compiler(a.compiler, projekt); say('Compiler: ', comp)
    dosbox = finde_dosbox(a.dosbox); say('DOSBox:   ', dosbox)
    orig = finde_ci(os.path.join(projekt, 'Original'), 'SCHICKM.EXE')
    if orig and md5(orig) != ORIGINAL_MD5:
        say('Hinweis: Original\\SCHICKM.EXE ist nicht die CD-Fassung 3.02 (andere MD5). Verglichen wird trotzdem.')
    root = baudir(a.baudir); say('Bauordner:', root)

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
    if fehler: abbruch('Uebersetzungsfehler (siehe oben bzw. Build\\bau.log)')

    say(f'[3/{N}] in der Entwicklungsumgebung linken (BC.EXE /b BLADEM.PRJ)')
    exe = os.path.join(src, 'BLADEM.EXE')
    if os.path.exists(exe): os.remove(exe)
    Dos(dosbox, root, a.sichtbar).run('SRC', ['C:\\BC\\BIN\\BC.EXE /b BLADEM.PRJ > LINK.LOG'], 600, 'Linken')
    if not os.path.exists(exe):
        say(open(os.path.join(src, 'LINK.LOG'), 'rb').read().decode('cp437', 'replace')[-2000:] if os.path.exists(os.path.join(src, 'LINK.LOG')) else '')
        abbruch('BLADEM.EXE wurde nicht erzeugt (Linkfehler)')
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
        if not gleich: say('Original\\SCHICKM.EXE fehlt - nur MD5-Vergleich, und der weicht ab.')
    if gleich: say(''); say('BYTEGLEICH mit dem Original')
    if not a.fix:
        return 0 if gleich else 1
    if not gleich:
        abbruch('Fix-Modus nur nach bytegleichem Standardbau (er ist die Vergleichsfassung fuer fixcheck)')
    flags = ('-DFIX_BUGS=1' if a.fix == 'wichtig' else '-DFIX_BUGS=2') + (' -DFIX_EXPLOITS=0' if a.exploits_lassen else '')
    flags = (flags + ' ' + a.fixflags).strip()
    say('')
    fix_bau(projekt, root, dosbox, a.sichtbar, flags, a.fix.upper())
    say(''); say(f'Fix-EXE fertig: Build\\BLADEM_FIX_{a.fix.upper()}.EXE - starten mit BLADEM_FIX_{a.fix.upper()}_STARTEN.bat')
    return 0


if __name__ == '__main__':
    sys.exit(main())
