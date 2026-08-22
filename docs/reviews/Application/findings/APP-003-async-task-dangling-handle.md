# APP-003: `AsyncEventLoop::Task` retains a destroyed coroutine handle

- **Priority:** P0
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Location:** [`src/Utils/Concurrency/AsyncEventLoop.hpp`](../../../../src/Utils/Concurrency/AsyncEventLoop.hpp), lines 97-145
- **Dependencies:** None

## Observation

`promise_type::final_suspend` returns `std::suspend_never`, so a completed coroutine destroys its own frame. `Task` still stores the handle and its destructor and move assignment call `handle_.done()` on that now-invalid handle.

## Reasoning and impact

A coroutine handle is non-owning once its frame has been destroyed. Querying or destroying that handle is undefined behavior. The current startup also allows the task object and worker resume to race, making the lifetime failure timing-dependent.

## Recommended improvement

Define one ownership model and encode it consistently. For an owning task, use a suspending final state and let `Task` destroy the frame exactly once. For detached work, transfer ownership to an explicit scheduler object and clear the user-visible handle. Synchronize destruction with resume.

## Implementation boundary

Correct `Task`/promise ownership before changing event routing or shutdown logic.

## Acceptance criteria

- Every coroutine frame has exactly one owner until destruction.
- Destroying, moving, or completing a task never touches an invalid handle.
- Destruction concurrent with scheduler activity is either prevented by contract or synchronized.
- Sanitizer-enabled lifetime tests complete without invalid access.

## Suggested tests

Cover completion before task destruction, task destruction while suspended, move construction/assignment, multiple resumes, and loop destruction with outstanding tasks.
