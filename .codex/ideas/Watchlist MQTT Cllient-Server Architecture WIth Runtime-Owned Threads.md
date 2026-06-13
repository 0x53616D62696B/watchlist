# Watchlist MQTT Client/Server Architecture With Runtime-Owned Threads

## Summary
Build two Watchlist applications sharing one core messaging/runtime layer:

- `WatchlistClient`: Windows ImGui app for composing MQTT requests and showing server acknowledgements.
- `WatchlistServer`: Linux ImGui app that connects to a Mosquitto broker, receives requests, dispatches work, and publishes acknowledgements.
- Mosquitto is the real MQTT broker; the Watchlist server process is an MQTT service client.
- Webserver integration is deferred for v1, but server actions will be routed through a controller that can later be called from REST/WebSocket endpoints.
- Extend `ThreadPoolManager` into the central app runtime that owns all application threads.

## Runtime And Threading
- Refactor `ThreadPoolManager` into a runtime manager with two thread categories:
  - **Dedicated service threads** for long-lived loops: `ImGui`, `MQTT IO`, optionally `DatabaseWorker`.
  - **Worker pool tasks** for bounded work: scripts, device commands, database writes, AI prompt handling.
- Add APIs equivalent to:
  - `StartDedicatedThread(name, callable_with_stop_token)`
  - `EnqueueTask(callable)`
  - `StopAll()`
  - `JoinAll()`
- Use `std::jthread` and `std::stop_token` for dedicated threads so shutdown is cooperative and explicit.
- Keep the existing queue/condition-variable worker pool for short tasks, but prevent long-lived loops from consuming worker slots forever.
- ImGui must run entirely on its dedicated GUI thread: window creation, OpenGL context, frame loop, rendering, and shutdown all stay on that thread.
- MQTT runs on a dedicated service thread that owns connect/reconnect/subscription lifecycle and posts received messages into the app dispatcher.

## Messaging And Public Types
- Add `src/Watchlist/Messaging` with:
  - `MessageType`: `execute_script`, `send_to_device`, `cmd_to_device`, `store_database_value`, `query_AI_prompt`.
  - `MqttRequest`: `id`, `type`, `created_utc`, `source_client_id`, optional type-specific fields.
  - `MqttAck`: `request_id`, `status`, `message`, `completed_utc`.
- Use JSON envelopes for all MQTT payloads.
- MQTT topics:
  - Client publishes requests to `watchlist/requests`.
  - Server subscribes to `watchlist/requests`.
  - Server publishes acks to `watchlist/acks/{source_client_id}`.
  - Client subscribes to its own ack topic.
- Use Eclipse Paho MQTT C++ async client for cross-platform Windows/Linux MQTT client behavior.

## Server Behavior
- MQTT receive callback only parses, validates, logs, and queues dispatch; it must not block.
- Immediate handling on the MQTT/runtime dispatch path:
  - malformed message,
  - unknown message type,
  - missing required fields,
  - lightweight logging/routing.
- Worker-pool handling:
  - `execute_script`: allowlisted script executor stub for `script1`.
  - `send_to_device`: device sender stub.
  - `cmd_to_device`: device command stub.
  - `store_database_value`: SQLite key/value upsert.
  - `query_AI_prompt`: AI prompt stub.
- Add a SQLite `kv_store` table/API for `store_database_value`; do not reuse the existing device `items` table.

## ImGui
- Replace the current demo UI with an “MQTT Console” panel:
  - broker connection settings/status,
  - message type selector,
  - type-specific input fields,
  - Send button,
  - recent outbound requests,
  - recent acknowledgements and server activity.
- Client UI sends requests and displays acks.
- Server UI displays received requests, dispatch status, worker results, and ack history.

## Test Plan
- Unit tests for JSON parse/serialize round trips for all five message types.
- Unit tests for invalid/missing fields producing deterministic error acks.
- Unit tests for dispatcher routing: immediate handling vs worker-pool handling.
- Unit tests for runtime manager lifecycle: start dedicated thread, request stop, join cleanly, reject enqueue after stop.
- SQLite tests for `kv_store` initialize/upsert/get behavior.
- Manual integration:
  - run Mosquitto on Linux,
  - start `WatchlistServer`,
  - start Windows `WatchlistClient`,
  - send each message type,
  - confirm server log and client ack.

## Assumptions
- `ThreadPoolManager` becomes the owner of all runtime threads, but long-lived service threads are separate from the bounded worker queue.
- `std::jthread` is acceptable because the repo already targets C++23.
- Webserver hook remains phase two.
- v1 handlers may be stubs except `store_database_value`, which persists to SQLite.
- Preserve existing dirty worktree changes in `.gitignore` and `.codex/skills/code-change-workflow/SKILL.md`.
