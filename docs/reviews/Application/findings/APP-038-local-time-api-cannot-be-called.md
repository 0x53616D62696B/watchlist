# APP-038: Deduced-return `LocalTime` declaration is unusable by callers

- **Priority:** P2
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Logging/API
- **Location:** [`src/Utils/Logger/Logger.hpp`](../../../../src/Utils/Logger/Logger.hpp), line 22; [`src/Utils/Logger/Logger.cpp`](../../../../src/Utils/Logger/Logger.cpp), lines 3-8
- **Dependencies:** None

## Observation

The header declares `auto LocalTime(...)` without a trailing return type, while the return type is deduced only in the `.cpp` definition.

## Reasoning and impact

C++ callers cannot use a function with a placeholder return type until they have seen its definition. Translation units including only the header therefore cannot call the advertised API.

## Recommended improvement

Remove the unused helper, make it private to the implementation, or give it an explicit stable return type supported by the selected standard library.

## Acceptance criteria

- Every externally declared logger function is callable from a header-only consumer.
- Time-zone lookup failure has an explicit behavior.
- Unused API is removed rather than preserved speculatively.

## Suggested tests

Compile and link a separate translation unit that invokes every public logger function; include a time-zone-unavailable error case if `LocalTime` remains public.
