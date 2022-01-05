/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Packet Error Rate test sample for nRF52840
 */

#include <zephyr/types.h>
#include <zephyr.h>
#include <drivers/uart.h>

#include <device.h>
#include <soc.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/services/nus.h>
#include <settings/settings.h>

#include <stdio.h>

#include <logging/log.h>

#define LOG_MODULE_NAME per_test_app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define STACKSIZE CONFIG_BT_NUS_THREAD_STACK_SIZE
#define PRIORITY 7

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN	(sizeof(DEVICE_NAME) - 1)

static struct bt_conn *current_conn;
static struct bt_conn *auth_conn;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};


static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (err) {
		LOG_ERR("Connection failed (err %u)", err);
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Connected %s", log_strdup(addr));

	current_conn = bt_conn_ref(conn);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Disconnected: %s (reason %u)", log_strdup(addr), reason);

	if (auth_conn) {
		bt_conn_unref(auth_conn);
		auth_conn = NULL;
	}

	if (current_conn) {
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected    = connected,
	.disconnected = disconnected
};

#define OUTPUT_PIN_LED_1 17
#define OUTPUT_PIN_LED_2 20

static void configure_gpio(void)
{
	/* Conect the GPIOs by writing to the control pin. */
	NRF_P0->DIRSET = (1 << 13);
	NRF_P0->OUTSET = (1 << 13);

	NRF_P0->DIRSET = ((1 << OUTPUT_PIN_LED_1) | (1 << OUTPUT_PIN_LED_2));
	NRF_P0->OUTCLR = ((1 << OUTPUT_PIN_LED_1) | (1 << OUTPUT_PIN_LED_2));
}

void main(void)
{
	int err = 0;

	configure_gpio();

	bt_conn_cb_register(&conn_callbacks);

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth NOT initialized");
	}

	LOG_INF("Bluetooth initialized");

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd,
			      ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return;
	}

	LOG_INF("Advertising started");

	while (1){
		if (current_conn != NULL) {
			NRF_P0->OUTSET = (1 << OUTPUT_PIN_LED_1);
		}
		else {
			NRF_P0->OUTSET = (1 << OUTPUT_PIN_LED_1);
			k_sleep(K_MSEC(500));
			NRF_P0->OUTCLR = (1 << OUTPUT_PIN_LED_1);
			k_sleep(K_MSEC(500));
		}
	}
}

