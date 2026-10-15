# S4c configuration negative-test output (2026-10-15)

These captured outputs are local, mutable-worktree evidence. They exercise
fail-closed behavior only; they are not tool qualification or release evidence.

## Explicit ASN.1 selection with generated sources absent

Procedure: temporarily move `third_party/asn1-pusc-lib-generated` out of the
source tree; run
`cmake -S . -B build/pus-asn1-missing-final -DFF_USE_PUS_ASN1=ON`; restore the
directory. Configure exit status: **1**, as expected.

```text
-- The C compiler identification is GNU 15.2.0
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
CMake Error at CMakeLists.txt:63 (message):
  FF_USE_PUS_ASN1=ON but generated PUS-C ASN.1 sources are missing.  Run
  'make pus-asn1' and reconfigure, or set FF_USE_PUS_ASN1=OFF to select the
  open CCSDS backend.

-- Configuring incomplete, errors occurred!
```

## Reused lwIP checkout dirty-state rejection

Procedure: create an untracked probe file under `third_party/lwip`, run
`bash tools/scripts/fetch_third_party.sh lwip-only`, remove the probe, then rerun
the fetch command. First exit status: **1** (expected); clean checkout exit
status: **0**.

```text
[fetch] error: lwIP checkout is modified: /home/nicola/fsw/frick-fsw/third_party/lwip
?? UNTRACKED_S4C_FETCH_TEST
```

Clean retry:

```text
[fetch] done (lwip-only): /home/nicola/fsw/frick-fsw/third_party
```

The initial probe above established untracked-file rejection. A follow-up after
the fetch script was tightened to inspect ignored content also exercised both
wrong-revision and ignored-file states. These are local behavior probes, not
tool qualification; no immutable candidate manifest is asserted.

## Wrong-revision rejection

Procedure: create a temporary commit object with the same tree as the pinned
lwIP commit, detach the reused checkout at that different commit, then run
`bash tools/scripts/fetch_third_party.sh lwip-only`; restore the pinned HEAD and
rerun clean. Wrong-revision exit status: **1**; clean retry: **0**. Captured
output:

```text
[fetch] error: lwIP revision mismatch: expected 77dcd25a72509eb83f72b033d219b1d40cd8eb95, found 93fe03b12ee90eb71fd81921bae73fd951817b2c
```

## Ignored-file rejection

Procedure: add a temporary probe pattern to `third_party/lwip/.git/info/exclude`,
create the ignored probe file, and run the same fetch command. The fetch failed
with exit status **1**. The exclude-file change and probe were removed, then a
clean fetch returned **0**. Captured output:

```text
[fetch] error: lwIP checkout is modified: /home/nicola/fsw/frick-fsw/third_party/lwip
!! S4C_IGNORED_PROBE
```

Raw outputs are retained as `2026-10-15-lwip-wrong-revision.log`,
`2026-10-15-lwip-ignored-file.log`, and `2026-10-15-lwip-clean-retry.log`.
The probe covers lwIP's reused checkout; the same `verify_checkout` function is
used for the optional ASN.1 repository, whose own runtime inputs still require
provenance/licensing review.

## Tracked-file rejection follow-up

Procedure: temporarily append a unique marker to the tracked `README` file in
the lwIP checkout, run the same fetch command, restore the file, and rerun a
clean fetch. Tracked-dirty exit status: **1**; clean retry: **0**. Captured
output:

```text
[fetch] error: lwIP checkout is modified: /home/nicola/fsw/frick-fsw/third_party/lwip
 M README
```

Raw output and clean retry are retained as `2026-10-15-lwip-tracked-file.log`
and `2026-10-15-lwip-tracked-clean-retry.log`.
