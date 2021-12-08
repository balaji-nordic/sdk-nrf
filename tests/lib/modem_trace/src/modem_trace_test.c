/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <modem/modem_trace.h>

extern int unity_main(void);

void test_modem_trace_init(void)
{
	TEST_ASSERT_EQUAL(modem_trace_init(), 0);
}

void main(void)
{
	(void)unity_main();
}
