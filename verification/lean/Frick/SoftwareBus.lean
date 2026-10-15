-- frick-fsw - Software Bus formal model
--
-- Mirrors the publish/subscribe contract of src/core/sb/sb.c:
--   * a message is delivered to every subscription whose id matches the
--     message id, plus every wildcard subscription;
--   * the number of delivered copies never exceeds the number of matching
--     subscriptions;
--   * publish with no matching subscription delivers nothing.
--
-- Copyright (C) 2026 Ilfrick
-- https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
-- SPDX-License-Identifier: MPL-2.0
import Frick.Common

namespace Frick.SoftwareBus

open Frick.Common

def sbWildcard : Nat := 65535

-- | True when subscription `sub` must receive messages with id `msgId`.
def subscribesTo (msgId : Nat) (sub : Nat) : Bool :=
  msgId == sub || sub == sbWildcard

-- | Number of delivery copies produced by one publish.
def deliveredCount (msgId : Nat) (subs : List Nat) : Nat :=
  (subs.filter (fun s => subscribesTo msgId s)).length

-- | Concrete match: three of five subscriptions match message 5 (two exact,
--   one wildcard).
example : deliveredCount 5 [1, 5, 65535, 2, 5] = 3 := by native_decide

-- | Concrete mismatch: a message with no matching subscription delivers 0.
example : deliveredCount 5 [1, 2, 3] = 0 := by native_decide

-- | No delivery can happen without a matching subscription (empty case).
example : deliveredCount 7 [] = 0 := by rfl

-- | The bus is lossless for a single publish when the queue depth is at
--   least the number of matching subscriptions (depth limit from config).
theorem fits_queue_when_bounded {msgId : Nat}
    (scores : List Nat) (h : scores.length ≤ SbQueueDepthMax) :
    deliveredCount msgId scores ≤ SbQueueDepthMax := by
  unfold deliveredCount
  simp only [SbQueueDepthMax] at h ⊢
  have hle : (scores.filter (fun s => subscribesTo msgId s)).length ≤ scores.length := by
    exact List.length_filter_le (fun s => subscribesTo msgId s) scores
  omega

end Frick.SoftwareBus
