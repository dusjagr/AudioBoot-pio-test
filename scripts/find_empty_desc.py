import re
from pathlib import Path

content = Path("/home/dusjagr/Documents/PlatformIO/Projects/251011-201954-attiny85/wav/index.html").read_text(encoding="utf-8")

cards = re.split(r'<div class="card">', content)[1:]
for card in cards:
    if '<div class="desc-box"></div>' in card:
        match = re.search(r'<div class="title">(.*?)</div>', card)
        if match:
            print(f"Missing description for: {match.group(1)}")
