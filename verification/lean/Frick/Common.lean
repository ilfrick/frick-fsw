-- frick-fsw - Common types and sizes, mirrored from src/core/common/ff_types.h
-- and config/fsw_config.h. SPDX-License-Identifier: AGPL-3.0-only
namespace Frick.Common

def StatusOk : Int := 0

structure AppId where
  -- 16 bit application ids, 1-based
  val : Nat

abbrev PipeId := Nat
abbrev TaskId := Nat

-- Software Bus message payload bound (FF_SB_MSG_PAYLOAD_MAX)
def SbMsgPayloadMax : Nat := 512

-- Software Bus queue depth limit
def SbQueueDepthMax : Nat := 32

-- TM/TC frame bounds (FF_PUS_MAX_TM_FRAME / FF_PUS_MAX_TC_FRAME)
def PusMaxFrame : Nat := 1024

-- PUS TM/TC session header lengths (PUS-C tailoring)
def PusTcSecHdrLen : Nat := 5
def PusTmSecHdrLen : Nat := 13
def CcsdsPrimaryHdrLen : Nat := 6
def FrameCrcLen : Nat := 2

end Frick.Common