#include "OptaConfigStorage.h"

// Top-down skeleton - bodies filled in bottom-up.

MountResult mountPartition(uint32_t partitionIndex) {
  // Mount LittleFS via MBRBlockDevice(&root, partitionIndex), over QSPIFBlockDevice.
  // Return NotFormatted if the mount fails because the partition has never been formatted,
  // Error for any other failure, Mounted on success. Never formats here.
  return MountResult::Error;
}

bool formatAndMountPartition(uint32_t partitionIndex) {
  // Format the given MBR partition and mount a fresh LittleFS filesystem on it.
  return false;
}

bool readFile(const char* path, String& out) {
  // Read the full contents of `path` from the mounted filesystem into `out`.
  return false;
}

bool writeFile(const char* path, const String& contents) {
  // Write `contents` to `path`, overwriting any existing file.
  return false;
}

bool deleteFile(const char* path) {
  // Delete `path` from the mounted filesystem.
  return false;
}

bool isWellFormedJson(const String& json) {
  // Attempt to parse `json` (e.g. via ArduinoJson's deserializeJson) and return whether it
  // succeeded.
  return false;
}

bool hasRequiredFields(const String& json, const char* requiredFieldPaths[], size_t count) {
  // Parse `json`, then confirm each dot-separated path in requiredFieldPaths resolves to a
  // present field. Presence only - no type or value checking.
  return false;
}
