#!/usr/bin/env python3
"""Validate embedded packages without credentials or network access."""
from pathlib import Path
import hashlib
import json

root = Path(__file__).resolve().parents[1]
packages = json.loads((root / "skills-lock.json").read_text())
inventory = json.loads((root / "vendor/skills/inventory.json").read_text())
for relative, expected in inventory.items():
    path = root / relative
    if "link" in expected:
        assert path.is_symlink() and str(path.readlink()) == expected["link"], relative
    else:
        assert path.is_file(), relative
        assert hashlib.sha256(path.read_bytes()).hexdigest() == expected["sha256"], relative
count = 0
for package, metadata in packages.items():
    assert len(metadata["commit"]) == 40, package
    assert (root / "vendor/skills" / package / "LICENSE").is_file(), package
    for name in metadata["skills"]:
        link = root / ".agents/skills" / name
        source = root / "vendor/skills" / package / "skills" / name
        assert link.is_symlink() and link.resolve() == source.resolve(), name
        text = (link / "SKILL.md").read_text()
        assert text.startswith("---") and "name:" in text, name
        count += 1
instructions = (root / "AGENTS.md").read_text()
for name in ("caveman", "ponytail", "i-have-adhd"):
    assert f".agents/skills/{name}/SKILL.md" in instructions, name
print(f"PASS: {len(packages)} packages, {count} skills, {len(inventory)} bundled files; session instructions present.")
