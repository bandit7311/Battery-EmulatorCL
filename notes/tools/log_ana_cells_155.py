import re, sys, collections, statistics
CELLIDS=[0x5F7,0x5F4,0x5F2,0x5F1,0x5F0,0x5ED,0x5EA,0x5D9,0x5EC,0x5D6,0x5CC,0x5CB,0x5C9,0x5B7,0x5B5,0x5B4,0x5AC,0x5AD,0x5A1,0x5DD]
pat=re.compile(r'^\(([\d.]+)\)\s+RX(\d)\s+([0-9A-Fa-f]+)\s+\[(\d+)\]\s*((?:[0-9A-Fa-f]{2}\s*)*)')
def load(fn):
    out=[];off=0;prev=0.0;marks=[]
    for line in open(fn,errors='replace'):
        m=pat.match(line)
        if not m:
            if 'not printed' in line: marks.append((prev+off,line.strip()))
            continue
        t=float(m.group(1))
        if t<prev-5: off+=prev
        prev=t
        d=bytes(int(x,16) for x in m.group(5).split())
        out.append((t+off,int(m.group(3),16),d))
    return out,marks
def cells(d,idx):
    raw=[(d[0]<<4)|(d[1]>>4),((d[1]&15)<<8)|d[2],(d[3]<<4)|(d[4]>>4),((d[4]&15)<<8)|d[5],(d[6]<<4)|(d[7]>>4)]
    n=1 if idx==19 else 5
    res=[]
    for k in range(n):
        mv=raw[k]+2000
        res.append(mv if 2500<=mv<=4500 else 0)
    return res
fn=sys.argv[1]
fr,marks=load(fn)
print("FILE",fn,"frames",len(fr),"dur %.0f s"%(fr[-1][0]-fr[0][0]),"marks",len(marks))
cnt=collections.Counter(i for _,i,_ in fr)
print("IDs:",' '.join('%X:%d'%(i,c) for i,c in sorted(cnt.items())))
# cell rounds
cur=[0]*96; seen=set(); rounds=[]
for t,i,d in fr:
    if i in CELLIDS and len(d)==8:
        idx=CELLIDS.index(i)
        c=cells(d,idx)
        for k,v in enumerate(c): cur[idx*5+k]=v
        seen.add(idx)
        if len(seen)==20:
            ok=[v for v in cur if v]
            if len(ok)==96:
                rounds.append((t,max(ok),min(ok),sum(ok)/96.0,cur.index(max(ok))+1,cur.index(min(ok))+1,sum(ok)))
            seen=set()
print("cell rounds (all 96 valid):",len(rounds))
for r in rounds[::max(1,len(rounds)//25)]:
    print("  t=%7.1f max=%d (z%d) min=%d (z%d) spread=%d mean=%.1f sum=%.3f V"%(r[0],r[1],r[4],r[2],r[5],r[1]-r[2],r[3],r[6]/1000))
if rounds:
    sp=[r[1]-r[2] for r in rounds]
    print("  spread min/max/first/last:",min(sp),max(sp),sp[0],sp[-1])
# 0x155 series
s=[]
for t,i,d in fr:
    if i==0x155 and len(d)==8:
        cur_raw=((d[1]&15)<<8)|d[2]; soc_raw=(d[4]<<8)|d[5]; v=(((d[6]&0x7F)<<8)|d[7])*0.5
        if cur_raw==0xFFF or soc_raw>40000: continue
        s.append((t,cur_raw*0.25-500,soc_raw*0.0025,v,d[0]*300,d[3],d[6]>>7))
print("0x155 valid:",len(s))
if s:
    step=max(1,len(s)//20)
    for x in s[::step]:
        print("  t=%7.1f I=%7.2f A SOC=%6.2f%% U=%6.1f V Pmax=%d W B3=%02X drv=%d"%x)
    print("  Imin/Imax %.2f %.2f ; SOC min/max %.2f %.2f"%(min(a[1] for a in s),max(a[1] for a in s),min(a[2] for a in s),max(a[2] for a in s)))
# 0x424/0x425
for ID in (0x424,0x425):
    ss=[(t,d) for t,i,d in fr if i==ID and len(d)==8]
    print("0x%X frames %d"%(ID,len(ss)))
    last=None;n=0
    for t,d in ss:
        key=d if ID==0x425 else d[:6]+d[7:]
        if key!=last:
            if n<12 or n%40==0:
                if ID==0x425:
                    mx=((((d[4]&3)<<7)|(d[5]>>1))*10)+1000; mn=((((d[6]&1)<<8)|d[7])*10)+1000
                    print("   t=%7.1f %s max=%d min=%d"%(t,d.hex(' '),mx,mn))
                else:
                    print("   t=%7.1f %s Tmin=%d Tmax=%d SOH=%d reg=%dW dis=%dW"%(t,d.hex(' '),d[4]-40,d[7]-40,d[5],d[2]*500,d[3]*500))
            n+=1;last=key
    print("   changes:",n)
# UDS
print("UDS frames (18DADBF1 request / 18DAF1DB answer):")
for t,i,d in fr:
    if i in (0x18DADBF1,0x18DAF1DB,0x18DADCF1,0x18DAF1DC):
        print("   t=%7.1f %X %s"%(t,i,d.hex(' ')))
for m in marks[:5]: print("MARK",m)
