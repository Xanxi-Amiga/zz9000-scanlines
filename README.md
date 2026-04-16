# ZZ9000 Scanlines V2

CRT scanline effect for the MNT ZZ9000 graphics card on Amiga computers.

Adds CRT scanline simulation to the ZZ9000 scandoubler output via FPGA firmware patches and an AmigaOS control tool.

---

## Features

- 3 scanline modes + off
- Automatic activation in AGA scandoubled modes and RTG below 350 lines
- Disabled in interlaced modes and high-resolution RTG
- CLI tool `ZZScanlines` for scripting and startup-sequence
- GUI tool `ZZScanlinesGUI` for interactive control from Workbench
- Compatible with ZZ9000 firmware 1.14

---

## Scanline modes

| Mode | Name | Pattern | Description |
|------|------|---------|-------------|
| 0 | Off | — | No effect |
| 1 | Classic | 0% / 100% / 0% / 100% | Every other line black — strong CRT effect |
| 2 | Soft | 100% / 62% / 100% / 62% | Softer alternating attenuation |
| 3 | Gradient | 100% / 75% / 50% / 75% / 100% | Smooth fade — most realistic CRT look |

Scanlines are active when:
- AGA scandoubler is active (`scale_y = 1`)
- OR RTG resolution is below 350 lines (e.g. Quake, Doom)
- AND the screen is not interlaced

---

## Installation

1. **Back up** the existing `BOOT.bin` on your ZZ9000 SD card
2. Copy `BOOT_v2.bin` to your ZZ9000 SD card and rename it `BOOT.bin`
3. Copy `ZZScanlines` and `ZZScanlinesGUI` to a directory in your path (e.g. `SYS:Utilities/`)
4. Power cycle your Amiga (full power off/on — not just reset)

> **Note:** Scanlines persist across soft resets because the FPGA state is preserved. To disable scanlines at startup, add `ZZScanlines 0` to your `S:Startup-Sequence`. To enable them automatically, add your preferred `ZZScanlines` command there.

---

## ZZScanlines — CLI usage

```
ZZScanlines <mode> [parity]

  mode   : 0=off  1=classic  2=soft  3=gradient
  parity : 0=odd lines dark (default)  1=even lines dark
  note   : active in AGA scandoubled modes and RTG below 350 lines

Examples:
  ZZScanlines 0          turn off
  ZZScanlines 1          classic CRT
  ZZScanlines 2          soft
  ZZScanlines 3          gradient
  ZZScanlines 1 1        classic, inverted parity
```

---

## ZZScanlinesGUI — Graphical interface

Launch `ZZScanlinesGUI` from Workbench or Shell. Select mode and parity, then click **Apply**. Close the window with the standard close gadget.

Requires AmigaOS 2.0+ (intuition.library v37+).

---

## Building from source

Requires [bebbo's amiga-gcc](https://github.com/bebbo/amiga-gcc).

```bash
# ZZScanlines CLI
m68k-amigaos-gcc -O2 -noixemul -o ZZScanlines ZZScanlines.c -lamiga

# ZZScanlinesGUI
m68k-amigaos-gcc -O2 -noixemul -o ZZScanlinesGUI ZZScanlinesGUI.c -lamiga

# OS 3.1 icon (run on Amiga after copying to same dir as ZZScanlinesGUI)
m68k-amigaos-gcc -O2 -noixemul -o MakeIcon MakeIcon.c -lamiga
```

The FPGA firmware patches are in `video_formatter.v` and `mntzorro.v`. Build with Vivado ML Standard 2025.2 against the ZZ9000 firmware 1.14 project.

---

## Technical details

The scanline effect is implemented entirely in the ZZ9000 PL (FPGA) domain:

- `video_formatter.v` — pixel pipeline, scanline pattern generation
- `mntzorro.v` — Zorro register interface for runtime control

Register map (Zorro offset from board base):

| Offset | Register | Values |
|--------|----------|--------|
| `0x100C` | `scanline_width` | 0=off, 1=classic, 2=soft, 3=gradient |
| `0x100E` | `scanline_parity` | 0=odd dark, 1=even dark |

Pipeline alignment: `counter_y` is delayed 2 cycles (`counter_y_d1/d2`) to align with the 4-stage pixel pipeline (`pixout32 → pixout32_dly → pixout32_dly2 → pixout → pixout_r`).

---

## Compatibility

- Amiga 4000 with ZZ9000 (Zynq Z-7020)
- ZZ9000 firmware 1.14 base
- AmigaOS 3.x
- Tested on AGA low-resolution modes and RTG with Quake/Doom

---

## Credits

- **Xanxi (Hakim Hassani)** — scanlines V2 design and implementation
- **MiDWaN (Dimitris Panokostas)** — ZZ9000 firmware 1.14 base, driver support
- **awmosoft (Antony Mo)** — hardware testing and feedback

---

## License

GPL-3.0-or-later — see [LICENSE](LICENSE)

This project modifies firmware originally Copyright (C) 2019-2022 Lukas F. Hartmann / MNT Research GmbH, licensed under GPL-3.0-or-later.
