# src/ModbusMqtt

Main bridge sketch (not yet implemented). Will contain:

- `ModbusMqtt.ino` — entry point (`setup()`/`loop()`)
- `ModbusHandler.h` / `.cpp` — Modbus TCP client (Ethernet side)
- `MqttHandler.h` / `.cpp` — MQTT client (WiFi side)
- `ConfigLoader.h` / `.cpp` — loads network/mapping config from LittleFS

Promoted here from `draft/` once changes are reviewed and accepted.
