/*
 * frick-fsw - Minimal assertion-based test helper
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_TEST_H
#define FF_TEST_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

static int ff_test_checks = 0;
static int ff_test_failures = 0;

#define FF_TEST_ASSERT(cond)                                                                       \
    do {                                                                                           \
        ff_test_checks++;                                                                          \
        if (!(cond)) {                                                                             \
            ff_test_failures++;                                                                    \
            (void)fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                  \
        }                                                                                          \
    } while (0)

#define FF_TEST_PRINT_SUMMARY(name)                                                                \
    do {                                                                                           \
        (void)printf("%-24s checks=%d failures=%d\n", name, ff_test_checks, ff_test_failures);     \
    } while (0)

/* FF_TEST_MAIN(t1, t2, ...) generates a main() that executes every listed
 * void (*)(void) test routine and reports the cumulative result. */
#define FF_TEST_MAIN(...)                                                                          \
    int main(void)                                                                                 \
    {                                                                                              \
        void (*ff_tests[])(void) = {__VA_ARGS__};                                                  \
        for (size_t i = 0; i < (sizeof(ff_tests) / sizeof(ff_tests[0])); i++) {                    \
            ff_tests[i]();                                                                         \
        }                                                                                          \
        FF_TEST_PRINT_SUMMARY(__FILE__);                                                           \
        return (ff_test_failures == 0) ? 0 : 1;                                                    \
    }

#endif /* FF_TEST_H */
