import sys,re
sys.argv=[sys.argv[0],sys.argv[1]]
exec(open('ana1.py').read().split("fn=sys.argv[1]")[0])
fn=sys.argv[1]
fr,marks=load(fn)
cur=[0]*96; ts=[0]*96
rows=[]
for t,i,d in fr:
    if i in CELLIDS and len(d)==8:
        idx=CELLIDS.index(i)
        for k,v in enumerate(cells(d,idx)):
            if v: cur[idx*5+k]=v; ts[idx*5+k]=t
            else: pass
        if sum(1 for c in cur if c)>=90:
            ok=[c for c in cur if c]
            rows.append((t,max(ok),min(ok),cur.index(max(ok))+1,cur.index(min(ok))+1,sum(ok)/1000.0*96/len(ok),max(ts)-min(ts)))
print(fn.split('/')[-1],"cell rows:",len(rows))
step=max(1,len(rows)//22)
for r in rows[::step]+rows[-1:]:
    print("  t=%7.1f max=%d z%d min=%d z%d spread=%d sum=%.2f V (age span %.0f s)"%(r[0],r[1],r[3],r[2],r[4],r[1]-r[2],r[5],r[6]))
if rows:
    sp=[r[1]-r[2] for r in rows]; print("  spread min/max/first/last",min(sp),max(sp),sp[0],sp[-1])
# histogram last
print("  zero cells:",[i+1 for i,c in enumerate(cur) if not c]); print("  valid:",sum(1 for c in cur if c))
