import sys,re
d={}
for l in open(sys.argv[1]):
    m=re.match(r'^22([0-9A-F]{4}):((?:[0-9A-F]{2} ?)+)',l.strip(),re.I)
    if m: d[m.group(1).upper()]=bytes.fromhex(m.group(2).replace(' ',''))[3:]
def u(k): return int.from_bytes(d[k],'big')
cells=[]
for base in (0x9021,0x9041,0x9061,0x9081):
    for i in range(32):
        k='%04X'%(base+i)
        if k in d and len(d[k])==2: cells.append((k,u(k)))
print("cells found",len(cells))
v=[c[1] for c in cells]
print("sum %.3f V  mean %.4f V  min %.4f max %.4f  spread %.2f mV"%(sum(v)/1024,sum(v)/len(v)/1024,min(v)/1024,max(v)/1024,(max(v)-min(v))/1024*1000))
print("9001 raw",u('9001'),"-> SOC %% %.2f"%(u('9001')*98.92/10192))
print("9005 pack V %.1f"%(u('9005')*0.1),"9006 sum %.3f V"%(u('9006')/1024),"9007 %.4f 9009 %.4f"%(u('9007')/1024,u('9009')/1024),"9008 nr",u('9008'),"900A nr",u('900A'))
print("9003 SOH %.2f"%(u('9003')/100),"9012 temp avg %.2f"%((u('9012')/16)),"min",u('9013')/16,"max",u('9014')/16)
b=d['912B']; print("912B len",len(b),b.hex(' '))
data=b[20:32] if len(b)>=32 else b[-12:]
bits=[]
for bi,by in enumerate(data):
    for k in range(8):
        if by&(0x80>>k): bits.append(96-(bi*8+k))
print("912B bits set -> cells(1..96, first bit=96):",sorted(bits),"count",len(bits))
# map to voltage: cell numbers 1..96 -> index
byk=[x[1] for x in cells]
if len(byk)>=96:
    bal=[byk[n-1] for n in bits]; oth=[byk[n-1] for n in range(1,97) if n not in bits]
    print("mean balancing cells %.4f V, others %.4f V"%(sum(bal)/len(bal)/1024,sum(oth)/len(oth)/1024))
    rank=sorted(range(1,97),key=lambda n:-byk[n-1])
    print("top20 highest cells:",sorted(rank[:20]))
    print("overlap with bits:",len(set(rank[:20])&set(bits)))
    print("per balancing cell mV rel. to min:", [(n,round((byk[n-1]-min(byk))/1024*1000,1)) for n in sorted(bits)])
def cnt(k,fac=1024.0):
    r=u(k); return (r^0x80000000)/fac
for k in ('924F','9250','9251','9252','9262','9263'): print(k,d[k].hex(),"%.3f"%cnt(k))
for k in ('9259','925C','925A','9261','91C1','9267','9003','9215','9216','91D0','F4 9A'.replace(' ','')): 
    if k in d: print(k,d[k].hex(' '),u(k))
print("91D1",d['91D1'].hex(' ')); print("91D2",d['91D2'].hex(' '))
for k in ('9271','9272','9273','9274','9270','926E','926F'): print(k,list(d[k]))
