# Attack of the PETSCII Robots - MEGA65 Port

Ported to the MEGA65 from the Amiga version (https://github.com/zeropolis79/PETSCIIRobots-Amiga).

## Features
- **VIC-IV Display**: Full color, 320x200 graphics / FCM mode utilizing VIC-IV capabilities and palette registers ($D100-$D3FF) for Amiga-like visuals.
- **LOADIFF-compatible IFF support**: Supports loading Amiga IFF ILBM and raw screen data for Intro, Game, and GameOver screens.
- **DMA Audio MOD Player**: 4-channel ProTracker MOD playback using MEGA65 hardware audio registers ($D711, $D720-$D75F, audio crossbar $D6F4/$D6F5, hardware multiplier $D770-$D777).
- **Dual Compiler Support**: Builds with **cc65** (`cl65`) as the primary compiler, with optional **oscar64** compilation support.
- **D81 Disk Image Generation**: Easily packaged onto a `.d81` disk using `cc1541`.

## Requirements
- `cc65` (V2.18+ / V2.19+)
- `cc1541` (for generating D81 disk images)
- Optional: `oscar64`

## Building

### Build with cc65 (default):
```bash
make
```
This builds `PETRobots.prg` and the bootable `PETRobots.d81` disk image.

### Build with oscar64:
```bash
make COMPILER=oscar64
```

### Clean:
```bash
make clean
```

## Running
On MEGA65 hardware or in an emulator such as Xemu (`xmega65`):
- Mount or load `PETRobots.d81` or run `PETRobots.prg`.

## Controls
- **Movement**: `I` `J` `K` `L` or Joystick Port 2
- **Fire**: `W` `A` `S` `D`
- **Actions**:
  - `Z`: Search
  - `M`: Move
  - `Space`: Use item
  - `1`: Cycle weapon
  - `2`: Cycle item
  - `Tab`: Live Map
  - `ESC`: Pause
  - `F6`: Toggle Music
