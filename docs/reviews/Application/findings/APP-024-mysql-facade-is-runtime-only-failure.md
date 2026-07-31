# APP-024: MySQL implementation compiles but every operation throws

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Storage
- **Location:** [`src/Utils/Storage/MySQLDatabase.cpp`](../../../../src/Utils/Storage/MySQLDatabase.cpp), lines 11-76; [`CMakeLists.txt`](../../../../CMakeLists.txt), lines 111-114
- **Dependencies:** APP-016

## Observation

`MySQLDatabase` satisfies the complete `IDatabase` type at compile time, stores connection settings, and is compiled into `Application`, but every database method throws `logic_error`.

## Reasoning and impact

Callers cannot determine capability without executing an operation. Dependency injection or configuration can select a type that appears valid and fails only in production runtime paths.

## Recommended improvement

Exclude the facade from production targets until implemented, or represent unavailable backends in factory/configuration capability checks. If retained as an example, place it in an explicit example target and namespace.

## Acceptance criteria

- Production configuration cannot construct an unsupported backend.
- Backend availability is discoverable before executing CRUD operations.
- Application target contains only usable storage implementations.

## Suggested tests

Test backend selection with available/unavailable clients and verify configuration fails before application startup when a requested backend is unsupported.

## Resolution

- **Status:** Resolved
- **Implementation:** Deleted the throwing `MySQLDatabase` facade and removed it from all target composition. Storage documentation now declares SQLite as the sole supported backend and requires a working client integration and explicit capability before another backend can be exposed.
- **Validation:** Repository and generated compile-command searches found no constructible or compiled MySQL backend. `Application` and the SQLite unit-test target built successfully, and all seven existing SQLite CTest cases passed.
