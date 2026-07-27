# Test 001: Provisioning Tool

## Test description

Verifies the provisioning sketch (`provisioning/OptaProvisioning/OptaProvisioning.ino`) end to
end: it mounts (or formats, with confirmation) its LittleFS partition, brings up its WiFi
access point, and correctly serves GET/DELETE/POST for `network.json` and `mapping.json`
independently, including rejecting invalid JSON.

Mosquitto and the Modbus emulator are not needed for this test - provisioning never touches
MQTT or Modbus.

## Pre-test checklist

- [ ] Copy each folder under `libraries/` into your Arduino sketchbook's `libraries/`
      directory. Re-copy after any local edit to a library.
- [ ] Update the `AP_SSID`/`AP_PASSWORD` placeholders in
      `provisioning/OptaProvisioning/OptaProvisioning.ino` before uploading it. These are
      gitignored-in-spirit placeholders - never commit real values. `AP_PASSWORD` must be
      **8-63 characters** (WPA2 minimum) - a shorter password makes the access point fail to
      start silently, with only a status-code log line to go on.
- [ ] If this specific physical Opta has never had its QSPI flash partitioned before, run the
      official `QSPIFormat.ino` example once first (Arduino IDE: File > Examples >
      STM32H747_System > QSPIFormat) - see `docs/partitioning-notes.md`. Without this, partition
      4 doesn't exist yet and mounting/formatting will fail with `BD_ERROR_INVALID_PARTITION`.
- [ ] Copy `config/network.example.json` -> `config/network.json` and
      `config/mapping.example.json` -> `config/mapping.json` on your laptop, and fill in real
      values (WiFi SSID/password, IPs) - these are the files this test will push to the
      device. They're gitignored - never commit real credentials.
- [ ] Prepare one deliberately invalid JSON file to test the validation-rejection path, e.g.
      save the following as `config/network.invalid.json` (missing `wifi.password`):
      ```json
      {"ethernet": {"mac": "DE:AD:BE:EF:00:01", "ip": "192.168.1.10", "subnetMask": "255.255.255.0", "gateway": "192.168.1.1"}, "wifi": {"ssid": "test"}}
      ```
- [ ] Opta connected via USB with the Arduino IDE Serial Monitor open.
- [ ] Be ready to connect your laptop's WiFi to the Opta's access point (its SSID/password
      from the placeholders above) once the sketch is running - the curl commands below only
      reach the Opta while your laptop is joined to that network.
- [ ] If running these commands from PowerShell: use `curl.exe`, not bare `curl` - PowerShell
      aliases plain `curl` to `Invoke-WebRequest`, which doesn't understand curl's flags and
      will fail with a confusing "Cannot find drive" error. Plain `curl` works fine from
      Command Prompt (`cmd.exe`), where no such alias exists.

## Steps

1. Upload `OptaProvisioning.ino` to the Opta. Watch the Serial Monitor during boot.
   - If the partition isn't yet formatted, the sketch will prompt: type `y` and press Enter
     to format it, or `n` to halt.
   - Confirm the Serial Monitor reports the access point starting.
2. On your laptop, connect to the Opta's WiFi access point.
3. Confirm connectivity, e.g. `ping 192.168.4.1` (or whatever `AP_IP` is set to).
4. `curl.exe -i http://192.168.4.1/network.json` — expect `404 Not Found` (nothing pushed yet).
5. `curl.exe -i --data-binary @config/network.json http://192.168.4.1/network.json` — expect
   `200 OK`. Check the Serial Monitor for "network.json: received and validated OK."
6. `curl.exe -i http://192.168.4.1/network.json` — expect `200 OK` with the same JSON content
   echoed back, confirming the round trip.
7. Repeat steps 4-6 for `mapping.json` (`config/mapping.json`).
8. `curl.exe -i -X DELETE http://192.168.4.1/network.json` — expect `200 OK`.
9. `curl.exe -i http://192.168.4.1/network.json` — expect `404 Not Found` again, confirming
   delete worked.
10. `curl.exe -i --data-binary @config/network.invalid.json http://192.168.4.1/network.json` —
    expect `422 Unprocessable Entity`. Check the Serial Monitor for "network.json: received
    but FAILED validation." Then re-push the real `config/network.json` to leave the device
    in a valid state.

## Expected result

- Access point comes up at the configured IP with the configured SSID/password.
- GET on a missing file returns 404; GET after a successful push returns 200 with matching
  content.
- POST of well-formed, schema-complete JSON returns 200 and logs a validation pass to Serial.
- POST of malformed/incomplete JSON returns 422 and logs a validation failure to Serial.
- DELETE removes the file (subsequent GET returns 404 again).
- `network.json` and `mapping.json` behave independently of each other throughout.

## Screenshots to capture

- Opta Serial Monitor (boot/mount messages, and each push's validation result)
- Terminal running the curl commands (request + response for each step)
