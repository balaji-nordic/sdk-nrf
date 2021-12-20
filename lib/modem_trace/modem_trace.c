/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>
#include <kernel.h>

#include "modem_trace.h"
#include "nrf_modem_at.h"

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
#include "nrfx_uarte.h"
#endif
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
#include "SEGGER_RTT.h"
#endif

static uint32_t max_trace_size_bytes;

static void trace_stop_timer_handler(struct k_timer *timer);

K_TIMER_DEFINE(trace_stop_timer, trace_stop_timer_handler, NULL);


static void trace_stop_timer_handler(struct k_timer *timer)
{
	nrf_modem_at_printf("AT%%XMODEMTRACE=0");
}

//TODO: Place these in ifdefs for UART and RTT
static const nrfx_uarte_t uarte_inst = NRFX_UARTE_INSTANCE(1);

static void uart_init(void)
{
	const nrfx_uarte_config_t config = {
		.pseltxd = DT_PROP(DT_NODELABEL(uart1), tx_pin),
		.pselrxd = DT_PROP(DT_NODELABEL(uart1), rx_pin),
		.pselcts = NRF_UARTE_PSEL_DISCONNECTED,
		.pselrts = NRF_UARTE_PSEL_DISCONNECTED,

		.hal_cfg.hwfc = NRF_UARTE_HWFC_DISABLED,
		.hal_cfg.parity = NRF_UARTE_PARITY_EXCLUDED,
		.baudrate = NRF_UARTE_BAUDRATE_1000000,

		.interrupt_priority = NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY,
		.p_context = NULL,
	};

	nrfx_uarte_init(&uarte_inst, &config, NULL);
}


//static int trace_rtt_channel;
static char rtt_buffer[CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE];

int modem_trace_init(void)
{
	if (IS_ENABLED(CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART)) {
		uart_init();
	}

	if (IS_ENABLED(CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT)) {
		SEGGER_RTT_AllocUpBuffer("modem_trace", rtt_buffer, sizeof(rtt_buffer),
					 SEGGER_RTT_MODE_NO_BLOCK_SKIP);
	}

	return 0;
}

int modem_trace_start(enum modem_trace_mode trace_mode, uint16_t duration, uint32_t max_size)
{
	char at_cmd[sizeof("AT%%XMODEMTRACE=1,X")];

	sprintf(at_cmd, "AT%%%%XMODEMTRACE=1,%hu", trace_mode);

	nrf_modem_at_printf(at_cmd);

	if (duration != 0)
	{
		k_timer_start(&trace_stop_timer, K_SECONDS(duration), K_SECONDS(0));
	}
	max_trace_size_bytes = max_size;

	return 0;
}

int modem_trace_process(const uint8_t *data, uint32_t len)
{
	static uint32_t total_trace_size_rcvd = 0;

	total_trace_size_rcvd += len;

	if (total_trace_size_rcvd > max_trace_size_bytes)
	{
		/* Skip sending  to transport medium as the current trace wont fit.
		 * Disable traces (see API doc for reasoning).
		 */
		nrf_modem_at_printf("AT%%XMODEMTRACE=0");
	}
	else if (total_trace_size_rcvd == max_trace_size_bytes)
	{
		nrfx_uarte_tx(&uarte_inst, data, len);
		nrf_modem_at_printf("AT%%XMODEMTRACE=0");
	}
	else
	{
		nrfx_uarte_tx(&uarte_inst, data, len);
	}

	return 0;
}
