import re,sys
t=open('/root/.claude/uploads/d375fa2f-c971-5950-88b2-8fe2db5ec214/22d72d9b-RBMS_MCPU_RL_06xx_V1.7_20220601T125202.xml',errors='replace').read()
want=[w.upper() for w in sys.argv[1:]]
for m in re.finditer(r'<Data Name="([^"]*)">(.*?)</Data>',t,re.S):
    name,body=m.group(1),m.group(2)
    mn=re.search(r'<Mnemonic>DID_\$([0-9A-Fa-f]{4})(\.\d+)?</Mnemonic>',body)
    if not mn or mn.group(1).upper() not in want: continue
    bits=re.search(r'<Bits count="(\d+)"( signed="1")?',body); by=re.search(r'<Bytes count="(\d+)"( ascii="1")?',body)
    sc=re.search(r'<Scaled([^/]*)/?>',body)
    sz=('%sbit%s'%(bits.group(1),' signed' if bits.group(2) else '')) if bits else (('%sbyte%s'%(by.group(1),' ascii' if by.group(2) else '')) if by else '?')
    print("$%s%s | %s | %s |%s"%(mn.group(1),mn.group(2) or '',name,sz,sc.group(1).strip() if sc else ''))
