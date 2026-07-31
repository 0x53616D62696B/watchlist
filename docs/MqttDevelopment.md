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
