// ModbusMqtt.ino
// Top-down skeleton: setup()/loop() call stubs, filled in bottom-up.
// Generic connectivity/storage mechanics live in libraries/ (see each library's header).
// This sketch owns configuration semantics, validation, and the actual bridge logic.

#include <OptaConfigStorage.h>
#include <OptaEthernetSupport.h>
#include <OptaWifiSupport.h>
#include <OptaModbusSupport.h>
#include <OptaMqttSupport.h>
#include <ArduinoJson.h>

// ---- Configuration ----

// Same LittleFS user partition the provisioning sketch writes to.
// See docs/partitioning-notes.md (partition 4, offset 7MB, size 7MB).
const uint32_t USER_PARTITION_INDEX = 4;

// Parsed from network.json - see docs/network-config-schema.md.
struct NetworkConfig {
  // ethernet: mac, ip, subnetMask, gateway
  // wifi: ssid, password, ip, subnetMask, gateway
};

// One entry per register - see docs/mapping-config-schema.md.
struct RegisterMapping {
  // name, registerType, address, registerCount, direction, topic
};

// Parsed from mapping.json - see docs/mapping-config-schema.md.
struct MappingConfig {
  // modbus: targetIp, targetPort
  // mqtt: brokerIp, brokerPort, clientId
  // pollIntervalMs
  // registers: RegisterMapping list
};

NetworkConfig networkConfig;
MappingConfig mappingConfig;

// ---- Stub declarations ----

bool loadConfig();
bool validateConfig();
void haltForReprovisioning();

// Thin wrapper stubs: pull the relevant fields out of networkConfig/mappingConfig and call
// the corresponding library begin*() function. All retry/reconnect mechanics live in the
// libraries - loop() calls their maintain*()/poll*() functions directly, no wrapper needed.
void setupEthernet();
void setupWifi();
void setupModbusClient();
void setupMqttClient();

// Builds the write-direction topic list from mappingConfig.registers and calls
// OptaMqttSupport::subscribeTopics(). OptaMqttSupport re-invokes this automatically on
// reconnect internally, since subscriptions don't survive a session restart.
void setupRegisterTopics();

// Registered with OptaMqttSupport::subscribeTopics() as the message handler. Maps an
// incoming topic back to a register, validates payload length, and writes it to Modbus.
// This is application logic - the library has no concept of "register."
void onMqttMessage(const char* topic, const uint8_t* payload, int payloadLength);

// Polls direction:"read" registers on mappingConfig.pollIntervalMs, reading each via
// OptaModbusSupport::readRegisters() and publishing its raw wire-order value via
// OptaMqttSupport::publish(). Drops (does not buffer) values that can't be published if
// MQTT isn't currently connected - no queuing, consistent with the no-intermediate-
// processing design.
void pollAndPublishReadRegisters();

// Confirms an incoming write payload's length matches the target register's declared
// registerCount before any write is attempted.
bool validateWritePayloadLength(const RegisterMapping& target, int payloadLength);

// ---- setup/loop ----

void setup() {
  Serial.begin(9600);

  MountResult mountResult = mountPartition(USER_PARTITION_INDEX);
  if (mountResult != MountResult::Mounted || !loadConfig() || !validateConfig()) {
    haltForReprovisioning();
  }

  setupEthernet();
  setupWifi();
  setupModbusClient();
  setupMqttClient();
  setupRegisterTopics();
}

void loop() {
  maintainEthernetConnection();
  maintainStationConnection();
  maintainModbusConnection();
  maintainMqttConnection();

  pollAndPublishReadRegisters();
  pollIncomingMessages();
}

// ---- Stub bodies (to be implemented bottom-up) ----

bool loadConfig() {
  // Read network.json and mapping.json (via OptaConfigStorage::readFile()) and parse them
  // into networkConfig/mappingConfig. Return false if either file is missing or not
  // well-formed JSON (OptaConfigStorage::isWellFormedJson()).
  return false;
}

bool validateConfig() {
  // Confirm required fields are present (OptaConfigStorage::hasRequiredFields(), with this
  // sketch supplying the required-field-path list for each file) and enforce the semantic
  // rule that direction "write" is only valid when registerType is "holding" (see
  // docs/mapping-config-schema.md). Both checks are application-specific and stay here.
  return false;
}

void haltForReprovisioning() {
  while (true) {
    Serial.println("Configuration invalid or missing. Re-run the provisioning sketch, then re-flash this sketch.");
    delay(5000);
  }
}

void setupEthernet() {
  // OptaEthernetSupport::beginEthernet() using networkConfig.ethernet (mac/ip/subnetMask/gateway).
}

void setupWifi() {
  // OptaWifiSupport::beginStation() using networkConfig.wifi (ssid/password/ip/subnetMask/gateway).
}

void setupModbusClient() {
  // OptaModbusSupport::beginModbusClient() using mappingConfig.modbus (targetIp/targetPort).
}

void setupMqttClient() {
  // OptaMqttSupport::beginMqttClient() using mappingConfig.mqtt (brokerIp/brokerPort/clientId).
  // No TLS, no auth - see docs/mapping-config-schema.md.
}

void setupRegisterTopics() {
  // Build a topic array from every direction:"write" entry in mappingConfig.registers and
  // call OptaMqttSupport::subscribeTopics(topics, count, onMqttMessage).
}

void onMqttMessage(const char* topic, const uint8_t* payload, int payloadLength) {
  // Find the register mapped to `topic`, call validateWritePayloadLength(), and on success
  // call OptaModbusSupport::writeHoldingRegisters() with the payload.
}

void pollAndPublishReadRegisters() {
  // On mappingConfig.pollIntervalMs, for each direction:"read" register call
  // OptaModbusSupport::readRegisters() and OptaMqttSupport::publish() with its raw
  // wire-order values.
}

bool validateWritePayloadLength(const RegisterMapping& target, int payloadLength) {
  return false;
}
