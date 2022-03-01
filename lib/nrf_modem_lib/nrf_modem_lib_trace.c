/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>
#include <kernel.h>
#include <zephyr.h>
#include <modem/nrf_modem_lib_trace.h>
#include <nrf_errno.h>
#include <nrf_modem.h>
#include <nrf_modem_at.h>
#include <logging/log.h>
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
#include <nrfx_uarte.h>
#endif
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
#include <SEGGER_RTT.h>
#endif
#ifdef CONFIG_MEMFAULT
#include "memfault/panics/assert.h"
#endif

LOG_MODULE_REGISTER(nrf_modem_lib_trace, CONFIG_NRF_MODEM_LIB_LOG_LEVEL);

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
/* Maximum time to wait for a UART transfer to complete before giving up.*/
#define UART_TX_WAIT_TIME_MS 100
#define UNUSED_FLAGS 0

static const nrfx_uarte_t uarte_inst = NRFX_UARTE_INSTANCE(1);

/* Semaphore used to check if the last UART transfer was complete. */
static K_SEM_DEFINE(tx_sem, 1, 1);
#endif

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
static int trace_rtt_channel;
static char rtt_buffer[CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE];
#endif

static bool is_transport_initialized;

struct trace_data_t {
	void *fifo_reserved; /* 1st word reserved for use by fifo */
	const uint8_t * const data;
	uint32_t len;
};

K_FIFO_DEFINE(trace_fifo);

#define TRACE_THREAD_STACK_SIZE 2048
#define TRACE_THREAD_PRIORITY CONFIG_NRF_MODEM_LIB_TRACE_THREAD_PRIO

#ifdef CONFIG_MEMFAULT

static struct {
	uint8_t modem_trace_data[2048 /* size of modem trace buffer to capture */];
	int offset;
} s_modem_trace_data;

void memfault_capture_modem_trace(void)
{
	// Copy trace data into `s_modem_trace_data`
	//  or if that's already the buffer being used, there is
	// nothing to do here!

	// trigger the capture of a coredump which includes the modem trace buffer
	MEMFAULT_ASSERT(0);
}

void store_to_memfault(const uint8_t *data, const uint32_t len)
{
	static bool copy_trace;

	if (!copy_trace) {
		if (data[0] == 0xef && data[1] == 0xbe) {
			/* Ensure that we also start storing from a header. */
			copy_trace = true;
		}
	}
	if (copy_trace) {
		NRF_P0_NS->DIRSET = (1 << 17);
		if (s_modem_trace_data.offset + len <=
			sizeof(s_modem_trace_data.modem_trace_data)) {
			NRF_P0_NS->OUTSET = (1 << 17);
			memcpy(
				&s_modem_trace_data.modem_trace_data[s_modem_trace_data.offset],
				data,
				len);
			s_modem_trace_data.offset += len;
			NRF_P0_NS->OUTCLR = (1 << 17);
		} else {
			LOG_INF("Calling memfault_capture_modem_trace");
			memfault_capture_modem_trace();
		}
	}
}
#endif

void trace_handler_thread(void)
{
#ifdef CONFIG_MEMFAULT
	s_modem_trace_data.offset = 0;
	int skip_cnt = 0;
#endif
	while (1) {
		struct trace_data_t *trace_data = k_fifo_get(&trace_fifo, K_FOREVER);
		const uint8_t * const data = trace_data->data;
		const uint32_t len = trace_data->len;

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
		/* Split RAM buffer into smaller chunks to be transferred using DMA. */
		const uint32_t MAX_BUF_LEN = (1 << UARTE1_EASYDMA_MAXCNT_SIZE) - 1;
		uint32_t remaining_bytes = len;
		nrfx_err_t err;

#ifdef CONFIG_MEMFAULT
		/* Delay until lte gets connected and memfault comm is established. */
		if (skip_cnt > 10000) {
			store_to_memfault(data, len);
		} else {
			skip_cnt++;
		}
#endif
		while (remaining_bytes) {
			size_t transfer_len = MIN(remaining_bytes, MAX_BUF_LEN);
			uint32_t idx = len - remaining_bytes;

			if (k_sem_take(&tx_sem, K_MSEC(UART_TX_WAIT_TIME_MS)) != 0) {
				LOG_WRN("UARTE TX not available!");
				break;
			}
			err = nrfx_uarte_tx(&uarte_inst, &data[idx], transfer_len);
			if (err != NRFX_SUCCESS) {
				LOG_ERR("nrfx_uarte_tx error: %d", err);
				k_sem_give(&tx_sem);
				break;
			}
			remaining_bytes -= transfer_len;
		}
		/* Wait for last UART transfer to finish */
		k_sem_take(&tx_sem, K_FOREVER);
		k_sem_give(&tx_sem);
#endif

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
		uint32_t remaining_bytes = len;

		while (remaining_bytes) {
			uint16_t transfer_len = MIN(remaining_bytes,
						CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE);
			uint32_t idx = len - remaining_bytes;

			SEGGER_RTT_WriteSkipNoLock(trace_rtt_channel, &data[idx], transfer_len);
			remaining_bytes -= transfer_len;
		}
#endif
		int ret = nrf_modem_trace_processed_callback(data, len);

		__ASSERT(ret == 0, "nrf_modem_trace_processed_callback returns error %d for "
						"data = %p, len = %d", ret, data, len);

		k_free(trace_data);
	}
}

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
static void uarte_callback(nrfx_uarte_event_t const *p_event, void *p_context)
{
	if (k_sem_count_get(&tx_sem) != 0) {
		LOG_ERR("uart semaphore not in use");
		return;
	}

	if (p_event->type == NRFX_UARTE_EVT_ERROR) {
		LOG_ERR("uarte error 0x%04x", p_event->data.error.error_mask);

		k_sem_give(&tx_sem);
	}

	if (p_event->type == NRFX_UARTE_EVT_TX_DONE) {
		k_sem_give(&tx_sem);
	}
}

