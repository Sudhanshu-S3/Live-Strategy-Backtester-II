import urllib.request, json, time

rows = []
end_time = None
target = 10000

while len(rows) < target:
    url = 'https://api.binance.com/api/v3/klines?symbol=BTCUSDT&interval=1m&limit=1000'
    if end_time:
        url += f'&endTime={end_time}'
    with urllib.request.urlopen(url) as r:
        batch = json.loads(r.read())
    if not batch:
        break
    rows = batch + rows
    end_time = batch[0][0] - 1
    print(f'Fetched {len(rows)} rows...')
    time.sleep(0.5)

with open('tests/data/benchmark_data.csv', 'w') as f:
    f.write('timestamp,open,high,low,close,volume\n')
    for row in rows[-target:]:
        f.write(f'{row[0]},{row[1]},{row[2]},{row[3]},{row[4]},{row[5]}\n')

print('Done! Saved to tests/data/benchmark_data.csv')
