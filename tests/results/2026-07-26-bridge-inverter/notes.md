# Test 002: Bridge Sketch - Inverter Use Case — Result

**Date:** 2026-07-26
**Procedure:** `tests/procedures/002-bridge-inverter.md`
**Outcome:** PASS for Sections A, B, C. **Section D (power dispatch commands) was not executed
this run** - see "Not yet tested" below.

No screenshot image files this time - evidence is the pasted Serial Monitor / Mosquitto broker
log text captured during the test session itself (quoted below), rather than a photographed
Serial Monitor the way the provisioning test was documented.

## What was confirmed

**Section A/B - telemetry (read path).** All 11 read-direction topics received messages,
confirmed via `mosquitto_sub -t "site1/#" -v`:

```
site1/telemetry/opta-bridge-01/inv-0001/voltage_a
site1/telemetry/opta-bridge-01/inv-0001/voltage_b
site1/telemetry/opta-bridge-01/inv-0001/voltage_c
site1/telemetry/opta-bridge-01/inv-0001/current_a
site1/telemetry/opta-bridge-01/inv-0001/current_b
site1/telemetry/opta-bridge-01/inv-0001/current_c
site1/telemetry/opta-bridge-01/inv-0001/active_power_total
site1/telemetry/opta-bridge-01/inv-0001/reactive_power_total
site1/telemetry/opta-bridge-01/inv-0001/status
site1/telemetry/opta-bridge-01/inv-0001/alarm
site1/telemetry/opta-bridge-01/inv-0001/fault
```

Mosquitto's own broker log additionally confirmed payload *sizes* were correct - e.g.
`current_a`/`current_b`/`current_c`/`active_power_total` (all `registerCount: 2`) each showed
exactly 4 bytes (2 bytes x 2 registers), matching the wire-format design. Actual byte *values*
against what was set in ModSim were not independently re-verified beyond this.

**Section C - start/stop command (write path).**
- A correctly-sized (2-byte) payload published to
  `site1/command/opta-bridge-01/inv-0001/start_stop` updated ModSim's holding register 0 to
  the published value, confirmed visually in ModSim's grid.
- A deliberately wrong-sized (4-byte) payload to the same topic was rejected: Serial Monitor
  logged `onMqttMessage: payload length mismatch for topic
  site1/command/opta-bridge-01/inv-0001/start_stop`, confirming
  `validateWritePayloadLength()` catches a structurally invalid write before it reaches Modbus.

## Not yet tested

- **Section D (power dispatch commands)** - `real_power`/`reactive_power` writes to holding
  registers 1-2 and 3-4 were never exercised this run. Should be run before considering the
  write path fully covered.
- **Section E (reconnection)** - optional per the procedure; not attempted.

## Open issue (unresolved)

During the wrong-length rejection test, the Serial Monitor also logged:
```
OptaMqttSupport: beginMessage() failed.
pollAndPublishReadRegisters: publish() failed for status
```
A telemetry publish for `status` failed right around the same time the incoming (rejected)
command was being processed. Root cause unknown - possibly a concurrency interaction between
an outgoing publish and an incoming message arriving on the same MQTT connection. Only
observed once; not yet confirmed whether it's reproducible. Worth real investigation before
relying on this in continuous operation, since telemetry publishing and command receiving will
overlap constantly in normal use.

## Issues found and fixed during this test session

Several real bugs were caught and fixed while getting this test to pass (all committed):

1. **Ethernet cable unplugged** (environmental, not a code issue) - caused `beginEthernet()`
   to keep failing until noticed.
2. **`WiFi.begin()`/`MqttClient::connect()`/`Ethernet` link/`ModbusTCPClient::begin()` all
   checked their result immediately** instead of polling for the real outcome - the actual
   handshake/negotiation takes a moment. Fixed by adding a retry-with-delay pattern (10
   attempts, 500ms apart) to all four `begin*()` functions, matching a known-working reference
   sketch. This was the root cause of the WiFi/MQTT "connection failed" symptoms seen early in
   this session, not a credentials or network config problem.
3. **Router WiFi security mode reported as "Auto"** (`encryptionType=8` from a diagnostic
   scan) - a likely contributor to intermittent WiFi connection instability observed even
   after the retry fix. Not confirmed resolved (no confirmation the router setting was
   changed to an explicit WPA2-Personal mode).
4. **HTTP POST body truncation for larger payloads** in `OptaProvisioning.ino` -
   `client.readBytes()`'s default ~1s timeout was too short for `mapping.json` (much larger
   than `network.json`) arriving over multiple network segments, silently truncating the body
   before it reached the JSON parser. Fixed with a longer `client.setTimeout(5000)`.
5. **ModSim bound to `0.0.0.0`** instead of the specific target IP (environmental emulator
   config, not a code issue) - fixed by binding ModSim's listener to `192.168.1.50` directly.
6. **Modbus unit/slave ID mismatch** - ModSim has a fixed unit ID of `1` (not configurable in
   the tool), while `ArduinoModbus` defaults to `0`. This was the actual cause of every single
   Modbus register read returning 0 regardless of address, not a ModSim register-range issue
   as first suspected. Fixed by hardcoding `MODBUS_UNIT_ID = 1` in `OptaModbusSupport` and
   using the explicit-unit-ID overloads of `requestFrom()`/`beginTransmission()`.
7. **PowerShell tooling gotchas** (environmental, not code issues, now documented in
   `001-provisioning.md`): bare `curl` resolves to `Invoke-WebRequest` in PowerShell, not real
   curl - use `curl.exe`. `--data-binary @file` needs the argument quoted
   (`"@file"`) or PowerShell misparses `@` as its splatting operator.
8. **A missing `@` in a `curl` command** caused the literal string `"mapping.json"` to be sent
   as the request body instead of the file's contents - a usage error, not a code bug, but the
   422 rejection it produced looked identical to a real validation failure and took some
   diagnosis to distinguish (a UTF-8 BOM was suspected and ruled out first).

Also added during this session (now committed): diagnostic Serial logging throughout
`OptaConfigStorage` (specific JSON parse errors, specific missing required fields),
`OptaWifiSupport`/`OptaMqttSupport`/`OptaEthernetSupport`/`OptaModbusSupport` (specific
status/error codes on connection failure), and `OptaProvisioning.ino` (exact byte count
missing on a truncated body) - all of which made the above bugs actually diagnosable instead
of just "it doesn't work."
