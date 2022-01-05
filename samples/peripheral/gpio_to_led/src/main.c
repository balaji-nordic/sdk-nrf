/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <logging/log.h>
#include <nrfx_gpiote.h>
#include <nrfx_dppi.h>
#include <helpers/nrfx_gppi.h>

LOG_MODULE_REGISTER(app);

#define INPUT_PIN 17
#define OUTPUT_PIN 2

static void inp_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	LOG_INF("Input");
}

void main(void)
{
	nrfx_err_t err;

	err = nrfx_gpiote_init(0);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("nrfx_gpiote_init error: %08x", err);
		return;
	}

	nrfx_gpiote_in_config_t const in_config = {
		.sense = NRF_GPIOTE_POLARITY_TOGGLE,
		.pull = NRF_GPIO_PIN_PULLUP,
		.is_watcher = false,
		.hi_accuracy = true,
		.skip_gpio_setup = false,
	};

	/* Initialize input pin to generate event on high to low transition
	 * (falling edge)
	 */
	err = nrfx_gpiote_in_init(INPUT_PIN, &in_config, &inp_handler);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("nrfx_gpiote_in_init error: %08x", err);
		return;
	}

	nrfx_gpiote_out_config_t const out_config = {
		.action = NRF_GPIOTE_POLARITY_TOGGLE,
		.init_state = 1,
		.task_pin = true,
	};

	/* Initialize output pin. SET task will turn the LED on,
	 * CLR will turn it off and OUT will toggle it.
	 */
	err = nrfx_gpiote_out_init(OUTPUT_PIN, &out_config);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("nrfx_gpiote_out_init error: %08x", err);
		return;
	}

	nrfx_gpiote_in_event_enable(INPUT_PIN, true);
	nrfx_gpiote_out_task_enable(OUTPUT_PIN);

	LOG_INF("nrfx_gpiote initialized");

	/* Allocate a DPPI channel. */
	uint8_t channel;
	err = nrfx_dppi_channel_alloc(&channel);

	if (err != NRFX_SUCCESS) {
		LOG_ERR("(D)PPI channel allocation error: %08x", err);
		return;
	}
	/* Configure endpoints of the channel so that the input pin event is
	 * connected with the output pin OUT task. This means that each time
	 * the button is pressed, the LED pin will be toggled.
	 */
	nrfx_gppi_channel_endpoints_setup(channel,
		nrfx_gpiote_in_event_addr_get(INPUT_PIN),
		nrfx_gpiote_out_task_addr_get(OUTPUT_PIN));

	err = nrfx_dppi_channel_enable(channel);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("Failed to enable (D)PPI channel, error: %08x", err);
		return;
	}

	LOG_INF("(D)PPI configured, leaving main()");
}
