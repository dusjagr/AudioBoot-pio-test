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

FILENAME_RE = re.compile(r"^visual-(\d+)-(.+?)(?:-(low|med|high))?\.wav$", re.IGNORECASE)

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
    # Map (id, slug) -> { 'default': filename, 'low': filename, 'med': filename, 'high': filename }
    grouped = {}
    
    for p in sorted(WAV_DIR.glob("visual-*.wav")):
        m = FILENAME_RE.match(p.name)
        if not m:
            continue
        vid = int(m.group(1))
        slug = m.group(2)
        brightness = m.group(3) # None, low, med, or high
        
        # If we have a brightness suffix, slug might capture it if regex isn't greedy enough?
        # Actually with non-greedy .+? and optional group, let's be careful.
        # visual-40-christmas-tree-aura-low.wav
        # group 1: 40
        # group 2: christmas-tree-aura
        # group 3: low
        # But wait, if filename is visual-40-christmas-tree-aura.wav
        # group 2: christmas-tree-aura
        # group 3: None
        
        # To make regex robust for cases where slug contains hyphens:
        # We know brightnesses are fixed set.
        pass

    # Let's retry the regex approach to be safer or just string parsing.
    # filename prefix: "visual-"
    # then ID "-"
    # then slug
    # then optional "-low", "-med", "-high"
    # then ".wav"
    
    entries = []
    # collecting raw first
    files = sorted(list(WAV_DIR.glob("visual-*.wav")))
    
    # We'll use a dictionary to aggregate
    data = {}
    
    for p in files:
        name = p.name
        if not name.startswith("visual-") or not name.endswith(".wav"):
            continue
        
        # Strip extension
        base = name[:-4] # remove .wav
        parts = base.split("-")
        # visual, id, ...
        if len(parts) < 3:
            continue
            
        try:
            vid = int(parts[1])
        except ValueError:
            continue
            
        # Check for brightness suffix
        brightness = "default"
        slug_parts = parts[2:]
        
        if slug_parts[-1] in ["low", "med", "high"]:
            brightness = slug_parts[-1]
            slug_parts = slug_parts[:-1]
            
        slug = "-".join(slug_parts)
        key = (vid, slug)
        
        if key not in data:
            data[key] = {
                "id": vid,
                "slug": slug,
                "title": slug_to_title(slug),
                "variants": {}
            }
        data[key]["variants"][brightness] = name

    # Convert to list
    entries = list(data.values())
    # Sort by numeric id then slug
    entries.sort(key=lambda e: (e["id"], e["slug"]))
    return entries


