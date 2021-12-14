/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
#include "nrfx_uarte.h"
#endif
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
#include "SEGGER_RTT.h"
#endif

static const nrfx_uarte_t uarte_inst = NRFX_UARTE_INSTANCE(1);

static void trace_uart_init(void)
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

#define RTT_BUF_SZ (CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE)
//static int trace_rtt_channel;
static char rtt_buffer[RTT_BUF_SZ];

int modem_trace_init(void)
{
	if (IS_ENABLED(CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART)) {
		trace_uart_init();
	}

	if (IS_ENABLED(CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT)) {
		SEGGER_RTT_AllocUpBuffer("modem_trace", rtt_buffer, RTT_BUF_SZ,
					 SEGGER_RTT_MODE_NO_BLOCK_SKIP);
	}

	return 0;
}
