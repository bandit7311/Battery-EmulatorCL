# Dekodiert einen DDT-Dump (Zeilen "22xxxx:62 xx xx ...") mit der passenden DDT2000-XML.
# Aufruf: python3 -I decode_evc_dump.py <ecu.xml> <dump.txt> <ausgabe.md>
# Bitzaehlung: BitOffset ab MSB des ersten Datenbytes. Schreibt eine Markdown-Tabelle.
import re,sys,xml.etree.ElementTree as ET
U='/root/.claude/uploads/d375fa2f-c971-5950-88b2-8fe2db5ec214/'
xmlf=sys.argv[1] if len(sys.argv)>1 else U+'36c02ace-EMS3194_0A40_300_V2.1_20200730T160049.xml'
dump=sys.argv[2] if len(sys.argv)>2 else U+'a80d130b-1762262155_EMS3194_0A40_300_V2.1_20200730T160049.txt'
OUT=sys.argv[3] if len(sys.argv)>3 else 'EVC_Dump_dekodiert.md'
t=open(xmlf,encoding='utf8',errors='ignore').read()
t=re.sub(r' xmlns(:\w+)?="[^"]*"','',t); t=re.sub(r'<(/?)(\w+):',r'<\1',t)
t=re.sub(r'<\?xml-stylesheet[^>]*\?>','',t)
root=ET.fromstring(t.encode('utf8'))
data={}
for d in root.iter('Data'):
    n=d.get('Name'); b=d.find('Bits')
    if b is None: continue
    items={int(i.get('Value')):i.get('Text') for i in b.iter('Item') if i.get('Value','').lstrip('-').isdigit()}
    sc=b.find('Scaled')
    data[n]=dict(cnt=int(b.get('count') or 8),signed=b.get('signed')=='1',items=items,
        sc=None if sc is None else (float(sc.get('Step') or 1),float(sc.get('Offset') or 0),sc.get('Unit') or ''),
        var=(d.findtext('Comment') or '').strip())
reqs={}
for r in root.iter('Request'):
    s=r.findtext('Sent/SentBytes')
    if s and s.startswith('22') and len(s)==6:
        reqs[s[2:]]=[(di.get('Name'),int(di.get('FirstByte')),int(di.get('BitOffset') or 0)) for di in r.iter('DataItem')]
ans={}
for l in open(dump):
    m=re.match(r'22([0-9A-F]{4}):62 ([0-9A-F ]+)',l)
    if m: ans[m.group(1)]=bytes.fromhex(m.group(2).replace(' ',''))
neg=[l.strip() for l in open(dump) if ':7F' in l or ':' in l and l.split(':')[1].strip()=='']
rows=[]
for did,b in sorted(ans.items()):
    full=bytes.fromhex(did)+b[2:]  # b = 62? we captured after 62: did bytes + data
    payload=b[2:]
    for (nm,fb,bo) in reqs.get(did,[(None,4,0)]):
        d=data.get(nm)
        if d is None: rows.append((did,nm or '?','',payload.hex(' ').upper(),'(nicht dekodiert)'));continue
        nb=(bo+d['cnt']+7)//8; i0=fb-4
        seg=payload[i0:i0+nb]
        if len(seg)<nb: rows.append((did,nm,d['var'],payload.hex(' ').upper(),'(Antwort zu kurz)'));continue
        v=int.from_bytes(seg,'big'); v=(v>>(nb*8-bo-d['cnt']))&((1<<d['cnt'])-1)
        if d['signed'] and v>=1<<(d['cnt']-1): v-=1<<d['cnt']
        if d['items'] and v in d['items']: out=f"{v} = {d['items'][v]}"
        elif d['sc']: out=f"{v*d['sc'][0]+d['sc'][1]:.10g} {d['sc'][2]}".strip()
        else: out=str(v)
        rows.append((did,nm,d['var'],seg.hex(' ').upper(),out))
out=['# EVC-Dump a80d130b (Auto wach, 04.11.2025 13:15 UTC) dekodiert mit EMS3194_0A40_300_V2.1','','Quelle: 36c02ace-…xml + a80d130b-…txt. Bitzaehlung: BitOffset ab MSB des ersten Datenbytes (an $3202=05 und $343F geprueft).','','| DID | Name | Variable | Rohbytes | Wert |','|---|---|---|---|---|']
for r in rows: out.append('| $%s | %s | %s | %s | %s |'%tuple(x.replace('|','/') for x in r))
open(OUT,'w').write('\n'.join(out)+'\n')
print(len(ans),'DIDs,',len(rows),'Zeilen; nicht dekodiert:',sum('nicht dek' in r[4] or 'kurz' in r[4] for r in rows)); print('Ablehnungen im Dump:',neg[:14])
