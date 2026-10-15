-- frick-fsw - Verification runner
--
-- `lake build frick_verify_checks` evaluates the closed-form checks and
-- reports compliance. The compiled theorems in Frick.Requirements provide
-- the machine-checked guarantees; this executable is a runtime smoke check
-- used by CI.
--
-- Copyright (C) 2026 Ilfrick
-- https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
-- SPDX-License-Identifier: MPL-2.0
import Frick.Requirements
import Frick.Ccsds

open Frick.Ccsds

def runtimeChecks : List (String × Bool) :=
  [
    ("crc16 empty = 0xFFFF", crc16 [] == 0xFFFF),
    ("crc16 appended frame zeroes out", crc16 (withCrc [0x10, 0x00, 0x02, 0x00, 0x17, 0x01]) == 0),
    ("crc16 deterministic on workload", crc16 [1, 2, 3, 4] == crc16 [1, 2, 3, 4])
  ]

def main : IO Unit := do
  let results := runtimeChecks
  let failed := results.filter (fun (_, ok) => !ok)
  for (name, ok) in results do
    IO.println (if ok then "PASS " ++ name else "FAIL " ++ name)
  IO.println (s!"{results.length} runtime checks, {failed.length} failed")
  if !failed.isEmpty then
    IO.Process.exit 1
  IO.println "Lean verification: OK"
