#!/bin/sh
# Makes the player download: dist/SpaceCadetPKG.zip. It holds make_spacecadet_pkg.bat, the
# built game, and pkg.exe for Windows, and no game data.
#
# Build the game with "make pkg" first. Then run this in plain ubuntu:24.04:
#   docker run --rm -v "$PWD:/work" -w /work ubuntu:24.04 sh tools/release.sh
set -eu

ROOT=$(pwd)
OUTPUT="$ROOT/dist/SpaceCadetPKG"
PSL1GHT_COMMIT=7e1bd96307eeac8000f18fd75e643d2fa826b5c1

for file in build/pkg/USRDIR/EBOOT.BIN build/pkg/PARAM.SFO build/pkg/ICON0.PNG; do
    [ -f "$file" ] || { echo "missing $file: run make pkg first" >&2; exit 1; }
done

apt-get update -qq > /dev/null
apt-get install -y -qq --no-install-recommends ca-certificates git zip gcc-mingw-w64-x86-64 > /dev/null

rm -rf "$OUTPUT" "$ROOT/dist/SpaceCadetPKG.zip"
mkdir -p "$OUTPUT/spacecadet_pkg_files"
WORK=$(mktemp -d)

git clone -q https://github.com/ps3dev/PSL1GHT "$WORK/psl1ght"
git -C "$WORK/psl1ght" checkout -q "$PSL1GHT_COMMIT"
# pkg.c's Windows folder walk stores paths with a leading slash, so the PS3 would not find
# USRDIR/EBOOT.BIN. Its POSIX walk works with MinGW, and needs far more stack than the 1 MB default.
sed -i '291s/#ifdef _WIN32/#if 0/' "$WORK/psl1ght/tools/sfo_pkg/pkg.c"
x86_64-w64-mingw32-gcc -O2 -static -s -Wl,--stack,67108864 -include dirent.h -include sys/stat.h \
    "$WORK/psl1ght/tools/sfo_pkg/pkg.c" "$WORK/psl1ght/tools/sfo_pkg/sha1.c" \
    -o "$OUTPUT/spacecadet_pkg_files/pkg.exe"
cp "$WORK/psl1ght/LICENSE" "$OUTPUT/spacecadet_pkg_files/PSL1GHT_LICENSE.txt"
rm -rf "$WORK"

cp build/pkg/USRDIR/EBOOT.BIN build/pkg/PARAM.SFO build/pkg/ICON0.PNG "$OUTPUT/spacecadet_pkg_files/"
# Windows needs CRLF line ends in a .bat file.
sed 's/$/\r/' tools/release/make_spacecadet_pkg.bat > "$OUTPUT/make_spacecadet_pkg.bat"
sed 's/$/\r/' tools/release/README.txt > "$OUTPUT/README.txt"

(cd "$ROOT/dist" && zip -qr SpaceCadetPKG.zip SpaceCadetPKG)
ls -la "$OUTPUT" "$OUTPUT/spacecadet_pkg_files" | cut -c30-
