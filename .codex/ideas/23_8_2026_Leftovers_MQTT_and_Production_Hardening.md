# MQTT Leftovers and Production Hardening

## Summary

Continue sequentially on `feature/july_codex_merge__manual` from commit `98f019f`. Preserve the unrelated untracked `.codex/review.md`.

Complete the missing broker/Linux validation and add production hardening: TLS with username/password authentication, topic ACLs, durable QoS 1 deduplication, secure integration tests, and Ubuntu GitHub Actions. Keep the anonymous loopback broker for local development.

Each subagent starts from the preceding commit and creates one focused commit. If committing is unavailable, it must stop and wait for the user to commit before the next step.

## Implementation Steps

### 1. Add secure MQTT connection configuration

- Add `MqttConnectionOptions` containing host, port, client ID, username, password, TLS enablement, and CA certificate path.
- Load options from `WATCHLIST_MQTT_HOST`, `WATCHLIST_MQTT_PORT`, `WATCHLIST_MQTT_CLIENT_ID`, `WATCHLIST_MQTT_TLS`, `WATCHLIST_MQTT_USERNAME`, `WATCHLIST_MQTT_PASSWORD`, and `WATCHLIST_MQTT_CA_FILE`.
- Keep existing `Connect` and `Run` overloads for anonymous local compatibility; add options-based overloads for secure operation.
- Fail closed when credentials are supplied without TLS, TLS lacks a CA file, certificate validation fails, or required settings are malformed.
- Do not expose or persist secrets through `ConsoleState`, logs, or the GUI.
- Enable Paho’s static SSL asynchronous target and OpenSSL on Windows and Linux.
- Preserve QoS 1 and non-retained delivery.

Commit: `Add authenticated TLS MQTT configuration`.

### 2. Add a secure Mosquitto profile

- Preserve the existing anonymous broker on loopback port `1883`.
- Add a separate Compose profile/service on loopback port `8883` using TLS, password authentication, and ACLs.
- Generate test CA/server certificates at fixture runtime with SANs for `localhost` and `127.0.0.1`; write generated assets under an ignored build directory and commit no private keys.
- Generate the Mosquitto password file at runtime.
- Use separate identities:
  - each client username equals its MQTT client ID and may publish `watchlist/requests` and read only `watchlist/acks/{username}`;
  - `watchlist-server` may read requests and publish all acknowledgement topics.
- Document external production broker provisioning separately from test credentials.

Commit: `Add secure Mosquitto development and test profile`.

### 3. Add durable request deduplication

- Add a SQLite-backed request ledger using the server database.
- Persist request ID, canonical-payload SHA-256 fingerprint, state, final serialized acknowledgement, and timestamps.
- Atomically claim a valid request before worker dispatch.
- Apply these semantics:
  - same ID and fingerprint while active: do not dispatch again; return an `in_progress` acknowledgement;
  - same ID and fingerprint after completion: replay the exact stored acknowledgement;
  - same ID with different content: reject with a deterministic error;
  - entries left active across restart: mark `indeterminate`, never rerun automatically, and return a stable outcome-unknown acknowledgement.
- Reconcile active entries before subscribing to requests.
- Prune terminal entries older than 30 days at startup and at most once per 24 hours; never prune active entries.
- Use short SQLite transactions, a busy timeout, and concurrency tests to prevent duplicate worker admission.
- Leave malformed requests outside the ledger because they cannot trigger side effects.

Commit: `Add durable MQTT request deduplication`.

### 4. Expand secure and deduplication tests

- Add unit tests for environment parsing, fail-closed validation, secret redaction, ledger claims, concurrent duplicates, exact acknowledgement replay, conflicting payloads, crash recovery, and 30-day pruning.
- Extend Docker-backed tests to cover:
  - valid TLS/password connection;
  - invalid CA and invalid password rejection;
  - client/server ACL enforcement;
  - QoS 1 delivery and reconnect/resubscription over TLS;
  - duplicate delivery causing one worker execution;
  - replay after server restart;
  - conflicting request-ID rejection;
  - indeterminate crash-state handling.
- Continue testing the anonymous profile to preserve local-development compatibility.
- Keep broker tests opt-in for normal local unit-test runs.

Commit: `Test secure MQTT and durable deduplication`.

### 5. Add Linux CI validation

- Add an Ubuntu GitHub Actions workflow that:
  - initializes required submodules;
  - installs Ninja, OpenSSL development files, and Mosquitto CLI tools;
  - configures and builds with warnings as errors;
  - runs all unit tests;
  - runs anonymous and secure Docker-backed MQTT integration tests;
  - verifies fixture cleanup even after failure.
- Use generated test credentials only; require no repository or GitHub secrets.
- Retain bounded CTest and Compose timeouts.
- Run the strict MSVC build and non-broker tests locally after the cross-platform CMake changes.
- Push only when authorized; if push permission is unavailable, stop and ask the user to push so GitHub Actions can run.

Commit: `Add Linux MQTT validation workflow`.

### 6. Complete validation and record evidence

- Run the documented `mosquitto_pub`/`mosquitto_sub` anonymous and TLS flows.
- Confirm Windows configure/build and all non-broker tests.
- Confirm the Ubuntu workflow passes unit, anonymous broker, secure broker, reconnect, ACL, and deduplication tests.
- Update MQTT development and branch notes with exact commands, environment, commit hashes, and results.
- Remove the stale “Docker/Mosquitto tests were not run” statement.
- Do not modify or commit `.codex/review.md`.

Commit: `Record MQTT production-hardening validation`.

## Public Interfaces

- Add `MqttConnectionOptions` and options-based `MqttService::Connect`/`Run` overloads.
- Keep current anonymous overloads source-compatible.
- Document acknowledgement statuses `ok`, `error`, `in_progress`, and `indeterminate`.
- Add an internal durable request-ledger API; it is not exposed to GUI code.
- Continue using QoS 1. QoS 2 is not implemented because application-level deduplication is required regardless of broker delivery mode.

## Acceptance Criteria

- Anonymous local development remains functional on `localhost:1883`.
- Secure clients validate the broker certificate and authenticate on `localhost:8883`.
- Unauthorized topic access, bad credentials, and invalid certificates fail.
- A request ID can execute a side effect at most once during normal operation and concurrent redelivery.
- Completed duplicates replay the original acknowledgement; conflicting payloads are rejected.
- Crash-ambiguous work is not automatically repeated.
- Windows strict build/non-broker tests and Ubuntu full MQTT tests pass.
- All six steps have distinct commits, and the branch is clean except for the preserved user-owned `.codex/review.md`.
