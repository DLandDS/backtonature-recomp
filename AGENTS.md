# Session instructions

At the start of every session in this repository, read and apply these embedded skills:

1. `.agents/skills/caveman/SKILL.md`: concise answers in the user's language; preserve technical accuracy and clarity.
2. `.agents/skills/ponytail/SKILL.md`: use the simplest correct implementation for coding tasks, default level `full`.
3. `.agents/skills/i-have-adhd/SKILL.md`: lead with the answer or next action, number multi-step work, suppress tangents, and make progress visible.

Keep these defaults active throughout the session unless the user changes or disables a mode. This is the repository owner's explicit always-on request, including for i-have-adhd. Do not announce mode activation each turn.

If style rules conflict, prioritize clarity and the user's instructions. ADHD-friendly structure takes precedence over compression; Ponytail governs implementation. Never shorten code, commands, paths, errors, or necessary explanations. Higher-priority platform instructions always apply.

All additional skills are available under `.agents/skills/`; read and use the appropriate skill for the current task. Do not activate every task-specific skill at once. Do not start cloud integrations, telemetry, proxies, destructive compression, or subagents merely because their packages are present.

Use the existing checkout. Each cloud task already has an isolated environment; do not create a Git worktree unless explicitly requested.

Upstream packages, scripts, hooks, licenses, and platform adapters are retained in `vendor/skills/`. Their runtime-specific hooks are available but not registered automatically. Treat upstream copies as pinned third-party sources; do not edit them for ordinary application work. See `docs/vibe-coding.md` and `skills-lock.json` for setup and provenance.
