#!/usr/bin/env python3
"""Read results.csv and update the Results section of README.md."""

import csv
import sys
from pathlib import Path

PROJECT_DIR = Path(__file__).resolve().parent.parent
CSV_PATH = PROJECT_DIR / "results.csv"
README_PATH = PROJECT_DIR / "README.md"

START_MARKER = "<!-- results-start -->"
END_MARKER = "<!-- results-end -->"

COLUMNS = ["Instance", "Compact LP", "Compact Time(s)", "ColGen LP", "ColGen Time(s)"]
DISPLAY = ["Instance", "Compact LP", "Time (s)", "ColGen LP", "Time (s)", "Gap (%)"]


def load_csv(path: Path) -> list[dict[str, str]]:
    with open(path) as f:
        return list(csv.DictReader(f))


def build_table(rows: list[dict[str, str]]) -> str:
    lines = []
    lines.append("| " + " | ".join(DISPLAY) + " |")
    lines.append("| " + " | ".join("---:" if h != "Instance" else "---" for h in DISPLAY) + " |")
    for row in rows:
        compact_lp = float(row["Compact LP"])
        colgen_lp = float(row["ColGen LP"])
        gap = (colgen_lp - compact_lp) / colgen_lp * 100
        vals = [row[c] for c in COLUMNS] + [f"{gap:.2f}"]
        lines.append("| " + " | ".join(vals) + " |")
    return "\n".join(lines)


def update_readme(table: str, n_rows: int) -> None:
    text = README_PATH.read_text()
    start = text.find(START_MARKER)
    end = text.find(END_MARKER)
    if start == -1 or end == -1:
        print(f"Error: markers {START_MARKER} / {END_MARKER} not found in README.md",
              file=sys.stderr)
        sys.exit(1)
    before = text[: start + len(START_MARKER)]
    after = text[end:]
    README_PATH.write_text(before + "\n" + table + "\n" + after)
    print(f"Updated {README_PATH} with {n_rows} rows.")


if __name__ == "__main__":
    if not CSV_PATH.exists():
        print(f"Error: {CSV_PATH} not found. Run compare.sh first.", file=sys.stderr)
        sys.exit(1)
    rows = load_csv(CSV_PATH)
    if not rows:
        print("Error: CSV is empty.", file=sys.stderr)
        sys.exit(1)
    table = build_table(rows)
    update_readme(table, len(rows))