# APP-043: SQLite tests omit destructive and transactional cases

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Tests/storage
- **Location:** [`tests/UnitTests/Storage/SQLiteDatabaseTests.cpp`](../../../../tests/UnitTests/Storage/SQLiteDatabaseTests.cpp), lines 43-127
- **Dependencies:** APP-001, APP-022, APP-023, APP-041
- **Status:** Resolved

## Observation

Existing tests cover successful CRUD, sorting, replace, remove, and clear behavior. They do not cover duplicate insertion, transaction rollback, invalid persisted values, legacy/unknown schemas, or initialization misuse.

## Reasoning and impact

The riskiest storage behavior is schema evolution and failure atomicity, yet the suite would pass while initialization deletes data or `ReplaceAll` leaves unexpected state after an insert error.

## Recommended improvement

Turn the storage contract into a reusable test suite and add direct SQLite fixtures for schema/migration behavior and raw invalid rows. Assert database contents after every failure.

## Acceptance criteria

- Every supported migration and rejection path is tested.
- Duplicate/invalid writes prove transactional rollback.
- Tests distinguish validation, constraint, and storage exceptions.

## Suggested tests

Cover duplicate `AddItem`, duplicate IDs inside `ReplaceAll`, unknown/partial legacy schemas, invalid numeric rows, operations before `Initialize`, and database reopen persistence.

## Resolution

The SQLite suite now exercises the storage contract's success, constraint, validation, schema, migration, corruption, and transactional failure paths. Each rejected mutation or migration asserts the pre-existing logical contents and schema version remain unchanged.

## Validation

Eighteen tests cover duplicate add, prevalidated and injected transactional replacement failure, invalid raw rows, supported migration, rollback, unknown/legacy/future schemas, exact schema checks, operations before initialization, persistence/reopen, and CRUD/query behavior.
