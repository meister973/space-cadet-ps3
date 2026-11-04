#!/bin/sh
# Installs the PS3 toolchain into one folder, with no root access and nothing system-wide.
# Linux x64 only, including WSL on Windows. The Docker image runs this same script.
#
#   sh tools/install_toolchain.sh [folder]      default folder: $HOME/ps3dev
#
# Pinned by hash: ps3dev's prebuilt nightly, and NVIDIA's libCg from Ubuntu, which cgcomp
# (the shader compiler) loads at run time.
set -eu

TOOLCHAIN_URL=https://github.com/ps3dev/ps3dev/releases/download/nightly-2026-10-12/ps3dev-linux-X64.tar.gz
TOOLCHAIN_SHA256=76b1b3bc9ac260a4ece86da520225da7f2f4e334174e37b850093ff925eeb900
LIBCG_URL=https://archive.ubuntu.com/ubuntu/pool/multiverse/n/nvidia-cg-toolkit/libcg_3.1.0013-5build1_amd64.deb
LIBCG_SHA256=42f90c8377857226b7a477d0a56f5c8354a49149f4d743329efda268e5cfd841

TARGET=${1:-$HOME/ps3dev}
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

fetch() {
    echo "downloading $(basename "$1")"
    curl -fsSL --retry 3 -o "$WORK/$3" "$1"
    echo "$2  $WORK/$3" | sha256sum -c - >/dev/null
}

fetch "$TOOLCHAIN_URL" "$TOOLCHAIN_SHA256" ps3dev.tar.gz
fetch "$LIBCG_URL" "$LIBCG_SHA256" libcg.deb

# Some nightlies wrap everything in one top folder, others put bin/ ppu/ spu/ at the root.
mkdir -p "$WORK/unpack" "$TARGET"
tar -xzf "$WORK/ps3dev.tar.gz" -C "$WORK/unpack"
SOURCE="$WORK/unpack"
if [ "$(ls -A "$SOURCE" | wc -l)" = 1 ] && [ -d "$SOURCE/$(ls -A "$SOURCE")" ]; then
    SOURCE="$SOURCE/$(ls -A "$SOURCE")"
fi
cp -a "$SOURCE/." "$TARGET/"

dpkg-deb -x "$WORK/libcg.deb" "$WORK/libcg"
mkdir -p "$TARGET/cg/lib"
cp "$WORK/libcg/usr/lib/x86_64-linux-gnu/libCg.so" "$TARGET/cg/lib/"

# A broken shader compiler fails here, not halfway through a game build.
printf 'void main(float2 t:TEXCOORD0,out float4 c:COLOR,uniform sampler2D s){c=tex2D(s,t);}\n' > "$WORK/t.fcg"
LD_LIBRARY_PATH="$TARGET/cg/lib" "$TARGET/bin/cgcomp" -f "$WORK/t.fcg" "$WORK/t.fpo" >/dev/null

echo "toolchain ready in $TARGET"
