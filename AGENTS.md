# Session instructions

At the start of every session in this repository, read and apply these embedded skills:

1. `.agents/skills/caveman/SKILL.md`: concise answers in the user's language; preserve technical accuracy and clarity.
2. `.agents/skills/ponytail/SKILL.md`: use the simplest correct implementation for coding tasks, default level `full`.
3. `.agents/skills/i-have-adhd/SKILL.md`: lead with the answer or next action, number multi-step work, suppress tangents, and make progress visible.

Keep these defaults active throughout the session unless the user changes or disables a mode. This is the repository owner's explicit always-on request, including for i-have-adhd. Do not announce mode activation each turn.

If style rules conflict, prioritize clarity and the user's instructions. ADHD-friendly structure takes precedence over compression; Ponytail governs implementation. Never shorten code, commands, paths, errors, or necessary explanations. Higher-priority platform instructions always apply.

All additional skills are available under `.agents/skills/`; read and use the appropriate skill for the current task. Do not activate every task-specific skill at once. Do not start cloud integrations, telemetry, proxies, destructive compression, or subagents merely because their packages are present.

Use the existing checkout. Each cloud task already has an isolated environment; do not create a Git worktree unless explicitly requested.

Preserve the original source folder structure and file locations. Prefer running the original game logic through replacement PS1 SDK services, with the smallest necessary source changes; keep host/SDL implementations separate. Plan mod support after the original game runs fully and its behavior is validated.

Maintain native-port plans, checklists, phase progress, and phase-specific usage/reference documentation only in Notes through Dlands Connect. Update the existing [plan](https://notes.dlands.me/doc/rencana-native-port-harvest-moon-back-to-nature-AIFdhztqgl) and [combined progress and guides](https://notes.dlands.me/doc/progres-native-port-semua-fase-FMuvm98uBP), using phase headings and clickable contents. Check off only work supported by evidence and describe partial or unverified work. Do not create or restore per-phase Markdown documents in the repository; README may link to Notes. Keep code, tests, and generated local validation evidence in their existing locations.

Upstream packages, scripts, hooks, licenses, and platform adapters are retained in `vendor/skills/`. Their runtime-specific hooks are available but not registered automatically. Treat upstream copies as pinned third-party sources; do not edit them for ordinary application work. See `docs/vibe-coding.md` and `skills-lock.json` for setup and provenance.