static bool uart_init(void)
{
	const uint8_t irq_priority = DT_IRQ(DT_NODELABEL(uart1), priority);
	const nrfx_uarte_config_t config = {
		.pseltxd = DT_PROP(DT_NODELABEL(uart1), tx_pin),
		.pselrxd = DT_PROP(DT_NODELABEL(uart1), rx_pin),
		.pselcts = NRF_UARTE_PSEL_DISCONNECTED,
		.pselrts = NRF_UARTE_PSEL_DISCONNECTED,

		.hal_cfg.hwfc = NRF_UARTE_HWFC_DISABLED,
		.hal_cfg.parity = NRF_UARTE_PARITY_EXCLUDED,
		.baudrate = NRF_UARTE_BAUDRATE_1000000,

		.interrupt_priority = irq_priority,
		.p_context = NULL,
	};

	IRQ_CONNECT(DT_IRQN(DT_NODELABEL(uart1)),
		irq_priority,
		nrfx_isr,
		&nrfx_uarte_1_irq_handler,
		UNUSED_FLAGS);
	return (nrfx_uarte_init(&uarte_inst, &config, &uarte_callback) ==
		NRFX_SUCCESS);
}
#endif

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
static bool rtt_init(void)
{
	trace_rtt_channel = SEGGER_RTT_AllocUpBuffer("modem_trace", rtt_buffer, sizeof(rtt_buffer),
						     SEGGER_RTT_MODE_NO_BLOCK_SKIP);

	return (trace_rtt_channel > 0);
}
#endif

int nrf_modem_lib_trace_init(void)
{
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
	is_transport_initialized = uart_init();
#endif
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
	is_transport_initialized = rtt_init();
#endif

	if (!is_transport_initialized) {
		return -EBUSY;
	}
	return 0;
}

int nrf_modem_lib_trace_start(enum nrf_modem_lib_trace_mode trace_mode)
{
	if (!is_transport_initialized) {
		return -ENXIO;
	}

	if (nrf_modem_at_printf("AT%%XMODEMTRACE=1,%hu", trace_mode) != 0) {
		return -EOPNOTSUPP;
	}

	return 0;
}

int nrf_modem_lib_trace_process(const uint8_t *data, uint32_t len)
{
	if (!is_transport_initialized) {
		int err = nrf_modem_trace_processed_callback(data, len);

		__ASSERT(err == 0,
			"nrf_modem_trace_processed_callback failed with error code %d", err);
		return -ENXIO;
	}

	struct trace_data_t trace_data = { .data = data, .len = len };
	size_t size = sizeof(struct trace_data_t);
	char *mem_ptr = k_malloc(size);

	__ASSERT(mem_ptr != 0, "Out of memory");

	memcpy(mem_ptr, &trace_data, size);

	k_fifo_put(&trace_fifo, mem_ptr);

	return 0;
}

int nrf_modem_lib_trace_stop(void)
{
	__ASSERT(!k_is_in_isr(),
		"nrf_modem_lib_trace_stop cannot be called from interrupt context");

	if (nrf_modem_at_printf("AT%%XMODEMTRACE=0") != 0) {
		return -EOPNOTSUPP;
	}

	return 0;
}

K_THREAD_DEFINE(trace_thread_id, TRACE_THREAD_STACK_SIZE, trace_handler_thread,
	NULL, NULL, NULL, TRACE_THREAD_PRIORITY, 0, 0);
