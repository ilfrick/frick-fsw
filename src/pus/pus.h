/*
 * frick-fsw - PUS (Packet Utilisation Standard) layer
 *
 * The PUS layer sits between the transport (CCSDS Space Packets) and the
 * applications. It is intentionally backend agnostic: everything that
 * distinguishes one PUS product from another (secondary header layout, CRC,
 * transfer framing, ASM/CLTU) lives behind the FF_Pus_Backend_t vtable.
 *
 * Three backends are defined:
 *   - FF_PusBackendN7    : PUS-C ASN.1 (N7 Space / ESA asn1-pusc-lib,
 *                          ASN1SCC-compiled) DEFAULT backend when FF_USE_PUS_ASN1
 *                          is enabled (SRS-REQ-050, ADR-0016).
 *   - FF_PusBackendOpen  : open reference backend on the local CCSDS codec
 *                          (always available; the fallback).
 *   - FF_PusBackendPusopen : adapter for the commercial PUSopen(R) C library.
 *
 * Services implemented by the reference core: 1, 3, 5, 6, 8, 9, 11 (stub),
 * 17, 20. See docs/architecture/pus-layer.md for the tailoring matrix.
 *
 * Copyright (C) 2026 Ilfrick
 * https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
 * SPDX-License-Identifier: AGPL-3.0-only
 */
#ifndef FF_PUS_H
#define FF_PUS_H

#include "common/ff_types.h"
#include "fsw_config.h"
#include "core/evs/evs.h"
#include "pus/pus_types.h"

typedef struct
{
    const char *name;
    FF_Status_t (*init)(void);
    FF_Status_t (*encode_tm)(const FF_Pus_TmHdr_t *hdr, const uint8_t *data, uint16_t data_len,
                             uint8_t *out, size_t out_cap, uint16_t *out_len);
    FF_Status_t (*decode_tc)(const uint8_t *raw, uint16_t len, uint16_t *apid, FF_Pus_TcHdr_t *hdr,
                             const uint8_t **data, uint16_t *data_len);
} FF_Pus_Backend_t;

typedef void (*FF_Pus_TmSink_t)(const uint8_t *packet, uint16_t len);
typedef FF_Status_t (*FF_Pus_Handler_t)(const FF_Pus_TcHdr_t *tc, const uint8_t *data,
                                        uint16_t len);
typedef FF_Status_t (*FF_Pus_DataProvider_t)(uint8_t id, uint8_t *out, uint16_t *len);
typedef FF_Status_t (*FF_Pus_ParamSetter_t)(uint8_t id, const uint8_t *data, uint16_t len);
typedef FF_Status_t (*FF_Pus_FunctionHandler_t)(const uint8_t *args, uint16_t len);

/* ---- Lifecycle --------------------------------------------------- */
FF_Status_t FF_PusInit(void);
FF_Status_t FF_PusSetBackend(const FF_Pus_Backend_t *backend);
const FF_Pus_Backend_t *FF_PusGetBackend(void);
void FF_PusSetTmSink(FF_Pus_TmSink_t sink);

/* ---- Telecommand ingestion and telemetry emission ---------------- */
FF_Status_t FF_PusIngest(const uint8_t *raw, uint16_t len);
FF_Status_t FF_PusSendTm(uint8_t service, uint8_t subservice, const uint8_t *data, uint16_t len);
FF_Status_t FF_PusSendVerification(const FF_Pus_TcHdr_t *tc, FF_Pus_Stage_t stage, uint8_t success,
                                   uint8_t code);
FF_Status_t FF_PusEmitEvent(uint16_t event_id, uint8_t evs_type, const uint8_t *data, uint16_t len);

/* ---- Registration ------------------------------------------------- */
FF_Status_t FF_PusRegisterService(uint8_t service, uint8_t subservice, FF_Pus_Handler_t handler);
FF_Status_t FF_PusRegisterCoreServices(void);
FF_Status_t FF_PusRegisterHkProvider(uint8_t id, FF_Pus_DataProvider_t provider);
FF_Status_t FF_PusRegisterParamProvider(uint8_t id, FF_Pus_DataProvider_t provider);
FF_Status_t FF_PusRegisterParamSetter(uint8_t id, FF_Pus_ParamSetter_t setter);
FF_Status_t FF_PusRegisterFunction(uint8_t id, FF_Pus_FunctionHandler_t fn);
FF_Status_t FF_PusRegisterMemoryRegion(const void *base, uint32_t size);

/* ---- Statistics --------------------------------------------------- */
uint16_t FF_PusNextTmCounter(void);
uint16_t FF_PusPeekTmCounter(void);
uint32_t FF_PusGetTcCount(void);

/* ---- Backends ----------------------------------------------------- */
extern const FF_Pus_Backend_t FF_PusBackendOpen;

#if defined(FF_USE_PUS_ASN1) && (FF_USE_PUS_ASN1 == 1)
/* PUS-C ASN.1 (N7 Space / ESA) default backend (SRS-REQ-050). */
extern const FF_Pus_Backend_t FF_PusBackendN7;
#endif

#if defined(FRICK_USE_PUSOPEN) && (FRICK_USE_PUSOPEN == 1)
extern const FF_Pus_Backend_t FF_PusBackendPusopen;
#endif

#endif /* FF_PUS_H */
