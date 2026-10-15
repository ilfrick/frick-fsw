-- frick-fsw - S4c/S4d bounded contract models
--
-- These are abstract pure-logic models of selected project requirements. They
-- do not prove the C implementation, MMIO behavior, protocol conformance, or
-- target timing.
--
-- Copyright (C) 2026 Ilfrick
-- https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
-- SPDX-License-Identifier: MPL-2.0
import Frick.LeanOmega

namespace Frick.S4c

def canFdPayloadLength (dlc : Nat) : Nat :=
  match dlc with
  | 0 => 0
  | 1 => 1
  | 2 => 2
  | 3 => 3
  | 4 => 4
  | 5 => 5
  | 6 => 6
  | 7 => 7
  | 8 => 8
  | 9 => 12
  | 10 => 16
  | 11 => 20
  | 12 => 24
  | 13 => 32
  | 14 => 48
  | 15 => 64
  | _ => 0

theorem req_srs062_dlc_max_payload : canFdPayloadLength 15 = 64 := by rfl

def validFabricRange (size offset width : Nat) : Prop :=
  width > 0 ∧ offset ≤ size ∧ width ≤ size - offset

theorem req_srs067_fabric_range_rejects_overrun
    {size offset width : Nat} (h : validFabricRange size offset width) :
    offset + width ≤ size := by
  rcases h with ⟨_hw, ho, hwidth⟩
  omega

def nmtNext (command : Nat) : Option Nat :=
  if command = 1 then some 5
  else if command = 2 then some 4
  else if command = 128 then some 127
  else none

theorem req_srs063_operational_command : nmtNext 1 = some 5 := by rfl
theorem req_srs063_stopped_command : nmtNext 2 = some 4 := by rfl
theorem req_srs063_preop_command : nmtNext 128 = some 127 := by rfl
theorem req_srs063_unknown_command_rejected (c : Nat)
    (h : c ≠ 1 ∧ c ≠ 2 ∧ c ≠ 128) : nmtNext c = none := by
  simp [nmtNext, h.1, h.2.1, h.2.2]

def heartbeatDue (period now : Nat) : Bool :=
  decide (period > 0 ∧ now % period = 0)

theorem req_srs063_heartbeat_disabled : heartbeatDue 0 0 = false := by
  simp [heartbeatDue]

theorem req_srs063_heartbeat_due_at_period (p : Nat) (hp : p > 0) :
    heartbeatDue p p = true := by
  simp [heartbeatDue, hp]

def boundedDrainPollCount (polls limit : Nat) : Nat := min polls limit

theorem req_srs064_drain_poll_count_bounded (polls limit : Nat) :
    boundedDrainPollCount polls limit ≤ limit := by
  exact Nat.min_le_right polls limit

def defaultPusBackend : String := "open"
def n7BackendEnabled (enabled : Bool) : Bool := enabled

theorem req_srs050_open_backend_is_default : defaultPusBackend = "open" := by rfl
theorem req_srs050_n7_is_opt_in : n7BackendEnabled false = false := by rfl
end Frick.S4c
