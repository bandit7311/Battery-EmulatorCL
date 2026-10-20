import sys,collections
exec(open('ana1.py').read().split("fn=sys.argv[1]")[0])
U='/root/.claude/uploads/d375fa2f-c971-5950-88b2-8fe2db5ec214/'
for f in ['22aaf176-canmitlog.log','598b197c-canmitlog.log','ab5f2c05-Ladelog_LBC_EVC.log']:
    fr,_=load(U+f); print("==",f)
    cur=None;buf=b'';exp=0
    for t,i,d in fr:
        if i==0x18DAF1DB:
            pci=d[0]>>4
            if pci==0:
                n=d[0]&15; cur=(t,d[1:1+n]); print(" t=%7.1f %s"%(t,d[1:1+n].hex(' ')))
            elif pci==1:
                exp=((d[0]&15)<<8)|d[1]; buf=d[2:]; st=t
            elif pci==2:
                buf+=d[1:]
                if len(buf)>=exp: print(" t=%7.1f MULTI(%d) %s"%(st,exp,buf[:exp].hex(' '))); buf=b''
