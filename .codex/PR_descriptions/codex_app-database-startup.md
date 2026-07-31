# Make database startup stable and non-destructive

## Summary

- resolve platform user-data defaults or `--database-path <file>`;
- normalize/create parents and fail without CWD fallback;
- preserve filesystem paths through SQLiteCpp;
- remove production seeding and add resolver/Unicode/restart tests;
- resolve APP-002, APP-042, and APP-052.

## Validation

- MSVC `/W4 /WX` focused build passed.
- path/storage integration selection passed 14/14 tests.
- `git diff --check` passed.
