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

void main(void)
{
	/* Read the levels of Input pins periodically and copy the level to the led pins. */
	nrf_gpio_pin_dir_set(LED1_PIN, NRF_GPIO_PIN_DIR_OUTPUT);
	nrf_gpio_pin_dir_set(LED2_PIN, NRF_GPIO_PIN_DIR_OUTPUT);
	nrf_gpio_pin_dir_set(INPUT_PIN_1, NRF_GPIO_PIN_DIR_INPUT);
	nrf_gpio_pin_dir_set(INPUT_PIN_2, NRF_GPIO_PIN_DIR_INPUT);

	while(1){
		nrf_gpio_pin_write(LED1_PIN, nrf_gpio_pin_read(INPUT_PIN_1));
		nrf_gpio_pin_write(LED2_PIN, nrf_gpio_pin_read(INPUT_PIN_2));
		k_sleep(K_MSEC(5));
	}
}
