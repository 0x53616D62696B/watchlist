# APP-005: Detached generator producer can use a destroyed loop

- **Priority:** P0
- **Kind:** Defect
- **Confidence:** High
- **Subsystem:** Concurrency
- **Status:** Resolved
- **Location:** [`src/Utils/Concurrency/EventLoopGenerator.hpp`](../../../../src/Utils/Concurrency/EventLoopGenerator.hpp), lines 134-147
- **Dependencies:** None

## Observation

`process_event_sequence` starts a detached thread that captures `this`. `EventLoopGenerator` neither owns nor joins that producer thread.

## Reasoning and impact

The loop can be destroyed while the generator is sleeping or producing its next event. The detached thread then calls `schedule_event` through a dangling pointer, causing use-after-free of the mutex, queue, and condition variable. Destruction of the worker thread does not protect the detached producer.

## Recommended improvement

Make producer threads owned lifecycle state, preferably `std::jthread` instances with stop tokens. Destruction must request producer cancellation, join all producers, then stop and join the consumer worker in a documented order.

## Implementation boundary

Fix ownership before refining shutdown admission (APP-026) or ID generation (APP-025).

## Acceptance criteria

- No thread that references the loop can outlive it.
- Destruction safely interrupts or drains an active generator.
- Repeated construction/destruction under load has no invalid accesses or leaked threads.

## Suggested tests

Destroy the loop immediately after starting long and short sequences, destroy it while `pattern_action` runs, and repeat under thread/lifetime sanitizers.

## Resolution

`EventLoopGenerator` now owns every sequence producer in a `std::jthread`. Shutdown closes admission, requests producer cancellation, joins every producer, and only then joins the queue worker after it drains accepted events. No detached thread retains the loop address.

## Validation

`EventLoopGeneratorTests.ImmediateDestructionCancelsAndJoinsProducer` repeatedly destroys loops immediately after starting long generators and asserts bounded completion. The complete focused test set was also repeated ten times without a failure.
