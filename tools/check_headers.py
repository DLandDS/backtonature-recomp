"""Check that each public domain header compiles without the umbrella header."""
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DOMAINS = ('graphics', 'input', 'geometry', 'sound', 'text', 'ui', 'game')


def main() -> int:
    headers = [header for domain in DOMAINS for header in sorted((ROOT / 'include' / domain).glob('*.h'))]
    failed = 0
    with tempfile.TemporaryDirectory(prefix='btnx-headers-') as temporary:
        folder = Path(temporary)
        for header in headers:
            source = folder / 'probe.c'
            intermediate = folder / 'probe.i'
            assembly = folder / 'probe.s'
            source.write_text('#include "' + header.relative_to(ROOT / 'include').as_posix() + '"\nvoid header_probe(void) {}\n')
            cpp = subprocess.run(['wibo', '/psyq/BIN/CPPPSX.EXE', '-lang-c', '-I', str(ROOT / 'include'), str(source), str(intermediate)], capture_output=True, text=True)
            result = subprocess.run(['wibo', '/psyq/BIN/CC1PSX.EXE', '-quiet', '-O2', str(intermediate), '-o', str(assembly)], capture_output=True, text=True) if cpp.returncode == 0 else cpp
            if result.returncode != 0:
                failed += 1
                print(str(header.relative_to(ROOT)) + ': ' + result.stderr[-2000:])
    print(f'{len(headers)} public domain headers checked; {failed} failed')
    return 1 if failed else 0


if __name__ == '__main__':
    raise SystemExit(main())
