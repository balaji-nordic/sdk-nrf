/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(app);

void main(void)
{
	while(1)
	{
		NRF_P0_NS->DIRSET = (1 << 2);
		NRF_P0_NS->OUTSET = (1 << 2);

		k_sleep(K_SECONDS(1));
		NRF_P0_NS->OUTCLR = (1 << 2);

		k_sleep(K_SECONDS(1));
	}
}
