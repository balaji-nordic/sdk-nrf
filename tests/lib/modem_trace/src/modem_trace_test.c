/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <stdbool.h>
#include <modem/modem_trace.h>

extern int unity_main(void);

bool runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART;

void test_modem_trace_init(void)
{
	runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART = true;

	TEST_ASSERT_EQUAL(modem_trace_init(), 0);
}

void main(void)
{
	(void)unity_main();
}
