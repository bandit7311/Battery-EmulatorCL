import sys,collections
exec(open('ana1.py').read().split("fn=sys.argv[1]")[0])
U='/root/.claude/uploads/d375fa2f-c971-5950-88b2-8fe2db5ec214/'
fr,_=load(U+'22aaf176-canmitlog.log')
def tab(poly):
    t=[]
    for i in range(256):
        c=i
        for _ in range(8): c=((c<<1)^poly)&255 if c&128 else (c<<1)&255
        t.append(c)
    return t
TABS={p:tab(p) for p in (0x1D,0x2F,0x07,0x31,0x9B,0x39,0x1C)}
def crc(tb,data,init):
    c=init
    for b in data: c=tb[c^b]
    return c
for ID,tgt in ((0x1C9,1),(0x0C5,6),(0x157,7),(0x157,6),(0x423,7),(0x19F,7),(0x1A1,7),(0x1C7,5),(0x0EC,2),(0x155,7)):
    ss=[d for t,i,d in fr if i==ID and len(d)>tgt][:3000]
    if not ss: print(hex(ID),"none"); continue
    n=len(ss); best=None
    for p,tb in TABS.items():
        for init in range(256):
            # xorout deduced from first frame, verify all
            for order in ('pre',):
                data0=[b for k,b in enumerate(ss[0]) if k!=tgt]
                xo=crc(tb,data0,init)^ss[0][tgt]
                ok=sum(1 for d in ss if crc(tb,[b for k,b in enumerate(d) if k!=tgt],init)^xo==d[tgt])
                if best is None or ok>best[0]: best=(ok,p,init,xo)
    print("%X tgt B%d frames=%d best crc8 match %d/%d poly=%02X init=%02X xorout=%02X"%(ID,tgt,n,best[0],n,best[1],best[2],best[3]))
