/*
 * frick-fsw - Native host main entry
 *
 * On the flight target (RTEMS) control arrives through POSIX_Init() in
 * pal_rtems.c; this translation unit supplies the plain main() for the Linux
 * host simulator only.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "fsw_config.h"
#include "main/fsw_main.h"

#if !FF_HAS_RTEMS
int main(void)
{
    return ff_fsw_main();
}
#endif