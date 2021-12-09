/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "nrfx_uarte.h"

int modem_trace_init(void)
{
	nrfx_uarte_init(NULL, NULL, NULL);

	return 0;
}
