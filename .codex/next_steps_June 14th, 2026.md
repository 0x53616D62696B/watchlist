# Watchlist MQTT Next Steps

## Goal
Move the current client/server runtime from the local `MqttService` stub to real MQTT communication through a Mosquitto broker.

## Recommended Order

1. Add the real Paho MQTT C++ dependency.
   - Decide whether to use `FetchContent`, a git submodule, or system packages.
   - Prefer a clearly documented approach that works on both Windows client builds and Linux server builds.
   - Account for Paho C++'s native dependency on Paho C.

2. Replace the `MqttService` stub with a real async MQTT client.
   - Keep the current `MqttService` interface if possible.
   - Implement connect, reconnect, subscribe callbacks, publish, and cooperative shutdown.
   - Ensure the MQTT IO loop runs on a dedicated runtime-owned `std::jthread`.

3. Run Mosquitto locally for development.
   - Start with Docker or a native Mosquitto install.
   - Example Docker command:

     ```powershell
     docker run --rm -it -p 1883:1883 eclipse-mosquitto
     ```

   - Add a small development Mosquitto config if anonymous local connections are blocked by default.

4. Wire `WatchlistClient` to publish and subscribe.
   - Publish request JSON envelopes to `watchlist/requests`.
   - Subscribe to `watchlist/acks/{client_id}`.
   - Make the MQTT Console `Send` button publish through `MqttService`, not only append to local UI state.

5. Wire `WatchlistServer` to receive, dispatch, and acknowledge.
   - Subscribe to `watchlist/requests`.
   - Parse and validate messages in the MQTT callback.
   - Queue worker-pool dispatch for request handlers.
   - Publish acknowledgements to `watchlist/acks/{source_client_id}`.

6. Add manual and automated integration checks.
   - Verify topics with `mosquitto_pub` and `mosquitto_sub` before relying on the GUI.
   - Add a documented manual flow for starting broker, server, and client.
   - Consider an integration test that is skipped unless a broker endpoint is explicitly configured.

## Current Caveat
`WatchlistClient` and `WatchlistServer` currently use the local `MqttService` stub. The UI and dispatcher/database paths exist, but the applications do not yet exchange messages through a real Mosquitto broker.
