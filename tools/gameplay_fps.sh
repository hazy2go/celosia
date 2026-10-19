#!/bin/bash
# Scripted New Game -> skip intro -> from the altar right and back through the
# hall out to the field (timed in guest VBlanks), then prints real game frames
# per second (iosGPFrameInit calls, sampled every 10) per 5 s window.
# Usage: tools/gameplay_fps.sh [seconds]   (PS2X_* switches pass through)
cd "$(dirname "$0")/.."
IN="2100v:START:18v;2700v:START:18v"
for t in $(seq 56 2 90); do IN="$IN;$((t * 60))v:CROSS:18v"; done
IN="$IN;6600v:START:18v;9000v:CROSS:18v;9600v:CROSS:18v;10500v:LS_RIGHT:300v;10800v:LS_DOWN:9000v"
SOTC_TRACE_CALLS=0x1c3628 SOTC_TRACE_EVERY=10 PS2X_STATS=1 PS2X_INPUT_SCRIPT="$IN" \
    ./celosia autotest "${1:-150}" 30 >/dev/null 2>&1
awk '/^\[stats\] t=/ { split($2, a, "="); if (seen) print a[2] "s", (cnt - last) / 5.0, "fps"; last = cnt; seen = 1 }
     /sotc:call\] 0x1c3628 count=/ { match($0, /count=[0-9]+/); cnt = substr($0, RSTART + 6, RLENGTH - 6) + 0 }' logs/autotest.log | tail -8
