#!/usr/bin/env python3
"""Kommentiert get_ttx/get_tx/get_tx2-Aufrufe mit dem Spieltext.
Liest immer die Originaldatei aus git (HEAD) -> idempotent.
Aufruf: annotate.py [--write] [--preview] [seg-dateien...]"""
import sys, re, os, subprocess, glob, collections
sys.path.insert(0, '/root/sternenschweif/agT/tools_kommentar')
from lib import *
from rules import RULES, DTX, DNG_EXTRA, feature_file
import manual

WRITE = '--write' in sys.argv
PREVIEW = '--preview' in sys.argv
args = [a for a in sys.argv[1:] if not a.startswith('--')]

ARCH = archive_names()
ENUM = {}
for m in re.finditer(r'(ARCHIVE_FILE_\w+)\s*=\s*(\d+)', open(SRC + '/common.h', encoding='latin-1').read()):
    ENUM[m.group(1)] = int(m.group(2))


def git_orig(fn):
    return subprocess.run(['git', '-C', SRC, 'show', 'HEAD:' + fn], capture_output=True).stdout.decode('latin-1')


def const_file(expr):
    """ARCHIVE_FILE_X [+ n] -> Dateiname oder None"""
    e = expr.strip()
    m = re.fullmatch(r'(ARCHIVE_FILE_\w+)\s*(?:([+-])\s*(\d+))?', e)
    if not m or m.group(1) not in ENUM:
        return None
    n = ENUM[m.group(1)]
    if m.group(2):
        n = n + int(m.group(3)) if m.group(2) == '+' else n - int(m.group(3))
    name = ARCH.get(n, '')
    return name or None


def rule_file(fn, func, acc):
    if acc == 'get_ttx':
        return 'TEXT.LTX', 'global geladen'
    m = re.match(r'DNG(\d\d)_', func or '')
    d = int(m.group(1)) if m else DNG_EXTRA.get(func)
    if d and re.match(r'seg0(7[7-9]|8\d|9[01])', fn):
        if acc == 'get_tx':
            return DTX[d], 'Dungeon %d: prepare_dungeon_area() load_tx(gs_dungeon_id + ARCHIVE_FILE_DNGS_DTX)' % d
        return None, 'kein tx2 in Dungeons'
    m = re.match(r'tevent_(\d{3})', func or '')
    if m and acc == 'get_tx2' and re.match(r'seg1(09|1[0-8])', fn):
        ev = int(m.group(1))
        return feature_file(ev), 'TRV_load_textfile(%d)' % ev
    for (fre, funre, a, tf, why) in RULES:
        if a == acc and re.match(fre, fn) and re.search(funre, func or ''):
            return tf, why
    return None, 'keine Regel'


# ---- berechnete Indizes ----
TTX_PATTERNS = [
    (r'.*\+\s*(235|0xeb)|^(235|0xeb)\s*\+.*', 'TEXT.LTX 236..287: Ortsname'),
    (r'(g_temple_god|hero->god_id)\s*\+\s*21|21\s*\+\s*gs_month|entry->month\s*\+\s*0x15', 'TEXT.LTX 22..35: Gott/Monat'),
    (r'.*(spell_id|first_spell \+ i)\s*\+\s*(106|0x6a)', 'TEXT.LTX 107..: Zaubername'),
    (r'.*(talent_id|first_talent \+ i)\s*\+\s*(48|0x30)|48\s*\+\s*talent_id', 'TEXT.LTX 48..99: Talentname'),
    (r'(48\s*\+\s*g_itemsdat.*item_subtype_id|a\.a\[\d\]\s*\+\s*48)', 'TEXT.LTX 48..56: Waffentalent'),
    (r'\(hero\d?->sex.*\?\s*(593|0x251)\s*:\s*(9|0x9)\)\s*\+\s*hero\d?->typus', 'TEXT.LTX 10..21 / 594..605: Heldentyp (m/w)'),
    (r'349\s*\+\s*gs_day_of_week', 'TEXT.LTX 349..355: Wochentag'),
    (r'357\s*\+\s*gs_special_date_id', 'TEXT.LTX 358..: Feiertag'),
    (r'disease_id\s*\+\s*0x193', 'TEXT.LTX 404..410: Krankheit'),
    (r'551\s*\+\s*get_current_season\(\)', 'TEXT.LTX 551..554: Jahreszeit'),
    (r'i\s*\+\s*192', 'TEXT.LTX 192..203: Zauberklasse'),
    (r'411\s*\+\s*\w+', 'TEXT.LTX 412..: Eigenschaft'),
]


