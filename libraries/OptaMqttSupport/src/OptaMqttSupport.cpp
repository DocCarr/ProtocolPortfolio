#include "OptaMqttSupport.h"

// Top-down skeleton - bodies filled in bottom-up.

bool beginMqttClient(IPAddress brokerIp, uint16_t brokerPort, const char* clientId) {
  // Connect to the broker (e.g. ArduinoMqttClient's MqttClient::connect()).
  return false;
}

bool isMqttConnected() {
  // Return whether the MQTT connection is currently open.
  return false;
}

bool subscribeTopics(const char* topics[], size_t count, MqttMessageHandler handler) {
  // Store `topics`/`handler` for reuse on reconnect, and subscribe to each topic now.
  return false;
}

bool maintainMqttConnection() {
  // If not connected, reconnect with the last-supplied broker settings, then re-subscribe
  // to the most recent topic list.
  return false;
}

bool publish(const char* topic, const uint8_t* payload, int payloadLength) {
  // Publish `payload` to `topic`. Return false (without buffering) if not connected.
  return false;
}

void pollIncomingMessages() {
  // Check for pending incoming messages and invoke the registered handler for each.
}
