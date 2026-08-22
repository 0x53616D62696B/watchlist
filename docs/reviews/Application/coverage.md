# Application review coverage

This audit maps every in-scope project-owned build region, type, and free function to its finding IDs. “No finding” means the unit was inspected and no independently actionable change was identified at this revision; it is not a permanent approval of future changes.

Third-party implementation files under `libs/**` were not opened or reviewed. CMake statements that attach/link those dependencies were reviewed only as Application integration surfaces.

## File inventory

Every in-scope file is listed here; the later tables provide code-unit detail.

| In-scope file | Detailed coverage |
|---|---|
| [`CMakeLists.txt`](../../../CMakeLists.txt) | Build, version, and tooling |
| [`CMakePresets.json`](../../../CMakePresets.json) | Build, version, and tooling |
| [`CMakeUserPresetsExample.json`](../../../CMakeUserPresetsExample.json) | Build, version, and tooling |
| [`GitVersion.yml`](../../../GitVersion.yml) | Build, version, and tooling |
| [`cmake/CompilerSettings.cmake`](../../../cmake/CompilerSettings.cmake) | Build, version, and tooling |
| [`cmake/GitVersionConfig.cmake`](../../../cmake/GitVersionConfig.cmake) | Build, version, and tooling |
| [`.vscode/settings.json`](../../../.vscode/settings.json) | Build, version, and tooling |
| [`.vscode/launch.json`](../../../.vscode/launch.json) | Build, version, and tooling |
| [`.vscode/tasks.json`](../../../.vscode/tasks.json) | Build, version, and tooling |
| [`src/Precompiled.hpp`](../../../src/Precompiled.hpp) | Build, version, and tooling |
| [`src/Watchlist/main.cpp`](../../../src/Watchlist/main.cpp) | Startup and Watchlist worker |
| [`src/Watchlist/SQLiteThreadWorker.hpp`](../../../src/Watchlist/SQLiteThreadWorker.hpp) | Startup and Watchlist worker |
| [`src/Watchlist/SQLiteThreadWorker.cpp`](../../../src/Watchlist/SQLiteThreadWorker.cpp) | Startup and Watchlist worker |
| [`src/Gui/Gui.hpp`](../../../src/Gui/Gui.hpp) | GUI platform/bootstrap |
| [`src/Gui/Gui.cpp`](../../../src/Gui/Gui.cpp) | GUI platform/bootstrap |
| [`src/Gui/MyApp.hpp`](../../../src/Gui/MyApp.hpp) | Application UI |
| [`src/Gui/MyApp.cpp`](../../../src/Gui/MyApp.cpp) | Application UI |
| [`src/Utils/Concurrency/ThreadPoolManager.hpp`](../../../src/Utils/Concurrency/ThreadPoolManager.hpp) | Thread pool |
| [`src/Utils/Concurrency/AsyncEventLoop.hpp`](../../../src/Utils/Concurrency/AsyncEventLoop.hpp) | AsyncEventLoop |
| [`src/Utils/Concurrency/EventLoopCoroutine.hpp`](../../../src/Utils/Concurrency/EventLoopCoroutine.hpp) | EventLoopCoroutine |
| [`src/Utils/Concurrency/EventLoopGenerator.hpp`](../../../src/Utils/Concurrency/EventLoopGenerator.hpp) | EventLoopGenerator |
| [`src/Utils/Logger/Logger.hpp`](../../../src/Utils/Logger/Logger.hpp) | Logging and profiling |
| [`src/Utils/Logger/Logger.cpp`](../../../src/Utils/Logger/Logger.cpp) | Logging and profiling |
| [`src/Utils/Profiling/TracyProfiling.hpp`](../../../src/Utils/Profiling/TracyProfiling.hpp) | Logging and profiling |
| [`src/Utils/Storage/DatabaseItem.hpp`](../../../src/Utils/Storage/DatabaseItem.hpp) | Storage |
| [`src/Utils/Storage/IDatabase.hpp`](../../../src/Utils/Storage/IDatabase.hpp) | Storage |
| [`src/Utils/Storage/SQLiteDatabase.hpp`](../../../src/Utils/Storage/SQLiteDatabase.hpp) | Storage |
| [`src/Utils/Storage/SQLiteDatabase.cpp`](../../../src/Utils/Storage/SQLiteDatabase.cpp) | Storage |
| [`src/Utils/Storage/MySQLDatabase.hpp`](../../../src/Utils/Storage/MySQLDatabase.hpp) | Storage |
| [`src/Utils/Storage/MySQLDatabase.cpp`](../../../src/Utils/Storage/MySQLDatabase.cpp) | Storage |
| [`tests/UnitTests/Storage/SQLiteDatabaseTests.cpp`](../../../tests/UnitTests/Storage/SQLiteDatabaseTests.cpp) | Tests |

