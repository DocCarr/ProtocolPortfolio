# TODO

Planned top-down (define structure first), implemented bottom-up (build and verify helpers,
then wire them into the structure). Expect this list to change as implementation uncovers
details that don't fit the current structure.

## 1. Configuration schema

- [ ] Finalize `network.json` schema (ethernet static IP/subnet/gateway, wifi ssid/password)
- [ ] Finalize `mapping.json` schema: modbus target IP/port, mqtt broker IP/port, and the
      register list (name, register type, address, register count, direction, topic)
- [ ] Document the finalized schema (e.g. in `docs/`)
- [ ] Update `config/*.example.json` to match

## 2. Provisioning tool (`provisioning/OptaProvisioning`)

- [x] Resolve the LittleFS partitioning issue (`docs/partitioning-notes.md`) — user data lives
      in partition 4 (offset 7 MB, size 7 MB), mounted via `MBRBlockDevice`
- [ ] WiFi access point + local web server hosting an upload form
- [ ] Upload handling for `network.json` and `mapping.json`
- [ ] Basic validation of uploaded JSON before writing (well-formed, required fields present)
- [ ] Write validated files to the designated LittleFS partition
- [ ] Manual test: provision a device end-to-end, confirm files land correctly on flash

## 3. Main sketch — top-down skeleton (`src/ModbusMqtt`)

- [ ] `setup()`/`loop()` structure calling stubs: load config, set up Ethernet, set up WiFi,
      set up Modbus client, set up MQTT client, run bridge
- [ ] Header interfaces (function signatures) for `ConfigLoader`, `ModbusHandler`,
      `MqttHandler` defined before bodies are implemented
- [ ] Defined behavior for missing/corrupt config at boot (no valid config found on LittleFS)

## 4. Bottom-up implementation

- [ ] `ConfigLoader`: mount LittleFS, read + parse `network.json`/`mapping.json`, validate,
      expose parsed config to the rest of the sketch
- [ ] Ethernet setup from parsed network config (static IP on the isolated Modbus subnet)
- [ ] WiFi setup from parsed network config (SSID/password, broker-reachable network)
- [ ] `ModbusHandler`: TCP client to target device; read input/holding registers and write
      holding registers per the mapping config
- [ ] `MqttHandler`: connect to broker; subscribe to write-direction topics; publish
      read-direction topics
- [ ] Bridging triggers:
  - [ ] Poll read-direction registers on an interval, publish raw register values to MQTT
  - [ ] On incoming MQTT message for a write-direction topic, structurally validate payload
        length against the register's declared width, then write to Modbus
  - [ ] Reconnect/retry handling for Ethernet, WiFi, Modbus, and MQTT, independent of each other
- [ ] Wire the skeleton's stubs to the real implementations above

## 5. Testing — inverter use case

- [ ] Write test procedures (from `tests/procedures/TEMPLATE.md`) covering: each telemetry
      register group, start/stop command, real/reactive power dispatch, status/alarm/fault
- [ ] Configure Mosquitto broker + Modbus emulator to match `mapping.json`'s register set
- [ ] Run tests, capture screenshots, record results under `tests/results/`
- [ ] Feed anything uncovered back into the schema/skeleton/implementation above
