"""Read modular unity sources in their original retail compilation order."""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ALIASES = json.loads((ROOT / 'docs/retail-symbols.json').read_text())


def expand_source(path: Path, seen=None) -> str:
    seen = set() if seen is None else seen
    path = path.resolve()
    if path in seen:
        return ''
    seen.add(path)
    text = path.read_text(encoding='utf-8')
    def include(match):
        name = match.group(1)
        for target in (path.parent / name, ROOT / 'include' / name):
            if target.is_file():
                return expand_source(target, seen)
        return match.group()
    text = re.sub(r'^\s*#include\s+"([^"]+)"\s*$', include, text, flags=re.M)
    # Compiler output uses these original names after the alias macros expand.
    return re.sub(r'\b[A-Za-z_]\w*\b', lambda match: ALIASES.get(match.group(), match.group()), text)
