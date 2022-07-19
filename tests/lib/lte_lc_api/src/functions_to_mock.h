/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <nrf_modem_at.h>

int nrf_modem_at_notif_handler_set(nrf_modem_at_notif_handler_t callback);


int nrf_modem_at_printf(const char *fmt, ...);


int nrf_modem_at_cmd(void *buf, size_t len, const char *fmt, ...);


int nrf_modem_at_cmd_async(nrf_modem_at_resp_handler_t callback, const char *fmt, ...);


int nrf_modem_at_err_type(int error);


int nrf_modem_at_err(int error);


int nrf_modem_at_cmd_filter_set(const struct nrf_modem_at_cmd_filter *filters,
		size_t len);
