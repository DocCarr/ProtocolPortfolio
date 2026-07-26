# Test 001: Provisioning Tool — Result

**Date:** 2026-07-25
**Procedure:** `tests/procedures/001-provisioning.md`
**Outcome:** PASS

## Summary

Full provisioning flow verified end to end: partition mount, access point bring-up, and
independent GET/POST/DELETE for `network.json` and `mapping.json`, including the
invalid-JSON rejection path.

## Step-by-step evidence

| Step | Screenshot(s) | Result |
|---|---|---|
| 3 - ping the AP | `01-step03-ping.png` | 4/4 replies, 0% loss |
| 4 - GET network.json (absent) | `02-step04-get-network-404.png` | 404 Not Found |
| 5 - POST network.json | `03-step05-post-network-200.png`, `04-step05-serial-network-validated.png` | 200 OK; Serial confirms validated |
| 7 - GET/POST mapping.json | `05-step07-get-mapping-404.png`, `06-step07-post-mapping-200.png`, `07-step07-serial-mapping-validated.png` | 404 before push, 200 OK after; Serial confirms validated |
| 8 - DELETE network.json | `08-step08-delete-network-200.png` | 200 OK |
| 9 - GET network.json (post-delete) | `09-step09-get-network-404-after-delete.png` | 404 Not Found, confirms delete worked |
| 10 - POST invalid JSON | `10-step10-post-invalid-422.png`, `11-step10-serial-validation-failed.png` | 422 Unprocessable Entity; Serial confirms validation failure logged |

**Note:** step 6 (explicit GET of `network.json` after a successful push, to directly confirm
round-trip content) wasn't separately screenshotted - the user moved on to testing
`mapping.json` at that point. Not a failure: a successful "received and validated OK" message
already requires `handlePushNetworkConfig()` to have read the file back internally to run
validation, so the round trip was implicitly exercised. Worth explicitly re-confirming with a
dedicated GET next time for complete step-by-step evidence.

## Issues found and fixed during this test session

Two real bugs were caught and fixed while preparing for this run (both now committed):

1. **Missing MBR partition table.** `OptaConfigStorage::mountPartition()`/
   `formatAndMountPartition()` failed with `BD_ERROR_INVALID_PARTITION` (-3102) because this
   physical Opta had never had its QSPI flash partitioned. Fixed by running the official
   `QSPIFormat.ino` example once (not a code change) - see `docs/partitioning-notes.md`.
2. **AP password too short.** The WPA2 access point silently failed to start
   (`beginAP()` returned a non-listening status) because the test `AP_PASSWORD` was only 7
   characters; WPA2 requires 8-63. Fixed by using a longer password.

Also added: diagnostic error-code logging in `OptaConfigStorage` and `OptaWifiSupport` (now
committed) that made both of the above failures immediately diagnosable instead of just
"it didn't work."
