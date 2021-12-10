/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <stdbool.h>

#include "modem/modem_trace.h"
#include "mock_nrfx_uarte.h"

extern int unity_main(void);

bool runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART = false;
bool runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT = false;

void setUp(void)
{
	mock_nrfx_uarte_Init();
}

void tearDown(void)
{
	mock_nrfx_uarte_Verify();

	runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART = false;
	runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT = false;
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

	TEST_ASSERT_EQUAL(modem_trace_init(), 0);
}

void test_modem_trace_init_rtt_transport_medium(void)
{
	runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT = true;

	TEST_ASSERT_EQUAL(modem_trace_init(), 0);
}

void main(void)
{
	(void)unity_main();
}
