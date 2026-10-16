# Vibe coding starter pack

Embedded agent skills for every session: Caveman for concise communication, Ponytail for minimal correct code, and i-have-adhd for actionable structure. `AGENTS.md` activates these defaults for agents that read repository instructions. Start a new session in the `backtonature-recomp` checkout to load newly discovered skills.

## Included packages

Complete tracked upstream snapshots are in `vendor/skills/caveman`, `vendor/skills/ponytail`, and `vendor/skills/i-have-adhd`, including scripts, hooks, adapters, documentation, and licenses. Exact source commits and all installed skill names are recorded in `skills-lock.json`.

`.agents/skills/` contains relative links to every canonical upstream skill, with adjacent assets and scripts preserved. No global installation or download is required for the three default instruction skills. Node.js 24 and Python 3.12 are available in this cloud environment for bundled JavaScript and Python scripts.

## Runtime boundaries

Bundling source does not build every upstream application or enable plugins in other hosts. Hooks are not automatically trusted or registered. Caveman's optional cloud/proxy/SDK/browser packages require their own documented build, configuration, and credentials. `caveman-compress` includes its Python scripts but actual model-backed compression requires an authenticated Claude CLI, which is not installed in this environment. Skill availability is distinct from those optional integrations being operational.

## Validation

Run `python3 scripts/check-skills.py` to verify pinned source inventory, skill discovery links, bundled files, and session instructions. For compression CLI help: `cd .agents/skills/caveman-compress && python3 -m scripts --help`.

## Licenses

Caveman is Apache-2.0; Ponytail and i-have-adhd are MIT. Original licenses and notices are preserved inside each vendored package.
