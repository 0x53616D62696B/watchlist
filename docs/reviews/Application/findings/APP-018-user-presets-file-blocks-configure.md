# APP-018: Missing local user presets blocks every configure path

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Build
- **Location:** [`CMakeLists.txt`](../../../../CMakeLists.txt), lines 3-9; [`CMakePresets.json`](../../../../CMakePresets.json), complete file
- **Dependencies:** None

## Observation

Top-level CMake fails unless ignored `CMakeUserPresets.json` exists. The tracked preset file exposes only hidden base presets, so a clean checkout has no usable tracked configure preset and even direct `cmake -S` is rejected.

## Reasoning and impact

Configuration policy is coupled to one developer-local file. CI, package builds, IDE import, and new contributors cannot configure from tracked inputs even when all actual tools and dependencies are available.

## Recommended improvement

Make tracked presets sufficient for a portable default and treat user presets as optional overrides. Validate required tools/paths through normal CMake discovery and actionable diagnostics rather than file existence.

## Acceptance criteria

- A clean checkout can configure with tracked inputs plus installed dependencies.
- User presets can override local toolchains without being mandatory.
- CI/direct configure paths do not fabricate a user file.

## Suggested tests

Configure with no user presets, with a minimal override, and with the example customized. Verify equivalent target selection and clear missing-tool messages.
