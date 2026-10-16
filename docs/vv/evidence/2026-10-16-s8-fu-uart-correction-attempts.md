# UART corrective-cycle attempts (2026-10-16)

This note preserves superseded attempts for the source delta responding to
FRICK-REVIEW-0325. It does not replace the final passing verification record.

1. A direct host `cmake --build build/ci --target test_link_uart` attempt failed
   because `build/ci/CMakeCache.txt` was created in the container as
   `/workspace/build/ci`; the host build directory is container-owned and its
   Ninja lock could not be written. This was an environment/path failure, not a
   test result. The controlled container `make ci-local` path was used.
2. First container `make ci-local` after changing the test PTY path failed to
   compile: the test still required the removed fixed-path macro, and
   `_XOPEN_SOURCE 600` did not declare `mkdtemp` under the configured GNU C99
   build. The stale `#error` guard was removed and the test feature level raised
   to `_XOPEN_SOURCE 700`; the next cycle compiled and passed.
3. No result from either failed attempt is reported as a pass or used to infer
   source acceptance. Final passing test and full-cycle identities are in the
   current verification-results entry and candidate manifest.
4. The first final-document auxiliary cycle on 2026-10-16 completed smoke,
   GSW/license/metrics, Doxygen, formatting and diff checks, but failed the
   trailing JSON parse because a comma was omitted after the updated
   `status.json` note while reconciling PA 0331. The malformed status file was
   corrected and JSON-validated before rerunning the applicable cycle. The
   failed command output is retained at
   `2026-10-16-s8-fu-final-aux-checks.log`; it is not reported as a pass.
