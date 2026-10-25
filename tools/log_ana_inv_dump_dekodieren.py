import re,html
X='/root/.claude/uploads/d375fa2f-c971-5950-88b2-8fe2db5ec214/34f702f7-INV_0220_750_V1.4_20210414T141546.xml'
s=open(X,encoding='utf-8',errors='ignore').read()
# Data definitions
defs={}
for m in re.finditer(r'<Data Name="([^"]+)"[^>]*>(.*?)</Data>',s,re.S):
    n,b=m.group(1),m.group(2)
    d={'comment':(re.search(r'<Comment><!\[CDATA\[(.*?)\]\]>',b,re.S) or [None,''])[1].strip().replace('\n',' ')[:80]}
    bm=re.search(r'<Bits([^>]*)>(.*?)</Bits>',b,re.S)
    by=re.search(r'<Bytes([^>]*)/?>',b)
    if bm:
        a=bm.group(1); d['bits']=int((re.search(r'count="(\d+)"',a) or [0,8])[1]); d['signed']='signed' in a.lower()
        sc=re.search(r'<Scaled([^>]*?)/?>',bm.group(2))
        d['sc']={k:v.strip() for k,v in re.findall(r'(\w+)="([^"]*)"',sc.group(1))} if sc else {}
        d['list']=dict(re.findall(r'<Item Value="(-?\d+)" Text="([^"]*)"',bm.group(2)))
    elif by:
        d['bytes']=int((re.search(r'count="(\d+)"',by.group(1)) or [0,1])[1]); d['ascii']='ascii' in by.group(1)
    defs[n]=d
# Requests
reqs={}
for m in re.finditer(r'<Request Name="([^"]+)">(.*?)</Request>',s,re.S):
    b=m.group(2)
    sb=re.search(r'<SentBytes>([0-9A-F]+)</SentBytes>',b)
    if not sb: continue
    items=[(i[0],int(i[1]),int(i[2]) if i[2] else 0) for i in re.findall(r'<DataItem Name="([^"]+)" Ref="\d+" FirstByte="(\d+)"(?: BitOffset="(\d+)")?',b)]
    reqs[sb.group(1)]=(m.group(1),items)
def val(d,raw_bytes,fb,bo):
    if 'bytes' in d:
        bs=raw_bytes[fb-1:fb-1+d['bytes']]
        return bs.decode('ascii','replace') if d['ascii'] else bs.hex(' ')
    n=d['bits']; nb=(n+bo+7)//8
    bs=raw_bytes[fb-1:fb-1+nb]
    if len(bs)<nb: return None
    v=int.from_bytes(bs,'big')
    v=(v>>(nb*8-bo-n))&((1<<n)-1)
    if d.get('signed') and v>=1<<(n-1): v-=1<<n
    L=d['list']
    if L and str(v) in L: return "%d = %s"%(v,L[str(v)])
    sc=d['sc']
    st=float((sc.get('Step') or '1')); off=float(sc.get('Offset') or 0); dv=float(sc.get('DivideBy') or 1)
    if sc.get('Format')=='bin': return format(v,'0%db'%n)
    r=(v*st+off)/dv
    return ("%g %s"%(round(r,4),sc.get('Unit',''))).strip() if (st!=1 or off or dv!=1 or sc.get('Unit')) else str(v)
if __name__=='__main__':
    out=[]
    for l in open('newdump/inv_25-10_1138.txt'):
        l=l.strip()
        if not l: continue
        k,v=l.split(':',1); raw=bytes.fromhex(v.strip().replace(' ',''))
        if k not in reqs: out.append("%s | (keine Anfrage in XML)"%k); continue
        rn,items=reqs[k]
        for (n,fb,bo) in items:
            d=defs.get(n)
            if not d: out.append("%s | %s | %s | ?"%(k,rn,n)); continue
            out.append("%s | %s | %s | %s | %s"%(k,rn.replace('DataRead.',''),n,val(d,raw,fb,bo),d['comment']))
    open('ana/inv_decoded.txt','w').write('\n'.join(out))
    print(len(out)); 
