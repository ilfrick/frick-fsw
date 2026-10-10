/*
 * frick-fsw - Common types and status codes
 *
 * Part of the frick-fsw flight software architecture.
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_TYPES_H
#define FF_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * Status/return type used across the whole flight software.
 * FF_SUCCESS is 0, all errors are negative.
 */
typedef int32_t FF_Status_t;

enum {
    FF_SUCCESS = 0,
    FF_ERROR = -1,
    FF_INVALID_ARGUMENT = -2,
    FF_NO_RESOURCE = -3,
    FF_TIMEOUT = -4,
    FF_NOT_IMPLEMENTED = -5,
    FF_NAME_TOO_LONG = -6,
    FF_NOT_FOUND = -7,
    FF_BUSY = -8,
    FF_BAD_SIZE = -9,
    FF_BUFFER_FULL = -10,
    FF_CRC_ERROR = -11,
    FF_PARSE_ERROR = -12,
    FF_ALREADY_EXISTS = -13,
    FF_NOT_READY = -14
};

/* Compile-time assert usable in C99 */
#define FF_STATIC_ASSERT(cond, msg) typedef char ff_static_assert_##msg[(cond) ? 1 : -1]

/* Opaque handle aliases. Handles are always small integer indices. */
typedef uint16_t FF_AppId_t;
typedef uint16_t FF_PipeId_t;
typedef uint16_t FF_TaskId_t;
typedef uint16_t FF_MutexId_t;
typedef uint16_t FF_QueueId_t;

#define FF_INVALID_ID 0xFFFFu

#endif /* FF_TYPES_H */
