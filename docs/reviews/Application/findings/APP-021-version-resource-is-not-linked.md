# APP-021: Generated Windows version resource is unused

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Build/versioning
- **Location:** [`cmake/GitVersionConfig.cmake`](../../../../cmake/GitVersionConfig.cmake), lines 160-197; [`CMakeLists.txt`](../../../../CMakeLists.txt), lines 23 and 91-92
- **Dependencies:** APP-019, APP-020

## Observation

`generate_version_resource` writes `version.rc` and exports `VERSION_RC_FILE`, but the file is never attached to `Application` with `target_sources`.

## Reasoning and impact

Windows builds pay generation cost and log success while the executable does not receive the promised file/product version metadata. This gives a false signal that packaging metadata is complete.

## Recommended improvement

Generate the resource in the build tree and add it to `Application` on Windows, or remove the function until packaging uses it.

## Acceptance criteria

- The generated resource is either absent by design or compiled into the executable.
- Windows file properties match the configured version.
- Non-Windows configuration does not reference the resource.

## Suggested tests

Inspect target sources at configure time and query built executable version properties in a Windows integration test.
