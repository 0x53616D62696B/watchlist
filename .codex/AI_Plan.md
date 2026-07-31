# MQTT Client/Server Integration

## Summary

Implement the six steps in `.codex/next_steps.md` sequentially through subagents. Each step must end with a committed handoff before the next step begins.

Fixed decisions:

- Paho MQTT C++ and Paho C via `FetchContent`.
- Git submodules documented as fallback.
- Anonymous local MQTT for this milestone.
- QoS 1, non-retained messages.
- Existing `MqttService` API preserved where practical, with status/error reporting added.
- MQTT runs on a runtime-owned dedicated `std::jthread`.
- GUI publishing uses an `AppState` command callback.
- Server dispatch is asynchronous.
- Mosquitto uses Docker Compose.
- Integration tests use a Docker-managed broker fixture.
- TLS/authentication and QoS 2 are future improvements.

## Subagent Steps

### 1. Add Paho dependencies

- Add pinned, compatible Paho C++ and Paho C dependencies through `FetchContent`.
- Disable unnecessary examples, tests, documentation, and SSL features.
- Link asynchronous Paho targets to client, server, and relevant test targets.
- Verify Windows and Linux configuration/builds.
- Document the Git-submodule fallback.

Commit requirement:

- Create a commit containing only Step 1 changes.
- If commit creation is unavailable, stop after staging or preparing the changes and ask me to create the commit.
- Do not start Step 2 until the commit hash is provided or I confirm the handoff.

### 2. Replace the MQTT stub

- Implement real Paho asynchronous transport in `MqttService`.
- Preserve `Connect`, `Subscribe`, `Publish`, `Disconnect`, and `Connected`.
- Add lifecycle/error reporting, automatic reconnect, QoS 1, and non-retained delivery.
- Route callbacks safely through registered handlers.
- Ensure cooperative shutdown and runtime-owned dedicated-thread execution.
- Add transport-level tests against a broker.

Commit requirement:

- Create a dedicated Step 2 commit.
- If permissions prevent committing, prepare the complete changes and wait for me to commit them.
- Record the resulting commit hash before Step 3.

### 3. Add Docker Mosquitto development environment

- Add Docker Compose configuration.
- Add an explicit local anonymous Mosquitto configuration.
- Document Windows PowerShell and Linux startup, shutdown, and troubleshooting commands.
- Verify `mosquitto_pub`/`mosquitto_sub` communication on `localhost:1883`.

Commit requirement:

- Commit Compose, configuration, and documentation as Step 3.
- If unable to commit, wait for my commit before continuing.

### 4. Wire the client and GUI

- Add a thread-safe outbound command callback to `AppState`.
- Install the callback from `WatchlistClient`.
- Make the GUI `Send` button serialize and publish requests to `watchlist/requests`.
- Subscribe to `watchlist/acks/{client_id}`.
- Record outbound messages, acknowledgements, connection status, and failures.
- Keep GUI code independent of concrete MQTT service ownership.
- Retain a client-specific GUI/controller layer as the fallback if the callback design proves unsafe.

Commit requirement:

- Commit all Step 4 changes separately.
- If commit permissions are unavailable, wait for me to create the commit and provide its hash.

### 5. Wire the server

- Subscribe to `watchlist/requests`.
- Parse and validate payloads in the MQTT callback.
- Return immediately after queuing valid requests.
- Publish worker-completion acknowledgements to `watchlist/acks/{source_client_id}`.
- Publish deterministic validation errors when possible.
- Preserve existing dispatcher/database behavior.
- Handle QoS 1 duplicate delivery as a documented idempotency concern.
- Verify shutdown ordering for worker tasks and acknowledgement publication.

Commit requirement:

- Create a separate Step 5 commit.
- If unable to commit, stop and wait for my commit.
- Do not begin Step 6 without the Step 5 commit handoff.

### 6. Add integration checks and documentation

- Add Docker-backed broker integration tests.
- Keep ordinary unit tests broker-independent.
- Verify request publication, server dispatch, acknowledgement publication, client reception, malformed messages, and reconnect behavior.
- Document CLI checks with `mosquitto_pub` and `mosquitto_sub`.
- Document broker/server/client startup order, topics, sample JSON, and troubleshooting.
- Document TLS/authentication and QoS 2 as future work.

Commit requirement:

- Create the final Step 6 commit containing tests and documentation.
- If commit creation is not permitted, wait for me to commit the prepared changes.

## Commit and Worktree Protocol

- Steps run strictly in order.
- Each subagent starts from the previous step’s commit.
- Use the active worktree by default.
- Create a separate `codex/...` branch and worktree only when isolation is necessary.
- Each subagent must report changed files, commit hash, branch/worktree, build/test results, limitations, and any information needed before the next step.
- A subagent must never silently continue after a failed commit.
- If a commit fails because of permissions, the subagent must pause and explicitly ask me to create the commit.
