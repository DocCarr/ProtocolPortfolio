#include "OptaMqttSupport.h"
#include <WiFi.h>
#include <ArduinoMqttClient.h>

// Confirmed against the installed ArduinoMqttClient library (Documents/Arduino/libraries/
// ArduinoMqttClient): MqttClient(Client&) rides on any Client-derived transport - WiFiClient
// here, since MQTT always runs over the WiFi station connection in this project.
// connect(ip, port)/connected()/subscribe(topic) return 1/0. Incoming messages don't arrive
// as (topic, payload) directly: onMessage(void(*)(int messageSize)) fires with just a size,
// and messageTopic()/available()/read() must be called from within that callback to retrieve
// the topic and payload - internalOnMessage() below bridges that into this library's
// (topic, payload, length) MqttMessageHandler interface. beginMessage(topic, size)/
// write(buf, size)/endMessage() publish; poll() both dispatches onMessage and handles
// keepalive, and must be called regularly (from pollIncomingMessages()).

namespace {

WiFiClient wifiTransport;
MqttClient mqttClient(wifiTransport);

MqttMessageHandler userHandler = nullptr;

bool haveConnectionSettings = false;
IPAddress savedBrokerIp;
uint16_t savedBrokerPort = 0;
String savedClientId;

String* savedTopics = nullptr;
size_t savedTopicCount = 0;

void internalOnMessage(int messageSize) {
  String topic = mqttClient.messageTopic();

  uint8_t* payload = new uint8_t[messageSize];
  int bytesRead = 0;
  while (bytesRead < messageSize && mqttClient.available()) {
    int b = mqttClient.read();
    if (b < 0) {
      break;
    }
    payload[bytesRead++] = (uint8_t)b;
  }

  if (userHandler != nullptr) {
    userHandler(topic.c_str(), payload, bytesRead);
  }
  delete[] payload;
}

bool resubscribeAll() {
  bool allOk = true;
  for (size_t i = 0; i < savedTopicCount; i++) {
    if (!mqttClient.subscribe(savedTopics[i])) {
      Serial.print("OptaMqttSupport: subscribe() failed for topic ");
      Serial.println(savedTopics[i]);
      allOk = false;
    }
  }
  return allOk;
}

}  // namespace

bool beginMqttClient(IPAddress brokerIp, uint16_t brokerPort, const char* clientId) {
  haveConnectionSettings = true;
  savedBrokerIp = brokerIp;
  savedBrokerPort = brokerPort;
  savedClientId = clientId;

  mqttClient.setId(clientId);
  if (!mqttClient.connect(brokerIp, brokerPort)) {
    Serial.print("OptaMqttSupport: connect() failed, connectError() = ");
    Serial.println(mqttClient.connectError());
    return false;
  }

  mqttClient.onMessage(internalOnMessage);
  return true;
}

bool isMqttConnected() {
  return mqttClient.connected();
}

bool subscribeTopics(const char* topics[], size_t count, MqttMessageHandler handler) {
  userHandler = handler;

  delete[] savedTopics;
  savedTopics = new String[count];
  for (size_t i = 0; i < count; i++) {
    savedTopics[i] = topics[i];
  }
  savedTopicCount = count;

  return resubscribeAll();
}

bool maintainMqttConnection() {
  if (isMqttConnected()) {
    return true;
  }

  if (!haveConnectionSettings) {
    return false;
  }

  if (!beginMqttClient(savedBrokerIp, savedBrokerPort, savedClientId.c_str())) {
    return false;
  }

  if (savedTopics != nullptr && savedTopicCount > 0) {
    return resubscribeAll();
  }
  return true;
}

bool publish(const char* topic, const uint8_t* payload, int payloadLength) {
  if (!isMqttConnected()) {
    return false;
  }

  if (!mqttClient.beginMessage(topic, (unsigned long)payloadLength)) {
    Serial.println("OptaMqttSupport: beginMessage() failed.");
    return false;
  }

  size_t written = mqttClient.write(payload, payloadLength);
  if (written != (size_t)payloadLength) {
    Serial.print("OptaMqttSupport: write() only sent ");
    Serial.print(written);
    Serial.print(" of ");
    Serial.println(payloadLength);
  }

  if (!mqttClient.endMessage()) {
    Serial.println("OptaMqttSupport: endMessage() failed.");
    return false;
  }
  return true;
}

void pollIncomingMessages() {
  mqttClient.poll();
}
