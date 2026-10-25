import pickle,sys
bench,car=pickle.load(open('cmp/stats.pkl','rb'))
PACK=[0xc5,0x155,0x1c9,0x424,0x425,0x43a,0x445,0x464,0x588,0x4ae,0x4af,0x659,0x6be]
def st(s,j,b):  # 0 always0, 1 always1, 2 varies
    if s['and'][j]>>b&1: return 1
    if not (s['or'][j]>>b&1): return 0
    return 2
for i in PACK:
    for k in sorted(bench):
        if k[0]!=i: continue
        b=bench[k]; L=k[1]
        rows=[]
        for j in range(L):
            for bit in range(7,-1,-1):
                sb=st(b,j,bit)
                cs={nm:st(car[nm][k],j,bit) for nm in car if k in car[nm]}
                allc=set(cs.values())
                if allc=={sb}: continue
                rows.append("B%d.b%d bench=%s car:%s"%(j,bit,"01v"[sb]," ".join("%s=%s"%(n[:6],"01v"[v]) for n,v in cs.items())))
        print("%s[%d] (bench n=%d)"%(hex(i),L,b['n']))
        for r in rows: print("   ",r)
