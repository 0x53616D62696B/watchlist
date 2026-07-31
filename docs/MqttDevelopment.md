# Local MQTT Development Broker

The repository provides an anonymous Eclipse Mosquitto `2.0.22` broker for
local development. Docker publishes its MQTT listener only on
`127.0.0.1:1883`, so other machines cannot connect through a host network
interface. The broker deliberately has no TLS, authentication, or persistent
storage for this milestone.

Do not deploy this configuration or change the Compose port mapping to
`1883:1883` on a shared network. Production use requires authentication, TLS,
and an appropriate persistence policy.

## Prerequisites

- Docker Desktop with Docker Compose v2 on Windows, or Docker Engine with the
  Compose plugin on Linux.
- `mosquitto_pub` and `mosquitto_sub` on the host for the end-to-end CLI check.
  The Windows Mosquitto installer provides both programs. Debian and Ubuntu
  provide them in the `mosquitto-clients` package.

Run all commands from the repository root.

## Runtime Startup Order

Start components in this order so subscriptions exist before non-retained
messages are published:

1. Start the broker with `docker compose up -d --wait mosquitto`.
2. Build and start `WatchlistServer`; it subscribes to `watchlist/requests`.
3. Build and start `WatchlistClient`; it subscribes to its acknowledgement
   topic and the GUI Send button publishes request envelopes.

```powershell
cmake --preset default
cmake --build --preset default --target WatchlistServer WatchlistClient
# terminal 1
.\build\default\WatchlistServer.exe
# terminal 2
.\build\default\WatchlistClient.exe
```

On Linux, use `./build/default/WatchlistServer` and
`./build/default/WatchlistClient`. The default client IDs are
`watchlist-server` and `watchlist-client`; both connect to `localhost:1883`.

## Topics and Envelopes

| Topic | Publisher | Subscriber | Purpose |
| --- | --- | --- | --- |
| `watchlist/requests` | clients | server | JSON request envelopes |
| `watchlist/acks/{client_id}` | server | matching client | success and validation acknowledgements |
| `watchlist/dev/health` | Compose health check | broker only | local broker readiness probe |

All Watchlist publications use QoS 1 and `retain=false`. A valid request:

```json
{"id":"request-1","type":"execute_script","created_utc":"2026-07-31T12:00:00Z","source_client_id":"cli-client","script_name":"script1"}
```

The corresponding acknowledgement has a runtime-generated completion time:

```json
{"request_id":"request-1","status":"ok","message":"script1 executed","completed_utc":"2026-07-31T12:00:01Z"}
```

A structurally valid envelope can still produce a deterministic error. This
request omits `script_name` but retains a safe client ID, so the server can
route the error:

```json
{"id":"bad-1","type":"execute_script","created_utc":"2026-07-31T12:00:00Z","source_client_id":"cli-client"}
```

```json
{"request_id":"bad-1","status":"error","message":"missing required field: script_name","completed_utc":"2026-07-31T12:00:01Z"}
```

Invalid JSON or an unsafe/missing `source_client_id` is recorded by the server
but cannot receive an acknowledgement because there is no safe reply topic.

## End-to-End CLI Requests

With the broker and `WatchlistServer` running, subscribe before publishing:

```powershell
mosquitto_sub -h localhost -p 1883 -q 1 -t "watchlist/acks/cli-client" -v
```

From another PowerShell terminal, publish the valid and malformed examples:

```powershell
mosquitto_pub -h localhost -p 1883 -q 1 -t "watchlist/requests" -m '{"id":"request-1","type":"execute_script","created_utc":"2026-07-31T12:00:00Z","source_client_id":"cli-client","script_name":"script1"}'
mosquitto_pub -h localhost -p 1883 -q 1 -t "watchlist/requests" -m '{"id":"bad-1","type":"execute_script","created_utc":"2026-07-31T12:00:00Z","source_client_id":"cli-client"}'
```

The same commands work in a Linux shell. Do not add Mosquitto's `-r` flag;
non-retained is the publisher default and the Watchlist production setting.

## Docker-Managed Integration Tests

Broker tests are opt-in and never join the ordinary `UnitTests` run. Configure
and build them explicitly:

```powershell
cmake --preset default -DWATCHLIST_BUILD_MQTT_INTEGRATION_TESTS=ON
cmake --build --preset default --target MqttTransportIntegrationTests
ctest --test-dir build/default -R "^Mqtt" --output-on-failure
```

CTest uses fixture setup/cleanup tests to start a healthy Compose broker and
run `docker compose down --remove-orphans` even when the integration executable
fails. Coverage includes transport delivery, request publication, asynchronous
server dispatch, acknowledgement publication/client reception, routable
validation errors, non-retained delivery, reconnect, resubscription, and
cooperative shutdown. Each fixture/test has a bounded timeout.

