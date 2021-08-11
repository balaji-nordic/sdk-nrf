/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <logging/log.h>
#if defined(CONFIG_MEMFAULT)
#include <memfault/metrics/metrics.h>
#include <memfault/ports/zephyr/http.h>
#include <memfault/core/data_packetizer.h>
#include <memfault/core/trace_event.h>
#include <memfault/ports/watchdog.h>
#endif

#define MODULE debug_module

#include "watchdog.h"
#include "modules_common.h"
#include "events/app_module_event.h"
#include "events/cloud_module_event.h"
#include "events/data_module_event.h"
#include "events/sensor_module_event.h"
#include "events/util_module_event.h"
#include "events/gps_module_event.h"
#include "events/modem_module_event.h"
#include "events/ui_module_event.h"
#include "events/debug_module_event.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_DEBUG_MODULE_LOG_LEVEL);

struct debug_msg_data {
	union {
		struct cloud_module_event cloud;
		struct util_module_event util;
		struct ui_module_event ui;
		struct sensor_module_event sensor;
		struct data_module_event data;
		struct app_module_event app;
		struct gps_module_event gps;
		struct modem_module_event modem;
	} module;
};

/* Forward declarations. */
static void message_handler(struct debug_msg_data *msg);

static struct module_data self = {
	.name = "debug",
	.msg_q = NULL,
};

/* Handlers */
static bool event_handler(const struct event_header *eh)
{
	if (is_modem_module_event(eh)) {
		struct modem_module_event *event = cast_modem_module_event(eh);
		struct debug_msg_data debug_msg = {
			.module.modem = *event
		};

		message_handler(&debug_msg);
	}

	if (is_cloud_module_event(eh)) {
		struct cloud_module_event *event = cast_cloud_module_event(eh);
		struct debug_msg_data debug_msg = {
			.module.cloud = *event
		};

		message_handler(&debug_msg);
	}

	if (is_gps_module_event(eh)) {
		struct gps_module_event *event = cast_gps_module_event(eh);
		struct debug_msg_data debug_msg = {
			.module.gps = *event
		};

		message_handler(&debug_msg);
	}

	if (is_sensor_module_event(eh)) {
		struct sensor_module_event *event =
				cast_sensor_module_event(eh);
		struct debug_msg_data debug_msg = {
			.module.sensor = *event
		};

		message_handler(&debug_msg);
	}

	if (is_ui_module_event(eh)) {
		struct ui_module_event *event = cast_ui_module_event(eh);
		struct debug_msg_data debug_msg = {
			.module.ui = *event
		};

		message_handler(&debug_msg);
	}

	if (is_app_module_event(eh)) {
		struct app_module_event *event = cast_app_module_event(eh);
		struct debug_msg_data debug_msg = {
			.module.app = *event
		};

		message_handler(&debug_msg);
	}

	if (is_data_module_event(eh)) {
		struct data_module_event *event = cast_data_module_event(eh);
		struct debug_msg_data debug_msg = {
			.module.data = *event
		};

		message_handler(&debug_msg);
	}

	if (is_util_module_event(eh)) {
		struct util_module_event *event = cast_util_module_event(eh);
		struct debug_msg_data debug_msg = {
			.module.util = *event
		};

		message_handler(&debug_msg);
	}

	return false;
}

#if defined(CONFIG_MEMFAULT)
#if defined(CONFIG_WATCHDOG_APPLICATION)
static void watchdog_handler(const struct watchdog_evt *evt)
{
	int err;

	switch (evt->type) {
	case WATCHDOG_EVT_START:
		err = memfault_software_watchdog_enable();
		if (err) {
			LOG_ERR("memfault_software_watchdog_enable, error: %d", err);
		}
		break;
	case WATCHDOG_EVT_FEED:
		err = memfault_software_watchdog_feed();
		if (err) {
			LOG_ERR("memfault_software_watchdog_feed, error: %d", err);
		}
		break;
	case WATCHDOG_EVT_TIMEOUT_INSTALLED:
		err = memfault_software_watchdog_update_timeout(evt->timeout);
		if (err) {
			LOG_ERR("memfault_software_watchdog_update_timeout, error: %d", err);
		}
		break;
	default:
		break;
	}
}
#endif /* defined(CONFIG_WATCHDOG_APPLICATION) */

