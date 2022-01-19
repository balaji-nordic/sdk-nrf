/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <errno.h>
#include <drivers/uart.h>
#include <zephyr.h>
#include "timing_consts.h"
#include "dtm.h"

#define OUTPUT_PIN_LED_1 17
#define OUTPUT_PIN_LED_2 20
#define RECEIVE_TEST_DURATION_SECONDS 2
#define ACCEPTED_PACKET_LOSS 0.1 /* 10% packet loss */

#define TEST_PAYLOAD_SIZE 37

static void per_test_timer_handler(struct k_timer *timer);

K_TIMER_DEFINE(per_test_timer, per_test_timer_handler, NULL);
static uint16_t num_packets_rcvd;

static void configure_leds(void)
{
	/* Conect the GPIOs by writing to the control pin. */
	NRF_P0->DIRSET = (1 << 13);
	NRF_P0->OUTSET = (1 << 13);

	NRF_P0->DIRSET = ((1 << OUTPUT_PIN_LED_1) | (1 << OUTPUT_PIN_LED_2));
	NRF_P0->OUTCLR = ((1 << OUTPUT_PIN_LED_1) | (1 << OUTPUT_PIN_LED_2));
}

static void led_on(uint8_t pin)
{
	NRF_P0->OUTSET = (1 << pin);
}

static void led_off(uint8_t pin)
{
	NRF_P0->OUTCLR = (1 << pin);
}

static void per_test_timer_handler(struct k_timer *timer)
{
	/* Number of packets in last test duration. */
	static uint32_t num_pkts_rcvd_in_last_interval = 0;

	NVIC_DisableIRQ(RADIO_IRQn);

	const uint32_t cumulative_pkt_count = dtm_rx_pkt_cnt_get();

	NVIC_EnableIRQ(RADIO_IRQn);

	const uint32_t num_packets_rcvd = cumulative_pkt_count - num_pkts_rcvd_in_last_interval;

	printk("Number of packets received = %d, cumulative count = %d, num_pkts_rcvd_in_last_interval = %d\n",
			num_packets_rcvd, cumulative_pkt_count, num_pkts_rcvd_in_last_interval);

	const uint32_t exp_on_air_pkt_duration_us = TEST_PDU_LENGTH_US(dtm_radio_mode_get());

	/* According to Direct Test Mode specification in Bluetooth Spec Part F, the
	 * distance between start bits of two consecutive packets is as follows.
	 * I(L) = ceil((L + 249) / 625) * 625 μs.
	 */
	/* Find the distance between start bits of 2 consecutive packets */
	uint16_t start_bits_distance_us = ceiling_fraction(exp_on_air_pkt_duration_us + 249, 625) * 625;

	uint16_t num_of_expected_packets = (RECEIVE_TEST_DURATION_SECONDS * 1000000) / start_bits_distance_us;

	printk("Expected number of packets = %d \n", num_of_expected_packets);

	uint16_t threshold = (1 - ACCEPTED_PACKET_LOSS) * num_of_expected_packets;

	if (num_packets_rcvd < threshold){
		printk("Expected packet error rate not met. Expected atleast %d packets\n", threshold);
		led_on(OUTPUT_PIN_LED_2);
		led_off(OUTPUT_PIN_LED_1);
	}
	else{
		printk("Expected mimimum number of packets %d received\n", threshold);
		printk("Packets lost = %d\n", num_of_expected_packets - num_packets_rcvd);
		led_on(OUTPUT_PIN_LED_1);
		led_off(OUTPUT_PIN_LED_2);
	}
	num_pkts_rcvd_in_last_interval = cumulative_pkt_count;
}

void main(void)
{
	int err;
	const struct device *uart;
	bool is_msb_read = false;
	uint8_t rx_byte;
	uint16_t dtm_cmd;
	uint16_t dtm_evt;
	int64_t msb_time;
	int64_t receiver_test_start_time;
	int64_t test_end_time;

	configure_leds();

	printk("Starting Direct Test Mode example\n");

	uart = device_get_binding("UART_0");
	if (!uart) {
		printk("Error during UART device initialization\n");
	}

	err = dtm_init();
	if (err) {
		printk("Error during DTM initialization: %d\n", err);
	}

	for (;;) {
		dtm_wait();

		err = uart_poll_in(uart, &rx_byte);
		if (err) {
			if (err != -1) {
				printk("UART polling error: %d\n", err);
			}

			/* Nothing read from the UART. */
			continue;
		}

		if (!is_msb_read) {
			/* This is first byte of two-byte command. */
			is_msb_read = true;
			dtm_cmd = rx_byte << 8;
			msb_time = k_uptime_get();

			/* Go back and wait for 2nd byte of command word. */
			continue;
		}

		/* This is the second byte read; combine it with the first and
		 * process command.
		 */
		if ((k_uptime_get() - msb_time) >
		    DTM_UART_SECOND_BYTE_MAX_DELAY) {
			/* More than ~5mS after msb: Drop old byte, take the
			 * new byte as MSB. The variable is_msb_read will
			 * remain true.
			 */
			dtm_cmd = rx_byte << 8;
			msb_time = k_uptime_get();
			/* Go back and wait for 2nd byte of command word. */
			continue;
		}

		/* 2-byte UART command received. */
		is_msb_read = false;
		dtm_cmd |= rx_byte;

		printk("Sending 0x%04X DTM command\n", dtm_cmd);

		enum dtm_cmd_code cmd_code = (dtm_cmd >> 14) & 0x03;

		if (dtm_cmd_put(dtm_cmd) != DTM_SUCCESS) {
			/* Extended error handling may be put here.
			 * Default behavior is to return the event on the UART;
			 * the event report will reflect any lack of success.
			 */
		}

		int64_t cmd_time = k_uptime_get();

		printk("Command %d time %lld\n",cmd_code, cmd_time);

		if (cmd_code == LE_RECEIVER_TEST)
		{
			num_packets_rcvd = 0;
			receiver_test_start_time = cmd_time;
			k_timer_start(&per_test_timer, K_SECONDS(RECEIVE_TEST_DURATION_SECONDS), K_SECONDS(RECEIVE_TEST_DURATION_SECONDS));
		}

		if (cmd_code == LE_TEST_END)
		{
			test_end_time = cmd_time;
			printk("Test END time %lld\n", test_end_time);
			led_off(OUTPUT_PIN_LED_1);
			led_off(OUTPUT_PIN_LED_2);
		}
		/* Retrieve result of the operation. This implementation will
		 * busy-loop for the duration of the byte transmissions on the
		 * UART.
		 */
		if (dtm_event_get(&dtm_evt)) {
			printk("Received 0x%04X DTM event\n", dtm_evt);

			/* Report command status on the UART. */

			/* Transmit MSB of the result. */
			uart_poll_out(uart, (dtm_evt >> 8) & 0xFF);

			/* Transmit LSB of the result. */
			uart_poll_out(uart, dtm_evt & 0xFF);
		}

		if (cmd_code == LE_TEST_END)
		{
			k_timer_stop(&per_test_timer);
		}
	}
}
