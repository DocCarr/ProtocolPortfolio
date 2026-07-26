# Test 002: Bridge Sketch - Inverter Use Case

## Test description

Verifies the bridge sketch (`src/ModbusMqtt/ModbusMqtt.ino`) end to end against the inverter
register set in `mapping.json`: each telemetry register group publishes correctly to MQTT,
status/alarm/fault publish correctly, and the write-direction commands (start/stop, real/
reactive power dispatch) correctly write to Modbus - including rejecting a structurally
invalid payload.

Requires Mosquitto and the ModSim Modbus emulator running and configured - see
`tests/procedures/environment-setup.md` first.

## Pre-test checklist

- [ ] Complete the Mosquitto and ModSim setup in `tests/procedures/environment-setup.md`.
- [ ] Copy each folder under `libraries/` into your Arduino sketchbook's `libraries/`
      directory. Re-copy after any local edit to a library.
- [ ] Copy `config/network.example.json` -> `config/network.json` and
      `config/mapping.example.json` -> `config/mapping.json`, fill in real values (lab WiFi
      credentials, isolated-subnet Ethernet IPs, broker IP) - these are gitignored, never
      commit real credentials.
- [ ] Provision the device with these files using `provisioning/OptaProvisioning/
      OptaProvisioning.ino` (see `tests/procedures/001-provisioning.md`) before this test -
      the bridge sketch loads config from the same LittleFS partition the provisioning sketch
      writes to.
- [ ] Upload `src/ModbusMqtt/ModbusMqtt.ino` to the Opta. Unlike the provisioning sketch, this
      one does **not** block on Serial - it's meant to run unattended, so the Serial Monitor
      is for observation only, not required for it to proceed.
- [ ] Ethernet cable connected between the Opta and the laptop's Ethernet adapter (isolated
      Modbus subnet).
- [ ] Laptop's WiFi connected to the lab network (matching `network.json`'s `wifi.*` fields) -
      not the Opta's provisioning access point, which is provisioning-only.
- [ ] Mosquitto running; ModSim running with holding/input register views configured per
      `environment-setup.md`.
- [ ] A terminal running `mosquitto_sub -h <broker-ip> -t "site1/#" -v` to watch all telemetry
      and command traffic, and a way to publish test commands (`mosquitto_pub`).

## Steps

### A. Telemetry register groups (read path)

1. In ModSim's input register grid, set `voltage_phase_a` (addresses 100-101) to a
   recognizable test value.
2. Within one `pollIntervalMs` cycle, confirm a message arrives on
   `site1/telemetry/opta-bridge-01/inv-0001/voltage_a` whose raw bytes match what you set.
3. Repeat for `voltage_phase_b`, `voltage_phase_c`, `current_phase_a`, `current_phase_b`,
   `current_phase_c`, `active_power_total`, `reactive_power_total` - change each in ModSim,
   confirm its matching topic updates.

### B. Status/alarm/fault

4. Set `status`, `alarm`, and `fault` (addresses 116/117/118) to different sentinel values in
   ModSim; confirm each publishes to its respective topic
   (`.../status`, `.../alarm`, `.../fault`).

### C. Start/stop command (write path)

5. Publish a correctly-sized payload (2 bytes, matching `registerCount: 1`) to
   `site1/command/opta-bridge-01/inv-0001/start_stop`.
6. Confirm in ModSim's holding register grid that address 0 updated to match.
7. Publish a payload of the **wrong length** (e.g. 4 bytes) to the same topic. Confirm the
   register in ModSim does **not** change, and the Serial Monitor logs a payload-length
   mismatch - this exercises `validateWritePayloadLength()`'s structural check.

### D. Power dispatch commands

8. Publish a 4-byte payload (matching `registerCount: 2`) to `.../real_power`; confirm holding
   registers 1-2 update in ModSim.
9. Repeat for `.../reactive_power` (registers 3-4).

### E. Reconnection (optional, not required for a pass)

10. Briefly unplug the Ethernet cable; confirm telemetry stops, then reconnect and confirm it
    resumes without re-flashing.
11. Briefly stop Mosquitto; confirm the sketch doesn't crash (publishes just fail/log), then
    restart Mosquitto and confirm both telemetry and command subscriptions resume on their own.

## Expected result

- Every telemetry register (three-phase voltage/current, power totals, status/alarm/fault)
  publishes its raw wire-order bytes to its mapped topic within one poll interval of changing.
- A correctly-sized write command updates the matching Modbus holding register(s).
- An incorrectly-sized write command is rejected (no register change), with a logged reason.
- (Optional) The sketch recovers from a dropped Ethernet link or MQTT connection without
  needing a re-flash.

## Screenshots to capture

- Opta Serial Monitor (boot messages, each publish, each received command, any validation
  rejections)
- ModSim's register grid (holding and input views)
- Terminal running `mosquitto_sub` (showing each telemetry/command message)
- Terminal running `mosquitto_pub` (each command sent, including the invalid-length one)
