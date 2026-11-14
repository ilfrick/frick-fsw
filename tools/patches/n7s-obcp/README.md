# frick-fsw - N7 Space OBCP engine VM-budget patch set (project-owned)
#
# These patches apply to the pinned third-party dependency
# n7space/n7s-obcp @ 55d6d97e7e7ff08a130698f7a922bcf58acb3d99 with the
# MicroPython submodule @ 647c8b96cae7e202c7a020395b7cfe65e5b8ce04.
#
# They are NOT stored inside the gitignored fetched clone
# (third_party/n7s-obcp/). They are applied deterministically at build time
# by tools/scripts/apply_n7s_obcp_patch.sh so that a pristine checkout of the
# pinned revisions can reproduce the VM-budget implementation.
#
# SPDX-License-Identifier: MPL-2.0 (project-owned patches; underlying
# n7space/n7s-obcp and MicroPython remain MIT)
