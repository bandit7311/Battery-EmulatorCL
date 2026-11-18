# Quelle: geschuetzte DTCs und Balancing, Zoe Ph1 (Datei vom Nutzer, 18.11.2026)

**Herkunft unbekannt, von Claude nicht geprueft** (keine Firmware vorhanden). Nutzer 18.11.: "es soll Ph1 sein". Der Text nennt keinen Pack. Hinweis von Claude: `21 07` ist im Zoe-Ph1-Treiber das Balancing-Register (`RENAULT-ZOE-GEN1-BATTERY.h:57`, 18 Bytes, ein Bit pro Zelle). Unveraenderter Originaltext darunter.

---

Yes, the code confirms it. The chain from the stored DTC to the balancing gate is:

1. When a DTC is stored, 0x9BFDC calls 0x96858. That looks up the DTC's class in the flash table at 0x9CB22 and ORs it into a flag word at RAM 0x400004FE (0x96710: flags |= class). P3090-11 is class 1, so bit 0 gets set.
2. The flag word is saved in the DTC block in EEPROM. It sits just ahead of the DTC slots, and the EEPROM image holds 00 01 followed by 30 90 11 …. RAM dumps right after boot show:

| EEPROM         | 0x400004FE | slot 0  |
|----------------|------------|---------|
| control        | 0000       | D00008… |
| stored P3090   | 0001       | 309011… |
| after 31 05 00 | 0000       | empty   |

3. The severity calculation (0x8D570) begins with:
bl   0x96774          ; returns (flags@0x4FE & 1)
cmpwi r3,1 ; beq set3
...                   ; or status (3,3)==2, (5,1)/(5,4)/(5,7)==1
set3: set_status(2, 1, 3)   ; via 0x8CB88 → 0x898EC → 0x897C0 → stb 0x400001E5
   Watching the setter at runtime: severity (2,1) is written as 3 with the stored P3090 and 0 on the control EEPROM, on every one of the 594 calls in a 6 s run.
4. The balancing gate (0xA7C00) reads status (2,1) via 0x8B810(2,1), which reads 0x400001E5. It allows bleeding only when that value is 0, the cycle timer (0x400019E8) is below 1800 s and the budget (0x400019EC) is non-zero. Otherwise the executor (0xA7C88) clears the bitmap at 0x4000182C, which is why 21 07 returns all zeros.
5. Clearing: the only thing that zeroes the flag word is 0x9674C. Its only caller is the protected-DTC clear (0x96954), reached through 0x976F8 from the 31 05 00 handler. The 14 FF FF FF path (0x9C930) skips class-1/2/4 slots and never resets the flag word. So 14 FF FF FF can't unblock balancing, and 31 05 00 can.

So the "no active faults" rule is wrong: a stored class-1 DTC holds severity at 3 indefinitely, whether or not the fault is still present. This severity path only checks bit 0 (class 1). I didn't trace whether stored class-2 or class-4 DTCs affect balancing elsewhere.

