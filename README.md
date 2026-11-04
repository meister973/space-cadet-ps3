# 3D Pinball Space Cadet for PS3

A PS3 homebrew port of 3D Pinball for Windows: Space Cadet. It uses the PSL1GHT toolchain.

The game code comes from the [Wii port][wii] by MaikelChan.
That port is based on the [decompilation][decomp] by k4zmu2a.
It already reads the game data on a big-endian CPU.

This project replaces the Wii layer with a PS3 layer: RSX video, a 48 kHz audio mixer, and DualShock 3 input.

This repository holds no game data. You must use the files from your own copy of the game.

[wii]: https://github.com/MaikelChan/SpaceCadetPinball
[decomp]: https://github.com/k4zmu2a/SpaceCadetPinball

## Install

You need a PS3 with CFW or PS3HEN, and Windows on your PC. You also need the 3D Pinball files from Windows XP: `PINBALL.DAT` and the `SOUND*.WAV` files. On Windows XP they are in `C:\Program Files\Windows NT\Pinball`.

1. Download `SpaceCadetPKG.zip` from the Releases page and unzip it.
2. Copy `make_spacecadet_pkg.bat` and the `spacecadet_pkg_files` folder into your Pinball folder, next to `PINBALL.DAT`.
3. Double-click `make_spacecadet_pkg.bat`. It makes `SpaceCadet.pkg` in the same folder.
4. Copy `SpaceCadet.pkg` to a USB stick.
5. On the PS3, install it with Package Manager, then Install Package Files.

If `PINBALL.DAT` is not in the folder, the `.bat` file stops. If the sound files are not in the folder, it warns you and makes the package anyway. The game then has no sound.

The package contains your game files. Do not share it.

The Full Tilt! table also works. Use its `CADET` folder instead of the Pinball folder.

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

There are two ways to build. Both use the same pinned toolchain: the ps3dev nightly and NVIDIA's libCg for the shader compiler.

Without Docker, on Linux x64 or WSL on Windows:

```sh
sh tools/install_toolchain.sh
sh tools/make.sh pkg
```

The first command installs the toolchain into `$HOME/ps3dev`. It needs no root access. To use another folder, give it as an argument. Then set `PS3DEV` to that folder for `tools/make.sh`.

With Docker, on Windows:

```powershell
pwsh -NoProfile -File tools/build.ps1 -Pkg
```

The first run builds the toolchain image from `docker/Dockerfile`. This takes a few minutes.

Both ways give `build/spacecadet.elf`, which RPCS3 boots, and `spacecadet.pkg`, which a PS3 installs. That package holds an empty `Data` folder. Copy the game files into `/dev_hdd0/game/PNBL00001/USRDIR/Data/` yourself, for example over FTP.

To make the player download, run `make pkg`, then run `tools/release.sh` in plain Ubuntu:

```powershell
docker run --rm -v "${PWD}:/work" -w /work ubuntu:24.04 sh tools/release.sh
```

It gives `dist/SpaceCadetPKG.zip`. It builds `pkg.exe` for Windows from the PSL1GHT source.

## Game data

File names can be in any case.

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
