/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdlib.h>
#include <stdbool.h>
#include "zephyr.h"
#include "uart.h"
#include "mosh_print.h"

/* During startup the uarts are always enabled. */
static bool uarts_enabled = true;

void disable_uarts(void)
{
	NRF_UARTE0_NS->TASKS_STOPRX = 1;
	NRF_UARTE0_NS->TASKS_STOPTX = 1;
	NRF_UARTE0_NS->ENABLE = UARTE_ENABLE_ENABLE_Disabled;

	NRF_UARTE1_NS->TASKS_STOPRX = 1;
	NRF_UARTE1_NS->TASKS_STOPTX = 1;
	NRF_UARTE1_NS->ENABLE = UARTE_ENABLE_ENABLE_Disabled;

	uarts_enabled = false;
}

void enable_uarts(void)
{
	NRF_UARTE0_NS->ENABLE = UARTE_ENABLE_ENABLE_Enabled;
	NRF_UARTE0_NS->TASKS_STARTRX = 1;
	NRF_UARTE0_NS->TASKS_STARTTX = 1;

	NRF_UARTE1_NS->ENABLE = UARTE_ENABLE_ENABLE_Enabled;
	NRF_UARTE1_NS->TASKS_STARTRX = 1;
	NRF_UARTE1_NS->TASKS_STARTTX = 1;

	uarts_enabled = true;
}

void toggle_uarts_state(void)
{
	if (!uarts_enabled) {
		enable_uarts();
		if (!k_is_in_isr()) {
			/* most_print uses mutex. And hence cant be used from an ISR. */
			mosh_print("Enabled UARTs");
		}
	} else {
		if (!k_is_in_isr()) {
			/* most_print uses mutex. And hence cant be used from an ISR. */
			mosh_print("Disabling UARTs");
		}
		k_sleep(K_MSEC(500)); /* allow little time for printing the notification */
		disable_uarts();
	}
}
