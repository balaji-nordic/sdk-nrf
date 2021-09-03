#include <stdlib.h>

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
#include <nrfx_uarte.h>
#endif

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
#include <SEGGER_RTT.h>
#endif

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
/* Use UARTE1 as a dedicated peripheral to print traces. */
static const nrfx_uarte_t uarte_inst = NRFX_UARTE_INSTANCE(1);
#endif


int32_t nrf_modem_os_trace_put(const uint8_t * const data, uint32_t len)
{
#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
	/* Max DMA transfers are 255 bytes.
	 * Split RAM buffer into smaller chunks
	 * to be transferred using DMA.
	 */
	uint32_t remaining_bytes = len;

	while (remaining_bytes) {
		uint8_t transfer_len = MIN(remaining_bytes, UINT8_MAX);
		uint32_t idx = len - remaining_bytes;

		nrfx_uarte_tx(&uarte_inst, &data[idx], transfer_len);
		remaining_bytes -= transfer_len;
	}
#endif

#ifdef CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
	/* First, let's check if the buffer has been correctly
	 * allocated for the modem trace
	 */
	if (trace_rtt_channel < 0) {
		return 0;
	}

	uint32_t remaining_bytes = len;

	while (remaining_bytes) {
		uint8_t transfer_len = MIN(remaining_bytes, RTT_BUF_SZ);
		uint32_t idx = len - remaining_bytes;

		SEGGER_RTT_WriteSkipNoLock(trace_rtt_channel, &data[idx],
			transfer_len);
		remaining_bytes -= transfer_len;
	}
#endif
	return 0;
}
