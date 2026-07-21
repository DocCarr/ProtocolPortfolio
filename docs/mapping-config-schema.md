# Mapping Configuration Schema (`mapping.json`)

Describes the Modbus target device, the MQTT broker, and the register-to-topic mapping the
bridge passes through with no interpretation or intermediate processing. Real values for a
given deployment live in `config/mapping.json`, which is gitignored (see `config/README.md`).
`config/mapping.example.json` is a placeholder instance of this schema, populated with an
illustrative inverter register set.

## `modbus`

| Field | Type | Description |
|---|---|---|
| `targetIp` | string (IPv4) | IP address of the Modbus TCP server (the device being bridged). |
| `targetPort` | number | TCP port of the Modbus server (typically 502). |

Single target device — this schema does not support bridging multiple Modbus devices.

## `mqtt`

| Field | Type | Description |
|---|---|---|
| `brokerIp` | string (IPv4) | IP address of the MQTT broker. |
| `brokerPort` | number | TCP port of the MQTT broker (typically 1883 for plain, unauthenticated connections). |
| `clientId` | string | MQTT client ID used when connecting to the broker. Must be unique if other clients connect to the same broker. |

No TLS and no username/password fields for now — the target broker is expected to allow
anonymous connections. Revisit this schema if a deployment needs broker authentication.

## `pollIntervalMs`

Single global interval, in milliseconds, at which all `direction: read` registers are polled
and republished. Applies to the whole bridge, not per-register.

## `registers`

A list of register↔topic mappings. Each entry is a combined record — register info and its one
mapped topic together — so there is no separate topic list and no join step. This matches the
established one-to-one mapping and "no intermediate processing" rule.

| Field | Type | Description |
|---|---|---|
| `name` | string | Human-readable label. Not used by the bridge logic — for logs, docs, and debugging only. |
| `registerType` | `"holding"` \| `"input"` | Which Modbus register table this entry reads from or writes to. |
| `address` | number | Starting Modbus register address. |
| `registerCount` | number | Number of consecutive 16-bit registers making up this value (e.g. 1 for an enum/command, 2 for a 32-bit measurement). |
| `direction` | `"read"` \| `"write"` | `read`: poll the register on `pollIntervalMs` and publish its raw value to `topic`. `write`: subscribe to `topic` and write incoming payloads to the register. |
| `topic` | string | MQTT topic mapped to this register. |

### Validation rules

- `direction: write` is only valid when `registerType: holding` — input registers are read-only
  on the Modbus device, so a write-direction input register is a configuration error.
- An incoming MQTT payload for a `write` register must structurally match `registerCount`
  (e.g. reject a 1-register payload for a 2-register target) before it is written — this is a
  wire-format check only, not an interpretation of the value.

## Topic and device ID naming convention (recommended, not required)

The `topic` and `name` fields are free-form strings — the bridge does not parse or depend on
any particular format. `config/mapping.example.json` follows a recommended convention, and new
mappings should follow it for consistency, but nothing in the schema enforces it.

**Device IDs** follow ISA S5.1-style instrumentation tag numbering: a short function/class
prefix, a hyphen, and a zero-padded sequential number (e.g. `inv-0001` for the first inverter).
The prefix identifies the device class at a glance; the padded number sorts and scans
predictably and leaves room to grow without changing width.

**Topics** follow a hierarchy *inspired by* the Sparkplug B MQTT convention:

```
<group_id>/<message_type>/<edge_node_id>/<device_id>/<point_name>
```

e.g. `site1/telemetry/opta-bridge-01/inv-0001/voltage_a`.

- `group_id` — a logical grouping for the deployment/site (`site1`).
- `message_type` — `telemetry` (device → broker, read-direction registers) or `command`
  (broker → device, write-direction registers).
- `edge_node_id` — identifies the bridge itself (the Opta), matching `mqtt.clientId`.
- `device_id` — the ISA-style device ID described above.
- `point_name` — the specific measurement or command point.

**This is not Sparkplug B compliance.** Real Sparkplug B is a full protocol: Protobuf-encoded
payloads, an `spBv1.0` namespace, defined message types (`NBIRTH`/`DBIRTH`/`NDATA`/`DDATA`/
`NCMD`/`DCMD`), a birth/death certificate lifecycle, and all of a device's metrics bundled into
one payload per topic rather than one topic per point. We only borrow the group/node/device
topic *hierarchy* for readability and familiarity — payloads remain the raw pass-through values
described below, and there is no birth/death lifecycle or bundled metrics. Do not point a real
Sparkplug B-aware consumer at these topics expecting protocol compatibility.

## Value representation

Values that span more than one register (`registerCount > 1`) are passed through as a raw array
of the 16-bit register values in Modbus wire order (e.g. `[18432, 0]`), both when publishing to
MQTT and when accepting an incoming write. The bridge does not interpret sign, float encoding, or
word order — that is left entirely to whatever consumes the MQTT topic.
