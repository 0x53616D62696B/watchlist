# Make logger records atomic and fatal logging terminal

## Summary

- serialize and synchronously flush complete logger records through one testable sink;
- remove the unusable and unused `LocalTime` public API;
- give fatal logging an explicit emit, flush, and `std::terminate` contract with test-only hooks;
- add deterministic concurrent-record, flush, direct-fatal, and default-termination tests;
- mark APP-037, APP-038, and APP-039 resolved with implementation and validation notes.

## Validation

- `cmake --build build/logger-harness --target LoggerUnitTests`
- `ctest --test-dir build/logger-harness -R Logger --output-on-failure` (5/5 passed)
- production-mode `Logger.cpp` smoke build and execution via `LoggerProductionSmoke`
- `git diff --check`
