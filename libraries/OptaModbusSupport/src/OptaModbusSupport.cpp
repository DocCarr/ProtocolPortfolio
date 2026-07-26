#include "OptaModbusSupport.h"
#include <Ethernet.h>
#include <ArduinoModbus.h>

// Confirmed against the installed ArduinoModbus library (Documents/Arduino/libraries/
// ArduinoModbus): ModbusTCPClient(Client&) rides on any Client-derived transport -
// EthernetClient here, since Modbus always runs over Ethernet in this project. begin(ip, port)
// and connected() return 1/0. Multi-register reads use requestFrom(type, address, count) then
// available()/read() per value (HOLDING_REGISTERS/INPUT_REGISTERS from ModbusClient.h); reads
// return -1 per value on failure. Multi-register writes use beginTransmission(type, address,
// count), write(value) per value, then endTransmission() - each returns 1/0.

namespace {

EthernetClient ethernetTransport;
ModbusTCPClient modbusClient(ethernetTransport);

bool haveSettings = false;
IPAddress savedTargetIp;
uint16_t savedTargetPort = 0;

}  // namespace

bool beginModbusClient(IPAddress targetIp, uint16_t targetPort) {
  haveSettings = true;
  savedTargetIp = targetIp;
  savedTargetPort = targetPort;

  if (!modbusClient.begin(targetIp, targetPort)) {
    Serial.println("OptaModbusSupport: ModbusTCPClient::begin() failed.");
    return false;
  }
  return true;
}

bool isModbusConnected() {
  return modbusClient.connected();
}

bool maintainModbusConnection() {
  if (isModbusConnected()) {
    return true;
  }

  if (!haveSettings) {
    return false;
  }

  return beginModbusClient(savedTargetIp, savedTargetPort);
}

bool readRegisters(ModbusRegisterType type, int address, int count, uint16_t* out) {
  int modbusType = (type == ModbusRegisterType::Holding) ? HOLDING_REGISTERS : INPUT_REGISTERS;

  int available = modbusClient.requestFrom(modbusType, address, count);
  if (available != count) {
    Serial.print("OptaModbusSupport: requestFrom() returned ");
    Serial.println(available);
    return false;
  }

  for (int i = 0; i < count; i++) {
    long value = modbusClient.read();
    if (value < 0) {
      Serial.println("OptaModbusSupport: read() failed mid-transfer.");
      return false;
    }
    out[i] = (uint16_t)value;
  }
  return true;
}

bool writeHoldingRegisters(int address, int count, const uint16_t* values) {
  if (!modbusClient.beginTransmission(HOLDING_REGISTERS, address, count)) {
    Serial.println("OptaModbusSupport: beginTransmission() failed.");
    return false;
  }

  for (int i = 0; i < count; i++) {
    if (!modbusClient.write(values[i])) {
      Serial.println("OptaModbusSupport: write() failed mid-transfer.");
      return false;
    }
  }

  if (!modbusClient.endTransmission()) {
    Serial.println("OptaModbusSupport: endTransmission() failed.");
    return false;
  }
  return true;
}
