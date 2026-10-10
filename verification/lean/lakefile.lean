-- frick-fsw formal verification project (Lean 4)
-- SPDX-License-Identifier: AGPL-3.0-only
import Lake
open Lake DSL

package frick_verify

lean_lib Frick

@[default_target]
lean_exe frick_verify_checks where
  root := `Main