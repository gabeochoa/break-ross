#!/bin/sh
# Guard: cached reveal % drifts from revealed_cells (past: f1ce7ca % from wrong grid, Territory manual reset).
if grep -rn "reveal_percentage" src/ | grep -v "get_reveal_percentage"; then echo "cached reveal_percentage forbidden - use get_reveal_percentage()"; exit 1; fi
echo "check_no_cached_reveal: ok"
