import re
from pathlib import Path

ROOT = Path("/home/dusjagr/Documents/PlatformIO/Projects/251011-201954-attiny85")
content = (ROOT / "include" / "visuals.h").read_text(encoding="utf-8")

# All functions
all_funcs = re.findall(r'inline void (matrix[A-Za-z0-9_]+)\s*\(', content)

# Documented functions
blocks = re.findall(r'/\*\s*(matrix[A-Za-z0-9_]+)\s*\n(.*?)(?:Params:|//|\*/)', content, re.DOTALL | re.MULTILINE)
documented = {m[0] for m in blocks}

print("Missing descriptions for:")
for f in all_funcs:
    if f not in documented:
        print(f" - {f}")
