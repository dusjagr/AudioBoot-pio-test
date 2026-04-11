import sys
from pathlib import Path

ROOT = Path("/home/dusjagr/Documents/PlatformIO/Projects/251011-201954-attiny85")
sys.path.append(str(ROOT / "scripts"))
from gen_wav_index import extract_descriptions, collect_wavs

descriptions = extract_descriptions()
entries = collect_wavs()

for e in entries:
    if e["id"] in [21, 24]:
        slug_match = e['slug'].replace('-', '').lower()
        print(f"ID: {e['id']} original match: {slug_match}")
        
        if slug_match == "cccrocket": slug_match = "ccc"
        if slug_match == "dnbdancer": slug_match = "dnb"
        if slug_match == "flagsfade": slug_match = "flagsshowfade"
        if slug_match == "larsondual": slug_match = "larsonscannerdual"
        
        print(f"Override match: {slug_match}")
        found = False
        for k, v in descriptions.items():
            if slug_match in k.lower():
                print(f"MATCHED key: {k}")
                found = True
                break
        if not found:
            print("NO MATCH FOUND!")
            for k in descriptions.keys():
                if "flags" in k.lower() or "larson" in k.lower():
                    print(f"Available: {k} (does {slug_match} in {k.lower()} ? {slug_match in k.lower()})")
