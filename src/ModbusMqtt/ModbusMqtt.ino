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
#include <stdio.h>

// ---- Configuration ----

// Same LittleFS user partition the provisioning sketch writes to.
// See docs/partitioning-notes.md (partition 4, offset 7MB, size 7MB).
const uint32_t USER_PARTITION_INDEX = 4;

// Fixed cap on registers, kept simple rather than a dynamic container. loadConfig() fails
// (triggering reprovisioning) if mapping.json declares more than this many.
const size_t MAX_REGISTERS = 32;

// Required field paths per docs/network-config-schema.md and docs/mapping-config-schema.md.
// Deliberately duplicated from provisioning/OptaProvisioning/OptaProvisioning.ino's copies -
// each sketch is compiled independently and this is sketch-level application data, not
// library code, so this matches the established mechanism-vs-semantics split rather than
// being an oversight.
const char* NETWORK_REQUIRED_FIELDS[] = {
  "ethernet.mac", "ethernet.ip", "ethernet.subnetMask", "ethernet.gateway",
  "wifi.ssid", "wifi.password", "wifi.ip", "wifi.subnetMask", "wifi.gateway"
};
const size_t NETWORK_REQUIRED_FIELD_COUNT = sizeof(NETWORK_REQUIRED_FIELDS) / sizeof(NETWORK_REQUIRED_FIELDS[0]);

const char* MAPPING_REQUIRED_FIELDS[] = {
  "modbus.targetIp", "modbus.targetPort",
  "mqtt.brokerIp", "mqtt.brokerPort", "mqtt.clientId",
  "pollIntervalMs", "registers"
};
const size_t MAPPING_REQUIRED_FIELD_COUNT = sizeof(MAPPING_REQUIRED_FIELDS) / sizeof(MAPPING_REQUIRED_FIELDS[0]);

// Parsed from network.json - see docs/network-config-schema.md.
struct NetworkConfig {
  uint8_t ethernetMac[6];
  IPAddress ethernetIp;
  IPAddress ethernetSubnetMask;
  IPAddress ethernetGateway;

  String wifiSsid;
  String wifiPassword;
  IPAddress wifiIp;
  IPAddress wifiSubnetMask;
  IPAddress wifiGateway;
};

// One entry per register - see docs/mapping-config-schema.md.
struct RegisterMapping {
  String name;
  ModbusRegisterType registerType;
  int address;
  int registerCount;
  bool isWrite;  // true = direction "write" (subscribe), false = "read" (publish)
  String topic;
};

// Parsed from mapping.json - see docs/mapping-config-schema.md.
struct MappingConfig {
  IPAddress modbusTargetIp;
  uint16_t modbusTargetPort;

  IPAddress mqttBrokerIp;
  uint16_t mqttBrokerPort;
  String mqttClientId;

  unsigned long pollIntervalMs;

  RegisterMapping registers[MAX_REGISTERS];
  size_t registerCount;
};

NetworkConfig networkConfig;
MappingConfig mappingConfig;

// Raw JSON text, read once in loadConfig() and used there for the hasRequiredFields() check
// without needing a second file read.
String rawNetworkJson;
String rawMappingJson;

// ---- Stub declarations ----

bool loadConfig();
bool validateConfig();
void haltForReprovisioning();

// Parses a colon-separated MAC address string (e.g. "DE:AD:BE:EF:00:01") into `mac`.
// Returns false if `macStr` is null or doesn't parse as six hex bytes.
bool parseMacAddress(const char* macStr, uint8_t mac[6]);

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

bool parseMacAddress(const char* macStr, uint8_t mac[6]) {
  if (macStr == nullptr) {
    return false;
  }

  int values[6];
  int matched = sscanf(macStr, "%x:%x:%x:%x:%x:%x", &values[0], &values[1], &values[2],
                        &values[3], &values[4], &values[5]);
  if (matched != 6) {
    return false;
  }

  for (int i = 0; i < 6; i++) {
    mac[i] = (uint8_t)values[i];
  }
  return true;
}

