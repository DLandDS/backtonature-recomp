"""Test the native file reader against independently sliced synthetic bytes.

Pass an HDT/BIN directory as the optional second argument to verify supplied
data as well. This proves span byte reads, not payload formats or retail identity.
"""
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
import tempfile


def run(executable, arguments, status=0, fragment=None):
    result = subprocess.run([executable, *map(str, arguments)], capture_output=True,
                            text=True, encoding="utf-8", errors="replace", timeout=30)
    if result.returncode != status or (fragment and fragment not in result.stdout + result.stderr):
        raise AssertionError(f"{arguments}: exit {result.returncode}\n{result.stdout}{result.stderr}")
    return result.stdout


def verify(executable, assets, output):
    hdt = (assets / "A_FILE.HDT").read_bytes()
    if len(hdt) < 132:
        raise AssertionError("Reference HDT needs at least 33 offsets")
    offsets = struct.unpack_from("<33I", hdt)
    listing = run(executable, ["--data-dir", assets, "--archive-list"]).splitlines()
    assert len(listing) == 34 and listing[1] == "index\toffset\tend\tspan_bytes"
    assert listing[0] == (f"# members=32 hdt_extra_unclassified={len(hdt) - 132} "
                          f"bin_prefix_unclassified={offsets[0]} "
                          f"bin_tail_unclassified={(assets / 'A_FILE.BIN').stat().st_size - offsets[-1]}")
    with (assets / "A_FILE.BIN").open("rb") as source:
        for member, (start, end) in enumerate(zip(offsets, offsets[1:])):
            assert list(map(int, listing[member + 2].split("\t"))) == [member, start, end, end - start]
            target = output / f"member-{member}.bin"
            run(executable, ["--data-dir", assets, "--archive-dump", member, "--output", target])
            source.seek(start)
            digest = hashlib.sha256()
            remaining = end - start
            while remaining:
                block = source.read(min(remaining, 65536))
                assert block, "Reference BIN is truncated"
                digest.update(block)
                remaining -= len(block)
            actual = hashlib.sha256()
            with target.open("rb") as dump:
                while block := dump.read(65536):
                    actual.update(block)
            assert target.stat().st_size == end - start
            assert actual.digest() == digest.digest(), f"member {member}: byte mismatch"
            target.unlink()
    return offsets


def main():
    executable = str(Path(sys.argv[1]).resolve())
    with tempfile.TemporaryDirectory(prefix="btn-archive-") as directory:
        root = Path(directory)
        if len(sys.argv) == 3:
            assets = Path(sys.argv[2]).resolve()
            offsets = verify(executable, assets, root)
            print(f"native-archive-data: 32 spans matched direct-read SHA-256; end={offsets[-1]}")
            return

        assets = root / "data with spaces"
        assets.mkdir()
        # Prefix, unaligned cross-sector member, empty member, and unclassified tail.
        offsets = [2047, 2049, 2049] + [2053] * 29 + [196613]
        hdt = struct.pack("<33I", *offsets) + b"\xFF\x00"
        data = bytes((i * 17 + 3) % 256 for i in range(196617))
        (assets / "A_FILE.HDT").write_bytes(hdt)
        (assets / "A_FILE.BIN").write_bytes(data)
        verify(executable, assets, root)
        run(executable, ["--help"], fragment="Usage:")
        invalid = [[], ["--data-dir"], ["--archive-list"],
                   ["--data-dir", assets, "--archive-list", "--length", 1],
                   ["--data-dir", assets, "--archive-list", "--archive-dump", 0],
                   ["--data-dir", assets, "--archive-dump", 32, "--output", root / "bad"],
                   ["--data-dir", assets, "--archive-dump", -1, "--output", root / "bad"],
                   ["--data-dir", assets, "--archive-dump", 0],
                   ["--data-dir", assets, "--archive-dump", 0, "--output", root / "bad", "--offset", 4294967296]]
        for arguments in invalid:
            run(executable, arguments, 2, "Invalid archive arguments")
        common = ["--data-dir", assets, "--archive-dump", 0]
        for arguments in (["--offset", 1, "--length", 2],
                          ["--offset", 4294967295, "--length", 4294967295], ["--offset", 3]):
            target = root / "invalid-range.bin"
            run(executable, [*common, *arguments, "--output", target], 1, "exceeds")
            assert not target.exists()
        for member, relative, length in ((0, 1, 1), (0, 2, 0), (1, 0, 0), (31, 65530, 65550)):
            target = root / f"partial-{member}-{relative}.bin"
            run(executable, ["--data-dir", assets, "--archive-dump", member, "--offset", relative,
                             "--length", length, "--output", target])
            assert target.read_bytes() == data[offsets[member] + relative:offsets[member] + relative + length]
        target = root / "hasil-é-日本.bin"
        run(executable, [*common, "--output", target])
        assert target.read_bytes() == data[offsets[0]:offsets[1]]
        target = root / "existing.bin"
        target.write_bytes(b"preserve")
        for destination in (target, assets / "A_FILE.BIN", assets / "A_FILE.HDT"):
            before = destination.read_bytes()
            run(executable, [*common, "--output", destination], 1, "Cannot create new output")
            assert destination.read_bytes() == before
        run(executable, [*common, "--output", root / "missing" / "output.bin"], 1, "Cannot create new output")
        run(executable, ["--data-dir", root / "missing", "--archive-list"], 1, "archive failed:")
        for bad_hdt in (b"", hdt[:131], struct.pack("<33I", *([3, 2] + [4] * 31)),
                        struct.pack("<33I", *([0] * 32 + [0xFFFFFFFF]))):
            (assets / "A_FILE.HDT").write_bytes(bad_hdt)
            run(executable, ["--data-dir", assets, "--archive-list"], 1, "archive failed:")
        (assets / "A_FILE.HDT").write_bytes(hdt)
        (assets / "A_FILE.BIN").write_bytes(data[:-5])
        run(executable, ["--data-dir", assets, "--archive-list"], 1, "exceeds BIN size")
        (assets / "A_FILE.HDT").write_bytes(bytes(132))
        (assets / "A_FILE.BIN").write_bytes(b"")
        verify(executable, assets, root)
    print("native-archive-cli: 32 member hashes, partial/multi-chunk reads, safe output and invalid input checks passed")


if __name__ == "__main__":
    main()
