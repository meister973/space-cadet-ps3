# 3D Pinball Space Cadet for PS3

A PS3 homebrew port of 3D Pinball for Windows: Space Cadet. It uses the PSL1GHT toolchain.

The game code comes from MaikelChan's [Wii port](https://github.com/MaikelChan/SpaceCadetPinball). That port is based on k4zmu2a's [decompilation](https://github.com/k4zmu2a/SpaceCadetPinball). It already reads the game data on a big-endian CPU. This project replaces the Wii layer with a PS3 layer: RSX video, a 48 kHz audio mixer, and DualShock 3 input.

This repository holds no game data. You must use the files from your own copy of the game.

## Controls

| Button | Action |
|---|---|
| L1 or L2 | Left flipper |
| R1 or R2 | Right flipper |
| Cross | Plunger (hold, then release) |
| D-pad left, right, up | Nudge the table |
| Start | Pause |
| Select | New game |

To quit, press the PS button and select Quit Game.

## Build

You need Docker and the `balatro-ps3dev:2` image. It is the ps3dev nightly with cgcomp's libCg (see `D:\Ps3 Port\docker\Dockerfile`).

```powershell
pwsh -NoProfile -File tools/build.ps1
```

The bootable file is `build/spacecadet.elf`. To make an installable package, run `make pkg` in the same container. The package holds an empty `Data` folder.

## Game data

Copy the files from your Windows XP Pinball folder into `/dev_hdd0/game/PNBL00001/USRDIR/Data/`. These are `PINBALL.DAT`, `pinball.exe`, and the `SOUND*.WAV` files. File names can be in any case.

The game asks for six sound files that were never released: SOUND2, SOUND37, SOUND44, SOUND59, SOUND62 and SOUND111. Those events are silent, as in the original game.

Music is optional. Convert `PINBALL.MID` to `PINBALL.ogg` and put it in the same folder.

The game writes a log to `/dev_hdd0/tmp/spacecadet_log.txt`.

## Tests

The unit tests cover the modules that do not need the PS3: WAV decoding, pixel conversion, screen layout, pad edges, sound channels, test scripts and file names.

```powershell
pwsh -NoProfile -File tools/test.ps1
```

`tools/run_rpcs3.ps1` starts the game in RPCS3. It can save frames to files and hold scripted pad buttons. For example:

```powershell
pwsh -NoProfile -File tools/run_rpcs3.ps1 -Expect 'frame=2400' -TimeoutSeconds 180 -Frames 520,2400 -PadScript '300 1','420 0'
```

`tools/ppm_to_png.py` converts the saved frames to PNG.

## License

MIT, as the decompilation. See `LICENSE`.
