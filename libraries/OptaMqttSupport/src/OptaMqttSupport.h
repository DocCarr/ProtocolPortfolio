#ifndef OPTA_MQTT_SUPPORT_H
#define OPTA_MQTT_SUPPORT_H

#include <Arduino.h>
#include <IPAddress.h>

// Callback invoked with a topic and its raw payload whenever a subscribed message arrives.
// This library has no knowledge of what a topic means - interpreting it is the caller's job.
typedef void (*MqttMessageHandler)(const char* topic, const uint8_t* payload, int payloadLength);

// Connects to the given broker. No TLS, no auth (see docs/mapping-config-schema.md).
bool beginMqttClient(IPAddress brokerIp, uint16_t brokerPort, const char* clientId);

// Returns true if the MQTT connection is currently open.
bool isMqttConnected();

// Subscribes to every topic in `topics`, and registers `handler` to be called for any
// incoming message on any of them.
bool subscribeTopics(const char* topics[], size_t count, MqttMessageHandler handler);

// If the connection has dropped, reconnects and re-subscribes to the most recent topic
// list passed to subscribeTopics(), since subscriptions don't survive a session restart.
bool maintainMqttConnection();

// Publishes `payload` to `topic`. Returns false if not currently connected - the caller
// decides whether to retry or drop the value; this library does not buffer or queue.
bool publish(const char* topic, const uint8_t* payload, int payloadLength);

// Dispatches any pending incoming messages, invoking the handler registered in
// subscribeTopics() for each.
void pollIncomingMessages();

#endif