Override the loopback port when `1883` is occupied:

```powershell
cmake --preset default -DWATCHLIST_BUILD_MQTT_INTEGRATION_TESTS=ON -DWATCHLIST_MQTT_TEST_PORT=2883
```

To use an already-managed broker instead of Docker, configure
`WATCHLIST_MQTT_MANAGE_TEST_BROKER=OFF` and set
`WATCHLIST_MQTT_TEST_HOST`/`WATCHLIST_MQTT_TEST_PORT`. The broker-restart case
is skipped without the Docker fixture, while the remaining transport/workflow
checks still run.

## Windows PowerShell

Start the broker and wait for its health check:

```powershell
docker compose up -d --wait mosquitto
docker compose ps
```

Verify the host-published `localhost:1883` port with one subscriber and one
publisher:

```powershell
$subscriber = Start-Job {
    mosquitto_sub -h localhost -p 1883 -t watchlist/dev/verification -C 1 -W 10
}
Start-Sleep -Seconds 1
mosquitto_pub -h localhost -p 1883 -t watchlist/dev/verification -m "watchlist-mqtt-ok"
Receive-Job -Job $subscriber -Wait
Remove-Job -Job $subscriber
```

The received output must be `watchlist-mqtt-ok`. Stop and remove the broker:

```powershell
docker compose down --remove-orphans
```

## Linux

Install the MQTT clients on Debian or Ubuntu if needed:

```bash
sudo apt-get update
sudo apt-get install mosquitto-clients
```

Start the broker and wait for its health check:

```bash
docker compose up -d --wait mosquitto
docker compose ps
```

Verify publish/subscribe traffic through the host-published
`localhost:1883` port:

```bash
output_file="$(mktemp)"
mosquitto_sub -h localhost -p 1883 -t watchlist/dev/verification -C 1 -W 10 >"$output_file" &
subscriber_pid=$!
sleep 1
mosquitto_pub -h localhost -p 1883 -t watchlist/dev/verification -m "watchlist-mqtt-ok"
wait "$subscriber_pid"
cat "$output_file"
rm "$output_file"
```

The received output must be `watchlist-mqtt-ok`. Stop and remove the broker:

```bash
docker compose down --remove-orphans
```

## Troubleshooting

Inspect service state, the resolved Compose configuration, and broker logs:

```text
docker compose ps
docker compose config
docker compose logs --tail 100 mosquitto
```

- If Docker cannot connect to its daemon, start Docker Desktop on Windows or
  the Docker service on Linux, then rerun `docker compose up`.
- If port `1883` is already allocated, stop the conflicting broker or process.
  On PowerShell, run `Get-NetTCPConnection -LocalPort 1883`. On Linux, run
  `ss -ltnp 'sport = :1883'`.
- If the container is unhealthy, inspect `docker compose logs mosquitto` for a
  configuration mount or syntax error. The health check publishes an anonymous
  message to `watchlist/dev/health` inside the container.
- If a client reports `Connection refused`, confirm the service is healthy and
  that it connects to plain MQTT at `localhost:1883`, not an SSL/TLS URI.
- If a client reports `Not authorized`, confirm the container loaded
  `docker/mosquitto/mosquitto.conf`, which explicitly enables anonymous access.
- If a subscription times out, start the subscriber before the publisher.
  Development messages are non-retained, so late subscribers do not receive
  earlier publications.
- If an integration configure succeeds but fixture setup says Docker is
  missing, install/start Docker or select an external broker with
  `WATCHLIST_MQTT_MANAGE_TEST_BROKER=OFF`.
- If fixture cleanup is needed after an interrupted test process, run
  `docker compose --project-name watchlist-mqtt-integration down --remove-orphans`.

The configuration has no named volumes. `docker compose down --remove-orphans`
therefore leaves no broker data behind.

## QoS 1 Duplicate Delivery

Watchlist requests and acknowledgements use QoS 1, which guarantees delivery
at least once rather than exactly once. A reconnect or lost acknowledgement
can therefore cause the broker to deliver the same request more than once.
`store_database_value` is naturally repeatable for the same key and value, but
device commands, scripts, and other side effects are not generally idempotent.

The request `id` is the intended idempotency key. This milestone does not yet
persist a processed-request ledger, so clients should reuse the same ID when
retrying and consumers must tolerate duplicate acknowledgements. A durable
deduplication store should be added before commands with non-repeatable side
effects are used in production.

## Future Transport Work

The checked-in broker is intentionally anonymous and unencrypted. Production
work must add TLS certificate validation, broker authentication/authorization,
secret provisioning, and a non-anonymous Mosquitto configuration. QoS 2 is
also deferred; adopt it only after measuring the extra handshake/storage cost
and defining end-to-end exactly-once semantics. These changes do not replace
application-level idempotency for side-effecting requests.
