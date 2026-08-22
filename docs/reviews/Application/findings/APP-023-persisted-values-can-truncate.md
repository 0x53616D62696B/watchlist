# APP-023: Persisted port and boolean values are unconstrained

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Storage
- **Location:** [`src/Utils/Storage/SQLiteDatabase.cpp`](../../../../src/Utils/Storage/SQLiteDatabase.cpp), lines 42-50, 86-123, and 138-157
- **Dependencies:** APP-022, APP-041

## Observation

The schema accepts any integer for `port` and `alive`. Reads cast `port` directly to `std::uint16_t` and treat every nonzero `alive` value as true.

## Reasoning and impact

Rows written externally or by a future migration can silently wrap negative or oversized ports and normalize corrupt boolean values. The returned `DatabaseItem` no longer represents persisted data faithfully, and invalid state is hidden rather than diagnosed.

## Recommended improvement

Add schema checks for `port BETWEEN 0 AND 65535` and `alive IN (0,1)`, validate values while migrating, and reject invalid query results before narrowing conversions.

## Acceptance criteria

- Invalid numeric values cannot be inserted through SQL or the API.
- Existing invalid rows cause a clear migration/read error, not truncation.
- Boundary ports 0 and 65535 round-trip exactly.

## Suggested tests

Insert `-1`, `65536`, and non-boolean alive values through raw SQL and verify rejection or explicit migration handling; test both boundaries.
