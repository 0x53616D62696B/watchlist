# Application review verification coverage

This is the post-fix audit for the 52 findings originally recorded at `f94cf4f`. It replaces the static-review line map with current architecture and executable regression evidence.

## P0/P1 regression map

Every P0/P1 finding maps to at least one automated regression gate. GoogleTest case names below are discovered as individual bounded CTest cases.

| Findings | Regression evidence |
|---|---|
| APP-001 | `StorageInitializationTest.ExactUnversionedDeviceSchemaMigratesAndPreservesRows`, rollback and rejection cases in `SQLiteDatabaseTests` |
| APP-002 | `PathTest.UnicodePathReopensAndWorkerDoesNotOverwriteData`, `DeviceStorageServiceTests.InitialRefreshAndCommandsReturnPersistedSnapshot` |
| APP-003, APP-004 | `MoveTransfersObserverWithoutInvalidatingFrame`, `DiscardingObserverDoesNotDestroyScheduledFrame`, exactly-once completion tests |
| APP-005 | `EventLoopGeneratorTests.ImmediateDestructionCancelsAndJoinsProducer` |
| APP-006 | `AsyncEventLoopTests.SimultaneousLoopsKeepDelayRoutingIndependent` and `SimultaneousLoopsKeepEventRoutingIndependent` |
| APP-007 | `EventLoopCoroutineTests.SimultaneousLoopsKeepDelayRoutingIndependent` |
| APP-008 | `AsyncEventLoopTests.ShutdownCancelsEventWaiterAndLongDelayPromptly`, stopped-admission and peer-cancellation cases |
| APP-009 | `EventLoopCoroutineTests.DestructionCancelsLongDelayPromptly` and `StopRejectsNewWorkExplicitly` |
| APP-010 | `ApplicationTests.SubsystemFailureBecomesProcessFailure`, `DependencyExceptionIsCaughtAndLaterSubsystemsDoNotRun`, `GuiRuntimeTests.BackgroundFailureRequestsExitAndReturnsFailure` |
| APP-011, APP-012 | `GuiRuntimeTests.EveryInitializationFailureCleansCompletedStagesInReverse`, `NormalAndFrameFailureUseSameCompleteCleanupPath` |
| APP-013 | `ThreadPoolManagerTest.ExplicitZeroIsRejected`, `UnknownHardwareConcurrencyFallsBackToOneWorker` |
| APP-014 | `ThreadPoolManagerTest.ConstructionFailureJoinsEveryCreatedWorkerAndRethrows` |
| APP-015 | `ThreadPoolManagerTest.ThreadCountReportsActualFixedCapacity`, accepted-work drain and progress cases |
| APP-016 | `ApplicationReview.BuildContracts` plus the explicit `/WX` `Application` build |
| APP-018 | `ApplicationReview.BuildContracts` and configure/list-preset validation with no `CMakeUserPresets.json` |
| APP-019 | `VersionPipeline.missing`, `VersionPipeline.fallback`, and strict-mode cases |
| APP-022 | Exact schema, partial schema, extra object, and unknown version cases in `SQLiteDatabaseTests` |
| APP-023 | `SQLiteDatabaseTest.BoundaryPortsRoundTripExactly`, SQL constraint and checked-decoder cases |
| APP-025 | `EventLoopGeneratorTests.IdentifiersArePerLoopAndDoNotWrap`, `SchedulingRaceClassifiesEverySubmission` |
| APP-026 | `ShutdownDrainsAcceptedEventsAndRejectsLaterWork`, scheduling race, and stopped-sequence cases |
| APP-027 | `AsyncEventLoopTests.ImmediateWorkRunsExactlyOnceWithoutManualResume` |
| APP-032 | `DeviceMonitorStateTests.MaximumLengthDomainTextIsOwnedWithoutTruncation` |

## Remaining finding coverage

| Findings | Regression or audit surface |
|---|---|
| APP-017, APP-036, APP-045, APP-046 | Profiling-off/on builds, PCH-off/on builds, header compile probes, and warnings-as-errors compile commands |
| APP-020, APP-021, APP-047, APP-048 | Nine `VersionPipeline.*` cases, build-tree generated artifacts, and the linked Windows resource build |
| APP-024 | `ApplicationReview.BuildContracts` rejects MySQL re-entry into the application graph |
| APP-028–APP-031 | Headless `GuiRuntimeTests` for close, affinity, geometry/DPI, initialization failures, and diagnostics; opt-in `Application.GuiSmoke` for real platform integration |
| APP-033–APP-035 | `DeviceMonitorStateTests`, `GuiHeaderCompileProbe`, and `MyAppHeaderCompileProbe` |
| APP-037–APP-039 | `LoggerTests` concurrency/flush/termination cases and subprocess death test |
| APP-040–APP-044, APP-051 | SQLite const-query compile usage, API/SQL validation, transaction failures, checked decoding, and unique parallel fixtures |
| APP-049 | Coroutine and async callback exception-observation cases; generator producer/action failures |
| APP-050 | `ApplicationTests`, uniform CTest timeouts, opt-in labelled GUI smoke, ASan+UBSan preset, and this complete audit |
| APP-052 | `PathTest.PlatformDefaultsHaveStableIdentityAcrossWorkingDirectories` |

## Current owned test graph

- `UnitTests`: startup composition, paths, storage service, GUI lifecycle/state, async loops, and SQLite.
- `ThreadPoolManagerTests`: fixed-capacity construction, failure, progress, and shutdown.
- `GeneratorLifecycleTests`: producer/admission/ID/exception lifecycle.
- `LoggerUnitTests`: atomic records, flushing, and fatal termination.
- `ThreadPoolHeaderCompileProbe`, `GuiHeaderCompileProbe`, and `MyAppHeaderCompileProbe`: PCH-independent public-header contracts.
- `VersionPipeline.*`: override, GitVersion validation, fallback, strict provenance, escaping, and non-ASCII metadata.
- `ApplicationReview.BuildContracts`: explicit target ownership, tracked presets, build-tree version output, warning policy, and sanitizer policy.
- `Application.GuiSmoke`: opt-in `GUI_SMOKE` integration test requiring a real supported display/context.

All deterministic tests use `WATCHLIST_TEST_TIMEOUT_SECONDS` (20 seconds by default). The real GUI smoke test uses the same bound but is never enabled by default.
