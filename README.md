# 3D Pinball Space Cadet for PS3

The Windows XP pinball game, running as homebrew on a PS3.

It plays the same table with the same physics, missions and sounds as the original. You bring the game files from your own copy of Windows XP. This repository has none of them.

## Install

You need a PS3 with CFW or PS3HEN and a Windows PC. You also need the 3D Pinball files from Windows XP. They are in `C:\Program Files\Windows NT\Pinball` on an XP machine.

1. Download `SpaceCadetPKG.zip` from the Releases page and unzip it.
2. Copy `make_spacecadet_pkg.bat` and the `spacecadet_pkg_files` folder into your Pinball folder, next to `PINBALL.DAT`.
3. Double-click `make_spacecadet_pkg.bat`. It makes `SpaceCadet.pkg` in the same folder.
4. Copy `SpaceCadet.pkg` to a USB stick and install it on the PS3 with Package Manager.

If `PINBALL.DAT` is missing, the `.bat` file stops. If the `SOUND*.WAV` files are missing, it warns you and makes the package anyway, and the game has no sound.

The package has your game files in it, so keep it to yourself.

The Full Tilt! version of the table works too. Use its `CADET` folder instead of the Pinball folder.

## Controls

| Button | Action |
|---|---|
| L1 or L2 | Left flipper |
| R1 or R2 | Right flipper |
| Cross | Plunger: hold to pull, release to launch |
| D-pad left, right, up | Nudge the table |
| Start | Pause |
| Select | New game |

To quit, press the PS button and pick Quit Game.

## Known gaps

Each time you quit, high scores and settings reset. The original kept them in the Windows registry.

There is no music unless you add it. The original played `PINBALL.MID` through the Windows MIDI synth. Convert that file to `PINBALL.ogg` and put it next to `PINBALL.DAT` before you run the `.bat` file.

The game asks for six sound files that no released copy has: SOUND2, SOUND37, SOUND44, SOUND59, SOUND62 and SOUND111. Those events are silent on Windows too.

The game writes a log to `/dev_hdd0/tmp/spacecadet_log.txt`. To report a problem, include that log.

## Building from source

You only need this to change the code. There are two ways, and both use the same pinned toolchain.

On Linux or WSL, without Docker:

```sh
sh tools/install_toolchain.sh
sh tools/make.sh pkg
```

The toolchain goes into `$HOME/ps3dev` and needs no root access.

On Windows, with Docker:

```powershell
pwsh -NoProfile -File tools/build.ps1 -Pkg
```

The first run builds the toolchain image from `docker/Dockerfile` and takes a few minutes.

Both give `build/spacecadet.elf` for RPCS3 and `spacecadet.pkg` with an empty `Data` folder. To make the release zip, run this after the build:

```powershell
docker run --rm -v "${PWD}:/work" -w /work ubuntu:24.04 sh tools/release.sh
```

## Testing

```powershell
pwsh -NoProfile -File tools/test.ps1
pwsh -NoProfile -File tools/play.ps1
```

The first command runs the unit tests. The second starts the game in RPCS3 with an Xbox pad as Player 1. Both RPCS3 scripts look for RPCS3 in a folder named `rpcs3` on your desktop. For any other folder, set `RPCS3_DIR`.

`tools/run_rpcs3.ps1` runs the game without a window. It can hold pad buttons from a script and save frames as images, which `tools/ppm_to_png.py` converts to PNG.

## Credits

The game code comes from the [Wii port][wii] by MaikelChan. That port is based on the [decompilation][decomp] by k4zmu2a, and it already handled the big-endian game data. This project adds the PS3 parts: RSX video, the audio mixer, DualShock 3 input and the packaging.

3D Pinball Space Cadet was made by Cinematronics and published by Microsoft.

[wii]: https://github.com/MaikelChan/SpaceCadetPinball
[decomp]: https://github.com/k4zmu2a/SpaceCadetPinball

## License

MIT, the same as the decompilation. See `LICENSE`.
