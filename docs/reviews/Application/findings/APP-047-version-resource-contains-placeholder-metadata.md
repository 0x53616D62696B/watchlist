# APP-047: Version resource metadata is placeholder text

- **Status:** Resolved

- **Priority:** P3
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Build/versioning
- **Location:** [`cmake/GitVersionConfig.cmake`](../../../../cmake/GitVersionConfig.cmake), lines 173-185
- **Dependencies:** APP-021

## Observation

The generated Windows resource uses `Your Company`, a generic copyright string, the CMake project name, and `${PROJECT_VERSION}` rather than reviewed product metadata and the full version identity.

## Reasoning and impact

Once APP-021 links the resource, users and installers will see placeholder publisher/product information. File properties may also omit branch/build metadata intended for diagnostics.

## Recommended improvement

Define product/publisher/copyright/resource-version inputs centrally and validate them during release configuration. Keep display version and numeric Windows version semantics explicit.

## Acceptance criteria

- No placeholder metadata appears in shipped executable properties.
- Numeric and display versions follow documented release rules.
- Metadata can be supplied reproducibly by packaging.

## Suggested tests

Inspect a built executable's version resource and compare every field with configured release metadata.

## Resolution

The Windows resource template now uses product `Watchlist`, company `Patrik Maraczek`, description `Watchlist device monitor`, copyright `Copyright (C) 2026 Patrik Maraczek`, and original filename `Watchlist.exe`. Numeric Windows fields use validated major/minor/patch components, while file/product display versions use the full semantic version. The CMake target remains `Application` and emits `Watchlist.exe`.

## Validation

- Built `Watchlist.exe` with version `7.8.9-beta.1+fixture`.
- Queried every required Windows `VersionInfo` string and confirmed no placeholder publisher/product metadata remains.
