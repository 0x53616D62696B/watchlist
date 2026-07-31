# APP-044: Test database naming and cleanup are fragile

- **Priority:** P3
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Tests/storage
- **Location:** [`tests/UnitTests/Storage/SQLiteDatabaseTests.cpp`](../../../../tests/UnitTests/Storage/SQLiteDatabaseTests.cpp), lines 11-31
- **Dependencies:** None
- **Status:** Resolved

## Observation

Test filenames use only a `steady_clock` tick value. Parallel processes can select the same path. `TearDown` calls throwing `filesystem::remove` without checking errors or preserving the primary test failure.

## Reasoning and impact

Collisions create cross-test contamination and flaky constraint failures. Cleanup exceptions can obscure the assertion that originally failed and can leave files behind without diagnostics.

## Recommended improvement

Use a per-test temporary directory/name with collision-resistant creation and an RAII cleanup guard using `error_code`. Report cleanup failures without masking an active test failure.

## Acceptance criteria

- Parallel test processes never share a database path.
- Cleanup runs after setup/test exceptions.
- Cleanup errors are visible and do not replace the primary failure.

## Suggested tests

Run many test binaries concurrently, simulate a locked file during cleanup, and verify unique paths plus deterministic diagnostics.

## Resolution

Every fixture owns an atomically created, random 128-bit-plus-sequence temporary directory. Creation retries only collisions, the database connection is destroyed before directory cleanup, and the RAII destructor uses non-throwing `error_code` cleanup with a GoogleTest failure diagnostic rather than masking the active exception.

## Validation

A parallel test creates 32 fixture directories concurrently and verifies every path is distinct and present. All storage tests exercise RAII cleanup on normal and exception paths.
