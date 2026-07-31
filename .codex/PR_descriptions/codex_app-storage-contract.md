# Version and validate the SQLite storage contract

## Summary

- introduce exact schema v1 validation and a non-destructive transactional migration from the sole supported unversioned device schema;
- reject legacy, unknown, partial, future, and extra-object schemas without mutation;
- define structured domain validation and defensive SQLite constraints;
- centralize checked row decoding and prevalidate bulk mutations;
- replace timestamp fixtures with collision-safe RAII directories and expand storage coverage to 18 comprehensive tests;
- mark APP-001, APP-022, APP-023, APP-041, APP-043, APP-044, and APP-051 resolved.

## Validation

- MSVC `/W4 /WX` storage harness build succeeded.
- `ctest --test-dir build/storage-harness --output-on-failure` passed 18/18 tests.
- `git diff --check` passed.
