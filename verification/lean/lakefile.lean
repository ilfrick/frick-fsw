-- frick-fsw formal verification project (Lean 4)
-- Copyright (C) 2026 Ilfrick
-- https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
-- SPDX-License-Identifier: AGPL-3.0-only
import Lake
open Lake DSL

package frick_verify

lean_lib Frick

@[default_target]
lean_exe frick_verify_checks where
  root := `Main