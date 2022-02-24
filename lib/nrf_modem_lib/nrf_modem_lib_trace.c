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
#include <storage/stream_flash.h>

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
#include <nrfx_uarte.h>
#endif
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
#include <SEGGER_RTT.h>
#endif

LOG_MODULE_REGISTER(nrf_modem_lib_trace, CONFIG_NRF_MODEM_LIB_LOG_LEVEL);

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
/* Maximum time to wait for a UART transfer to complete before giving up.*/
#define UART_TX_WAIT_TIME_MS 100
#define UNUSED_FLAGS 0

static const nrfx_uarte_t uarte_inst = NRFX_UARTE_INSTANCE(1);

/* Semaphore used to check if the last UART transfer was complete. */
static K_SEM_DEFINE(tx_sem, 1, 1);

#define WRITE_BUF_LEN 512
#define READ_BUF_LEN 512
#define EXT_FLASH_DEVICE DT_LABEL(DT_INST(0, jedec_spi_nor))

static const struct device *flash_dev;
static struct stream_flash_ctx stream;
static bool is_flash_init_done;
static uint8_t write_buf[WRITE_BUF_LEN];
static uint8_t read_buf[READ_BUF_LEN];
const uint32_t max_trace_size_bytes = 1 * 1024 * 1024;
static void flash_init(void);
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

#define TRACE_THREAD_STACK_SIZE (2 * 1024)
#define TRACE_THREAD_PRIORITY CONFIG_NRF_MODEM_LIB_TRACE_THREAD_PRIO

void flash_to_uart(uint32_t offset, uint32_t len)
{
	__ASSERT(len <= READ_BUF_LEN, "len not less than READ_BUF_LEN");
	//NRF_P0_NS->OUTSET = (1 << 19);
	int err = flash_read(flash_dev, offset, read_buf, len);
	//NRF_P0_NS->OUTCLR = (1 << 19);

	if (err) {
		LOG_ERR("flash_read returns error = %d", err);
	}

	//LOG_INF("Sending over UART");
	err = nrfx_uarte_tx(&uarte_inst, read_buf, len);

	if (err != NRFX_SUCCESS) {
		LOG_ERR("nrfx_uarte_tx returns error = %d", err);
	}
}

static void dump_trace_from_ext_flash_to_uart(void)
{
	uint32_t stored_trace_size = (1 * 1024 * 1024) + (500 * 1024);

	LOG_INF("Going to dump %d bytes of traces over UART1", stored_trace_size);

	for (uint32_t offset = 0; offset < stored_trace_size; offset += READ_BUF_LEN) {
		flash_to_uart(offset, READ_BUF_LEN);
	}
}



void trace_handler_thread(void)
{
	NRF_P0_NS->DIRSET = (1 << 17) | (1 << 18) | (1 << 19);
	nrf_modem_lib_trace_stop();

	flash_init();

	dump_trace_from_ext_flash_to_uart();

	LOG_INF("Erasing flash");
	int err = flash_erase(flash_dev, 0, max_trace_size_bytes);

	if (err != 0) {
		LOG_ERR("flash_erase returns error = %d", err);
	}

	nrf_modem_lib_trace_start(NRF_MODEM_LIB_TRACE_ALL);

	while (1) {
		struct trace_data_t *trace_data = k_fifo_get(&trace_fifo, K_FOREVER);
		const uint8_t * const data = trace_data->data;
		const uint32_t len = trace_data->len;

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
		/* Split RAM buffer into smaller chunks to be transferred using DMA. */
		if (is_flash_init_done) {
			uint32_t remaining_bytes = len;

			while (remaining_bytes) {
				size_t transfer_len = MIN(remaining_bytes, WRITE_BUF_LEN);
				uint32_t idx = len - remaining_bytes;

				remaining_bytes -= transfer_len;
				NRF_P0_NS->OUTSET = (1 << 17);
				int ret = stream_flash_buffered_write(&stream, &data[idx],
							 transfer_len, false);
				NRF_P0_NS->OUTCLR = (1 << 17);
				if (ret != 0) {
					LOG_ERR("stream_flash_buffered_write error %d", ret);
				}
			}
		}
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
	return (nrfx_uarte_init(&uarte_inst, &config, NULL) ==
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

static void flash_init(void)
{
	int err;

	flash_dev = device_get_binding(EXT_FLASH_DEVICE);
	if (flash_dev == NULL) {
		LOG_ERR("Failed to get flash device: %s", EXT_FLASH_DEVICE);
		return;
	}

	err = stream_flash_init(&stream, flash_dev, write_buf, WRITE_BUF_LEN, 0, 0, NULL);
	if (err) {
		LOG_ERR("stream_flash_init failed (err %d)", err);
	}
	is_flash_init_done = true;
}

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

	NRF_P0_NS->OUTSET = (1 << 18);
	k_fifo_put(&trace_fifo, mem_ptr);
	NRF_P0_NS->OUTCLR = (1 << 18);
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
