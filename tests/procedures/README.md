# Test Procedures

Manual test procedures for the Modbus-to-MQTT bridge, run against:

- **MQTT broker**: Mosquitto running locally on the test laptop.
- **Modbus device**: a Modbus server emulator running locally, standing in for the inverter.

Copy `TEMPLATE.md` to a new numbered file (e.g. `001-modbus-read-voltage.md`) for each test
procedure. Every procedure must keep the pre-test checklist from the template, including
updating WiFi credentials in the local (gitignored) config files.
