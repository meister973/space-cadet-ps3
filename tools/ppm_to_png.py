"""Convert the binary PPM frame dumps of a test run to PNG, with no third-party modules.

Usage: ppm_to_png.py <file.ppm> [...]
"""

import struct
import sys
import zlib


def read_ppm(path):
    with open(path, "rb") as handle:
        data = handle.read()
    fields = data.split(maxsplit=4)
    if fields[0] != b"P6" or fields[3] != b"255":
        raise ValueError(f"{path} is not an 8-bit binary PPM")
    return int(fields[1]), int(fields[2]), fields[4]


def png_chunk(kind, payload):
    return struct.pack(">I", len(payload)) + kind + payload + struct.pack(">I", zlib.crc32(kind + payload))


def write_png(path, width, height, pixels):
    rows = b"".join(b"\x00" + pixels[y * width * 3:(y + 1) * width * 3] for y in range(height))
    header = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    with open(path, "wb") as handle:
        handle.write(b"\x89PNG\r\n\x1a\n")
        handle.write(png_chunk(b"IHDR", header))
        handle.write(png_chunk(b"IDAT", zlib.compress(rows, 6)))
        handle.write(png_chunk(b"IEND", b""))


def main():
    for path in sys.argv[1:]:
        width, height, pixels = read_ppm(path)
        target = path[:-4] + ".png"
        write_png(target, width, height, pixels)
        print(target)


if __name__ == "__main__":
    main()
