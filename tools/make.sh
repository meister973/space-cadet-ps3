#!/bin/sh
# Runs make with the toolchain from tools/install_toolchain.sh, without Docker.
#
#   sh tools/make.sh            build build/spacecadet.elf
#   sh tools/make.sh pkg        build spacecadet.pkg
#
# PS3DEV picks another toolchain folder. Default: $HOME/ps3dev
set -eu

PS3DEV=${PS3DEV:-$HOME/ps3dev}
if [ ! -x "$PS3DEV/bin/cgcomp" ]; then
    echo "no toolchain in $PS3DEV: run sh tools/install_toolchain.sh first" >&2
    exit 1
fi

export PS3DEV
export PSL1GHT="$PS3DEV"
export PATH="$PATH:$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin"
export LD_LIBRARY_PATH="$PS3DEV/cg/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

cd "$(dirname "$0")/.."
exec make "$@"
