/*
 * frick-fsw - Table Services implementation
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "tbl.h"
#include "core/pal/pal.h"
#include <string.h>

typedef struct
{
    bool              used;
    char              name[FF_TBL_NAME_LEN];
    void             *image;
    size_t            size;
    FF_TblValidateFn  validate;
    bool              staged;
} ff_tbl_entry_t;

static ff_tbl_entry_t g_tables[FF_TBL_MAX_TABLES];
static uint8_t        g_staging[FF_TBL_MAX_TABLES][FF_TBL_MAX_SIZE];
static FF_MutexId_t   g_lock;

FF_Status_t FF_TblInit(void)
{
    memset(g_tables, 0, sizeof(g_tables));
    memset(g_staging, 0, sizeof(g_staging));
    return FF_PalMutexCreate(&g_lock, "tbl");
}

FF_TblHandle_t FF_TblRegister(const FF_Tbl_Def_t *def)
{
    if ((def == NULL) || (def->name == NULL) || (def->image == NULL) || (def->size == 0u))
    {
        return FF_TBL_INVALID_HANDLE;
    }
    if (def->size > FF_TBL_MAX_SIZE)
    {
        return FF_TBL_INVALID_HANDLE;
    }
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    for (uint16_t i = 0; i < FF_TBL_MAX_TABLES; i++)
    {
        if (!g_tables[i].used)
        {
            g_tables[i].used     = true;
            g_tables[i].image    = def->image;
            g_tables[i].size     = def->size;
            g_tables[i].validate = def->validate;
            g_tables[i].staged   = false;
            (void)strncpy(g_tables[i].name, def->name, FF_TBL_NAME_LEN - 1);
            FF_PalMutexUnlock(g_lock);
            return i;
        }
    }
    FF_PalMutexUnlock(g_lock);
    return FF_TBL_INVALID_HANDLE;
}

FF_Status_t FF_TblLoad(FF_TblHandle_t h, const void *src, size_t len)
{
    if ((h >= FF_TBL_MAX_TABLES) || !g_tables[h].used || (src == NULL))
    {
        return FF_NOT_FOUND;
    }
    if (len != g_tables[h].size)
    {
        return FF_BAD_SIZE;
    }
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    memcpy(g_staging[h], src, len);
    FF_PalMutexUnlock(g_lock);

    if (g_tables[h].validate != NULL)
    {
        if (g_tables[h].validate(g_staging[h], len) != FF_SUCCESS)
        {
            return FF_ERROR;
        }
    }
    g_tables[h].staged = true;
    return FF_SUCCESS;
}

FF_Status_t FF_TblActivate(FF_TblHandle_t h)
{
    if ((h >= FF_TBL_MAX_TABLES) || !g_tables[h].used)
    {
        return FF_NOT_FOUND;
    }
    if (!g_tables[h].staged)
    {
        return FF_NOT_READY;
    }
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    memcpy(g_tables[h].image, g_staging[h], g_tables[h].size);
    FF_PalMutexUnlock(g_lock);
    return FF_SUCCESS;
}

FF_Status_t FF_TblRevert(FF_TblHandle_t h)
{
    if ((h >= FF_TBL_MAX_TABLES) || !g_tables[h].used)
    {
        return FF_NOT_FOUND;
    }
    FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER);
    memcpy(g_staging[h], g_tables[h].image, g_tables[h].size);
    g_tables[h].staged = false;
    FF_PalMutexUnlock(g_lock);
    return FF_SUCCESS;
}

const void *FF_TblGet(FF_TblHandle_t h, size_t *size)
{
    if ((h >= FF_TBL_MAX_TABLES) || !g_tables[h].used)
    {
        return NULL;
    }
    if (size != NULL)
    {
        *size = g_tables[h].size;
    }
    return g_tables[h].image;
}

FF_Status_t FF_TblDump(FF_TblHandle_t h, void *out, size_t *len)
{
    if ((h >= FF_TBL_MAX_TABLES) || !g_tables[h].used || (out == NULL) || (len == NULL))
    {
        return FF_INVALID_ARGUMENT;
    }
    if (*len < g_tables[h].size)
    {
        return FF_BAD_SIZE;
    }
    memcpy(out, g_tables[h].image, g_tables[h].size);
    *len = g_tables[h].size;
    return FF_SUCCESS;
}
