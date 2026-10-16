#!/usr/bin/env python3
"""Extract the pristine SLUS_011.15 from your own disc image into baserom/.

    python3 tools/extract_baserom.py "path/to/Harvest Moon - Back to Nature.bin"
    python3 tools/extract_baserom.py path/to/SLUS_011.15

The argument is either a raw Mode 2/2352 disc image (.bin) of the US release,
or an SLUS_011.15 you have already copied off that disc. Either way the result
must hash to the US retail executable, or nothing is written: a modified or
translated executable would make every byte-match check meaningless.
"""
import argparse
import hashlib
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "baserom" / "SLUS_011.15"

SECTOR, USER_OFF, USER_LEN = 2352, 24, 2048
EXE_LEN = 323584

EXPECT_SHA1 = "07fbcffbdd96080d39939e569f84abe76b9d1039"
EXPECT_SHA256 = "34e3b35dcd3fb1cd9a7d4813b8a9ef987915826e3d436f1908ecbe2d4fdf1444"


def from_disc(data: bytes):
    """Find the PS-X EXE in a raw 2352-byte-sector image and read EXE_LEN bytes."""
    if len(data) % SECTOR:
        print(f"warning: image size {len(data)} is not a multiple of {SECTOR}", file=sys.stderr)
    for s in range(len(data) // SECTOR):
        if data[s * SECTOR + USER_OFF:s * SECTOR + USER_OFF + 8] == b"PS-X EXE":
            print(f"found PS-X EXE at sector {s} (image offset 0x{s * SECTOR:08X})")
            need = (EXE_LEN + USER_LEN - 1) // USER_LEN
            buf = b"".join(data[(s + i) * SECTOR + USER_OFF:(s + i) * SECTOR + USER_OFF + USER_LEN]
                           for i in range(need))
            return buf[:EXE_LEN]
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("source", type=Path, help="disc image (.bin, Mode 2/2352) or an SLUS_011.15 copy")
    args = parser.parse_args()

    if not args.source.is_file():
        print(f"missing input: {args.source}", file=sys.stderr)
        return 1
    data = args.source.read_bytes()
    buf = data if data[:8] == b"PS-X EXE" else from_disc(data)
    if buf is None:
        print("no 'PS-X EXE' found; expected a raw Mode 2/2352 .bin or SLUS_011.15", file=sys.stderr)
        return 1

    sha1 = hashlib.sha1(buf).hexdigest()
    sha256 = hashlib.sha256(buf).hexdigest()
    print(f"  size  ={len(buf)}")
    print(f"  sha1  ={sha1}")
    print(f"  sha256={sha256}")
    if sha1 != EXPECT_SHA1 or sha256 != EXPECT_SHA256:
        print("HASH MISMATCH -- refusing to write; this is not the unmodified US "
              "retail SLUS_011.15.", file=sys.stderr)
        return 1

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_bytes(buf)
    print(f"wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