## Build, version, and tooling

| File/region or code unit | Review result |
|---|---|
| `CMakeLists.txt:3-9` local-preset gate | APP-018 |
| `CMakeLists.txt:11-23` project/module/version setup | APP-019, APP-020, APP-021 |
| `CMakeLists.txt:25-65` project options and configuration setup | No finding beyond APP-045 for inherited flags |
| `CMakeLists.txt:67-88` SQLite/GTest dependency selection | Reviewed as integration only; no library content reviewed; no finding |
| `CMakeLists.txt:90-102` Application declaration and PCH | APP-034, APP-036; otherwise no finding |
| `CMakeLists.txt:111-119` Utils/GUI source selection | APP-016 |
| `CMakeLists.txt:121-138` ImGui/GLFW/GLAD integration | APP-016 (implicit source/dependency breadth); otherwise no finding at the integration boundary |
| `CMakeLists.txt:140-147` Tracy integration | APP-017 |
| `CMakeLists.txt:149-165` Application SQLite link and Watchlist sources | No finding beyond APP-016 |
| `CMakeLists.txt:166-168` `MYAPP` definition | No finding |
| `CMakeLists.txt:185-201` SQLite test target | APP-043, APP-050 |
| `cmake/CompilerSettings.cmake` C++ standard/PCH-independent settings | APP-045, APP-046 |
| `cmake/CompilerSettings.cmake` ccache discovery | No finding |
| `configure_version()` | APP-019, APP-048 |
| `generate_version_header()` | APP-020, APP-048 |
| `generate_version_resource()` | APP-021, APP-047 |
| `GitVersion.yml` active version policy | Reviewed; no finding |
| `GitVersionExperimental.yml` | Excluded: not consumed by Application configuration |
| `CMakePresets.json` hidden base presets | APP-018 |
| `CMakeUserPresetsExample.json` local overrides/tool paths | APP-018; otherwise no finding because it is explicitly a user-local template |
| `.vscode/settings.json` CMake/PCH/editor integration | Reviewed; no finding |
| `.vscode/launch.json` Application and Tracy launch entries | Reviewed; no finding |
| `.vscode/tasks.json` Application configure/build tasks | Reviewed; no finding |
| `src/Common/Version.hpp` | Excluded generated output; generator reviewed instead |
| `src/Common/VersionExample.hpp` | Excluded: not consumed by Application |
| `src/Precompiled.hpp` | APP-034, APP-036; otherwise no finding |

## Startup and Watchlist worker

| Code unit | Review result |
|---|---|
| `Watchlist::HasArgument` | Reviewed; no finding |
| `Watchlist::WaitForTracyIfRequested` | Reviewed; no finding |
| `Watchlist::RunApplication` composition and top-level catches | APP-010, APP-029, APP-050 |
| global `main` | Reviewed; no finding beyond APP-010 |
| `Watchlist::run_sqlitecpp_thread_worker` declaration | Reviewed; no finding |
| `Watchlist::run_sqlitecpp_thread_worker` database selection/initialization | APP-002, APP-052 |
| `Watchlist::run_sqlitecpp_thread_worker` seed/update/read behavior | APP-002 |

## GUI platform/bootstrap

