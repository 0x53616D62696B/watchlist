---
name: cmake-user-presets-sync
description: Keep watchlist CMake user preset files aligned. Use when Codex edits, reviews, renames, restructures, or adds settings to CMakeUserPresets.json in the watchlist repo so matching changes are also made to CMakeUserPresetsExample.json.
---

# CMake User Presets Sync

## Overview

Keep `CMakeUserPresets.json` and `CMakeUserPresetsExample.json` synchronized when changing user-facing CMake preset configuration in the watchlist repo.

## Workflow

1. Before editing, read both `CMakeUserPresets.json` and `CMakeUserPresetsExample.json`.
2. Apply the requested structural, preset, and setting changes to `CMakeUserPresets.json`.
3. Mirror the same relevant changes into `CMakeUserPresetsExample.json`.
4. Preserve local-only values where the example file intentionally uses placeholders or generic paths.
5. Avoid unrelated formatting churn so the diff shows only the intended preset changes.
6. After editing, inspect both diffs together to confirm the example remains an accurate tracked template.

## Review Checklist

- Confirm every added, removed, or renamed preset in `CMakeUserPresets.json` has the corresponding example-file treatment.
- Confirm inherited presets, cache variables, environment variables, binary directories, and configure/build/test preset relationships stay consistent.
- If only machine-local values changed and the example should not change, state that explicitly in the final response.
- Do not run CMake configure, builds, or tests solely for this skill unless the user asks.