static void send_memfault_data(void)
{
	bool data_available = memfault_packetizer_data_available();

	if (IS_ENABLED(CONFIG_DEBUG_MODULE_USE_CUSTOM_TRANSPORT) && data_available) {

		uint8_t data[CONFIG_DEBUG_MODULE_MEMFAULT_CHUNK_SIZE_MAX];
		size_t len = sizeof(data);

		while (memfault_packetizer_get_chunk(data, &len)) {
			struct debug_module_event *debug_module_event = new_debug_module_event();

			debug_module_event->type = DEBUG_EVT_MEMFAULT_DATA_READY;
			debug_module_event->data.memfault.len = len;

			BUILD_ASSERT(sizeof(debug_module_event->data.memfault.buf) >= sizeof(data));
			memcpy(debug_module_event->data.memfault.buf, data,
			       sizeof(debug_module_event->data.memfault.buf));

			EVENT_SUBMIT(debug_module_event);
		}

	} else if (data_available) {
		memfault_zephyr_port_post_data();
	}
}

static void add_gps_metrics(uint8_t satellites, uint32_t search_time,
			    enum gps_module_event_type event)
{
	int err;

	if (event == GPS_EVT_DATA_READY) {
		err = memfault_metrics_heartbeat_set_unsigned(
						MEMFAULT_METRICS_KEY(GpsTimeToFix),
						search_time);
		if (err) {
			LOG_ERR("Failed updating GpsTimeToFix metric, error: %d", err);
		}

		err = memfault_metrics_heartbeat_set_unsigned(
						MEMFAULT_METRICS_KEY(GPSFixSatellitesTracked),
						satellites);
		if (err) {
			LOG_ERR("Failed updating GPSFixSatellitesTracked metric, error: %d", err);
		}
	} else if (event == GPS_EVT_TIMEOUT) {
		err = memfault_metrics_heartbeat_set_unsigned(
						MEMFAULT_METRICS_KEY(GpsTimeoutSearchTime),
						search_time);
		if (err) {
			LOG_ERR("Failed updating GpsTimeoutSearchTime metric, error: %d", err);
		}

		err = memfault_metrics_heartbeat_set_unsigned(
						MEMFAULT_METRICS_KEY(GpsTimeoutSatellitesTracked),
						satellites);
		if (err) {
			LOG_ERR("Failed updating GpsTimeoutSatellitesTracked metric, error: %d",
				err);
		}
	} else {
		LOG_ERR("Unknown GPS module event type");
		return;
	}

	memfault_metrics_heartbeat_debug_trigger();
}

static void memfault_handle_event(struct debug_msg_data *msg)
{
	if (IS_EVENT(msg, app, APP_EVT_START)) {
		/* Register callback for watchdog events. Used to attach Memfault software
		 * watchdog.
		 */
#if defined(CONFIG_WATCHDOG_APPLICATION)
		watchdog_register_handler(watchdog_handler);
#endif
	}

	/* Send Memfault data at the same time application data is sent to save overhead
	 * compared to having Memfault SDK trigger regular updates independently. All data
	 * should preferably be sent within the same LTE RRC connected window.
	 */
	if ((IS_EVENT(msg, data, DATA_EVT_DATA_SEND)) ||
	    (IS_EVENT(msg, cloud, CLOUD_EVT_CONNECTED))) {
		send_memfault_data();
		return;
	}

	if ((IS_EVENT(msg, gps, GPS_EVT_TIMEOUT)) ||
	    (IS_EVENT(msg, gps, GPS_EVT_DATA_READY))) {
		add_gps_metrics(msg->module.gps.data.gps.satellites_tracked,
				msg->module.gps.data.gps.search_time,
				msg->module.gps.type);
		return;
	}
}
#endif /* defined(CONFIG_MEMFAULT) */

static void message_handler(struct debug_msg_data *msg)
{
	if (IS_EVENT(msg, app, APP_EVT_START)) {
		int err = module_start(&self);

		if (err) {
			LOG_ERR("Failed starting module, error: %d", err);
			SEND_ERROR(debug, DEBUG_EVT_ERROR, err);
		}
	}

#if defined(CONFIG_MEMFAULT)
	memfault_handle_event(msg);
#endif
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE_EARLY(MODULE, app_module_event);
EVENT_SUBSCRIBE_EARLY(MODULE, modem_module_event);
EVENT_SUBSCRIBE_EARLY(MODULE, cloud_module_event);
EVENT_SUBSCRIBE_EARLY(MODULE, gps_module_event);
EVENT_SUBSCRIBE_EARLY(MODULE, ui_module_event);
EVENT_SUBSCRIBE_EARLY(MODULE, sensor_module_event);
EVENT_SUBSCRIBE_EARLY(MODULE, data_module_event);
EVENT_SUBSCRIBE_EARLY(MODULE, util_module_event);
