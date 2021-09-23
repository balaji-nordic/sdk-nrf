#include <nrfx_uarte.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(modem_trace_uart, 3);

/* Use UARTE1 as a dedicated peripheral to print traces. */
static const nrfx_uarte_t uarte_inst = NRFX_UARTE_INSTANCE(1);

void modem_trace_uart_init(void)
{
	/* UART pins are defined in "nrf9160dk_nrf9160.dts". */
	const nrfx_uarte_config_t config = {
		/* Use UARTE1 pins routed on VCOM2. */
		.pseltxd = DT_PROP(DT_NODELABEL(uart1), tx_pin),
		.pselrxd = DT_PROP(DT_NODELABEL(uart1), rx_pin),
		.pselcts = NRF_UARTE_PSEL_DISCONNECTED,
		.pselrts = NRF_UARTE_PSEL_DISCONNECTED,

		.hal_cfg.hwfc = NRF_UARTE_HWFC_DISABLED,
		.hal_cfg.parity = NRF_UARTE_PARITY_EXCLUDED,
		.baudrate = NRF_UARTE_BAUDRATE_1000000,

		/* IRQ handler not used. Blocking mode.*/
		.interrupt_priority = NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY,
		.p_context = NULL,
	};

	/* Initialize nrfx UARTE driver in blocking mode. */
	/* TODO: use UARTE in non-blocking mode with IRQ handler. */
	nrfx_uarte_init(&uarte_inst, &config, NULL);
}

int32_t modem_trace_uart_put(const uint8_t *const data, uint32_t len)
{
	/* Max DMA transfers are 255 bytes.
	 * Split RAM buffer into smaller chunks
	 * to be transferred using DMA.
	 */
	uint32_t remaining_bytes = len;
	LOG_INF("modem_trace_uart_put: data = %x, len =  %d" , (uint32_t) data, len);

	while (remaining_bytes) {
		LOG_INF("Before Tx: Uart send: remaining = %d" , remaining_bytes);

		uint8_t transfer_len = MIN(remaining_bytes, UINT8_MAX);
		uint32_t idx = len - remaining_bytes;

		nrfx_uarte_tx(&uarte_inst, &data[idx], transfer_len);
		remaining_bytes -= transfer_len;
		LOG_INF("Before Tx: Uart send: remaining = %d" , remaining_bytes);
	}
	return 0;
}