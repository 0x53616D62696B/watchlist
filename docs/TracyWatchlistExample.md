# Tracy Watchlist Example

This project has a Tracy-enabled Watchlist example in the normal `Application` target.
When profiling is enabled, the app emits zones from:

- `src/Watchlist/main.cpp`: application startup and the current concurrency example.
- `src/Utils/Concurrency/AsyncEventLoop.hpp`: event loop setup, worker thread, generated events, event emission, and scheduled task bodies.
- `src/Gui/Gui.cpp`: GLFW/ImGui setup, per-frame UI work, rendering, and `FrameMark`.
- `src/Gui/MyApp.cpp`: Watchlist/CookBook windows, menu code, document tabs, and document contents.

The same source files still compile without Tracy. The profiling macros become empty unless `ENABLE_PROFILING` is enabled.

## Build The Example

Configure and build with the profiling preset:

```powershell
cmake --preset with-profiling
cmake --build --preset with-profiling --target Application
```

For short-lived runs where you want the process to stay alive for Tracy, use:

```powershell
cmake --preset with-profiling-no-exit
cmake --build --preset with-profiling-no-exit --target Application
```

The preset uses:

- `CMAKE_BUILD_TYPE=DebugTracy`
- `ENABLE_PROFILING=ON`
- `Tracy::TracyClient` linked into `Application`

The `with-profiling-no-exit` preset also sets `TRACY_NO_EXIT=ON` for Tracy.

## Run With Tracy

1. Start the Tracy profiler UI. If you already have a downloaded Tracy release, run its `Tracy.exe`.
2. Start the Watchlist app:

   ```powershell
   .\build\profiling\Application.exe
   ```

   Or, if you built the NO_EXIT preset:

   ```powershell
   .\build\profiling-no-exit\Application.exe
   ```

3. In Tracy, connect to the running process.
4. Look for these zones:

   - `main`
   - `ConcurrencyExamples`
   - `example_async_eloop`
   - `Async event loop` thread
   - `emit_event`
   - `process_events`
   - `ProcessGeneratedEvent`

The current `main.cpp` returns after the concurrency startup example, so the GUI zones are compiled in but are only visible after the early testing return is removed or the app is changed to call `ImGuiStart()`.

## VS Code CMake Tools

When using the CMake extension:

1. Select configure preset `with-profiling-no-exit`.
2. Select build preset `with-profiling-no-exit`.
3. Select launch target `Application`.
4. Run the target from CMake Tools.
5. Connect Tracy to `127.0.0.1:8086`.

The `.vscode/launch.json` configs also use CMake Tools target substitution, so the debugger launches either the active CMake launch target or the named `Application` target from the currently selected preset.

## Profiling The GUI Path

To profile the GUI frame loop, remove or comment the early return in `src/Watchlist/main.cpp`:

```cpp
//! Ending code here for Testing purpose
return EXIT_SUCCESS;
```

Then rebuild and run `Application` again. Tracy will show:

- `Watchlist GUI` thread name
- `ImGuiStart`
- `WatchlistGuiFrame`
- `NewFrame`
- `WatchlistUi`
- `RenderFrame`
- frame marks from `PROFILE_FRAME`

Each frame mark appears on Tracy's frame timeline and makes it easier to inspect frame-to-frame cost.

## Adding More Zones

Use the wrapper macros from `src/Utils/Profiling/TracyProfiling.hpp`:

```cpp
PROFILE_FUNCTION;              // Whole current function
PROFILE_SCOPE(MyWork);         // A named C++ scope identifier
PROFILE_SCOPE_TEXT("My work"); // A named string zone
PROFILE_MESSAGE("Loaded data");
PROFILE_THREAD("Worker name");
PROFILE_FRAME;                 // End of a rendered frame
```

Prefer `PROFILE_FUNCTION` at function entry and `PROFILE_SCOPE(...)` around expensive inner blocks.
Keep zones coarse enough that the timeline stays readable.
