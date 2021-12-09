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

bool runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART;


void setUp(void)
{
	mock_nrfx_uarte_Init();
}

void tearDown(void)
{
	mock_nrfx_uarte_Verify();
}

void test_modem_trace_init(void)
{
	runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART = true;

	__wrap_nrfx_uarte_init_ExpectAnyArgsAndReturn(NRFX_SUCCESS);

	TEST_ASSERT_EQUAL(modem_trace_init(), 0);
}

void main(void)
{
	(void)unity_main();
}
