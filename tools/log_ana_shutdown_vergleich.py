import re,sys,collections
pat=re.compile(r'^\(([\d.]+)\)\s+(RX|TX)(\d)\s+([0-9A-Fa-f]+)\s+\[(\d+)\]\s*((?:[0-9A-Fa-f]{2}\s*)*)')
fn=sys.argv[1]
T0=None;off=0.0;prev=0.0
b0=[];last155=None;gaps=[];ids=collections.Counter();first={};lastid={};inv=[];e426=[];n=0
seen500=[];txn=0
rows155=[]
for line in open(fn,errors='replace'):
    m=pat.match(line)
    if not m: continue
    t=float(m.group(1))
    if t<prev-5: off+=prev
    prev=t; t+=off
    if T0 is None: T0=t
    t-=T0
    i=int(m.group(4),16); d=bytes(int(x,16) for x in m.group(6).split())
    if m.group(2)=='TX': txn+=1
    ids[i]+=1; first.setdefault(i,t); lastid[i]=t
    if i==0x423 and d: 
        if not b0 or b0[-1][1]!=d[0]: b0.append((t,d[0]))
    if i==0x426 and len(d)>=3:
        if not e426 or e426[-1][1]!=d[2]: e426.append((t,d[2]))
    if i==0x155 and len(d)==8:
        if last155 is not None and t-last155>5: gaps.append((last155,t))
        last155=t
        cur=((d[1]&15)<<8)|d[2]; soc=(d[4]<<8)|d[5]
        if cur==0xFFF or soc>45000: inv.append(t)
print("== %s: Dauer %.0f s, TX=%d"%(fn.split('/')[-1],t,txn))
print(" 0x423 B0-Wechsel:",["%.0f:%02X"%(a,b) for a,b in b0[:14]])
print(" 0x426 B2-Wechsel:",["%.0f:%02X"%(a,b) for a,b in e426[:14]])
print(" Pack-Luecken (0x155 >5 s):",["%.0f-%.0f (%.0f s)"%(a,b,b-a) for a,b in gaps])
print(" ungueltige 0x155: %d insgesamt"%len(inv))
for g0,g1 in gaps:
    k=[x for x in inv if g1-1<=x<=g1+10]
    print("   nach Luecke bei %.0f s: ungueltige 0x155 in den ersten 10 s: %d"%(g1,len(k)))
print(" IDs mit Ende vor Logende (letzter Frame < Dauer-5 s):",{hex(i):round(lastid[i]) for i in lastid if lastid[i]<t-5})
