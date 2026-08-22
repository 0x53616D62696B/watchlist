# APP-040: Read-only database APIs are not const-correct

- **Priority:** P3
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Storage/API
- **Location:** [`src/Utils/Storage/IDatabase.hpp`](../../../../src/Utils/Storage/IDatabase.hpp), lines 24-43 and 48-52
- **Dependencies:** None

## Observation

`GetItem`, `GetAllItems`, `GetAllSortedById`, `ContainsItem`, and `CountItems` are logically read-only but are not `const` in the interface or implementations.

## Reasoning and impact

Consumers cannot query through a const database reference, and the interface does not distinguish logical mutation from observation. This complicates dependency contracts and tests.

## Recommended improvement

Mark logical queries `const` if the underlying client permits it, using internal mutable synchronization/cache state only where justified. If SQLiteCpp prevents const access, document and encapsulate that adapter constraint.

## Acceptance criteria

- Read-only use is expressible through `const IDatabase&`.
- Implementations preserve thread-safety and transaction semantics.
- Mutating operations remain visibly distinct.

## Suggested tests

Compile a const-interface consumer that invokes every query and run the existing query behavior tests through both mutable and const references.
