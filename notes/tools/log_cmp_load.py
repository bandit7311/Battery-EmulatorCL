import re,zipfile,glob,os,collections,pickle
U='/root/.claude/uploads/d375fa2f-c971-5950-88b2-8fe2db5ec214/'
rx=re.compile(r'^\(([\d.]+)\) (\S+) ([0-9A-Fa-f]+) \[(\d+)\] ?(.*)$')
def parse(lines,off=0.0):
    out=[]
    for l in lines:
        m=rx.match(l.strip())
        if not m: continue
        n=int(m.group(4)); toks=m.group(5).split()[:n]
        try: d=bytes(int(x,16) for x in toks)
        except ValueError: bad=globals().setdefault('BAD',[]); bad.append(l.strip()[:100]); continue
        out.append((float(m.group(1))+off,m.group(2).lower(),int(m.group(3),16),d))
    return out
def zipfiles(zf):
    z=zipfile.ZipFile(zf)
    res=[]
    for n in sorted(z.namelist()):
        if n.endswith('/') or not n.lower().endswith(('.txt','.log')): continue
        res.append((n,z.read(n).decode('latin1').splitlines()))
    return res
def load_zip_concat(zf):
    out=[]
    for n,ls in zipfiles(zf): out+=parse(ls)
    return out
def load_file(f): return parse(open(f,errors='ignore'))
