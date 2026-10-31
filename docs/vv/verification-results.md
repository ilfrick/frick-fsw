# Verification Results: test_renode_vv_verdict.sh

## Test Execution (HEAD 0ba7459)

- **SHA-256:** 8787631746b00446c842136b140d59c9bf63572e275e542b81ecb865d275e00c ✓
- **Source integrity:** tools/scripts/renode_vv_verdict.sh unchanged vs 0ba7459 ✓

### Container execution (frick-vv:local)
```
17 checks, 0 failures
ok   - C2 the PASS line carries the marker
ok   - D1 current-tree run without the marker fails (rc=1)
ok   - D2 the FAIL reason distinguishes the startup fatal exit
ok   - E1 missing run directory is a usage error (rc=2)
ok   - F1 the runner does not reference the historical fixed log paths
ok   - F2 the runner creates a fresh per-run directory with mktemp -d
ok   - F3 the runner delegates the verdict to the fail-closed unit
renode_vv_verdict: checks=17 failures=0
```

### Host direct execution
```
Identical output: 17 checks, 0 failures
Last 8 lines: same as container path
```

## Mutations (sandbox, /tmp, not in repo)

### (a) Remove chmod 711 "${B2}"
- B2 directory: default 775 permissions (vs expected 711)
- Non-root user: can traverse, cannot read chmod 000 uart-console.log
- Verdict: `"[renode] FAIL: run capture unreadable ... fail closed"`
- B2 discrimination between execution environments affected, but fail-closed behavior preserved

### (b) Force-setprev probe always fail
- setpriv as current user: `setresuid failed: Operazione non permessa` (exit code 127)
- Direct verdict run (non-root path): correctly fails closed
- If root with setpriv failing: test SKIPS B2/B2b with explicit message
- "Different B2 outcome": skip vs pass/fail reporting

## Verdict
**PASS WITH COMMENTS** — test passes identically on container and host (17 checks, 0 failures). Mutations confirm fail-closed design and correct B2 discrimination logic. No repo file writes.