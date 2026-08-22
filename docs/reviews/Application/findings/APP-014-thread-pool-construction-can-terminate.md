# APP-014: Partial worker construction can terminate the process

- **Priority:** P1
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Location:** [`src/Utils/Concurrency/ThreadPoolManager.hpp`](../../../../src/Utils/Concurrency/ThreadPoolManager.hpp), lines 111-119
- **Dependencies:** APP-013

## Observation

The constructor appends joinable `std::thread` objects directly to `workers`. If a later thread creation throws, the `ThreadPoolManager` destructor is not run, and destruction of already-created joinable threads invokes `std::terminate`.

## Reasoning and impact

Resource exhaustion or OS thread-creation failure turns a recoverable constructor exception into abrupt process termination.

## Recommended improvement

Use a construction guard that requests stop, notifies, and joins every successfully created worker before rethrowing. Publish the pool only after all requested workers exist.

## Acceptance criteria

- Failure creating worker N joins workers 0 through N-1.
- The original exception reaches the caller.
- No joinable thread object is destroyed during unwinding.

## Suggested tests

Inject a thread factory that fails at each creation index and verify join counts, exception propagation, and lack of termination.
