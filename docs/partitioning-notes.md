# Flash Partitioning

The Opta's 16 MB external QSPI flash (separate from the STM32H747's own 2 MB internal program
flash) is shared between Arduino's own firmware/update mechanisms and space available for our
own data. Setting up a filesystem naively risks erasing data those mechanisms depend on — this
happened once during prior work on this board (WiFi firmware wiped, then restored via the
recovery sketch noted below).

## Partition layout (confirmed)

Per Arduino's own `STM32H747_System/examples/QSPIFormat/QSPIFormat.ino` and the Opta memory
partitioning tutorial:

| Partition | Offset | Size | Purpose |
|---|---|---|---|
| 1 | 0 MB | 1 MB | WiFi module firmware + TLS certificates |
| 2 | 1 MB | 5 MB | OTA |
| 3 | 6 MB | 1 MB | Provisioning KVStore |
| 4 | 7 MB | 7 MB | **User data** (our LittleFS config storage) |
| — | 14 MB | 2 MB | Reserved (memory-mapped firmware copy) |

**The provisioning sketch must only write within partition 4 (offset 7 MB, size 7 MB).**
Partitions 1-3 and the reserved region at the end are all in active use by Arduino's own
firmware/update/provisioning mechanisms — not just partition 1, as first assumed. Writing into
partition 2 or 3 would corrupt OTA or provisioning data even though the WiFi firmware itself
(partition 1) would be untouched.

## Mounting the user partition

Use mbed's `MBRBlockDevice`, layered over `QSPIFBlockDevice`, referencing partition **index 4**
— not a raw byte offset:

```cpp
QSPIFBlockDevice root(...);
MBRBlockDevice userData(&root, 4);
```

## Recovery path

If the WiFi firmware/certificate partition is ever corrupted, Arduino ships a recovery sketch —
`WiFiFirmwareUpdater.ino` (from `arduino/ArduinoCore-mbed`) — that rewrites partition 1 with the
wireless module firmware and certificates. This is the same mechanism used previously to restore
WiFi functionality after it was wiped during earlier work on this board.

## Sources

- Arduino Opta Collective Datasheet (docs.arduino.cc) — confirms 16 MB external QSPI flash vs.
  2 MB internal STM32 flash.
- `arduino/ArduinoCore-mbed`: `WiFiFirmwareUpdater.ino` and
  `STM32H747_System/examples/QSPIFormat/QSPIFormat.ino` — partition layout and write mechanism.
- Opta memory partitioning tutorial (mirrored at opta.findernet.com/en/tutorial/memory-partitioning;
  the docs.arduino.cc version renders via JavaScript and couldn't be fetched directly).
- `arduino/docs-content` — notes the QSPI flash "is also filled with other data, such as the
  Wi-Fi firmware."

## Note on LittleFS and the PLC IDE

Arduino's Opta PLC IDE runtime does not support LittleFS (FatFS only there). This restriction is
specific to the PLC IDE runtime and does not apply here — this project uses plain Arduino
sketches, not the PLC IDE.