def resolve_index(arg):
    """-> ('one', n) | ('alt', [n...]) | ('range', a, b) | None"""
    a = arg.strip()
    n = parse_int(a)
    if n is not None:
        return ('one', n)
    m = re.fullmatch(r'random_schick\((\d+)\)\s*([+-])\s*(0x[0-9a-fA-F]+|\d+)', a)
    if m:
        k = int(m.group(1)); off = parse_int(m.group(3))
        off = off if m.group(2) == '+' else -off
        return ('range', 1 + off, k + off)
    m = re.fullmatch(r'(0x[0-9a-fA-F]+|\d+)\s*\+\s*random_schick\((\d+)\)', a)
    if m:
        off = parse_int(m.group(1)); k = int(m.group(2))
        return ('range', 1 + off, k + off)
    m = re.fullmatch(r'random_interval\((\d+),\s*(\d+)\)', a)
    if m:
        return ('range', int(m.group(1)), int(m.group(2)))
    m = re.fullmatch(r'.*\?\s*(0x[0-9a-fA-F]+|\d+)\s*:\s*(0x[0-9a-fA-F]+|\d+)', a)
    if m and '(' not in a.split('?')[-1]:
        return ('alt', [parse_int(m.group(1)), parse_int(m.group(2))])
    return None


stats = collections.Counter()
openlist = []
tsv_rows = set()
problems = []


def norm(s):
    s = s.upper()
    for a, b in (('Ä', 'AE'), ('Ö', 'OE'), ('Ü', 'UE'), ('ß', 'SS')):
        s = s.replace(a, b)
    return re.sub(r'[^A-Z0-9]', '', s)


