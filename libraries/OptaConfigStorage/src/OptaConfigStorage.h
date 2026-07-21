#ifndef OPTA_CONFIG_STORAGE_H
#define OPTA_CONFIG_STORAGE_H

#include <Arduino.h>

// This library only handles mechanics: mounting the filesystem, reading/writing/deleting
// files, and structural JSON checks. It has no knowledge of what any field means - required
// field paths and file contents are supplied entirely by the caller.

enum class MountResult {
  Mounted,
  NotFormatted,
  Error
};

// Mounts LittleFS on the given MBR partition index (over the onboard QSPI flash). Never
// formats - callers decide what to do based on the returned result.
MountResult mountPartition(uint32_t partitionIndex);

// Formats the given MBR partition index and mounts a fresh LittleFS filesystem on it.
// Callers are responsible for deciding when this is safe to invoke.
bool formatAndMountPartition(uint32_t partitionIndex);

// Reads the full contents of a file into `out`. Returns false if the file doesn't exist.
bool readFile(const char* path, String& out);

// Writes `contents` to a file, overwriting it if it already exists.
bool writeFile(const char* path, const String& contents);

// Deletes a file. Returns false if it didn't exist.
bool deleteFile(const char* path);

// Returns true if `json` parses as well-formed JSON.
bool isWellFormedJson(const String& json);

// Returns true if every path in `requiredFieldPaths` (dot-separated, e.g. "wifi.ssid") is
// present in `json`. Checks presence only - not value types or semantics.
bool hasRequiredFields(const String& json, const char* requiredFieldPaths[], size_t count);

#endif
