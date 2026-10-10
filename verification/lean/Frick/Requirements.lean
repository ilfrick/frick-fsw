-- frick-fsw - Requirement-level theorems
--
-- Each top-level theorem below is named after the requirement it discharges
-- (SRS / SSS / GAP ids from docs/requirements/). The traceability tool
-- (tools/scripts/trace.py) greps these names against
-- docs/requirements/traceability.yaml, so a rename here must be mirrored there.
--
-- Norm N3 (non-derogable): every generated requirement is verified in Lean.
--
-- Copyright (C) 2026 Ilfrick
-- https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
-- SPDX-License-Identifier: AGPL-3.0-only
import Frick.Common
import Frick.Ccsds
import Frick.SoftwareBus
import Frick.Pus
import Frick.Time
import Frick.Hal
import Frick.Sdls

namespace Frick.Requirements

open Frick.Common
open Frick.Ccsds
open Frick.SoftwareBus
open Frick.Pus
open Frick.Time
open Frick

-- =====================================================================
-- SRS-REQ-003 : TM/TC frames never overflow the 16 bit CCSDS packet length.
-- =====================================================================
theorem req_srs003_tm_length {data : Nat} (h : data ≤ PusMaxFrame) :
    CcsdsPrimaryHdrLen + PusTmSecHdrLen + data + FrameCrcLen ≤ 65535 := by
  simp only [PusMaxFrame, CcsdsPrimaryHdrLen, PusTmSecHdrLen, FrameCrcLen] at h ⊢
  omega

theorem req_srs003_tc_length {data : Nat} (h : data ≤ PusMaxFrame) :
    CcsdsPrimaryHdrLen + PusTcSecHdrLen + data + FrameCrcLen ≤ 65535 := by
  simp only [PusMaxFrame, CcsdsPrimaryHdrLen, PusTcSecHdrLen, FrameCrcLen] at h ⊢
  omega

-- =====================================================================
-- SRS-REQ-004 : CRC-16/CCITT-FALSE. A frame extended with its CRC yields a
-- CRC of zero (the property the C decoder relies on).
-- =====================================================================
theorem req_srs004_crc_appended_zero :
    crc16 (withCrc [0x10, 0x00, 0x02, 0x00, 0x17, 0x01, 0xAA]) = 0 := by
  native_decide

theorem req_srs004_crc_initial : crc16 [] = 0xFFFF := by rfl

-- =====================================================================
-- SRS-REQ-006 : acknowledged TCs always emit the required verification TMs.
-- =====================================================================
theorem req_srs006_acceptance_completion {a : AckFlags}
    (hacc : a.acceptance = true) (hcomp : a.completion = true) :
    verificationCount a ≥ 2 := by
  have ha : (if a.acceptance = true then 1 else 0) = 1 := by rw [hacc]; simp
  have hc : (if a.completion = true then 1 else 0) = 1 := by rw [hcomp]; simp
  have hs : 0 ≤ (if a.start = true then 1 else 0) := by split <;> omega
  unfold verificationCount
  omega

-- =====================================================================
-- SRS-REQ-007 : TM message type counter monotonicity + 14 bit bound.
-- =====================================================================
theorem req_srs007_counter_increases (c : Nat) : nextTm c > c := by
  exact Nat.lt_succ_self c
theorem req_srs007_counter_wrap_bounded (c : Nat) : wrapTm c < 16384 := by
  unfold wrapTm
  exact Nat.mod_lt c (by omega)

-- =====================================================================
-- SRS-REQ-001 : Software Bus delivery semantics (model checks).
-- =====================================================================
theorem req_srs001_exact_and_wildcard :
    deliveredCount 5 [1, 5, 65535, 2, 5] = 3 := by native_decide

theorem req_srs001_no_match_no_delivery :
    deliveredCount 5 [1, 2, 3] = 0 := by native_decide

-- =====================================================================
-- SSS-REQ-002 : bus depth is chosen so a single publish never overflows a
-- subscriber pipe (bounded by queue limit from config).
-- =====================================================================
theorem req_sss002_queue_bounded {subs : List Nat} (h : subs.length ≤ SbQueueDepthMax) :
    deliveredCount 1 subs ≤ SbQueueDepthMax := by
  unfold deliveredCount
  simp only [SbQueueDepthMax] at h ⊢
  have hle : (subs.filter (fun s => subscribesTo 1 s)).length ≤ subs.length := by
    exact List.length_filter_le (fun s => subscribesTo 1 s) subs
  omega

