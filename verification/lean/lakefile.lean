-- frick-fsw formal verification project (Lean 4)
-- Copyright (C) 2026 Ilfrick
-- https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
-- SPDX-License-Identifier: MPL-2.0
import Lake
open Lake DSL

package frick_verify

lean_lib Frick

@[default_target]
lean_exe frick_verify_checks where
  root := `Main
