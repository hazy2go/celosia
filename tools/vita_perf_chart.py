#!/usr/bin/env python3
"""Draws docs/vita/perf.svg: the game logic's milliseconds per frame in the
opening cinematic on a real Vita, per build ([frametime] "game", the heavier
canyon part), against the 33 ms a 30 fps frame has. Edit BUILDS and rerun.

Usage: tools/vita_perf_chart.py
"""
import os

# (label, ms) - measured on the device, same scene, same memory card state.
BUILDS = [
    ("CPU fixes", 245),
    ("code size, VU0", 241),
    ("direct transfers", 207),
]
TARGET_MS = 33.3

W, H = 720, 300
LEFT, RIGHT, TOP, BOTTOM = 150, 40, 76, 40
BAR_H, GAP = 34, 18
MAX_MS = 260


def x(ms):
    return LEFT + (W - LEFT - RIGHT) * ms / MAX_MS


def main():
    plot_h = len(BUILDS) * (BAR_H + GAP) - GAP
    height = TOP + plot_h + BOTTOM
    out = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{height}" viewBox="0 0 {W} {height}" '
           'font-family="-apple-system, Segoe UI, Helvetica, Arial, sans-serif" role="img" '
           'aria-label="Game logic milliseconds per frame in the opening on a PS Vita, per build">']
    out.append('<style>'
               '.bg{fill:#fcfcfb}.ink{fill:#0b0b0b}.ink2{fill:#52514e}.muted{fill:#898781}'
               '.grid{stroke:#e4e3df}.bar{fill:#2a78d6}.target{stroke:#0b0b0b}'
               '@media (prefers-color-scheme: dark){.bg{fill:#1a1a19}.ink{fill:#f0efec}.ink2{fill:#c3c2b7}'
               '.grid{stroke:#33332f}.bar{fill:#4f95e8}.target{stroke:#f0efec}}'
               '</style>')
    out.append(f'<rect class="bg" width="{W}" height="{height}" rx="8"/>')
    out.append(f'<text class="ink" x="{LEFT}" y="26" font-size="15" font-weight="600">Opening cinematic on a PS Vita: game logic per frame</text>')
    out.append(f'<text class="ink2" x="{LEFT}" y="44" font-size="12">milliseconds, lower is better; 30 fps needs the whole frame under 33 ms</text>')
    for ms in range(0, MAX_MS + 1, 50):
        gx = x(ms)
        out.append(f'<line class="grid" x1="{gx:.1f}" y1="{TOP - 4}" x2="{gx:.1f}" y2="{TOP + plot_h + 4}" stroke-width="1"/>')
        out.append(f'<text class="muted" x="{gx:.1f}" y="{TOP + plot_h + 22}" font-size="11" text-anchor="middle">{ms}</text>')
    for i, (label, ms) in enumerate(BUILDS):
        y = TOP + i * (BAR_H + GAP)
        out.append(f'<text class="ink2" x="{LEFT - 12}" y="{y + BAR_H / 2 + 4:.1f}" font-size="12" text-anchor="end">{label}</text>')
        out.append(f'<path class="bar" d="M{LEFT},{y} H{x(ms) - 4:.1f} a4,4 0 0 1 4,4 V{y + BAR_H - 4} a4,4 0 0 1 -4,4 H{LEFT} Z"/>')
        out.append(f'<text class="ink" x="{x(ms) + 8:.1f}" y="{y + BAR_H / 2 + 4:.1f}" font-size="12" font-weight="600">{ms} ms</text>')
    tx = x(TARGET_MS)
    out.append(f'<line class="target" x1="{tx:.1f}" y1="{TOP - 8}" x2="{tx:.1f}" y2="{TOP + plot_h + 4}" stroke-width="2" stroke-dasharray="4 3"/>')
    out.append(f'<text class="ink" x="{tx + 6:.1f}" y="{TOP - 10}" font-size="11">30 fps</text>')
    out.append('</svg>')
    path = os.path.join(os.path.dirname(__file__), '..', 'docs', 'vita', 'perf.svg')
    with open(path, 'w') as f:
        f.write('\n'.join(out) + '\n')
    print(os.path.normpath(path))


if __name__ == '__main__':
    main()
