/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <stdbool.h>
#include <kernel.h>
#include <string.h>

#include "modem_trace.h"
#include "mock_nrfx_uarte.h"
#include "mock_SEGGER_RTT.h"
#include "mock_nrf_modem_at.h"

extern int unity_main(void);

bool runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART = false;
bool runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT = false;

static bool trace_abort_received;
static int64_t trace_abort_timestamp = INT64_MAX;

void setUp(void)
{
	mock_nrfx_uarte_Init();
	mock_SEGGER_RTT_Init();
	mock_nrf_modem_at_Init();
}

void tearDown(void)
{
	mock_nrfx_uarte_Verify();
	mock_SEGGER_RTT_Verify();
	mock_nrf_modem_at_Verify();

	runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART = false;
	runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT = false;
	trace_abort_received = false;
	trace_abort_timestamp = INT64_MAX;
}
/* Suite teardown shall finalize with mandatory call to generic_suiteTearDown. */
extern int generic_suiteTearDown(int num_failures);

int test_suiteTearDown(int num_failures)
{
	return generic_suiteTearDown(num_failures);
}

static nrfx_err_t nrfx_uarte_init_callback(nrfx_uarte_t const *p_instance,
					   nrfx_uarte_config_t const *p_config,
					   nrfx_uarte_event_handler_t event_handler,
					   int no_of_calls)
{
	TEST_ASSERT(runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART == true);
	TEST_ASSERT(runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT == false);

	TEST_ASSERT_NOT_EQUAL(NULL, p_config);
	TEST_ASSERT_EQUAL(DT_N_S_uart_1_P_tx_pin, p_config->pseltxd);
	TEST_ASSERT_EQUAL(DT_N_S_uart_1_P_rx_pin, p_config->pselrxd);
	TEST_ASSERT_EQUAL(NRF_UARTE_PSEL_DISCONNECTED, p_config->pselcts);
	TEST_ASSERT_EQUAL(NRF_UARTE_PSEL_DISCONNECTED, p_config->pselrts);
	TEST_ASSERT_EQUAL(NRF_UARTE_HWFC_DISABLED, p_config->hal_cfg.hwfc);
	TEST_ASSERT_EQUAL(NRF_UARTE_PARITY_EXCLUDED, p_config->hal_cfg.parity);
	TEST_ASSERT_EQUAL(NRF_UARTE_BAUDRATE_1000000, p_config->baudrate);
	TEST_ASSERT_EQUAL(NRFX_UARTE_DEFAULT_CONFIG_IRQ_PRIORITY, p_config->interrupt_priority);
	TEST_ASSERT_EQUAL(NULL, p_config->p_context);

	TEST_ASSERT_NOT_EQUAL(NULL, p_instance);
	TEST_ASSERT_EQUAL(NRFX_UARTE1_INST_IDX, p_instance->drv_inst_idx);

	TEST_ASSERT_EQUAL(NULL, event_handler);

	return 0;
}

void test_modem_trace_init_uart_transport_medium(void)
{
	runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART = true;

	__wrap_nrfx_uarte_init_ExpectAnyArgsAndReturn(NRFX_SUCCESS);
	__wrap_nrfx_uarte_init_AddCallback(&nrfx_uarte_init_callback);

	TEST_ASSERT_EQUAL(0, modem_trace_init());
}

int rtt_init_callback(const char* sName,
					  void* pBuffer,
					  unsigned BufferSize,
					  unsigned Flags,
					  int no_of_calls)
{
	TEST_ASSERT(runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT == true);
	TEST_ASSERT(runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART == false);

	char * exp_sName = "modem_trace";

	TEST_ASSERT_EQUAL_CHAR_ARRAY(exp_sName, sName, sizeof(exp_sName));
	TEST_ASSERT_NOT_EQUAL(NULL, pBuffer);
	TEST_ASSERT_EQUAL(CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE, BufferSize);
	TEST_ASSERT_EQUAL(SEGGER_RTT_MODE_NO_BLOCK_SKIP, Flags);

	return 0;
}

void test_modem_trace_init_rtt_transport_medium(void)
{
	runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT = true;

	SEGGER_RTT_AllocUpBuffer_ExpectAnyArgsAndReturn(1);
	SEGGER_RTT_AllocUpBuffer_AddCallback(&rtt_init_callback);

	TEST_ASSERT_EQUAL(0, modem_trace_init());
}

void test_modem_trace_start_coredump_only(void)
{
	__wrap_nrf_modem_at_printf_ExpectAndReturn("AT%%XMODEMTRACE=1,1", 0);
	TEST_ASSERT_EQUAL(0, modem_trace_start(MODEM_TRACE_COREDUMP_ONLY, 0, 10));
}

void test_modem_trace_start_all(void)
{
	__wrap_nrf_modem_at_printf_ExpectAndReturn("AT%%XMODEMTRACE=1,2", 0);
	TEST_ASSERT_EQUAL(0, modem_trace_start(MODEM_TRACE_ALL, 0, 10));
}

void test_modem_trace_start_ip_only(void)
{
	__wrap_nrf_modem_at_printf_ExpectAndReturn("AT%%XMODEMTRACE=1,4", 0);
	TEST_ASSERT_EQUAL(0, modem_trace_start(MODEM_TRACE_IP_ONLY, 0, 10));
}

void test_modem_trace_start_lte_ip(void)
{
	__wrap_nrf_modem_at_printf_ExpectAndReturn("AT%%XMODEMTRACE=1,5", 0);
	TEST_ASSERT_EQUAL(0, modem_trace_start(MODEM_TRACE_LTE_IP, 0, 10));
}


static int nrf_modem_at_printf_callback(const char *fmt, int num_calls)
{
	if (strcmp(fmt, "AT%%XMODEMTRACE=0") == 0 )
	{
		trace_abort_received = true;
		trace_abort_timestamp = k_uptime_get();
	}

	return 0;
}

void test_modem_trace_start_with_duration(void)
{
	const uint16_t test_duration_in_sec = 2;
	const uint16_t extra_allowed_time_in_ms = 20;

	__wrap_nrf_modem_at_printf_ExpectAndReturn("AT%%XMODEMTRACE=1,5", 0);


	TEST_ASSERT_EQUAL(0, modem_trace_start(MODEM_TRACE_LTE_IP, test_duration_in_sec, 10));

	int64_t trace_start_time_stamp;
	trace_start_time_stamp = k_uptime_get();

	__wrap_nrf_modem_at_printf_AddCallback(&nrf_modem_at_printf_callback);
	__wrap_nrf_modem_at_printf_ExpectAndReturn("AT%%XMODEMTRACE=0", 0);

	k_sleep(K_MSEC(test_duration_in_sec * 1000 + extra_allowed_time_in_ms));

	TEST_ASSERT_TRUE(trace_abort_received);
	/* Verify that the trace session was aborted only after the required duration. */
	TEST_ASSERT_GREATER_OR_EQUAL(test_duration_in_sec * 1000,
								(trace_abort_timestamp - trace_start_time_stamp));
}


void main(void)
{
	(void)unity_main();
}
