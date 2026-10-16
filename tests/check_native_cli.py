"""Exercise the built SDL application with dummy video and synthetic files."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    executable = str(Path(sys.argv[1]).resolve())
    environment = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software",
                       SDL_AUDIODRIVER="dummy")
    checks = 0
    with tempfile.TemporaryDirectory(prefix="btn-runtime-") as directory:
        root = Path(directory)

        def run(arguments, status, fragment):
            nonlocal checks
            result = subprocess.run([executable, *arguments], cwd=root, env=environment,
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                    text=True, timeout=15)
            if result.returncode != status or fragment not in result.stdout:
                raise AssertionError(f"{arguments}: exit {result.returncode}\n{result.stdout}")
            checks += 1

        run(["--help"], 0, "Usage:")
        run([], 2, "not implemented")
        for arguments in (["--fps", "0"], ["--fps", "nan"], ["--volume", "101"],
                          ["--frames", "-1"], ["--frames", "999999999999999999999"],
                          ["--bind", "up=invalid-scancode"], ["--bind", "up=Escape"],
                          ["--bind", "missing=W"], ["--unknown", "value"]):
            run(["--runtime-test", *arguments], 2, "Invalid option/value")
        run(["--runtime-test", "--fps"], 2, "Missing value")
        common = ["--runtime-test", "--save-dir", str(root / "saves"), "--frames", "3"]
        run(common + ["--data-dir", str(root / "missing")], 1, "Required data file missing or empty")
        assets = root / "assets"
        assets.mkdir()
        (assets / "A_FILE.HDT").touch()
        run(common + ["--data-dir", str(assets)], 1, "Required data file missing or empty")
        (assets / "A_FILE.HDT").write_bytes(b"synthetic header")
        (assets / "A_FILE.BIN").write_bytes(b"synthetic member")
        run(common + ["--data-dir", str(assets)], 0, "archive contents are not validated")
        for fps in (30, 60, 144):
            run(common + ["--fps", str(fps), "--volume", "25", "--bind", "up=W"],
                0, "Runtime stopped: frames=3")
        blocked_save = root / "file-not-directory"
        blocked_save.write_text("keep this file")
        run(["--runtime-test", "--save-dir", str(blocked_save), "--frames", "1"],
            1, "Cannot prepare save directory")
        assert blocked_save.read_text() == "keep this file"
    print(f"native-sdl: {checks} CLI/path/framebuffer lifecycle checks passed")


if __name__ == "__main__":
    main()
