#!/usr/bin/env bash
set -euo pipefail

INPUT="security_workflow/03_detailed_design.md"
OUTPUT="security_workflow/03_detailed_design.pdf"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXPORTER="$SCRIPT_DIR/export_markdown_pdf.py"
CHROME_CONFIG="$SCRIPT_DIR/puppeteer_mermaid_config.json"

if ! command -v python3 >/dev/null 2>&1; then
  echo "Error: python3 not found. Install python3 first."
  exit 1
fi

python3 "$EXPORTER" "$INPUT" "$OUTPUT" --chrome-config "$CHROME_CONFIG"

echo "Generated: $OUTPUT"
