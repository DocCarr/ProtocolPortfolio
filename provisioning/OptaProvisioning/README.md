# provisioning/OptaProvisioning

Separate sketch (not yet implemented), run once before the main program, that hosts a WiFi
access point and a local web form to collect network and mapping configuration. Writes the
submitted config to the Opta's LittleFS filesystem for the main sketch to load at boot.

See `docs/partitioning-notes.md` for an open flash-partitioning issue affecting this sketch.
