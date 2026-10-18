#!/bin/bash
# A real Vita as a dev console: vitacompanion (FTP on 1337, commands on 1338)
# over WiFi, udcd_uvc (the screen as a USB camera) for pictures.
#
# Usage: VITA_IP=192.168.x.x tools/vita_dev.sh <command>
#   install           first time: the whole VPK's files into ux0:app/CELO00001
#   push              the new eboot.bin only (after ./celosia vita)
#   env [file]        ux0:data/celosia/env.txt from file (no file: delete it)
#   run [seconds]     kill, launch, wait, pull the logs to logs/vita/
#   bench [seconds]   run with the scripted New Game -> walk route (gameplay_fps.sh)
#   logs              pull log.txt / err.txt to logs/vita/
#   stats             the [stats] / [load] lines of the last pulled err.txt
#   shot [name]       one frame of the USB camera to shots/vita/<name>.png
#   kill | launch     the app
#   press <cmds>      vitacompanion input: "press cross; wait 100ms; release cross"
cd "$(dirname "$0")/.."
IP=${VITA_IP:?set VITA_IP to the Vita IP address}
TITLE=CELO00001
FTP="ftp://$IP:1337"
OUT=logs/vita
mkdir -p "$OUT"

cmd() { printf '%s\n' "$1" | nc -w 2 "$IP" 1338; }
put() { curl -s --ftp-create-dirs -T "$1" "$FTP/$2" || { echo "upload failed: $2" >&2; exit 1; }; }
pull() {
    curl -s "$FTP/ux0:/data/celosia/log.txt" -o "$OUT/log.txt"
    curl -s "$FTP/ux0:/data/celosia/err.txt" -o "$OUT/err.txt"
    echo "logs: $OUT/err.txt ($(wc -l <"$OUT/err.txt" | tr -d ' ') lines)"
}
stats() { grep -E '^\[(stats|load|vita)\]' "$OUT/err.txt" | grep -v '\[vita\] env' | tail -${1:-40}; }
# The lock screen pauses apps: peel it from the top right corner (a no-op
# on the home screen).
unlock() {
    cmd "press front-touch 0 1860 80; wait 50ms; press front-touch 0 1700 250; wait 50ms; press front-touch 0 1500 450; wait 50ms; press front-touch 0 1250 650; wait 50ms; press front-touch 0 1000 850; wait 50ms; press front-touch 0 800 1000; wait 100ms; release front-touch 0" >/dev/null
    sleep 1
}
run() {
    # The display is kept off between runs (no burn-in; nosleep keeps it awake).
    cmd "screen on" >/dev/null
    unlock
    cmd "quit $TITLE" >/dev/null
    sleep 1
    cmd "launch $TITLE" >/dev/null
    echo "running ${1:-90}s..."
    # A tap every 10 s (front touch: the game reads only the rear pad) so the
    # Vita never idles into standby mid-run.
    local left=${1:-90}
    while [ "$left" -gt 0 ]; do
        local step=$((left < 10 ? left : 10))
        sleep "$step"
        left=$((left - step))
        cmd "press front-touch 1 60 1040; wait 50ms; release front-touch 1" >/dev/null
    done
    cmd "quit $TITLE" >/dev/null
    cmd "screen off" >/dev/null
    sleep 1
    pull
}

VPK=build/vita/ps2xRuntime/ps2EntryRunner.vpk
case "$1" in
install)
    [ -f "$VPK" ] || { echo "no $VPK (./celosia vita)" >&2; exit 1; }
    tmp=$(mktemp -d)
    unzip -q "$VPK" -d "$tmp"
    (cd "$tmp" && find . -type f | sed 's|^\./||') | while read -r f; do put "$tmp/$f" "ux0:/data/celosia-pkg/$f"; done
    rm -rf "$tmp"
    cmd "promote ux0:data/celosia-pkg"
    echo "promoted to ux0:app/$TITLE"
    ;;
push)
    tmp=$(mktemp -d)
    unzip -q -o "$VPK" eboot.bin -d "$tmp" || exit 1
    cmd "quit $TITLE" >/dev/null
    put "$tmp/eboot.bin" "ux0:/app/$TITLE/eboot.bin"
    rm -rf "$tmp"
    echo "eboot.bin pushed"
    ;;
env)
    if [ -n "$2" ]; then put "$2" "ux0:/data/celosia/env.txt" && echo "env.txt set from $2"
    else curl -s "$FTP/" -Q "DELE ux0:/data/celosia/env.txt" >/dev/null; echo "env.txt deleted"; fi
    ;;
run) run "${2:-90}"; stats ;;
bench)
    # New Game, skip the intro, walk (tools/gameplay_fps.sh), on the guest clock.
    # Timed in guest VBlanks (60 a second), so slow loads on the Vita keep step.
    IN="2100v:START:18v;2700v:START:18v"
    for t in $(seq 56 2 90); do IN="$IN;$((t * 60))v:CROSS:18v"; done
    IN="$IN;6600v:START:18v;7500v:LS_UP:1200v"
    envfile=$OUT/bench-env.txt
    { echo "PS2X_INPUT_SCRIPT=$IN"; [ -f "$OUT/extra-env.txt" ] && cat "$OUT/extra-env.txt"; } >"$envfile"
    put "$envfile" "ux0:/data/celosia/env.txt"
    run "${2:-160}"
    stats
    ;;
logs) pull ;;
stats) stats "${2:-40}" ;;
shot)
    # udcd_uvc's modes: 960x544 at exactly 30.000030 or 60.000240 fps (others fail).
    mkdir -p shots/vita
    ffmpeg -loglevel error -y -f avfoundation -framerate 30.000030 -video_size 960x544 -i PSVita -frames:v 1 -update 1 "shots/vita/${2:-shot}.png" &&
        echo "shots/vita/${2:-shot}.png"
    ;;
kill) cmd "quit $TITLE" ;;
press) shift; cmd "$*" ;; # vitacompanion input, e.g. press cross; wait 100ms; release cross
launch) cmd "launch $TITLE" ;;
*) sed -n 2,15p "$0" | sed 's/^# \{0,1\}//'; exit 1 ;;
esac
