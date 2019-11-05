from pprint import pprint

with open('cachegrind.out', 'rb') as f:
    events = None
    while True:
        l = f.readline()
        if l is None:
            break
        if l.startswith(b"events: "):
            events = l.decode('ascii').split()[1:]
            break
    assert events is not None
    f.seek(-200, 2)  # SEEK_END
    while True:
        l = f.readline()
        if l is None:
            break
        if l.startswith(b"summary: "):
            summary = map(int, l.decode('ascii').split()[1:])
            break
    assert summary is not None

r = dict(zip(events, summary))

r['Ir'] -= 2068041
r['I1mr'] -= 1130
r['D1mr'] -= 13017
r['D1mw'] -= 2117
r['ILmr'] -= 1098
r['DLmr'] -= 7828
r['DLmw'] -= 1325
r['Bcm'] -= 14810
r['Bim'] -= 264

r['L1m'] = r['I1mr'] + r['D1mr'] + r['D1mw']
r['LLm'] = r['ILmr'] + r['DLmr'] + r['DLmw']
r['Bm'] = r['Bcm'] + r['Bim']
r['CEst'] = r['Ir'] + 10 * r.get('Bm', 0) + 10 * r['L1m'] + 100 * r['LLm']

print('')
print('instrs:    {: 10,d}'.format(r['Ir']))
print('l1 misses: {: 10,d} (i {:,d} + dr {:,d} + dw {:,d})'.format(r['L1m'], r['I1mr'], r['D1mr'], r['D1mw']))
print('ll misses: {: 10,d} (i {:,d} + dr {:,d} + dw {:,d})'.format(r['LLm'], r['ILmr'], r['DLmr'], r['DLmw']))
print('b misses:  {: 10,d} (cond {:,d} + indirect {:,d})'.format(r['Bm'], r['Bcm'], r['Bim']))
print('cycle estimate: {:,d}'.format(r['CEst']))