def generate_html(entries):
    cards = []
    for e in entries:
        title = f"{e['id']} — {html.escape(e['title'])}"
        
        # Determine main audio source (prefer default, then high, then whatever)
        vs = e["variants"]
        main_src = vs.get("default") or vs.get("high") or vs.get("med") or vs.get("low")
        if not main_src: continue
        
        # Gif path (assumed from id/slug)
        gif_filename = f"{e['id']:02d}_{e['slug'].replace('-','_')}.gif" # Older convention?
        # Actually in gen_visual_gifs.py: f"{vid:02d}_{slug}.gif"
        # Wait, gen_visual_gifs uses slug from the script list.
        # But here we derived slug from filename.
        # The slugs in filenames come from gen_visual_wavs.py which uses the same list.
        # So it should match.
        # HOWEVER, the gif filenames use underscores in recent edits?
        # Let's check the directory listing from previous turns.
        # 38_universe-creation.gif (dashes) or underscores?
        # Previous 'ls': 38_universe-creation.gif
        # But wait, python script said: f"{vid:02d}_{slug}.gif"
        # If slug has dashes, they remain dashes.
        # Let's check one: '31_pink-slime-sprouts.gif'.
        # So it is: XX_slug-with-dashes.gif.
        # Ah, the user previously updated index.html with `31_pink-slime-sprouts.gif`.
        # So format is f"{vid:02d}_{slug}.gif"
        
        gif_path = f"gifs/{e['id']:02d}_{e['slug']}.gif"
        
        card_html = f"""    <div class="card">
      <div class="title">{title}</div>
      <a href="{gif_path}" target="_blank">
        <img src="{gif_path}" onerror="this.onerror=null;this.src='../emu_out/{gif_path}';" alt="{html.escape(e['title'])}" loading="lazy" style="width:100%;border-radius:6px"/>
      </a>
      <audio controls preload="none" id="audio-{e['id']}">
        <source src="{main_src}" type="audio/wav" />
      </audio>
      <div class="variant-links">
        Brightness: 
"""
        # Add variant links
        # Order: Low, Med, High (Default)
        for b_label, b_key in [("Low", "low"), ("Med", "med"), ("High", "high")]:
            if b_key in vs:
                fname = vs[b_key]
                # We use a little JS to switch the source or just direct download links?
                # "Option of uploading different compiled wavs" -> implying playing them or downloading.
                # Let's provide links that switch the audio player source AND download links.
                card_html += f'<button class="btn-var" onclick="setAudio(\'audio-{e["id"]}\', \'{fname}\')">{b_label}</button> '
        
        card_html += f"""
      </div>
      <div class="links">
        <a href="{main_src}" id="link-{e['id']}">Download WAV</a>
      </div>
    </div>"""
        cards.append(card_html)

    return """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Visual WAV Library</title>
  <style>
    :root {
      --bg: #0f1115;
      --card: #151922;
      --text: #e6e6e6;
      --muted: #9aa4b2;
      --border: #2a2f3a;
      --link: #7aa2ff;
      --link-hover: #a5bfff;
      --shadow: 0 4px 16px rgba(0,0,0,0.35);
    }
    body { font-family: system-ui, -apple-system, Segoe UI, Roboto, Ubuntu, Cantarell, 'Helvetica Neue', Arial, sans-serif; margin: 2rem; color: var(--text); background: var(--bg); }
    h1 { margin-bottom: 0.25rem; }
    p.lead { margin-top: 0; color: var(--muted); }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 1rem; }
    .card { border: 1px solid var(--border); border-radius: 12px; padding: 1rem; background: var(--card); box-shadow: var(--shadow); }
    .title { font-weight: 600; margin: 0 0 0.5rem; color: var(--text); }
    .links { font-size: 0.9rem; margin-top: 0.5rem; color: var(--muted); }
    .links a { color: var(--link); }
    audio { width: 100%; margin-top: 0.5rem; filter: saturate(0.9) brightness(0.95); }
    .footer { margin-top: 2rem; font-size: 0.9rem; color: var(--muted); }
    a { color: var(--link); text-decoration: none; }
    a:hover { text-decoration: underline; color: var(--link-hover); }
    img { background: #0b0e13; border: 1px solid var(--border); }
    .btn-hip { display:inline-block; margin-right: .5rem; margin-top:.5rem; padding:.5rem .75rem; border-radius:8px; border:1px solid var(--border); background: linear-gradient(135deg,#2a2f3a,#1b1f29); color: var(--text); cursor:pointer; box-shadow: var(--shadow); font-size: 0.9rem;}
    .btn-hip:hover { filter: brightness(1.05); }
    .variant-links { margin-top: 0.5rem; font-size: 0.85rem; color: var(--muted); }
    .btn-var { background: var(--bg); border: 1px solid var(--border); color: var(--text); border-radius: 4px; padding: 2px 8px; cursor: pointer; margin-right: 4px; }
    .btn-var:hover { background: var(--border); }
    .hip-toast { position: fixed; left: 1rem; bottom: 1rem; background: #121620; border:1px solid var(--border); padding:.75rem 1rem; border-radius:10px; box-shadow: var(--shadow); color: var(--text); display:none; align-items:center; gap:.75rem; z-index: 100;}
    .hip-dot { width:10px; height:10px; border-radius:50%; background:#61d673; box-shadow:0 0 12px #61d673; display:inline-block; }
    .btn-stop { padding:.35rem .6rem; border-radius:8px; border:1px solid var(--border); background:#2a2f3a; color:var(--text); cursor:pointer; }
  </style>
  <script>
    function setAudio(id, file) {
      const audio = document.getElementById(id);
      audio.src = file;
      audio.load();
      // update download link too
      const link = document.getElementById(id.replace('audio-', 'link-'));
      if(link) link.href = file;
    }
  </script>
</head>
<body>
  <h1>Visual WAV Library</h1>
  <p class="lead">Inline players for all generated audio bootloader WAV files. Choose brightness level if available.</p>

  <div class="grid">
""" + "\n".join(cards) + """
  </div>

  <div class="footer">
    Tip: To get this page rendered on the web from the repo, enable GitHub Pages for this repository and set the source to the main branch (root). Then open <code>/wav/index.html</code> from your Pages site.
  </div>

  <div id="hipToast" class="hip-toast">
    <span class="hip-dot"></span>
    <span id="hipMsg">Uploading to Hip-Device… Set your volume to max and hold the device steady.</span>
    <button class="btn-stop" id="hipStop">Stop</button>
  </div>

  <script>
    // Inject a hip upload button into every card
    function uploadFromCard(card){
      const audio = card.querySelector('audio');
      if(!audio) return;
      try {
        audio.pause();
        audio.currentTime = 0;
        audio.playbackRate = 1.0;
        audio.volume = 1.0;
        audio.play();
        showHipToast('Uploading to Hip-Device… Set volume to MAX.');
      } catch(e){
        showHipToast('Could not auto-play. Click the play button on the audio control.');
      }
    }
    function showHipToast(msg){
      const t = document.getElementById('hipToast');
      const m = document.getElementById('hipMsg');
      m.textContent = msg;
      t.style.display = 'flex';
      clearTimeout(window.__hipToastTimer);
      window.__hipToastTimer = setTimeout(()=>{ t.style.display='none'; }, 8000);
    }
    function stopAllAudio(){
      document.querySelectorAll('audio').forEach(a=>{ try{ a.pause(); a.currentTime=0; }catch(e){} });
      const t = document.getElementById('hipToast');
      t.style.display='none';
    }
    document.getElementById('hipStop').addEventListener('click', stopAllAudio);
    // Add buttons at load
    document.querySelectorAll('.card').forEach(card => {
      const btn = document.createElement('button');
      btn.className = 'btn-hip';
      btn.innerText = 'Upload';
      btn.addEventListener('click', ()=>uploadFromCard(card));
      const links = card.querySelector('.links');
      if (links) links.prepend(btn); else card.appendChild(btn);
    });
  </script>
</body>
</html>
"""


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
