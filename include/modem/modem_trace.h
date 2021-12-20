/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**@file modem_trace.h
 *
 * @defgroup modem_trace nRF91 Modem traces
 * @{
 */
#ifndef MODEM_TRACE_H__
#define MODEM_TRACE_H__

#include <stdint.h>

/** @brief Initialize the modem trace module.
 *
 * Initializes the trace transport medium.
 *
 * @return Zero on success, non-zero otherwise.
 */
int modem_trace_init(void);

/**@brief Trace mode
 *
 * The trace mode can be used to filter the traces.
 */
enum modem_trace_mode {
	MODEM_TRACE_COREDUMP_ONLY = 1, /**< Coredump only. */
	MODEM_TRACE_ALL = 2, /**< LTE, IP, GNSS, and coredump. */
	MODEM_TRACE_IP_ONLY = 4, /**< IP. */
	MODEM_TRACE_LTE_IP = 5, /**< LTE and IP. */
};

/** @brief Start a trace session.
 *
 * This function sends AT command that requests the modem to start sending traces.
 *
 * @param trace_mode Trace mode
 * @param duration Trace duration in seconds. If set to 0, the trace session will continue until
 *				   @ref modem_trace_abort is called or until the required size of max trace data
 *				   (specified by the @ref max_size parameter) is received.
 * @param max_size Maximum size (in bytes) of trace data that should be received. The tracing will
 *				   be stopped after receiving @ref max_size bytes. If set to 0, the trace session
 *				   will continue until @ref modem_trace_abort is called or until the duration set
 *				   via the @ref duration parameter is reached.
 *				   To ensure the integrity of the trace output, the modem_trace module will never
 *				   skip a trace message . For this purpose, if it detects that a received trace
 *				   wont fit in the maximum allowed size, it will stop the trace session without
 *				   sending out that trace to the transport medium.
 *
 * @return Zero on success, non-zero otherwise.
 */
int modem_trace_start(enum modem_trace_mode trace_mode, uint16_t duration, uint32_t max_size);

/** @brief Process modem trace data
 *
 * This function receives data from libmodem and forwards to the selected (during compile time)
 * trace transport medium.
 *
 * @param data Memory buffer containing the modem trace data.
 * @param len  Memory buffer length.
 *
 * @return Zero on success, non-zero otherwise.
 */
int modem_trace_process(const uint8_t *data, uint32_t len);

/** @brief Abort an ongoing trace session
 *
 * This function aborts an ongoing trace session.
 *
 * @return Zero on success, non-zero otherwise.
 */
int modem_trace_abort(void);

#endif /* MODEM_TRACE_H__ */
/**@} */
