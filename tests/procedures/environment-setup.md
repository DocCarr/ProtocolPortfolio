# Test Environment Setup: Mosquitto Broker + Modbus Emulator

Reference doc for setting up the supporting infrastructure the bridge-sketch test procedures
depend on (e.g. `002-bridge-inverter.md`). Not itself a test - do this once per test session,
then follow the actual test procedure.

## Network topology reminder

The bridge sketch needs the laptop reachable on **two separate networks at once**:

1. **WiFi**, joined to whatever network `network.json`'s `wifi.*` fields describe (your lab
   WiFi/router) - this is where the Mosquitto broker needs to be reachable, since the bridge's
   WiFi station connection is how it reaches the broker.
2. **Ethernet**, on the isolated Modbus subnet described by `network.json`'s `ethernet.*`
   fields and `mapping.json`'s `modbus.targetIp` - a direct cable (or via a switch) between
   the Opta and the laptop's Ethernet adapter, with the adapter given a static IP on that same
   subnet. This is where the Modbus emulator runs, isolated from the lab network by design.

If your laptop only has one Ethernet port, a USB-to-Ethernet adapter works fine for the second
interface.

## Mosquitto broker setup

1. Install Mosquitto if not already present (https://mosquitto.org/download/).
2. Create a minimal config allowing anonymous connections, matching the no-auth design in
   `docs/mapping-config-schema.md`:
   ```
   listener 1883 0.0.0.0
   allow_anonymous true
   ```
3. Run it verbosely so activity is visible for screenshots: `mosquitto -c mosquitto-test.conf -v`
4. Allow `mosquitto.exe` through Windows Firewall if prompted - otherwise the Opta on the lab
   WiFi won't be able to reach it.
5. Sanity-check before involving the Opta at all, from a separate terminal:
   - `mosquitto_sub -h <laptop-wifi-ip> -t "#" -v` (watch everything)
   - `mosquitto_pub -h <laptop-wifi-ip> -t "test" -m "hello"` (confirm the sub above sees it)
6. Confirm `mapping.json`'s `mqtt.brokerIp` matches the laptop's actual WiFi IP on the lab
   network (not its Ethernet IP on the isolated subnet).

## Modbus emulator setup (ModSim + ModScan)

**ModSim** is the piece that matters here - it simulates the Modbus TCP *server* (slave) the
bridge polls and writes to. **ModScan** is a Modbus TCP *master* (client); it's optional for
this test, useful only as an independent way to poll ModSim directly and confirm the emulator
itself is working, separate from our own bridge sketch.

### ModSim (required)

1. Set the laptop's Ethernet adapter to a static IP **equal to `mapping.json`'s
   `modbus.targetIp`** (e.g. `192.168.1.50` in the example config) - that's the address the
   bridge will actually try to connect to.
2. Open ModSim, start a new connection as **Modbus TCP/IP**, port 502.
3. Configure a **holding register** view (addresses 0-4 in the example `mapping.json`, i.e.
   `start_stop_command` at 0, `real_power_dispatch` at 1-2, `reactive_power_dispatch` at 3-4)
   - this is where the bridge's write-direction commands land.
4. Configure a separate **input register** view (addresses 100-118 in the example, covering
   the three-phase voltage/current, power totals, and status/alarm/fault) - this is what the
   bridge polls and publishes as telemetry. You can type values directly into ModSim's grid to
   simulate live measurements.
5. **Addressing gotcha to watch for**: our sketch uses 0-based Modbus addresses (register 0 =
   the first register in a table), matching the `ArduinoModbus` library's convention. Classic
   Modbus tools including ModScan/ModSim sometimes use or display 1-based addressing instead
   (e.g. entering "1" for the first register, or a "40001"-style display). If reads/writes
   don't seem to hit the register you expect, this off-by-one convention mismatch is the first
   thing to check - I haven't been able to confirm which convention your specific installed
   version defaults to, so verify empirically (e.g. write a distinct value to one register at
   a time and see which address actually changed).

### ModScan (optional cross-check)

Point ModScan at the same target IP/port as an independent master, and try reading/writing a
register directly. If ModScan can talk to ModSim correctly, any problems seen from the Opta
side are isolated to the bridge sketch or the network path to it, not the emulator itself.

Menu paths above are based on the standard ModSim32/ModScan32 UI - your installed version may
differ slightly.
