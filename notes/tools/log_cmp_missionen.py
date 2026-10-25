import re,sys
def load(f):
    d={}
    for l in open(f,errors='ignore'):
        m=re.match(r'^22([0-9A-F]{4}):((?:[0-9A-F]{2} ?)+)',l.strip(),re.I)
        if m: d[m.group(1).upper()]=bytes.fromhex(m.group(2).replace(' ',''))[3:]
    return d
def lst(d,k,w): b=d[k]; return [int.from_bytes(b[i:i+w],'big') for i in range(0,len(b)-w+1,w)]
def table(f):
    d=load(f); now=int.from_bytes(d['91C1'],'big'); ptr=int.from_bytes(d['9279'],'big')
    s=lst(d,'9275',3);e=lst(d,'9276',3);ks=[(x^0x80000000)/32 for x in lst(d,'9277',4)];ke=[(x^0x80000000)/32 for x in lst(d,'9278',4)]
    us=list(d['9273']);ue=list(d['9274']);en=list(d['926E']);ef=list(d['926F']);md=list(d['9270'])
    newest=(32-ptr)%32
    out=[]
    for r in range(32):
        i=(newest+r)%32
        out.append((r,i,(now-s[i])/60.0,e[i]-s[i],md[i],us[i] if i<len(us) else None,ue[i] if i<len(ue) else None,ks[i],ke[i],en[i],ef[i]))
    return now,ptr,out
if __name__=='__main__':
    now,ptr,out=table(sys.argv[1]); print("91C1",now,"9279",ptr)
    print(" r  slot   vor_h  dauer_min mode usoc  km_start-end  Eout Ein")
    for r,i,ago,dur,m,u,v,k0,k1,eo,ei in out:
        print("%2d %3d %8.1f %6d  %d  %s->%s  %6.0f-%6.0f  %3d %3d"%(r,i,ago,dur,m,u,v,k0,k1,eo,ei))
