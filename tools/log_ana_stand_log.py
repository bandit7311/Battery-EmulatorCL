import re,sys
pat=re.compile(r'^\(([\d.]+)\)\s+(RX|TX)(\d)\s+([0-9A-Fa-f]+)\s+\[(\d+)\]\s*((?:[0-9A-Fa-f]{2}\s*)*)')
want={0x423:[],0x426:[],0x436:[],0x500:[],0x69F:[],0x155:[],0x424:[]}
uds=[];T0=None
for line in open(sys.argv[1],errors='replace'):
    m=pat.match(line)
    if not m: continue
    t=float(m.group(1)); i=int(m.group(4),16); d=bytes(int(x,16) for x in m.group(6).split())
    if T0 is None: T0=t
    if i in want: want[i].append((t-T0,d))
    if i in (0x18DADBF1,0x18DAF1DB): uds.append((t-T0,i,d.hex(' ')))
def changes(i,mask=None):
    last=None;out=[]
    for t,d in want[i]:
        k=bytes(0 if (mask and j in mask) else b for j,b in enumerate(d))
        if k!=last: out.append((t,d.hex(' '))); last=k
    return out
for i,mask in ((0x423,{7}),(0x426,None),(0x436,{1,2,3}),(0x69F,{6,7}),(0x424,{6,7}),(0x500,None)):
    c=changes(i,mask)
    print("ID %X: %d Aenderungen" % (i,len(c)))
    for t,d in c[:5]+(c[-3:] if len(c)>8 else []): print("   t=%7.1f  %s" % (t,d))
print("0x500 Frames: erster t=%.1f letzter t=%.1f" % (want[0x500][0][0],want[0x500][-1][0]))
print("UDS:")
for t,i,h in uds: print("  t=%.1f %X %s" % (t,i,h))
s=[(t,((d[1]&15)<<8|d[2])*0.25-500,((d[4]<<8)|d[5])*0.0025,d[0]*0.3) for t,d in want[0x155] if len(d)==8]
print("0x155: SOC %.2f -> %.2f %%, Strom min %.2f max %.2f A, B0 %.1f-%.1f kW" % (s[0][2],s[-1][2],min(x[1] for x in s),max(x[1] for x in s),min(x[3] for x in s),max(x[3] for x in s)))
