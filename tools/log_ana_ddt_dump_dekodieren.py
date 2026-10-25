import re,sys
def attrs(a): return dict(re.findall(r'([\w:]+)="([^"]*)"',a))
def load_xml(path):
    s=open(path,encoding='utf-8',errors='ignore').read()
    defs={}
    for m in re.finditer(r'<Data Name="([^"]+)"[^>]*>(.*?)</Data>',s,re.S):
        n,b=m.group(1),m.group(2); d={}
        cm=re.search(r'<Comment><!\[CDATA\[(.*?)\]\]>',b,re.S); d['comment']=(cm.group(1).strip().replace('\n',' ')[:90] if cm else '')
        bm=re.search(r'<Bits([^>]*)>(.*?)</Bits>',b,re.S); by=re.search(r'<Bytes([^>]*)/?>',b)
        if bm:
            a=attrs(bm.group(1)); d['bits']=int(a.get('count',8)); d['signed']='signed' in bm.group(1).lower()
            sc=re.search(r'<Scaled([^>]*?)/?>',bm.group(2)); d['sc']=attrs(sc.group(1)) if sc else {}
            d['list']=dict(re.findall(r'<Item Value="(-?\d+)" Text="([^"]*)"',bm.group(2)))
        elif by:
            a=attrs(by.group(1)); d['bytes']=int(a.get('count',1)); d['ascii']=a.get('ascii')=='1'
        else: d['bits']=8; d['sc']={}; d['list']={}; d['signed']=False
        defs[n]=d
    reqs={}
    for m in re.finditer(r'<Request Name="([^"]+)">(.*?)</Request>',s,re.S):
        b=m.group(2); sb=re.search(r'<SentBytes>([0-9A-F]+)</SentBytes>',b)
        if not sb: continue
        items=[]
        for dm in re.finditer(r'<DataItem ([^>]*?)/?>',b):
            a=attrs(dm.group(1))
            if 'Name' in a and 'FirstByte' in a: items.append((a['Name'],int(a['FirstByte']),int(a.get('BitOffset',0))))
        reqs[sb.group(1)]=(m.group(1),items)
    return defs,reqs
def val(d,raw,fb,bo):
    if 'bytes' in d:
        bs=raw[fb-1:fb-1+d['bytes']]
        return bs.decode('ascii','replace') if d['ascii'] else bs.hex(' ')
    n=d['bits']; nb=(n+bo+7)//8; bs=raw[fb-1:fb-1+nb]
    if len(bs)<nb: return None
    v=int.from_bytes(bs,'big'); v=(v>>(nb*8-bo-n))&((1<<n)-1)
    if d.get('signed') and v>=1<<(n-1): v-=1<<n
    L=d['list']
    if L and str(v) in L: return "%d = %s"%(v,L[str(v)])
    sc=d['sc']
    if sc.get('Format')=='bin': return format(v,'0%db'%n)
    st=float(sc.get('Step') or 1); off=float(sc.get('Offset') or 0); dv=float(sc.get('DivideBy') or 1)
    r=(v*st+off)/dv
    return ("%g %s"%(round(r,4),sc.get('Unit',''))).strip() if (st!=1 or off or dv!=1 or sc.get('Unit')) else str(v)
def main(xml,dump,out):
    defs,reqs=load_xml(xml); lines=[]; miss=[]
    for l in open(dump):
        l=l.strip()
        if not l: continue
        k,v=l.split(':',1); raw=bytes.fromhex(v.strip().replace(' ',''))
        if k not in reqs or not reqs[k][1]: miss.append(k); continue
        rn,items=reqs[k]
        for (n,fb,bo) in items:
            d=defs.get(n)
            lines.append("%s | %s | %s | %s | %s"%(k,rn.replace('DataRead.',''),n,val(d,raw,fb,bo) if d else '?',d['comment'] if d else ''))
    open(out,'w').write('\n'.join(lines)); print(len(lines),"Werte; ohne Anfrage/Items:",miss)
if __name__=='__main__': main(*sys.argv[1:4])
