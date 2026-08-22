# APP-022: Existing schemas are only partially validated

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Storage
- **Location:** [`src/Utils/Storage/SQLiteDatabase.cpp`](../../../../src/Utils/Storage/SQLiteDatabase.cpp), lines 5-51
- **Dependencies:** APP-001

## Observation

Initialization tests only whether `items` exists and whether it has `device_id`. A table with `device_id` but missing, renamed, or incorrectly typed remaining columns is accepted as initialized.

## Reasoning and impact

Subsequent statements fail at runtime, potentially after earlier startup work has succeeded. The code cannot distinguish a supported schema version from accidental or partially migrated state.

## Recommended improvement

Introduce explicit schema version metadata and validate the complete expected schema before use. Route each supported version through a transactional migration and reject unknown shapes without mutation.

## Acceptance criteria

- Initialization identifies exact supported schema versions.
- Missing, extra-critical, or incompatible columns produce a precise non-destructive error.
- Successful initialization guarantees all CRUD statements can prepare.

## Suggested tests

Create fixtures missing each required column, using incompatible types/constraints, and representing every supported version.