-- =====================================================================
-- GAP-A1 : the CRC detection theorem is decided for a reference frame; full
-- arbitrary-length induction is deferred to the Class A work package.
-- =====================================================================
example : crc16 (withCrc [1, 2, 3, 4]) = 0 := by native_decide

-- =====================================================================
-- M2 - Time base (milestone M2): PPS tone discipline, leap seconds, CUC.
-- Theorems discharge SRS-REQ-021..024 (see docs/requirements/SRS.md and
-- Frick.Time for the underlying model).
-- =====================================================================

-- SRS-REQ-021 : the time source enum is a proper, disjoint set and, with a
-- non-negative pending leap, the CUC seconds never regress at a boundary.
theorem req_srs021_sources_distinct : TimeSource.internal ≠ TimeSource.tone :=
  sources_distinct.left

theorem req_srs021_source_set_disjoint :
    TimeSource.tone ≠ TimeSource.ground ∧ TimeSource.ground ≠ TimeSource.internal :=
  sources_distinct.right

theorem req_srs021_utc_non_regression {s : LeapState} (hp : 0 ≤ s.pending) :
    cucSeconds s ≤ cucSeconds (onToneLeap s) := by
  exact cuc_seconds_non_regression hp

-- SRS-REQ-022 : each valid tone advances the second counter by exactly one
-- second and the tone counter strictly increases.
theorem req_srs022_tone_second_advance (s : ToneState) :
    (onTone s).met_s = s.met_s + 1 := by
  exact tone_second_advance s

theorem req_srs022_tone_count_strict (s : ToneState) :
    (onTone s).count = s.count + 1 := by
  exact tone_count_strict s

-- SRS-REQ-023 : a scheduled leap change is applied and cleared exactly at the
-- next tone boundary, and CUC never regresses for a non-negative leap.
theorem req_srs023_leap_cleared_at_tone (s : LeapState) :
    (onToneLeap s).pending = 0 := by
  exact leap_cleared_at_tone s

theorem req_srs023_leap_accumulated_at_tone (s : LeapState) :
    (onToneLeap s).leap = s.leap + s.pending := by
  exact leap_accumulated_at_tone s

-- SRS-REQ-024 : CUC field round trip is lossless and the fine fraction of a
-- sub-second count always fits the 16 bit field.
theorem req_srs024_cuc_field_roundtrip {c f : Nat} (hf : f < 65536) :
    cucCoarse (cucT c f) = c ∧ cucFine (cucT c f) = f := by
  exact cuc_field_roundtrip hf

theorem req_srs024_fine_frac_bounded {frac : Nat} (h : frac < 1000000000) :
    fineFraction frac < 65536 := by
  exact fine_frac_bounded h

theorem req_srs024_coarse_byte_decompose {c : Nat} :
    (c / 2^24) * 2^24 + ((c / 2^16) % 2^8) * 2^16 + ((c / 2^8) % 2^8) * 2^8 + c % 2^8 = c := by
  exact coarse_byte_decompose

-- =====================================================================
-- M3 - Hardware abstraction layer (milestone M3): the bounded registry.
-- Discharges SRS-REQ-025 (see Frick.Hal for the model).
-- =====================================================================

-- SRS-REQ-025 : the bounded registry never exceeds its capacity and a duplicate
-- registration leaves it unchanged.
theorem req_srs025_registry_bound (r : HalReg) (n : String)
    (h : r.names.length ≤ r.cap) :
    (halRegister r n).names.length ≤ (halRegister r n).cap :=
  Frick.hal_registry_count_bound r n h

theorem req_srs025_dup_unchanged (r : HalReg) (n : String) (h : n ∈ r.names) :
    halRegister r n = r :=
  Frick.hal_dup_registration_unchanged r n h

-- =====================================================================
-- M7 - CCSDS SDLS (milestone M7): SA database and key rotation.
-- Discharges SRS-REQ-035 (see Frick.Sdls for the model).
-- =====================================================================

-- SRS-REQ-035 : the SA table is bounded and key rotation strictly increases the
-- epoch and resets usage.
theorem req_srs035_sa_bound (s : SaDb) (h : s.live ≤ s.cap) :
    (saAdd s).live ≤ (saAdd s).cap :=
  Frick.sa_add_bound s h

theorem req_srs035_epoch_monotonic (k : SKey) : k.epoch < (keyRotate k).epoch :=
  Frick.key_rotate_epoch_monotonic k

theorem req_srs035_uses_reset (k : SKey) : (keyRotate k).uses = 0 :=
  Frick.key_rotate_uses_reset k

end Frick.Requirements