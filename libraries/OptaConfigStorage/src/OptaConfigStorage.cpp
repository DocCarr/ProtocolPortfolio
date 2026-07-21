#include "OptaConfigStorage.h"
#include <ArduinoJson.h>
#include "BlockDevice.h"
#include "MBRBlockDevice.h"
#include "LittleFileSystem.h"

using namespace mbed;

// This implementation follows the pattern confirmed in Arduino's own QSPIFormat.ino example
// (BlockDevice::get_default_instance(), explicit init(), MBRBlockDevice over that, and
// mbed::LittleFileSystem mounted on the resulting partition slice) rather than directly
// constructing a QSPIFBlockDevice, which is not how the reference example does it.

namespace {

BlockDevice* root = BlockDevice::get_default_instance();
MBRBlockDevice* userPartition = nullptr;
LittleFileSystem userFilesystem("user");
bool rootInitialized = false;
bool mounted = false;

const char* MOUNT_POINT = "/user/";

String pathFor(const char* filename) {
  String path(MOUNT_POINT);
  path += filename;
  return path;
}

// Returns the MBR partition slice for `partitionIndex`, initializing the underlying block
// device on first use. Returns nullptr if the block device fails to initialize. Assumes a
// single fixed partitionIndex is used for the lifetime of the sketch, matching how this
// library is actually called (always USER_PARTITION_INDEX from both sketches).
MBRBlockDevice* partitionFor(uint32_t partitionIndex) {
  if (!rootInitialized) {
    rootInitialized = (root->init() == BD_ERROR_OK);
    if (!rootInitialized) {
      return nullptr;
    }
  }
  if (userPartition == nullptr) {
    userPartition = new MBRBlockDevice(root, partitionIndex);
  }
  return userPartition;
}

}  // namespace

MountResult mountPartition(uint32_t partitionIndex) {
  MBRBlockDevice* partition = partitionFor(partitionIndex);
  if (partition == nullptr) {
    return MountResult::Error;
  }

  int err = userFilesystem.mount(partition);
  mounted = (err == 0);

  if (mounted) {
    return MountResult::Mounted;
  }

  // Matches Arduino's own QSPIFormat.ino example, which also doesn't distinguish specific
  // mount failure codes - any non-zero mount() result is treated as "no valid filesystem
  // present yet on this partition."
  return MountResult::NotFormatted;
}

bool formatAndMountPartition(uint32_t partitionIndex) {
  MBRBlockDevice* partition = partitionFor(partitionIndex);
  if (partition == nullptr) {
    return false;
  }

  int err = userFilesystem.reformat(partition);
  mounted = (err == 0);
  return mounted;
}

bool readFile(const char* path, String& out) {
  if (!mounted) {
    return false;
  }

  FILE* file = fopen(pathFor(path).c_str(), "r");
  if (file == nullptr) {
    return false;
  }

  out = "";
  char buffer[128];
  size_t bytesRead;
  while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    out.concat(buffer, bytesRead);
  }

  fclose(file);
  return true;
}

bool writeFile(const char* path, const String& contents) {
  if (!mounted) {
    return false;
  }

  FILE* file = fopen(pathFor(path).c_str(), "w");
  if (file == nullptr) {
    return false;
  }

  size_t written = fwrite(contents.c_str(), 1, contents.length(), file);
  fclose(file);
  return written == contents.length();
}

bool deleteFile(const char* path) {
  if (!mounted) {
    return false;
  }

  return remove(pathFor(path).c_str()) == 0;
}

bool isWellFormedJson(const String& json) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  return error == DeserializationError::Ok;
}

namespace {

// Walks `root` following the dot-separated segments of `path` (e.g. "wifi.ssid") and returns
// whether every segment resolves to a present key. Presence only - a field explicitly set to
// JSON null still counts as present. Uses ArduinoJson v6's containsKey(); if this project
// upgrades to ArduinoJson v7 (which deprecated containsKey()), this needs to switch to an
// is<JsonVariant>()-based presence check instead.
bool fieldPathExists(JsonVariantConst docRoot, const String& path) {
  JsonVariantConst current = docRoot;
  int start = 0;

  while (true) {
    int dot = path.indexOf('.', start);
    String segment = (dot == -1) ? path.substring(start) : path.substring(start, dot);

    if (!current.is<JsonObjectConst>()) {
      return false;
    }

    JsonObjectConst obj = current.as<JsonObjectConst>();
    if (!obj.containsKey(segment)) {
      return false;
    }
    current = obj[segment];

    if (dot == -1) {
      return true;
    }
    start = dot + 1;
  }
}

}  // namespace

bool hasRequiredFields(const String& json, const char* requiredFieldPaths[], size_t count) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    return false;
  }

  JsonVariantConst docRoot = doc.as<JsonVariantConst>();
  for (size_t i = 0; i < count; i++) {
    if (!fieldPathExists(docRoot, String(requiredFieldPaths[i]))) {
      return false;
    }
  }
  return true;
}
