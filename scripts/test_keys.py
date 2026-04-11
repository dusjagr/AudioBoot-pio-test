import sys
from pathlib import Path

ROOT = Path("/home/dusjagr/Documents/PlatformIO/Projects/251011-201954-attiny85")
sys.path.append(str(ROOT / "scripts"))
from gen_wav_index import extract_descriptions

descriptions = extract_descriptions()
print("Extracted keys:")
for k in descriptions.keys():
    if "flags" in k.lower() or "larson" in k.lower():
        print(f" -> {k}")
