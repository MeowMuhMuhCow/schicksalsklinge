import re, os, glob

ROOT = '/root/sternenschweif/agT'
SRC = ROOT + '/src'
TXT = ROOT + '/Spieltexte'

_cache = {}

def load_texts(name):
    """name like 'TEXT.LTX', 'F046.DTX', 'INFO1.TLK'"""
    if name in _cache:
        return _cache[name]
    p = TXT + '/texte/' + name + '.txt'
    if not os.path.exists(p):
        p = TXT + '/dialoge/' + name + '.txt'
    d = {}
    for line in open(p, encoding='utf-8'):
        m = re.match(r'\[(\d+)\] ?(.*)$', line.rstrip('\n'))
        if m:
            d[int(m.group(1))] = m.group(2)
    _cache[name] = d
    return d

def clean(t):
    for c in '±≡≥≤':
        t = t.replace(c, '')
    t = t.replace('^', ' ').replace('\t', ' ')
    t = t.replace('*/', '* /').replace('/*', '/ *')
    t = re.sub(r' +', ' ', t).strip()
    return t

def short(t, n=70):
    t = clean(t)
    if len(t) > n:
        t = t[:n - 3].rstrip() + '...'
    return t

# archive numbers
def archive_names():
    d = {}
    for line in open(TXT + '/dateinummern.tsv'):
        p = line.rstrip('\n').split('\t')
        if p[0].isdigit():
            d[int(p[0])] = p[2]
    return d

CALL_RE = re.compile(r'\b(get_ttx|get_tx2|get_tx)\(')

def find_calls(line):
    """yield (func, argstring, start, end) for each call in line (balanced parens on same line)"""
    res = []
    for m in CALL_RE.finditer(line):
        i = m.end()
        depth = 1
        j = i
        while j < len(line) and depth:
            if line[j] == '(':
                depth += 1
            elif line[j] == ')':
                depth -= 1
            j += 1
        if depth == 0:
            res.append((m.group(1), line[i:j - 1].strip(), m.start(), j))
        else:
            res.append((m.group(1), None, m.start(), len(line)))
    return res

def parse_int(s):
    s = s.strip()
    if re.fullmatch(r'0x[0-9a-fA-F]+', s):
        return int(s, 16)
    if re.fullmatch(r'\d+', s):
        return int(s)
    return None

FUNC_RE = re.compile(r'^[A-Za-z_][\w \*]*?\b([A-Za-z_]\w*)\s*\(')

def blank_comments(text):
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if text.startswith('/*', i):
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out.append(re.sub(r'[^\n]', ' ', text[i:j]))
            i = j
        elif text.startswith('//', i):
            j = text.find('\n', i)
            j = n if j < 0 else j
            out.append(' ' * (j - i))
            i = j
        elif c == '"' or c == "'":
            j = i + 1
            while j < n and text[j] != c:
                if text[j] == '\\':
                    j += 1
                j += 1
            out.append(c + ' ' * (j - i - 1) + c)
            i = j + 1
        else:
            out.append(c)
            i += 1
    return ''.join(out)

def functions(lines):
    """list per line index -> enclosing function name"""
    code = blank_comments('\n'.join(lines)).split('\n')
    # simple preprocessor: drop inactive branches for brace counting
    DEF = {'__BORLANDC__'}
    stack = []  # list of (active_parent, this_branch_active, any_taken)
    active = True
    for i, l in enumerate(code):
        s = l.strip()
        if s.startswith('#'):
            d = re.sub(r'^#\s*', '', s)
            if d.startswith(('ifdef', 'ifndef', 'if ')) or d == 'if':
                if d.startswith('ifdef'):
                    c = d.split()[1] in DEF
                elif d.startswith('ifndef'):
                    c = d.split()[1] not in DEF
                else:
                    e = d[2:].strip()
                    m = re.fullmatch(r'(!)?\s*defined\s*\(?\s*(\w+)\s*\)?', e)
                    if m:
                        c = (m.group(2) in DEF) != bool(m.group(1))
                    else:
                        c = e not in ('0',)
                stack.append((active, c))
                active = active and c
            elif d.startswith('el'):
                if stack:
                    par, c = stack[-1]
                    stack[-1] = (par, not c)
                    active = par and not c
            elif d.startswith('endif'):
                if stack:
                    par, c = stack.pop()
                    active = par
            code[i] = ''
        elif not active:
            code[i] = ''
    names = [None] * len(lines)
    depth = 0
    cur = None
    pending = None
    for i, l in enumerate(code):
        if depth == 0:
            m = FUNC_RE.match(l)
            if m and not l.startswith('#') and '=' not in l.split('(')[0]:
                pending = m.group(1)
        names[i] = cur
        for ch in l:
            if ch == '{':
                if depth == 0:
                    cur = pending
                    names[i] = cur
                depth += 1
            elif ch == '}':
                depth -= 1
                if depth == 0:
                    cur = None
    return names

def read_src(fn):
    return open(os.path.join(SRC, fn), encoding='latin-1').read().split('\n')
