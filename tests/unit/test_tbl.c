/*
 * frick-fsw - Table Services unit tests
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#include "../ff_test.h"
#include "core/pal/pal.h"
#include "core/tbl/tbl.h"
#include <string.h>

typedef struct
{
    uint32_t magic;
    uint16_t gain;
    uint16_t bias;
} test_cfg_t;

#define TEST_CFG_MAGIC 0x54434F4Eu /* 'TCON' */

static uint8_t g_image[sizeof(test_cfg_t)];

static FF_Status_t validate(const void *img, size_t size)
{
    const test_cfg_t *t = (const test_cfg_t *)img;
    if ((size != sizeof(test_cfg_t)) || (t->magic != TEST_CFG_MAGIC)) {
        return FF_ERROR;
    }
    return FF_SUCCESS;
}

static void test_load_activate(void)
{
    FF_TEST_ASSERT(FF_PalInit() == FF_SUCCESS);
    FF_TEST_ASSERT(FF_TblInit() == FF_SUCCESS);

    memset(&g_image, 0, sizeof(g_image));
    FF_TblHandle_t h = FF_TblRegister(&(FF_Tbl_Def_t){
        .name = "test_cfg", .image = g_image, .size = sizeof(g_image), .validate = validate});
    FF_TEST_ASSERT(h != FF_TBL_INVALID_HANDLE);

    test_cfg_t staged;
    staged.magic = TEST_CFG_MAGIC;
    staged.gain = 42u;
    staged.bias = 7u;

    FF_TEST_ASSERT(FF_TblLoad(h, &staged, sizeof(staged)) == FF_SUCCESS);

    /* Not yet active: image must still be untouched. */
    const test_cfg_t *cur = (const test_cfg_t *)FF_TblGet(h, NULL);
    FF_TEST_ASSERT(cur->magic != TEST_CFG_MAGIC);

    FF_TEST_ASSERT(FF_TblActivate(h) == FF_SUCCESS);

    cur = (const test_cfg_t *)FF_TblGet(h, NULL);
    FF_TEST_ASSERT(cur->magic == TEST_CFG_MAGIC);
    FF_TEST_ASSERT(cur->gain == 42u);
    FF_TEST_ASSERT(cur->bias == 7u);
}

FF_TEST_MAIN(test_load_activate)
