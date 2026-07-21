#include "OptaModbusSupport.h"

// Top-down skeleton - bodies filled in bottom-up.

bool beginModbusClient(IPAddress targetIp, uint16_t targetPort) {
  // Open the Modbus TCP connection (e.g. ArduinoModbus's ModbusTCPClient::begin()).
  return false;
}

bool isModbusConnected() {
  // Return whether the Modbus TCP connection is currently open.
  return false;
}

bool maintainModbusConnection() {
  // If the connection is closed, reopen it with the last-supplied target IP/port.
  return false;
}

bool readRegisters(ModbusRegisterType type, int address, int count, uint16_t* out) {
  // Issue a Modbus read (holding or input, per `type`) for `count` registers starting at
  // `address` and copy the raw wire-order values into `out`.
  return false;
}

bool writeHoldingRegisters(int address, int count, const uint16_t* values) {
  // Issue a Modbus write for `count` holding registers starting at `address`.
  return false;
}
