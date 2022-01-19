/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef __TIMING_CONSTS_H
#define  __TIMING_CONSTS_H
#include <hal/nrf_radio.h>

#define OCTET_DURATION_1MBIT_US (8)
#define OCTET_DURATION_2MBIT_US (4)

#define PREAMBLE_1MBIT_OCTETS   (1)
#define PREAMBLE_2MBIT_OCTETS   (2)

#define SYNC_WORD_SIZE_OCTETS   (4)

#define PACKET_START_TO_SYNC_WORD_END_TIME_1MBIT_US (OCTET_DURATION_1MBIT_US * (PREAMBLE_1MBIT_OCTETS + SYNC_WORD_SIZE_OCTETS))
#define PACKET_START_TO_SYNC_WORD_END_TIME_2MBIT_US (OCTET_DURATION_2MBIT_US * (PREAMBLE_2MBIT_OCTETS + SYNC_WORD_SIZE_OCTETS))

#define PACKET_START_TO_SYNC_WORD_END_TIME_US(radio_mode)                                        \
        ( radio_mode == NRF_RADIO_MODE_BLE_2MBIT ? PACKET_START_TO_SYNC_WORD_END_TIME_2MBIT_US : \
          PACKET_START_TO_SYNC_WORD_END_TIME_1MBIT_US )

#define HEADER_SIZE             (1)
#define LENGTH_SIZE             (1)
#define CRC_SIZE                (3)

#define SYNC_WORD_END_TO_CRC_END_OCTETS(payload_octets) ( HEADER_SIZE + LENGTH_SIZE + (payload_octets) + CRC_SIZE )

#define OCTET_DURATION_US(radio_mode) \
      ( (radio_mode == NRF_RADIO_MODE_BLE_2MBIT) ? OCTET_DURATION_2MBIT_US : OCTET_DURATION_1MBIT_US )

#define TEST_PDU_LENGTH_US(radio_mode)                  \
    ( PACKET_START_TO_SYNC_WORD_END_TIME_US(radio_mode) \
    + SYNC_WORD_END_TO_CRC_END_OCTETS(TEST_PAYLOAD_SIZE) * OCTET_DURATION_US(radio_mode) )

#endif // __TIMING_CONSTS_H