#!/usr/bin/env python3
"""
Generate wav/index.html and wav/README.md from the current contents of wav/*.wav.

- index.html: responsive grid with HTML5 audio players per file
- README.md: Markdown with a prominent link to index.html and a simple playlist

Usage:
  python3 scripts/gen_wav_index.py
"""
from pathlib import Path
import html
import re

ROOT = Path(__file__).resolve().parents[1]
WAV_DIR = ROOT / "wav"
INDEX_HTML = WAV_DIR / "index.html"
README_MD = WAV_DIR / "README.md"

FILENAME_RE = re.compile(r"^visual-(\d+)-(.+)\.wav$", re.IGNORECASE)

# Optional friendly title overrides per slug
TITLE_OVERRIDES = {
    "dnb-dancer": "DnB Dancer",
    "cote-azur": "Côte d’Azur",
    "five-eight-seam": "Five Eight Seam",
}


def slug_to_title(slug: str) -> str:
    # Use overrides if present
    if slug in TITLE_OVERRIDES:
        return TITLE_OVERRIDES[slug]
    # Default: kebab-case -> Title Case
    return slug.replace("-", " ").title()


def collect_wavs():
    entries = []
    for p in sorted(WAV_DIR.glob("visual-*.wav")):
        m = FILENAME_RE.match(p.name)
        if not m:
            continue
        vid = int(m.group(1))
        slug = m.group(2)
        title = slug_to_title(slug)
        entries.append({
            "id": vid,
            "slug": slug,
            "title": title,
            "filename": p.name,
            "rel": p.name,
        })
    # Sort by numeric id then slug
    entries.sort(key=lambda e: (e["id"], e["slug"]))
    return entries


def generate_html(entries):
    cards = []
    for e in entries:
        title = f"{e['id']} — {html.escape(e['title'])}"
        src = html.escape(e['rel'])
        card = (
            "    <div class=\"card\">"
            f"<div class=\"title\">{title}</div>"
            f"<audio controls preload=\"none\"><source src=\"{src}\" type=\"audio/wav\" /></audio>"
            f"<div class=\"links\"><a href=\"{src}\">Open file</a></div>"
            "</div>"
        )
        cards.append(card)

    return """<!DOCTYPE html>
<html lang=\"en\">
<head>
  <meta charset=\"utf-8\" />
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />
  <title>Visual WAV Library</title>
  <style>
    body { font-family: system-ui, -apple-system, Segoe UI, Roboto, Ubuntu, Cantarell, 'Helvetica Neue', Arial, sans-serif; margin: 2rem; color: #222; }
    h1 { margin-bottom: 0.25rem; }
    p.lead { margin-top: 0; color: #555; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 1rem; }
    .card { border: 1px solid #ddd; border-radius: 10px; padding: 1rem; background: #fff; box-shadow: 0 1px 2px rgba(0,0,0,0.05); }
    .title { font-weight: 600; margin: 0 0 0.5rem; }
    .links { font-size: 0.9rem; margin-top: 0.5rem; }
    audio { width: 100%; margin-top: 0.5rem; }
    .footer { margin-top: 2rem; font-size: 0.9rem; color: #666; }
    a { color: #0366d6; text-decoration: none; }
    a:hover { text-decoration: underline; }
  </style>
</head>
<body>
  <h1>Visual WAV Library</h1>
  <p class=\"lead\">Inline players for all generated audio bootloader WAV files. Open this page via GitHub Pages or locally in a browser to play.</p>

  <div class=\"grid\">\n""" + "\n".join(cards) + "\n  </div>\n\n  <div class=\"footer\">\n    Tip: To get this page rendered on the web from the repo, enable GitHub Pages for this repository and set the source to the main branch (root). Then open <code>/wav/index.html</code> from your Pages site.\n  </div>\n</body>\n</html>\n"


def generate_readme(entries):
    lines = []
    lines.append("# Visual WAV Library\n")
    lines.append("Open the interactive player page (recommended):\n")
    lines.append("\n- [Open web player (index.html)](./index.html)\n\n")
    lines.append("This folder contains audio bootloader WAV files generated for each visual. You can play them directly on GitHub using the controls below, or download them for use with the 8Bit Mixtape / TinyAudioBoot workflow.\n\n")
    lines.append("Notes:\n")
    lines.append("- GitHub’s HTML rendering supports the HTML5 audio player with `controls`, but autoplay is blocked.\n")
    lines.append("- If a browser can’t play inline, use the “Download” link.\n")
    lines.append("- Files are named `visual-<id>-<slug>.wav`.\n")
    lines.append("- Some Markdown renderers (including IDE previews) may not show inline audio controls; in that case use the web player page linked above.\n\n")
    lines.append("## Playlist\n\n")

    for e in entries:
        lines.append(f"### {e['id']} — {e['title']}\n")
        lines.append(f"<a href=\"{e['rel']}\">Download</a>\n\n")
        # Inline audio tag (may not render in all viewers, but works on GitHub web)
        lines.append("<audio controls>\n")
        lines.append(f"  <source src=\"{e['rel']}\" type=\"audio/wav\">\n")
        lines.append("  Your browser does not support the audio element. Use the download link above.\n")
        lines.append("</audio>\n\n")

    return "".join(lines)


def main():
    entries = collect_wavs()
    INDEX_HTML.write_text(generate_html(entries), encoding="utf-8")
    README_MD.write_text(generate_readme(entries), encoding="utf-8")
    print(f"Wrote {INDEX_HTML}")
    print(f"Wrote {README_MD}")


if __name__ == "__main__":
    main()
