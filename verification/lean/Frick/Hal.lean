-- SPDX-License-Identifier: MPL-2.0
import Frick.Common

-- frick-fsw formal model: HAL core registry (milestone M3).
--
-- Mirrors the bounded, allocation-free HAL registry (src/hal/hal.c) and proves
-- the two properties that matter for SRS-REQ-025: the number of registered
-- backends never exceeds the configured capacity, and a duplicate registration
-- leaves the registry unchanged. Lean 4 core only (no Mathlib).
--
-- Copyright (C) 2026 Ilfrick
-- https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
-- SPDX-License-Identifier: MPL-2.0

namespace Frick

/-- A HAL registry for one resource class: an ordered list of backend names
with a fixed capacity. -/
structure HalReg where
  cap : Nat
  names : List String

/-- Register a backend name. Returns the same registry unchanged when the class
is full or the name is already present (matching `FF_HAL_*` in `hal.c`). -/
def halRegister (r : HalReg) (n : String) : HalReg :=
  if r.names.length ≥ r.cap then r
  else if n ∈ r.names then r
  else { r with names := n :: r.names }

/-- SRS-REQ-025: the registry count never exceeds the capacity. -/
theorem hal_registry_count_bound (r : HalReg) (n : String) (h : r.names.length ≤ r.cap) :
    (halRegister r n).names.length ≤ (halRegister r n).cap := by
  unfold halRegister
  by_cases hfull : r.names.length ≥ r.cap
  · simp only [if_pos hfull]
    exact h
  · by_cases hmem : n ∈ r.names
    · simp only [if_neg hfull, if_pos hmem]
      exact h
    · simp only [if_neg hfull, if_neg hmem, List.length_cons]
      omega

/-- SRS-REQ-025: a duplicate registration leaves the registry unchanged. -/
theorem hal_dup_registration_unchanged (r : HalReg) (n : String) (h : n ∈ r.names) :
    halRegister r n = r := by
  unfold halRegister
  by_cases hfull : r.names.length ≥ r.cap
  · simp only [if_pos hfull]
  · simp only [if_neg hfull, if_pos h]

end Frick
