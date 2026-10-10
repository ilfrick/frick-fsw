-- frick-fsw - omega tactic stub
--
-- `omega` is part of the Lean core since 4.6.0. This module exists to make
-- the import graph explicit and to keep a single place to switch the tactic
-- solver if a future toolchain moves it again.
--
-- SPDX-License-Identifier: AGPL-3.0-only

/-- Tactic solver shim: omega ships with the Lean core. -/
macro "lean_omega_constraint" : tactic => `(tactic| omega)