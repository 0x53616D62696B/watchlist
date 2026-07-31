# APP-019: GitVersion is an unconditional configure dependency

- **Status:** Resolved

- **Priority:** P1
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Build/versioning
- **Location:** [`cmake/GitVersionConfig.cmake`](../../../../cmake/GitVersionConfig.cmake), lines 10-23; [`CMakeLists.txt`](../../../../CMakeLists.txt), lines 19-23
- **Dependencies:** APP-018

## Observation

Every configure invokes `configure_version`, which issues `FATAL_ERROR` when GitVersion is absent. There is no release-metadata override, archive fallback, or development default.

## Reasoning and impact

Building C++ sources requires an additional host executable and a suitable Git checkout. Source archives, constrained CI, offline onboarding, and packaging fail before compilation even though the application only needs version constants.

## Recommended improvement

Support an explicit version cache variable for packaging, use GitVersion when available, and provide a clearly marked development fallback when policy permits. Reserve strict failure for release workflows that require provenance.

## Acceptance criteria

- Offline/archive builds can provide or derive a deterministic version.
- Release builds can enforce GitVersion and clean metadata explicitly.
- The selected version source is visible in configure output.

## Suggested tests

Configure with GitVersion present, absent with an override, absent in development mode, and required in release mode.

## Resolution

Version resolution now follows `WATCHLIST_VERSION_OVERRIDE`, validated GitVersion JSON, then the deterministic `0.0.0-dev+unversioned` development fallback. Configure reports the selected source. `WATCHLIST_REQUIRE_GITVERSION` rejects overrides/fallbacks, while `WATCHLIST_REQUIRE_CLEAN_PROVENANCE` additionally rejects GitVersion output with uncommitted changes.

## Validation

- Configured successfully from an explicit archive version and from the installed GitVersion executable.
- Exercised the development fallback with GitVersion discovery disabled.
- Verified strict GitVersion and clean-provenance modes fail with actionable diagnostics.
