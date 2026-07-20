# ProtocolPortfolio
Implements a Modbus to MQTT protocol translation program on the Arduino Opta.

## Repository structure

- `draft/` — working files edited in place; copied into Arduino IDE manually once accepted.
- `src/` — main bridge sketch source.
- `provisioning/` — separate sketch that provisions network/mapping config onto the device.
- `config/` — configuration JSON. `.example.json` files are checked in; real `network.json` /
  `mapping.json` (with credentials) are gitignored.
- `docs/` — design notes and open issues.
- `tests/procedures/` — manual test procedures (Mosquitto broker + Modbus emulator on a laptop).
- `tests/results/` — screenshots and notes from each test run.
