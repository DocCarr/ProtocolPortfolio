# Configuration Files

- `network.example.json` / `mapping.example.json` — checked into git, placeholder values only.
- `network.json` / `mapping.json` — real values (WiFi credentials, actual device/broker IPs).
  Gitignored — never committed.

Before provisioning a device or running a test, copy the `.example.json` files to their real
names and fill in actual values. WiFi SSID/password should be updated in these local files
before each test run (see `tests/procedures/TEMPLATE.md`).
