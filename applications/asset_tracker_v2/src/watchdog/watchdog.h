/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**@file
 *
 * @brief   Watchdog module for asset tracker
 */

#ifndef WATCHDOG_H__
#define WATCHDOG_H__

#include <zephyr.h>

#ifdef __cplusplus
extern "C" {
#endif

enum watchdog_evt_type {
	WATCHDOG_EVT_START,
	WATCHDOG_EVT_TIMEOUT_INSTALLED,
	WATCHDOG_EVT_FEED
};

struct watchdog_evt {
	enum watchdog_evt_type type;
	uint32_t timeout;
};

/** @brief Watchdog library event handler.
 *
 *  @param[in] evt The event and any associated parameters.
 */
typedef void (*watchdog_evt_handler_t)(const struct watchdog_evt *evt);

int watchdog_init_and_start(void);

/** @brief
 *
 *  @warning The library only allows for one event handler to be registered
 *           at a time. A passed in event handler in this function will
 *           overwrite the previously set event handler.
 *
 *  @param evt_handler Event handler. Handler is de-registered if parameter is NULL.
 */
void watchdog_register_handler(watchdog_evt_handler_t evt_handler);

#ifdef __cplusplus
}
#endif

#endif /* WATCHDOG_H__ */
