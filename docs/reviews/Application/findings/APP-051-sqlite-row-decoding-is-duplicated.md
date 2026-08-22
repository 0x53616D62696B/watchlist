# APP-051: SQLite row decoding is duplicated

- **Priority:** P3
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Storage/maintainability
- **Location:** [`src/Utils/Storage/SQLiteDatabase.cpp`](../../../../src/Utils/Storage/SQLiteDatabase.cpp), lines 86-123 and 138-157
- **Dependencies:** APP-023

## Observation

`GetItem`, `GetAllItems`, and `GetAllSortedById` repeat the same five-column SELECT list and positional conversion into `DatabaseItem`.

## Reasoning and impact

Schema or validation changes must be copied into three paths. A changed column order or new invariant can silently behave differently between single-row, unsorted, and sorted queries.

## Recommended improvement

Centralize the canonical projection and row decoder, including checked numeric conversion from APP-023. Keep ordering clauses separate from decoding.

## Acceptance criteria

- All query paths share one field mapping and validation implementation.
- Adding/reordering a stored field requires one decoder change.
- Existing ordering and optional-result behavior remain unchanged.

## Suggested tests

Run identical representative and invalid rows through every query path and assert identical decoded values/errors.
