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

#define INPUT_PIN_1 17
#define INPUT_PIN_2 18
#define LED1_PIN 2
#define LED2_PIN 3


/* Configure endpoints of the channel so that the input pin polarity is followed by the output
 * pin.
 */
static void connect_pins(uint8_t input_pin, uint8_t output_pin)
{
	nrfx_err_t err;
	static bool gpiote_init_done = false;

	if (!gpiote_init_done) {
		err = nrfx_gpiote_init(0);
		if (err != NRFX_SUCCESS) {
			LOG_ERR("nrfx_gpiote_init error: %08x", err);
			return;
		}
	}
	gpiote_init_done = true;

	nrfx_gpiote_in_config_t const in_config = {
		.sense = NRF_GPIOTE_POLARITY_TOGGLE,
		.pull = NRF_GPIO_PIN_PULLUP,
		.is_watcher = false,
		.hi_accuracy = true,
		.skip_gpio_setup = false,
	};

	err = nrfx_gpiote_in_init(input_pin, &in_config, NULL);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("nrfx_gpiote_in_init error: %08x", err);
		return;
	}

	nrfx_gpiote_out_config_t const out_config = {
		.action = NRF_GPIOTE_POLARITY_TOGGLE,
		.init_state = NRF_GPIOTE_INITIAL_VALUE_LOW,
		.task_pin = true,
	};

	/* Initialize output pin. SET task will turn the LED on,
	 * CLR will turn it off and OUT will toggle it.
	 */
	err = nrfx_gpiote_out_init(output_pin, &out_config);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("nrfx_gpiote_out_init error: %08x", err);
		return;
	}

	nrfx_gpiote_in_event_enable(input_pin, true);
	nrfx_gpiote_out_task_enable(output_pin);

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
		nrfx_gpiote_in_event_addr_get(input_pin),
		nrfx_gpiote_out_task_addr_get(output_pin));

	err = nrfx_dppi_channel_enable(channel);
	if (err != NRFX_SUCCESS) {
		LOG_ERR("Failed to enable (D)PPI channel, error: %08x", err);
		return;
	}

	LOG_INF("Input pin %d connected to output pin %d.", input_pin, output_pin);
}

void main(void)
{
	connect_pins(INPUT_PIN_1, LED1_PIN);
	connect_pins(INPUT_PIN_2, LED2_PIN);
}