| Code unit | Review result |
|---|---|
| `GLFWInitialize` | APP-031; cleanup responsibility covered by APP-011 |
| `GLFWCreateWindow` | APP-011, APP-030 |
| `GLFWInitializeGL` | APP-012 |
| `GLFWSetWindowCallback` | Reviewed; no finding |
| `ImGuiSetStyle` | Reviewed; no finding |
| `ImGuiInitialize` | APP-011, APP-030 |
| `ImGuiNewFrame` | APP-029; otherwise no finding |
| `GLFWCloseWindow` | Reviewed; no finding |
| `ImGuiRender` | APP-029; otherwise no finding |
| `ImGuiShutdown` | APP-011 |
| `GLFWShutdown` | APP-011 |
| `KeyCallback` | Reviewed; no finding |
| `ImGuiStart` | APP-010, APP-011, APP-028, APP-029, APP-030, APP-050 |
| `Gui.hpp` declarations and `WINDOWTITLE` selection | APP-034; otherwise no finding |

## Application UI

| Code unit | Review result |
|---|---|
| `MyApp::WindowAppsFlags` | APP-033 |
| `MyApp::windowTitle` | APP-034; otherwise no finding |
| `MyApp::ShowAppMenuBar` | APP-033, APP-035 |
| `MyApp::ShowMenuFile` | APP-033, APP-035 |
| `MyApp::ShowWindow` | APP-028, APP-033 |
| `MyApp::MyDocument` state and simple state-transition methods | APP-033 |
| `MyApp::MyDocument::DisplayContents` | APP-033 |
| `MyApp::MyDocument::DisplayContextMenu` | APP-032, APP-033 |
| `MyApp::AppDocuments` and constructor | APP-033 |
| `MyApp::ShowAppDocuments` | APP-033 |
| `MyApp::NotifyOfDocumentsClosedElsewhere` | APP-033, APP-035 |
| `MyApp::ShowDockingDisabledMessage` | APP-035; otherwise no finding |
| Public/private declaration boundary in `MyApp.hpp` | APP-034, APP-035 |

## Thread pool

| Code unit | Review result |
|---|---|
| `Concurrency::ThreadPoolManager` type/dependency surface | APP-036 |
| constructor | APP-013, APP-014 |
| destructor | Reviewed with enqueue/shutdown contract; no additional finding |
| `enqueue` | APP-013, APP-036; otherwise no finding |
| `setMaxThreads` | APP-015 |
| `getMaxThreads` | APP-015 |
| `workerThread` | Reviewed; no finding beyond APP-013 through APP-015 |

## AsyncEventLoop

| Code unit | Review result |
|---|---|
| `AsyncEventLoop::Event` and `get_data` | Reviewed; no finding |
| `AsyncEventLoop::Task::promise_type` | APP-003, APP-049 |
| `AsyncEventLoop::Task` construction/destruction/move/resume | APP-003 |
| `AsyncEventLoop::Generator<T>::promise_type` | Reviewed; no finding |
| `AsyncEventLoop::Generator<T>` lifetime/iteration API | Reviewed; no finding |
| `AsyncEventLoop::Delay` | APP-006, APP-008 |
| `AsyncEventLoop::EventAwaiter` | APP-006, APP-008, APP-049 |
| `AsyncEventLoop` constructor | APP-006 |
| `AsyncEventLoop` destructor | APP-008 |
| `schedule_after` | APP-003, APP-006, APP-008, APP-049 |
| `schedule` | APP-027 |
| `create_event_stream` | Reviewed; no finding |
| `wait_for_event` | APP-003, APP-006, APP-008 |
| `process_events` | APP-003, APP-006, APP-008 |
| `emit_event` | APP-006, APP-008; otherwise no finding |
| worker `run` | APP-008 |
| `DelayedTask` and `EventWaiter` | APP-008 |

## EventLoopCoroutine

| Code unit | Review result |
|---|---|
| `EventLoopCoroutine::Task::promise_type` | APP-004, APP-049 |
| `EventLoopCoroutine::Task` construction/destruction/move | APP-004 |
| `EventLoopCoroutine::Delay` | APP-007, APP-009 |
| `EventLoopCoroutine` constructor | APP-007 |
| `EventLoopCoroutine` destructor | APP-009 |
| `schedule_after` | APP-004, APP-007, APP-009, APP-049 |
| worker `run` | APP-009 |
| `DelayedTask` | APP-009 |

