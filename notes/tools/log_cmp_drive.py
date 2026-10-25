import pickle,collections
C=pickle.load(open('cmp/car.pkl','rb'))
def win(frames,t0,t1): return [f for f in frames if t0<=f[0]<=t1]
B1=win(C['teraterm'],500,1460); B0=win(C['teraterm'],50,225)
K1=win(C['fahr'],220,339); K2=win(C['fahr'],1380,1477)
def stat(fr):
    S={}
    for t,tag,i,d in fr:
        s=S.setdefault((i,len(d)),{'n':0,'vals':[collections.Counter() for _ in d],'t':[]})
        s['n']+=1
        for k,b in enumerate(d): s['vals'][k][b]+=1
        s['t'].append(t)
    return S
SB=stat(B1); SK=stat(K1+K2)
nb155=SB[(0x155,8)]['n']; nk155=SK[(0x155,8)]['n']
print("Fenster: Bench %d Frames 0x155 (%.0f s), Auto %d Frames 0x155"%(nb155,nb155*0.01,nk155))
EVC=[0xec,0xed,0x157,0x19f,0x1a1,0x1c7,0x419,0x423,0x426,0x428,0x42f,0x435,0x436,0x4f7,0x500,0x511,0x69f]
print("Verhaeltnis zu 0x155 (Bench / Auto) und DLC")
for i in EVC:
    kb=[k for k in SB if k[0]==i]; kk=[k for k in SK if k[0]==i]
    rb=sum(SB[k]['n'] for k in kb)/nb155; rk=sum(SK[k]['n'] for k in kk)/nk155
    print("0x%03X bench %.4f auto %.4f  dlc bench %s auto %s"%(i,rb,rk,[k[1] for k in kb],[k[1] for k in kk]))
pickle.dump((SB,SK),open('cmp/drive.pkl','wb'))
