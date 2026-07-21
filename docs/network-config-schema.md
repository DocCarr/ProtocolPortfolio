# Network Configuration Schema (`network.json`)

Describes the Opta's two network interfaces: Ethernet, on the isolated Modbus TCP subnet, and
WiFi, on the network that reaches the MQTT broker. Both interfaces use a static IP — no DHCP —
so addressing stays predictable across reboots and reprovisioning.

Real values for a given deployment live in `config/network.json`, which is gitignored (see
`config/README.md`). This document describes the schema; `config/network.example.json` is a
placeholder instance of it.

## `ethernet`

| Field | Type | Description |
|---|---|---|
| `mac` | string (MAC address, colon-separated hex) | MAC address for the Opta's Ethernet interface. Required by the Ethernet setup call. |
| `ip` | string (IPv4) | Static IP assigned to the Opta's Ethernet port on the isolated Modbus subnet. |
| `subnetMask` | string (IPv4) | Subnet mask for the Ethernet interface. |
| `gateway` | string (IPv4) | Gateway address for the Ethernet interface. This subnet is isolated with no route out, so the value isn't expected to do any real routing — it's included because the Ethernet setup call requires one. |

## `wifi`

| Field | Type | Description |
|---|---|---|
| `ssid` | string | WiFi network SSID. |
| `password` | string | WiFi network password. |
| `ip` | string (IPv4) | Static IP assigned to the Opta's WiFi interface. |
| `subnetMask` | string (IPv4) | Subnet mask for the WiFi interface. |
| `gateway` | string (IPv4) | Gateway address for the WiFi interface. Unlike Ethernet's, this one is real — the WiFi network is expected to route to the MQTT broker (see `mapping.json`). |

## Notes

- All IP addresses are IPv4 dotted-decimal strings.
