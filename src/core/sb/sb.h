/*
 * frick-fsw - Software Bus (SB)
 *
 * Direct analogue of the cFS Software Bus: a publish/subscribe message bus
 * that decouples applications. Publishers send a message identifier plus a
 * small payload; the bus copies it into the input queue of every subscriber.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_SB_H
#define FF_SB_H

#include "common/ff_types.h"
#include "fsw_config.h"

/* Message identifier: 0xMMSS (MM = subsystem, SS = command). */
typedef uint16_t FF_MsgId_t;

typedef struct
{
    FF_MsgId_t msg_id; /* routing key                       */
    uint16_t apid;     /* source application id             */
    uint64_t time_ns;  /* publish timestamp                 */
    uint16_t length;   /* number of valid payload bytes      */
    uint8_t payload[FF_SB_MSG_PAYLOAD_MAX];
} FF_Msg_t;

typedef struct
{
    uint32_t published;
    uint32_t delivered;
    uint32_t dropped;
} FF_SbStats_t;

FF_Status_t FF_SbInit(void);
FF_Status_t FF_SbCreatePipe(FF_AppId_t app, uint16_t depth);
FF_Status_t FF_SbSubscribe(FF_MsgId_t msg_id, FF_AppId_t subscriber);
FF_Status_t FF_SbUnsubscribe(FF_MsgId_t msg_id, FF_AppId_t subscriber);
FF_Status_t FF_SbPublish(const FF_Msg_t *msg);
FF_Status_t FF_SbReceive(FF_AppId_t app, FF_Msg_t *msg, uint32_t timeout_ms);
FF_Status_t FF_SbGetStats(FF_SbStats_t *stats);

#endif /* FF_SB_H */
