# APP-001: Initialization drops legacy data

- **Priority:** P0
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Storage
- **Location:** [`src/Utils/Storage/SQLiteDatabase.cpp`](../../../../src/Utils/Storage/SQLiteDatabase.cpp), lines 38-40
- **Dependencies:** None

## Observation

`SQLiteDatabase::Initialize` drops the complete `items` table whenever the table exists but lacks the `device_id` column. No backup, data copy, explicit migration version, or user confirmation is involved.

## Reasoning and impact

An older or merely unexpected schema is normal persisted state, not proof that its contents are disposable. Opening such a database causes irreversible row loss before the caller can inspect or recover it. A failed replacement `CREATE TABLE` would leave the database with neither old nor new data.

## Recommended improvement

Replace the destructive branch with a versioned migration transaction. Validate the recognized source schema, create the destination schema separately, copy and transform rows, verify the result, and only then replace the old table. Reject unknown schemas without modifying them.

## Implementation boundary

Change migration and schema-version behavior only; do not combine it with UI or MySQL work.

## Acceptance criteria

- Opening each supported legacy schema preserves all convertible rows.
- Unknown schemas fail without changing tables or data.
- Migration failure rolls back atomically.
- Tests compare pre- and post-migration content, not just column existence.

## Suggested tests

Create a legacy database fixture with representative rows, migrate it, and verify every field. Inject a duplicate/invalid row to verify rollback, and supply an unknown schema to verify non-destructive failure.