def process(fn):
    text = git_orig(fn)
    lines = text.split('\n')
    blank = blank_comments(text).split('\n')
    F = functions(lines)
    # dynamic state per function: overrides after load_* calls
    dyn = {}  # access -> (file or 'UNKNOWN') ; reset at function change
    curf = None
    out = list(lines)
    for i, (l, b) in enumerate(zip(lines, blank)):
        if F[i] != curf:
            curf = F[i]
            dyn = {}
        items = []
        for (acc, arg, s, e) in find_calls(b):
            stats[acc + '_total'] += 1
            key = (fn, i + 1)
            if arg is None and key in manual.LINE:
                items.append(('raw', manual.LINE[key]))
                stats[acc + '_done'] += 1
                continue
            if arg is None:
                stats[acc + '_open'] += 1
                openlist.append((fn, i + 1, acc, '(Aufruf ueber mehrere Zeilen)', l.strip()[:80]))
                continue
            if key in manual.SKIP:
                stats[acc + '_open'] += 1
                openlist.append((fn, i + 1, acc + '(' + arg + ')', 'bewusst offen: ' + manual.SKIP[key], ''))
                continue
            if (fn, i + 1, arg) in manual.LINE or key in manual.LINE:
                c = manual.LINE.get((fn, i + 1, arg), manual.LINE.get(key))
                items.append(('raw', c))
                stats[acc + '_done'] += 1
                continue
            tf, why = rule_file(fn, F[i], acc)
            if acc in dyn:
                tf, why = dyn[acc]
            if tf in (None, 'UNKNOWN'):
                stats[acc + '_open'] += 1
                openlist.append((fn, i + 1, acc + '(' + arg + ')', 'Datei unklar: ' + why, F[i] or ''))
                continue
            tsv_rows.add((fn, F[i] if acc != 'get_ttx' else '*', acc, tf, why))
            r = resolve_index(arg)
            if r is None and acc == 'get_ttx':
                for pat, desc in TTX_PATTERNS:
                    if re.fullmatch(pat, arg.strip()):
                        r = ('desc', desc)
                        break
            if r is None:
                stats[acc + '_open'] += 1
                openlist.append((fn, i + 1, acc + '(' + arg + ')', 'berechneter Index', tf))
                continue
            T = load_texts(tf)
            if r[0] == 'one':
                if r[1] not in T:
                    problems.append((fn, i + 1, acc, arg, tf, 'Index fehlt in Datei'))
                    stats[acc + '_open'] += 1
                    continue
                if not clean(T[r[1]]):
                    stats[acc + '_open'] += 1
                    openlist.append((fn, i + 1, acc + '(' + arg + ')', 'leerer Text', tf))
                    continue
                items.append(('one', r[1], T[r[1]], tf, acc))
            elif r[0] == 'alt':
                if not all(x in T for x in r[1]):
                    problems.append((fn, i + 1, acc, arg, tf, 'Index fehlt'))
                    stats[acc + '_open'] += 1
                    continue
                for x in r[1]:
                    items.append(('one', x, T[x], tf, acc))
            elif r[0] == 'range':
                lo, hi = r[1], r[2]
                if not (lo in T and hi in T):
                    problems.append((fn, i + 1, acc, arg, tf, 'Bereich ausserhalb'))
                    stats[acc + '_open'] += 1
                    continue
                items.append(('range', lo, hi, T[lo], tf, acc))
            elif r[0] == 'desc':
                items.append(('raw', r[1]))
            stats[acc + '_done'] += 1
        # dynamic loads on this line (affect following lines)
        for m in re.finditer(r'\b(load_tx2|load_tx|load_tlk|load_informer_tlk|TRV_load_textfile)\s*\(([^;]*)\)\s*;', b):
            f, a = m.group(1), m.group(2)
            acc = 'get_tx2' if f in ('load_tx2', 'load_informer_tlk') else 'get_tx'
            if f == 'TRV_load_textfile':
                continue
            if 'load_tx2(' in a or a.count('?'):
                continue  # dispatcher with ?: handled by rules
            cf = const_file(a)
            if cf:
                dyn[acc] = (cf, '%s(%s) in derselben Funktion' % (f, a.strip()))
            else:
                if acc in dyn or True:
                    dyn[acc] = (None, 'nach %s(%s) Datei unbestimmt' % (f, a.strip()))
        if re.search(r'\bTRV_fight_event\s*\(|TRV_load_textfile\s*\(', b):
            dyn.pop('get_tx', None)
        elif re.search(r'\bdo_fight\s*\(', b) and re.match(r'seg1(09|1[0-8])', fn):
            dyn['get_tx'] = (None, 'nach do_fight() ohne TRV_load_textfile')
        if not items:
            continue
        # build comment
        uniq = []
        for it in items:
            if it not in uniq:
                uniq.append(it)
        n = len(uniq)
        lim = 70 if n == 1 else (45 if n == 2 else 32)
        parts = []
        for it in uniq:
            if it[0] == 'raw':
                parts.append(it[1])
            elif it[0] == 'one':
                parts.append('"%s"' % short(it[2], lim))
            elif it[0] == 'range':
                parts.append('%d..%d zufaellig, z.B. "%s"' % (it[1], it[2], short(it[3], max(lim - 25, 25))))
        cmt = '/* ' + ' / '.join(parts) + ' */'
        cmt = cmt.replace('zufaellig', 'zufällig')
        # existing comment with same content?
        code_end = len(b.rstrip())
        rest = l[code_end:]
        if rest.strip():
            nr = norm(rest)
            dup = all(it[0] != 'one' or norm(clean(it[2]))[:14] in nr for it in uniq) and any(it[0] == 'one' for it in uniq)
            if dup:
                stats['dup_skipped'] += 1
                if PREVIEW: print('DUP %s:%d %s' % (fn, i + 1, l.strip()[:200]))
                continue
        cut = code_end
        if b[:code_end].endswith('\\'):
            cut = len(b[:code_end - 1].rstrip())
            out[i] = l[:cut] + ' ' + cmt + l[cut:]
        else:
            out[i] = l[:cut] + ' ' + cmt + l[cut:]
        if PREVIEW:
            print('%s:%d [%s] %s' % (fn, i + 1, F[i], out[i].strip()[:230]))
    new = '\n'.join(out)
    try:
        new.encode('latin-1')
    except UnicodeEncodeError as ex:
        problems.append((fn, 0, '', '', '', 'latin-1: %s' % ex))
        return
    if WRITE and new != text:
        with open(os.path.join(SRC, fn), 'w', encoding='latin-1', newline='\n') as f:
            f.write(new)


files = args or sorted(os.path.basename(p) for p in glob.glob(SRC + '/seg*.c'))
for fn in files:
    process(fn)

print('STATS', dict(stats))
for p in problems:
    print('PROBLEM', p)
with open('/root/sternenschweif/agT/tools_kommentar/open.tsv', 'w') as f:
    for o in openlist:
        f.write('\t'.join(str(x) for x in o) + '\n')
with open('/root/sternenschweif/agT/tools_kommentar/map_raw.tsv', 'w') as f:
    for r in sorted(tsv_rows):
        f.write('\t'.join(str(x) for x in r) + '\n')
