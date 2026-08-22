# APP-039: `LOG_FATAL` has no defined fatal-behavior contract

- **Priority:** P2
- **Kind:** Improvement
- **Confidence:** High
- **Subsystem:** Logging/API
- **Location:** [`src/Utils/Logger/Logger.hpp`](../../../../src/Utils/Logger/Logger.hpp), lines 12-35; [`src/Utils/Logger/Logger.cpp`](../../../../src/Utils/Logger/Logger.cpp), lines 21-51
- **Dependencies:** APP-037

## Observation

`LOG_FATAL` is only a severity character passed to the same best-effort `Log` function as informational messages. Logging exceptions are swallowed and execution continues.

## Reasoning and impact

Call sites and operators may interpret "fatal" as an unrecoverable condition with guaranteed flush and termination/exception semantics, while some logging APIs use it as severity only. The project does not state which meaning applies, so callers cannot safely rely on either behavior.

## Recommended improvement

Either rename the level to avoid control-flow implications or define a dedicated fatal path with explicit flush and termination/exception policy. Do not hide the policy inside a variadic macro.

## Acceptance criteria

- Fatal semantics are documented and match runtime behavior.
- Required records are flushed before termination when termination is selected.
- Tests can observe the policy without killing the main test process.

## Suggested tests

Use a subprocess or injectable fatal handler to verify record emission, flush, and the selected exit/exception behavior.
