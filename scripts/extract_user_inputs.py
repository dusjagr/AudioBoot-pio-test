#!/usr/bin/env python3
import sys
from pathlib import Path

def extract_user_inputs(src_path: Path, dst_path: Path) -> None:
    lines = src_path.read_text(encoding="utf-8", errors="ignore").splitlines()
    out = []
    out.append("# Cascade Chat – User Inputs Only\n")
    out.append(f"Source: {src_path}\n\n")

    i = 0
    n = len(lines)
    while i < n:
        line = lines[i]
        if line.strip().startswith("### User Input"):
            # Write the heading
            out.append(line + "\n")
            i += 1
            # Copy subsequent lines until next section heading (starts with ### ) or EOF
            while i < n:
                nxt = lines[i]
                if nxt.strip().startswith("### ") and not nxt.strip().startswith("### User Input"):
                    break
                out.append(nxt + "\n")
                i += 1
            out.append("\n")
            continue
        i += 1

    dst_path.write_text("".join(out), encoding="utf-8")

if __name__ == "__main__":
    # Defaults for this project structure
    base = Path(__file__).resolve().parents[1]
    src = base / "chat_protocolls" / "Enhancing and Organizing LED Visuals.md"
    dst = base / "chat_protocolls" / "Enhancing and Organizing LED Visuals - User Inputs.md"

    # Optional CLI args: input output
    if len(sys.argv) >= 2:
        src = Path(sys.argv[1])
    if len(sys.argv) >= 3:
        dst = Path(sys.argv[2])

    extract_user_inputs(src, dst)
    print(f"Wrote filtered user inputs to: {dst}")
