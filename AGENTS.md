## Code Conventions

### Comment Discipline

- Default to self-documenting code. Prefer named helpers, constants, types, and small functions over explanatory comments.
- Do not add comments that restate code, narrate control flow, label obvious sections, or explain what a well-named symbol already says.
- Comments are allowed when they capture non-obvious **why**: hardware timing, protocol compatibility, datasheet constraints, race/lifetime assumptions, safety tradeoffs, or historical regression context.
- Keep comments concise and local. Aim for one line; use two only when the constraint genuinely needs it.
- If a comment wants to become a paragraph, first try to extract or rename code so the comment becomes unnecessary. Keep the paragraph only when the historical context is essential.
- When touching existing code, remove stale, redundant, or "what" comments in the edited area instead of preserving comment noise.

### Architecture Boundaries

- Treat `hardware/ESP32` and `hardware/windows_linux` as HAL/platform code. HAL files must not include `applicationInternal/...` headers or depend on application-layer internals.
- Put cross-layer constants or tiny pure helpers needed by HAL code in `src/shared`, or introduce an explicit lower-level interface. Do not pull application headers downward to share one value.
- Keep protobuf encode/decode and hub command semantics in the hub/application layer. ESP-NOW and WebSocket HAL APIs should describe generic byte/message transport unless the HAL behavior is truly protocol-specific.
- Keep MQTT, ESP-NOW, and WebSocket as valid first-class hub transports. Avoid fixes that only preserve one transport path unless the branch explicitly narrows scope.

### Control Flow

- Prefer guard clauses and early returns over nested conditionals. Keep the happy path flat and visible.
- When an error branch logs and exits, return immediately from that branch instead of wrapping the remaining work in an `else`.

<!-- gitnexus:start -->
# GitNexus — Code Intelligence

This project is indexed by GitNexus as **OMOTE_FORK** (3436 symbols, 5518 relationships, 147 execution flows). Use the GitNexus MCP tools to understand code, assess impact, and navigate safely.

> Index stale? Run `node .gitnexus/run.cjs analyze` from the project root — it auto-selects an available runner. No `.gitnexus/run.cjs` yet? `npx gitnexus analyze` (npm 11 crash → `npm i -g gitnexus`; #1939).

## Always Do

- **MUST run impact analysis before editing any symbol.** Before modifying a function, class, or method, run `impact({target: "symbolName", direction: "upstream"})` and report the blast radius (direct callers, affected processes, risk level) to the user.
- **MUST run `detect_changes()` before committing** to verify your changes only affect expected symbols and execution flows. For regression review, compare against the default branch: `detect_changes({scope: "compare", base_ref: "main"})`.
- **MUST warn the user** if impact analysis returns HIGH or CRITICAL risk before proceeding with edits.
- When exploring unfamiliar code, use `query({query: "concept"})` to find execution flows instead of grepping. It returns process-grouped results ranked by relevance.
- When you need full context on a specific symbol — callers, callees, which execution flows it participates in — use `context({name: "symbolName"})`.

## Never Do

- NEVER edit a function, class, or method without first running `impact` on it.
- NEVER ignore HIGH or CRITICAL risk warnings from impact analysis.
- NEVER rename symbols with find-and-replace — use `rename` which understands the call graph.
- NEVER commit changes without running `detect_changes()` to check affected scope.

## Resources

| Resource | Use for |
|----------|---------|
| `gitnexus://repo/OMOTE_FORK/context` | Codebase overview, check index freshness |
| `gitnexus://repo/OMOTE_FORK/clusters` | All functional areas |
| `gitnexus://repo/OMOTE_FORK/processes` | All execution flows |
| `gitnexus://repo/OMOTE_FORK/process/{name}` | Step-by-step execution trace |

## CLI

| Task | Read this skill file |
|------|---------------------|
| Understand architecture / "How does X work?" | `.claude/skills/gitnexus/gitnexus-exploring/SKILL.md` |
| Blast radius / "What breaks if I change X?" | `.claude/skills/gitnexus/gitnexus-impact-analysis/SKILL.md` |
| Trace bugs / "Why is X failing?" | `.claude/skills/gitnexus/gitnexus-debugging/SKILL.md` |
| Rename / extract / split / refactor | `.claude/skills/gitnexus/gitnexus-refactoring/SKILL.md` |
| Tools, resources, schema reference | `.claude/skills/gitnexus/gitnexus-guide/SKILL.md` |
| Index, status, clean, wiki CLI commands | `.claude/skills/gitnexus/gitnexus-cli/SKILL.md` |

<!-- gitnexus:end -->
