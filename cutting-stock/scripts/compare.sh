#!/bin/bash
# Compare compact LP vs column generation LP on all instances
# Usage: ./compare.sh [time_limit]

set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD="$PROJECT_DIR/build"
DATA="$PROJECT_DIR/data"
CSV="$PROJECT_DIR/results.csv"
TLIMIT="${1:-60}"

if [ ! -f "$BUILD/compact" ] || [ ! -f "$BUILD/colgen" ]; then
    echo "Build first: mkdir -p build && cd build && cmake .. && make -j"
    exit 1
fi

{
echo "Instance,Compact LP,Compact Time(s),ColGen LP,ColGen Time(s)"

for f in "$DATA"/*; do
    name=$(head -1 "$f")

    # Compact LP
    out_c=$("$BUILD/compact" "$f" "$TLIMIT" 2>/dev/null)
    clp=$(echo "$out_c" | grep "Compact LP:" | awk '{print $3}')
    clp_t=$(echo "$out_c" | grep "Compact LP:" | sed 's/.*(\(.*\)s)/\1/')

    # Column generation LP
    out_g=$("$BUILD/colgen" "$f" 2>/dev/null)
    glp=$(echo "$out_g" | grep "ColGen LP:" | awk '{print $3}')
    gt=$(echo "$out_g" | grep "ColGen LP:" | sed 's/.*(\(.*\)s)/\1/')

    echo "$name,$clp,$clp_t,$glp,$gt"
done
} > "$CSV"

echo "Results written to $CSV"
python3 "$SCRIPT_DIR/report.py"