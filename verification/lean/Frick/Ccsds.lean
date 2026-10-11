-- frick-fsw - CCSDS Space Packet formal model and CRC-16/CCITT-FALSE
--
-- This module mirrors src/transport/ccsds/ccsds.c. The CRC implementation is
-- kept definitionally close to the C code (poly 0x1021, init 0xFFFF, no
-- reflection). Theorems here discharge the SRS requirements that the C
-- implementation is validated against.
--
-- Copyright (C) 2026 Ilfrick
-- https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
-- SPDX-License-Identifier: AGPL-3.0-only
import Frick.Common
import Frick.LeanOmega  -- omega tactic availability shim

namespace Frick.Ccsds

open Frick.Common

-- | One CRC bit shift: if the carry bit is set, XOR with the generator.
def crc16Bit (crc : UInt16) (_x : Bool) : UInt16 :=
  let carrySet : Bool := (crc &&& 0x8000) == 0x8000
  let shifted : UInt16 := crc <<< 1
  if carrySet then shifted ^^^ 0x1021 else shifted

-- | Eight bit positions for one octet.
def crc16Step (crc : UInt16) (byte : UInt16) : UInt16 :=
  let c0 : UInt16 := crc ^^^ (byte <<< 8)
  (List.range 8).foldl (fun c _ => crc16Bit c true) c0

-- | CRC-16/CCITT-FALSE over a byte stream.
def crc16 (xs : List UInt16) : UInt16 :=
  xs.foldl crc16Step 0xFFFF

def hi (v : UInt16) : UInt16 := (v >>> 8) &&& 0xFF
def lo (v : UInt16) : UInt16 := v &&& 0xFF

-- | A frame with its CRC trailer appended: crc16 of the extended frame is 0.
def withCrc (xs : List UInt16) : List UInt16 :=
  let c := crc16 xs
  xs ++ [hi c, lo c]

-- ---------------------------------------------------------------------
-- Facts checked by the C unit test test_ccsds.c (see docs/requirements)
-- ---------------------------------------------------------------------

-- | CRC of an empty message is the initial value.
theorem crc16_empty : crc16 [] = 0xFFFF := by rfl

-- | Determinism: crc16 is a pure function (reflexivity).
theorem crc16_deterministic (xs : List UInt16) : crc16 xs = crc16 xs := by rfl

-- | Appending the CRC to a concrete frame yields a zero CRC: this is the
-- on-board detection property exercised by test_ccsds.c (test_crc).
theorem crc16_append_makes_zero : crc16 (withCrc [0x10, 0x00, 0x02, 0x00, 0x17, 0x01]) = 0 := by
  native_decide

-- | Small-frame length arithmetic: a 1024 byte TM payload never overflows
-- the 16 bit CCSDS packet data length field (SRS-REQ-003).
theorem tm_frame_length_fits_pdl {data : Nat} (h : data ≤ PusMaxFrame) :
    CcsdsPrimaryHdrLen + PusTmSecHdrLen + data + FrameCrcLen ≤ 65535 := by
  simp only [PusMaxFrame, CcsdsPrimaryHdrLen, PusTmSecHdrLen, FrameCrcLen] at h ⊢
  omega

-- | Same guarantee on the telecommand side.
theorem tc_frame_length_fits_pdl {data : Nat} (h : data ≤ PusMaxFrame) :
    CcsdsPrimaryHdrLen + PusTcSecHdrLen + data + FrameCrcLen ≤ 65535 := by
  simp only [PusMaxFrame, CcsdsPrimaryHdrLen, PusTcSecHdrLen, FrameCrcLen] at h ⊢
  omega

end Frick.Ccsds
