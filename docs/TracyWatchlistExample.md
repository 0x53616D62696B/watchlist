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

The preset uses:

- `CMAKE_BUILD_TYPE=DebugTracy`
- `ENABLE_PROFILING=ON`
- `Tracy::TracyClient` linked into `Application`
- `TRACY_NO_EXIT=ON`, so short-lived runs stay available until Tracy connects

## Run With Tracy

1. Start the Tracy profiler UI. If you already have a downloaded Tracy release, run its `Tracy.exe`.
2. Start the Watchlist app:

   ```powershell
   .\build\profiling\Application.exe
   ```

   Or keep the short-lived example open in the console while you connect Tracy:

   ```powershell
   .\build\profiling\Application.exe --wait-for-tracy
   ```

3. In Tracy, connect to the running process.
   The default data port is `127.0.0.1:8086`. If Tracy cannot connect, check that Windows Firewall or antivirus software is not blocking the application from opening a local TCP listener.
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

1. Select configure preset `with-profiling`.
2. Select build preset `with-profiling`.
3. Select launch target `Application`.
4. Build `Application` normally with CMake Tools.
5. For a normal short run, use the CMake Tools run action.
6. For a run that waits for Tracy to connect, use Run and Debug with `CMake: Application target (wait for Tracy)`.
7. Connect Tracy to `127.0.0.1:8086`.

The `.vscode/launch.json` configs use CMake Tools target substitution. The `wait for Tracy` debugger configs pass `--wait-for-tracy` to the same active or named `Application` target without creating a second build.

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