## EventLoopGenerator

| Code unit | Review result |
|---|---|
| `EventLoopGenerator::Event` | Reviewed; no finding |
| constructor | Reviewed; no finding beyond lifecycle findings |
| destructor | APP-005, APP-026 |
| `schedule_event` | APP-025, APP-026 |
| `event_generator` | Reviewed; no independent finding |
| `process_event_sequence` | APP-005, APP-026 |
| worker `run` | APP-005, APP-026 |

## Logging and profiling

| Code unit | Review result |
|---|---|
| `LogLevel` | APP-039; otherwise no finding |
| `LocalTime` | APP-038 |
| `ToString(std::source_location)` | Reviewed; no finding |
| `Log` | APP-037, APP-039 |
| `LOG_*` and `LOG_EXCEPTION` macros | APP-039; otherwise no finding |
| Profiling-on macro mappings | APP-017; otherwise no finding |
| Profiling-off empty macro mappings | Reviewed; no finding |

## Storage

| Code unit | Review result |
|---|---|
| `Utils::Storage::DatabaseItem` | APP-041 |
| `Utils::Storage::IDatabase` lifecycle/mutation contract | APP-041; otherwise no finding |
| `IDatabase` query contract | APP-040 |
| `TableExists` | Reviewed; no finding |
| `TableHasColumn` | APP-001, APP-022 |
| `SQLiteDatabase` constructor | APP-042 |
| `SQLiteDatabase::Initialize` | APP-001, APP-022, APP-023 |
| `SQLiteDatabase::AddItem` | APP-023, APP-041 |
| `SQLiteDatabase::UpsertItem` | APP-023, APP-041 |
| `SQLiteDatabase::GetItem` | APP-023, APP-040, APP-051 |
| `SQLiteDatabase::GetAllItems` | APP-023, APP-040, APP-051 |
| `SQLiteDatabase::ReplaceAll` | APP-002, APP-043 |
| `SQLiteDatabase::GetAllSortedById` | APP-023, APP-040, APP-051 |
| `SQLiteDatabase::ContainsItem` | APP-040 |
| `SQLiteDatabase::CountItems` | APP-040 |
| `SQLiteDatabase::RemoveItem` | Reviewed; no finding |
| `SQLiteDatabase::Clear` | Reviewed; no finding |
| `MySQLConnectionSettings` | APP-024 |
| `MySQLDatabase` constructor and CRUD methods | APP-024 |
| `MySQLDatabase::ThrowMissingClient` | APP-024 |

## Tests

| Code unit | Review result |
|---|---|
| `SQLiteDatabaseTest::SetUp` | APP-044 |
| `SQLiteDatabaseTest::TearDown` | APP-044 |
| `Router`/`Printer` fixtures | Reviewed; no finding |
| `AddItemStoresNewDevice` | APP-043 (coverage gap context); test itself has no finding |
| `GetItemReturnsEmptyOptionalForMissingId` | Reviewed; no finding |
| `UpsertItemAddsAndUpdatesDevice` | Reviewed; no finding |
| `ReplaceAllClearsOldDevicesAndAddsNewDevices` | APP-043 (missing rollback counterpart) |
| `GetAllSortedByIdReturnsAscendingIds` | Reviewed; no finding |
| `RemoveItemDeletesExistingDevice` | Reviewed; no finding |
| `ClearRemovesAllDevices` | Reviewed; no finding |
| Missing startup/GUI/concurrency test targets and cases | APP-050 |

## Explicit exclusions

| Path/target | Reason |
|---|---|
| `libs/**` | Third-party library content expressly excluded |
| `development/**` and target `Development` | Development target expressly excluded |
| `src/Examples/**`, `MultithreadDemo`, `ConcurrencyExamples` | Example/standalone targets, not Application behavior |
| standalone `Logger` executable behavior under `DEBUG_LOGGER` | Unrelated executable target; shared logger implementation was reviewed |
| prose under `docs/**` outside this review | Not selected in the agreed “Application + support” boundary |
