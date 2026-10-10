/*
 * frick-fsw - Table Services (TBL)
 *
 * Registry of on-board configuration tables with a load/validate/activate
 * lifecycle (image is staged, validated, then atomically activated).
 * Analogue of the cFS Table Services.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_TBL_H
#define FF_TBL_H

#include "common/ff_types.h"
#include "fsw_config.h"

typedef uint16_t FF_TblHandle_t;
#define FF_TBL_INVALID_HANDLE 0xFFFFu

/* Returns FF_SUCCESS if the staged image is valid. */
typedef FF_Status_t (*FF_TblValidateFn)(const void *image, size_t size);

typedef struct
{
    const char *name;
    void *image; /* active, application owned buffer */
    size_t size;
    FF_TblValidateFn validate; /* optional, may be NULL */
} FF_Tbl_Def_t;

FF_Status_t FF_TblInit(void);
FF_TblHandle_t FF_TblRegister(const FF_Tbl_Def_t *def);
FF_Status_t FF_TblLoad(FF_TblHandle_t h, const void *src, size_t len);
FF_Status_t FF_TblActivate(FF_TblHandle_t h);
FF_Status_t FF_TblRevert(FF_TblHandle_t h);
const void *FF_TblGet(FF_TblHandle_t h, size_t *size);
FF_Status_t FF_TblDump(FF_TblHandle_t h, void *out, size_t *len);

#endif /* FF_TBL_H */
