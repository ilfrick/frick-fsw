-- frick-fsw - Formal model: time base (PPS tone discipline, leap seconds, CUC)
--
-- Mirrors src/core/time/time.c and src/core/time/pps.c (milestone M2):
--   * tone discipline: the second counter advances by exactly one second at
--     each valid 1 Hz tone;
--   * leap-second scheduling: a pending leap is applied and cleared exactly at
--     the next tone boundary; with a non-negative pending leap the CUC
--     seconds never regress across the boundary;
--   * CUC arithmetic (CCSDS 301.0-B-4): the coarse/fine fields are recovered
--     losslessly from the T-field value, the fine fraction of any sub-second
--     count always fits 16 bits and the 4-octet big-endian coarse bytes
--     recombine to the original value.
--
-- Copyright (C) 2026 Ilfrick
-- https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
-- SPDX-License-Identifier: AGPL-3.0-only
import Frick.Common

namespace Frick.Time

-- =====================================================================
-- Time source selection (SRS-REQ-021)
-- =====================================================================

inductive TimeSource where
  | internal | tone | ground
deriving Repr, DecidableEq

-- | The three sources in the C enum form a proper, disjoint set.
theorem sources_distinct :
    TimeSource.internal ≠ TimeSource.tone ∧
    TimeSource.tone ≠ TimeSource.ground ∧
    TimeSource.ground ≠ TimeSource.internal := by
  decide

-- =====================================================================
-- Tone discipline (SRS-REQ-022)
-- =====================================================================

structure ToneState where
  count : Nat   -- number of valid tones received
  met_s : Nat   -- whole seconds advanced by the tones
deriving Repr

def onTone (s : ToneState) : ToneState :=
  { count := s.count + 1, met_s := s.met_s + 1 }

-- | Each valid tone advances the second counter by exactly one second.
theorem tone_second_advance (s : ToneState) : (onTone s).met_s = s.met_s + 1 := by
  simp [onTone]

-- | The tone counter strictly increases at every tone.
theorem tone_count_strict (s : ToneState) : (onTone s).count = s.count + 1 := by
  simp [onTone]

-- =====================================================================
-- Leap-second application at the tone boundary (SRS-REQ-023)
-- =====================================================================

structure LeapState where
  tone_s : Nat   -- whole seconds advanced by tones (never adjusted)
  leap : Int     -- accumulated leap-second offset added by CUC
  pending : Int  -- scheduled leap change, applied at the next tone
deriving Repr

def onToneLeap (s : LeapState) : LeapState :=
  { tone_s := s.tone_s + 1, leap := s.leap + s.pending, pending := 0 }

-- | The scheduled leap change is consumed exactly at the boundary.
theorem leap_cleared_at_tone (s : LeapState) : (onToneLeap s).pending = 0 := by
  simp [onToneLeap]

-- | The accumulated leap offset steps by the pending amount.
theorem leap_accumulated_at_tone (s : LeapState) : (onToneLeap s).leap = s.leap + s.pending := by
  simp [onToneLeap]

-- | CUC seconds are the tone seconds plus the accumulated leap offset.
def cucSeconds (s : LeapState) : Int := (s.tone_s : Int) + s.leap

-- | With a non-negative pending leap the CUC seconds never regress across
--   the tone boundary.
theorem cuc_seconds_non_regression {s : LeapState} (hp : 0 ≤ s.pending) :
    cucSeconds s ≤ cucSeconds (onToneLeap s) := by
  simp [cucSeconds, onToneLeap]
  omega

-- =====================================================================
-- CCSDS Unsegmented Time Code arithmetic (SRS-REQ-024)
-- =====================================================================

-- | T-field value: 4-octet coarse (seconds) + 2-octet fine (2^-16 s).
def cucT (coarse : Nat) (fine : Nat) : Nat := coarse * 65536 + fine

def cucCoarse (v : Nat) : Nat := v / 65536
def cucFine (v : Nat) : Nat := v % 65536

-- | The coarse field is recovered losslessly from the T-field value.
theorem cuc_coarse_recovered {c f : Nat} (hf : f < 65536) : cucCoarse (cucT c f) = c := by
  unfold cucCoarse cucT
  rw [Nat.add_comm]
  have hpos : 0 < 65536 := by omega
  have hdiv : (f + c * 65536) / 65536 = f / 65536 + c :=
    Nat.add_mul_div_right f c hpos
  rw [hdiv]
  have hfz : f / 65536 = 0 := Nat.div_eq_of_lt hf
  rw [hfz]
  omega

-- | The fine field is recovered losslessly from the T-field value.
theorem cuc_fine_recovered {c f : Nat} (hf : f < 65536) : cucFine (cucT c f) = f := by
  unfold cucFine cucT
  rw [Nat.add_comm]
  have hmul0 : (c * 65536) % 65536 = 0 :=
    Nat.mod_eq_zero_of_dvd ⟨c, Nat.mul_comm c 65536⟩
  have hsum := Nat.add_mod f (c * 65536) 65536
  rw [hsum, hmul0]
  simp [Nat.mod_eq_of_lt hf]

-- | Combined lossless round trip of the CUC fields (SRS-REQ-024).
theorem cuc_field_roundtrip {c f : Nat} (hf : f < 65536) :
    cucCoarse (cucT c f) = c ∧ cucFine (cucT c f) = f := by
  constructor
  · exact cuc_coarse_recovered hf
  · exact cuc_fine_recovered hf

-- | The fine field derived from a sub-second fraction always fits 16 bits.
def fineFraction (frac : Nat) : Nat := frac * 65536 / 1000000000

theorem fine_frac_bounded {frac : Nat} (h : frac < 1000000000) : fineFraction frac < 65536 := by
  unfold fineFraction
  have hlt : frac * 65536 < 65536 * 1000000000 := by
    have h1 : frac * 65536 < (frac + 1) * 65536 :=
      Nat.mul_lt_mul_of_pos_right (Nat.lt_succ_self frac) (by omega : 0 < 65536)
    have h2 : (frac + 1) * 65536 ≤ 1000000000 * 65536 :=
      Nat.mul_le_mul_right 65536 (Nat.succ_le_of_lt h)
    have hlt' : frac * 65536 < 1000000000 * 65536 := Nat.lt_of_lt_of_le h1 h2
    rw [Nat.mul_comm 1000000000 65536] at hlt'
    exact hlt'
  exact (Nat.div_lt_iff_lt_mul (by omega : 0 < 1000000000)).mpr hlt

-- | Big-endian 4-octet decomposition of the coarse field: the four octets of
--   FF_TimeCucEncode always recombine to the original value.
theorem coarse_byte_decompose {c : Nat} :
    (c / 2^24) * 2^24 + ((c / 2^16) % 2^8) * 2^16 + ((c / 2^8) % 2^8) * 2^8 + c % 2^8 = c := by
  omega

end Frick.Time