bool loadConfig() {
  if (!readFile("network.json", rawNetworkJson) || !readFile("mapping.json", rawMappingJson)) {
    Serial.println("loadConfig: could not read network.json/mapping.json.");
    return false;
  }

  JsonDocument networkDoc;
  if (deserializeJson(networkDoc, rawNetworkJson)) {
    Serial.println("loadConfig: network.json is not valid JSON.");
    return false;
  }

  JsonDocument mappingDoc;
  if (deserializeJson(mappingDoc, rawMappingJson)) {
    Serial.println("loadConfig: mapping.json is not valid JSON.");
    return false;
  }

  // Required-field check happens here, before extraction, not in validateConfig() - fields
  // are pulled out as const char* below and handed to fromString()/parseMacAddress() without
  // a null check, so a missing field must be caught before that point, not after.
  if (!hasRequiredFields(rawNetworkJson, NETWORK_REQUIRED_FIELDS, NETWORK_REQUIRED_FIELD_COUNT)) {
    Serial.println("loadConfig: network.json is missing required fields.");
    return false;
  }
  if (!hasRequiredFields(rawMappingJson, MAPPING_REQUIRED_FIELDS, MAPPING_REQUIRED_FIELD_COUNT)) {
    Serial.println("loadConfig: mapping.json is missing required fields.");
    return false;
  }

  // --- network.json ---
  if (!parseMacAddress(networkDoc["ethernet"]["mac"].as<const char*>(), networkConfig.ethernetMac)) {
    Serial.println("loadConfig: invalid ethernet.mac.");
    return false;
  }
  bool ipsOk = true;
  ipsOk &= networkConfig.ethernetIp.fromString(networkDoc["ethernet"]["ip"].as<const char*>());
  ipsOk &= networkConfig.ethernetSubnetMask.fromString(networkDoc["ethernet"]["subnetMask"].as<const char*>());
  ipsOk &= networkConfig.ethernetGateway.fromString(networkDoc["ethernet"]["gateway"].as<const char*>());

  networkConfig.wifiSsid = networkDoc["wifi"]["ssid"].as<String>();
  networkConfig.wifiPassword = networkDoc["wifi"]["password"].as<String>();
  ipsOk &= networkConfig.wifiIp.fromString(networkDoc["wifi"]["ip"].as<const char*>());
  ipsOk &= networkConfig.wifiSubnetMask.fromString(networkDoc["wifi"]["subnetMask"].as<const char*>());
  ipsOk &= networkConfig.wifiGateway.fromString(networkDoc["wifi"]["gateway"].as<const char*>());

  // --- mapping.json ---
  ipsOk &= mappingConfig.modbusTargetIp.fromString(mappingDoc["modbus"]["targetIp"].as<const char*>());
  mappingConfig.modbusTargetPort = mappingDoc["modbus"]["targetPort"].as<uint16_t>();

  ipsOk &= mappingConfig.mqttBrokerIp.fromString(mappingDoc["mqtt"]["brokerIp"].as<const char*>());
  mappingConfig.mqttBrokerPort = mappingDoc["mqtt"]["brokerPort"].as<uint16_t>();
  mappingConfig.mqttClientId = mappingDoc["mqtt"]["clientId"].as<String>();

  if (!ipsOk) {
    Serial.println("loadConfig: one or more IP address fields failed to parse.");
    return false;
  }

  mappingConfig.pollIntervalMs = mappingDoc["pollIntervalMs"].as<unsigned long>();

  JsonArray registersArray = mappingDoc["registers"].as<JsonArray>();
  if (registersArray.size() > MAX_REGISTERS) {
    Serial.print("loadConfig: too many registers (");
    Serial.print(registersArray.size());
    Serial.print("), max is ");
    Serial.println(MAX_REGISTERS);
    return false;
  }

  mappingConfig.registerCount = 0;
  for (JsonObject reg : registersArray) {
    RegisterMapping& target = mappingConfig.registers[mappingConfig.registerCount];
    target.name = reg["name"].as<String>();
    target.registerType = (reg["registerType"].as<String>() == "holding")
                               ? ModbusRegisterType::Holding
                               : ModbusRegisterType::Input;
    target.address = reg["address"].as<int>();
    target.registerCount = reg["registerCount"].as<int>();
    target.isWrite = (reg["direction"].as<String>() == "write");
    target.topic = reg["topic"].as<String>();
    mappingConfig.registerCount++;
  }

  return true;
}

bool validateConfig() {
  // Required-field presence is already checked in loadConfig() (before field extraction, to
  // avoid null dereferences there) - this function only enforces the semantic rule below.

  // Semantic rule from docs/mapping-config-schema.md: write-direction registers must be
  // holding registers (input registers are read-only on the Modbus device).
  for (size_t i = 0; i < mappingConfig.registerCount; i++) {
    const RegisterMapping& reg = mappingConfig.registers[i];
    if (reg.isWrite && reg.registerType != ModbusRegisterType::Holding) {
      Serial.print("validateConfig: register '");
      Serial.print(reg.name);
      Serial.println("' has direction=write but registerType is not holding.");
      return false;
    }
  }

  return true;
}

void haltForReprovisioning() {
  while (true) {
    Serial.println("Configuration invalid or missing. Re-run the provisioning sketch, then re-flash this sketch.");
    delay(5000);
  }
}

void setupEthernet() {
  // Failure isn't fatal here - loop()'s maintainEthernetConnection() will keep retrying.
  if (!beginEthernet(networkConfig.ethernetMac, networkConfig.ethernetIp,
                      networkConfig.ethernetSubnetMask, networkConfig.ethernetGateway)) {
    Serial.println("setupEthernet: beginEthernet() failed - will keep retrying in loop().");
  }
}

void setupWifi() {
  if (!beginStation(networkConfig.wifiSsid.c_str(), networkConfig.wifiPassword.c_str(),
                     networkConfig.wifiIp, networkConfig.wifiSubnetMask, networkConfig.wifiGateway)) {
    Serial.println("setupWifi: beginStation() failed - will keep retrying in loop().");
  }
}

void setupModbusClient() {
  if (!beginModbusClient(mappingConfig.modbusTargetIp, mappingConfig.modbusTargetPort)) {
    Serial.println("setupModbusClient: beginModbusClient() failed - will keep retrying in loop().");
  }
}

void setupMqttClient() {
  // No TLS, no auth - see docs/mapping-config-schema.md.
  if (!beginMqttClient(mappingConfig.mqttBrokerIp, mappingConfig.mqttBrokerPort,
                        mappingConfig.mqttClientId.c_str())) {
    Serial.println("setupMqttClient: beginMqttClient() failed - will keep retrying in loop().");
  }
}

void setupRegisterTopics() {
  // Build a topic array from every mappingConfig.registers entry with isWrite == true and
  // call OptaMqttSupport::subscribeTopics(topics, count, onMqttMessage).
}

void onMqttMessage(const char* topic, const uint8_t* payload, int payloadLength) {
  // Find the register mapped to `topic`, call validateWritePayloadLength(), and on success
  // call OptaModbusSupport::writeHoldingRegisters() with the payload.
}

void pollAndPublishReadRegisters() {
  // On mappingConfig.pollIntervalMs, for each entry with isWrite == false call
  // OptaModbusSupport::readRegisters() and OptaMqttSupport::publish() with its raw
  // wire-order values.
}

bool validateWritePayloadLength(const RegisterMapping& target, int payloadLength) {
  return false;
}
