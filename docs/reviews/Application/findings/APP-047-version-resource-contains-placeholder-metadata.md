# APP-047: Version resource metadata is placeholder text

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
