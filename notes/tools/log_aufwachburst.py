#!/usr/bin/env python3
"""Aufwachburst aus einem OVMS-CAN-Mitschnitt (crtd) auswerten.
Aufruf: python3 -I tools/log_aufwachburst.py CANLOG_Aufwachburst_01-11_2134CET.crtd.txt [Burstlaenge_s]
Zeigt: Frames je Sekunde, den Burst (erste Sekunden, in Reihenfolge), alle 0x350,
erstes/letztes Auftreten jeder ID, Periode je ID (Median).
"""
import re, sys, collections, statistics

path = sys.argv[1]
burst_s = float(sys.argv[2]) if len(sys.argv) > 2 else 0.25
rows = []
for line in open(path):
    m = re.match(r'(\d+\.\d+) 1R11 ([0-9A-Fa-f]+)((?: [0-9A-Fa-f]{2})*)\s*$', line.rstrip())
    if m:
        rows.append((float(m.group(1)), m.group(2).upper(), m.group(3).strip()))
t0 = rows[0][0]
print(f"Frames: {len(rows)}  Dauer: {rows[-1][0]-t0:.3f} s  IDs: {len({r[1] for r in rows})}")

print("\nFrames je Sekunde (ab erstem Frame):")
per = collections.Counter(int(r[0]-t0) for r in rows)
for k in sorted(per):
    print(f"  s{k:02d}: {per[k]}")

print(f"\nBurst: erste {burst_s} s in Reihenfolge")
for t, i, d in rows:
    if t - t0 <= burst_s:
        print(f"  +{t-t0:6.3f}  {i:>3}  {d}")

print("\nAlle 0x350:")
for t, i, d in rows:
    if i == '350':
        print(f"  +{t-t0:6.3f}  {d}")

print("\nID: Anzahl, erstes, letztes Auftreten, Periode (Median, ms)")
by = collections.defaultdict(list)
for t, i, d in rows:
    by[i].append(t - t0)
for i in sorted(by, key=lambda x: by[x][0]):
    ts = by[i]
    per_ms = round(statistics.median([(b-a)*1000 for a, b in zip(ts, ts[1:])]), 0) if len(ts) > 2 else ''
    print(f"  {i:>3}: {len(ts):3d}  {ts[0]:7.3f}  {ts[-1]:7.3f}  {per_ms}")
