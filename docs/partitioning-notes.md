# Open Issue: LittleFS Partitioning

The Opta WiFi module's firmware/data can share flash space with the LittleFS region we plan to
use for provisioning-written configuration. Setting up our filesystem naively risks erasing data
the WiFi module depends on.

Needs investigation before the provisioning sketch is finalized:

- Confirm the flash layout Arduino's mbed core uses for the Opta.
- Determine a partition scheme that keeps our LittleFS region separate from anything the WiFi
  module relies on.
- Document the safe partition boundaries here once confirmed.
