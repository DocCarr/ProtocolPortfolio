#ifndef OPTA_MODBUS_SUPPORT_H
#define OPTA_MODBUS_SUPPORT_H

#include <Arduino.h>
#include <IPAddress.h>

enum class ModbusRegisterType {
  Holding,
  Input
};

// Connects the Modbus TCP client to the given target device.
bool beginModbusClient(IPAddress targetIp, uint16_t targetPort);

// Returns true if the Modbus TCP connection is currently open.
bool isModbusConnected();

// If the connection has dropped, reopens the socket using the settings supplied to the
// most recent beginModbusClient() call. Modbus TCP is stateless request/response, so no
// other state needs to be restored on reconnect.
bool maintainModbusConnection();

// Reads `count` consecutive registers of the given type starting at `address` into `out`
// (must have space for `count` uint16_t values, in wire order). Returns false on any
// Modbus error.
bool readRegisters(ModbusRegisterType type, int address, int count, uint16_t* out);

// Writes `count` consecutive holding registers starting at `address` from `values` (wire
// order). Returns false on any Modbus error. Only holding registers are writable.
bool writeHoldingRegisters(int address, int count, const uint16_t* values);

#endif
