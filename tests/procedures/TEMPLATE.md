# Test Procedure Template

Copy this file to a new name (e.g. `001-modbus-read-voltage.md`) for each new test procedure.

## Pre-test checklist

- [ ] Copy each folder under `libraries/` into your Arduino sketchbook's `libraries/`
      directory (e.g. `Documents/Arduino/libraries/`) so both sketches can compile. Re-copy
      after any local edit to a library, since Arduino IDE reads from the sketchbook copy,
      not this repo directly.
- [ ] If provisioning the device this run: update the `AP_SSID`/`AP_PASSWORD` placeholders in
      `provisioning/OptaProvisioning/OptaProvisioning.ino` before uploading it. These are the
      provisioning access point's own credentials (separate from the WiFi credentials being
      provisioned onto the device below) - never commit real values.
- [ ] Copy `config/network.example.json` -> `config/network.json` and `config/mapping.example.json`
      -> `config/mapping.json` if not already done, and update them with the current WiFi
      SSID/password and correct IPs for this test setup. These files are gitignored - never
      commit real credentials.
- [ ] Mosquitto broker running on laptop; note its IP/port.
- [ ] Modbus server emulator running; note its IP/port and the register values under test.
- [ ] Opta connected via USB with the Arduino IDE Serial Monitor open.

## Test description

(What this test verifies.)

## Steps

1. ...

## Expected result

...

## Screenshots to capture

- Opta Serial Monitor
- Mosquitto terminal
- Modbus emulator
