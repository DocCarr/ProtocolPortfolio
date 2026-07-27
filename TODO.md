# TODO

Planned top-down (define structure first), implemented bottom-up (build and verify helpers,
then wire them into the structure). Expect this list to change as implementation uncovers
details that don't fit the current structure.

## 1. Configuration schema

- [x] Finalize `network.json` schema (ethernet mac/static IP/subnet/gateway, wifi static
      IP/subnet/gateway plus ssid/password) — `docs/network-config-schema.md`
- [x] Finalize `mapping.json` schema: modbus target IP/port, mqtt broker IP/port, and the
      register list (name, register type, address, register count, direction, topic) —
      `docs/mapping-config-schema.md`
- [x] Document the finalized schema — `docs/network-config-schema.md`,
      `docs/mapping-config-schema.md`
- [x] Update `config/*.example.json` to match

## 2. Shared libraries (`libraries/`)

Generic connectivity/storage mechanics, reused by both sketches. Application-specific
semantics (config parsing, validation rules, bridge logic) stay in the sketches — see
sections 4 and 5.

- [x] `OptaConfigStorage` — header/stub skeleton: mount/read/write/delete, well-formed JSON
      check, required-field-presence check (caller supplies the field-path list)
- [x] `OptaConfigStorage` — bottom-up implementation: `BlockDevice::get_default_instance()` ->
      `MBRBlockDevice` -> `mbed::LittleFileSystem`, POSIX file I/O, ArduinoJson-based checks —
      confirmed against the real `QSPIFormat.ino` example, not just assumed
- [x] `OptaWifiSupport` — header/stub skeleton: access point mode and station mode
      (including reconnection) as distinct functions
- [x] `OptaWifiSupport` — bottom-up implementation: `WiFi.config()`/`beginAP()`/`begin()`/
      `status()`, verified against the installed Opta core's actual WiFi headers
- [x] `OptaEthernetSupport` — header/stub skeleton: static-IP setup and reconnection
- [x] `OptaEthernetSupport` — bottom-up implementation: `Ethernet.begin(mac, ip, dns, gateway,
      subnet)`, link state via `linkStatus()`, verified against the installed core
- [x] `OptaModbusSupport` — header/stub skeleton: connect/reconnect, generic register
      read/write by address/type/count
- [x] `OptaModbusSupport` — bottom-up implementation: `ModbusTCPClient` over an owned
      `EthernetClient`, `requestFrom()`/`read()` and `beginTransmission()`/`write()`/
      `endTransmission()`, verified against the installed `ArduinoModbus` library
- [x] `OptaMqttSupport` — header/stub skeleton: connect/reconnect, topic-list subscribe with
      auto-resubscribe on reconnect, publish, generic message dispatch
- [x] `OptaMqttSupport` — bottom-up implementation: `MqttClient` over an owned `WiFiClient`,
      bridging its `onMessage(size)`/`messageTopic()`/`read()` callback into this library's
      `(topic, payload, length)` handler interface, verified against the installed
      `ArduinoMqttClient` library

## 3. Provisioning tool (`provisioning/OptaProvisioning`)

- [x] Resolve the LittleFS partitioning issue (`docs/partitioning-notes.md`) — user data lives
      in partition 4 (offset 7 MB, size 7 MB), mounted via `MBRBlockDevice`
- [x] Top-down skeleton: WiFi access point + local web server (test harness: accepts direct
      `curl` PUT/POST of config files; no upload form — see note below), delegating
      partition mount to `OptaConfigStorage` and AP setup to `OptaWifiSupport`
- [x] Independent get/delete/push handling for `network.json` and `mapping.json` — implemented
      via manual HTTP parsing over `WiFiServer`/`WiFiClient` (no `WebServer.h` exists for this
      board; confirmed absent from the installed core and every other installed library)
- [x] Push writes the incoming file directly, then reads it back and validates it (well-formed
      JSON, required fields present), logging pass/fail to Serial
- [x] Manual test: provision a device end-to-end, confirm files land correctly on flash —
      `tests/procedures/001-provisioning.md`, passed, see
      `tests/results/2026-07-25-provisioning/`

**Note on validation order**: the write-then-validate flow above is a deliberate test-harness
simplification (matches curl-based file transfer, not a real upload UI) - good enough to
exercise the main sketch's config-loading logic. A production provisioning path should validate
*before* writing, so invalid data never lands on flash even transiently.

## 4. Main sketch — top-down skeleton (`src/ModbusMqtt`)

- [x] `setup()`/`loop()` structure calling stubs: load config, set up Ethernet, set up WiFi,
      set up Modbus client, set up MQTT client, run bridge — delegating connectivity
      mechanics to the libraries in section 2
- [x] Header interfaces (function signatures) for the five shared libraries defined before
      bodies are implemented
- [x] Defined behavior for missing/corrupt config at boot: halt completely, looping a serial
      reprovisioning message (no partial startup with unvalidated config)

## 5. Bottom-up implementation

- [x] Sketch-level `loadConfig()`/`validateConfig()`: parse `network.json`/`mapping.json` into
      the sketch's config structs using the libraries' file/JSON checks, and enforce the
      semantic rule that direction "write" is only valid when registerType is "holding" —
      also bounds-checks each register's `registerCount` against `MAX_REGISTER_WIDTH`
- [x] Wire `setupEthernet()`/`setupWifi()`/`setupModbusClient()`/`setupMqttClient()` to the
      corresponding library `begin*()` calls using the parsed config
- [x] `setupRegisterTopics()`/`onMqttMessage()`: build the write-direction topic list, dispatch
      incoming messages back to their mapped register
- [x] Bridging triggers:
  - [x] `pollAndPublishReadRegisters()`: poll read-direction registers on an interval via
        `OptaModbusSupport`, publish raw register values via `OptaMqttSupport`
  - [x] `validateWritePayloadLength()`: structurally validate an incoming write payload
        against the register's declared width before writing to Modbus

## 6. Testing — inverter use case

- [x] Write test procedures (from `tests/procedures/TEMPLATE.md`) covering: each telemetry
      register group, start/stop command, real/reactive power dispatch, status/alarm/fault —
      `tests/procedures/002-bridge-inverter.md`, plus `tests/procedures/environment-setup.md`
- [x] Configure Mosquitto broker + Modbus emulator to match `mapping.json`'s register set
- [x] Run tests, capture screenshots, record results under `tests/results/` — see
      `tests/results/2026-07-26-bridge-inverter/notes.md`. Telemetry (Sections A/B) and the
      start/stop write + invalid-length rejection (Section C) passed; **power dispatch
      (Section D) was not executed this run** and one MQTT publish failure during the
      rejection test remains unexplained - both noted as open items in the results file.
- [x] Feed anything uncovered back into the schema/skeleton/implementation above — Modbus
      unit ID fix, connection retry-with-delay pattern (all four libraries), and the HTTP
      body-truncation fix were all found during this testing and already committed
