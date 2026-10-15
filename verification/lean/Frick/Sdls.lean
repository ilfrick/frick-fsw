-- SPDX-License-Identifier: MPL-2.0
import Frick.Common

-- frick-fsw formal model: CCSDS SDLS SA database and key rotation (milestone M7).
--
-- Mirrors the bounded SA registry and the key lifecycle of src/security/ff_sdls.c
-- and proves the properties that matter for SRS-REQ-035: the number of security
-- associations never exceeds the capacity, and rotating a key strictly increases
-- the key epoch and resets the usage counter. Lean 4 core only (no Mathlib).
--
-- Copyright (C) 2026 Ilfrick
-- https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
-- SPDX-License-Identifier: MPL-2.0

namespace Frick

/-- Security-association registry for one bounded table. -/
structure SaDb where
  cap : Nat
  live : Nat

/-- Add an SA; unchanged when the table is full (matching FF_SdlsSaAdd). -/
def saAdd (s : SaDb) : SaDb :=
  if s.live < s.cap then { s with live := s.live + 1 } else s

/-- SRS-REQ-035: the SA count never exceeds the capacity. -/
theorem sa_add_bound (s : SaDb) (h : s.live ≤ s.cap) : (saAdd s).live ≤ (saAdd s).cap := by
  unfold saAdd
  by_cases hf : s.live < s.cap
  · simp only [if_pos hf]
    omega
  · simp only [if_neg hf]
    exact h

/-- Session-key state: rotation epoch and usage counter. -/
structure SKey where
  epoch : Nat
  uses : Nat

/-- Rotate the session key: zeroise (usage) and derive a new key (epoch + 1). -/
def keyRotate (k : SKey) : SKey :=
  { k with epoch := k.epoch + 1, uses := 0 }

/-- SRS-REQ-035: rotation strictly increases the key epoch (no key reuse). -/
theorem key_rotate_epoch_monotonic (k : SKey) : k.epoch < (keyRotate k).epoch := by
  unfold keyRotate
  exact Nat.lt_succ_self k.epoch

/-- SRS-REQ-035: rotation resets the usage counter. -/
theorem key_rotate_uses_reset (k : SKey) : (keyRotate k).uses = 0 := by
  rfl

end Frick
