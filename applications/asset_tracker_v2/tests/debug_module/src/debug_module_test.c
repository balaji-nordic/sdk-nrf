/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mock_modules_common.h>
#include <mock_event_manager.h>
#include <watchdog/mock_watchdog_app.h>
#include <memfault/metrics/mock_metrics.h>
#include <memfault/core/mock_data_packetizer.h>

#include "app_module_event.h"
#include "gps_module_event.h"
#include "debug_module_event.h"
#include "data_module_event.h"

extern struct event_listener __event_listener_debug_module;

#define DEBUG_MODULE_EVT_HANDLER(eh) __event_listener_debug_module.notification(eh)

const char * const g_memfault_metrics_id_GPSFixSatellitesTracked;
const char * const g_memfault_metrics_id_GpsTimeToFix;
const char * const g_memfault_metrics_id_GpsTimeoutSatellitesTracked;
const char * const g_memfault_metrics_id_GpsTimeoutSearchTime;

/* Dummy structs to please linker. The EVENT_SUBSCRIBE macros in debug_module.c
 * depend on these to exist. But since we are unit testing, we dont need
 * these subscriptions and hence these structs can remain uninitialized.
 */
const struct event_type __event_type_gps_module_event;
const struct event_type __event_type_debug_module_event;
const struct event_type __event_type_app_module_event;
const struct event_type __event_type_data_module_event;
const struct event_type __event_type_cloud_module_event;
const struct event_type __event_type_modem_module_event;
const struct event_type __event_type_sensor_module_event;
const struct event_type __event_type_ui_module_event;
const struct event_type __event_type_util_module_event;

/* The following is required because unity is using a different main signature
 * (returns int) and zephyr expects main to not return value.
 */
extern int unity_main(void);

/* Suite teardown finalizes with mandatory call to generic_suiteTearDown. */
extern int generic_suiteTearDown(int num_failures);

static watchdog_evt_handler_t debug_module_watchdog_evt_handler;

int test_suiteTearDown(int num_failures)
{
	return generic_suiteTearDown(num_failures);
}

static void watchdog_callback(watchdog_evt_handler_t handler, int number_of_calls)
{
	/* Latch thew for future use. */
	debug_module_watchdog_evt_handler = handler;
}

static void validate_debug_data_ready_evt(struct event_header *eh, int no_of_calls)
{
	struct debug_module_event *event = cast_debug_module_event(eh);

	TEST_ASSERT_EQUAL(DEBUG_EVT_MEMFAULT_DATA_READY, event->type);
}

static void setup_debug_module_in_init_state(void)
{
	struct app_module_event *app_module_event = new_app_module_event();

	app_module_event->type = APP_EVT_START;

	static struct module_data expected_module_data = {
		.name = "debug",
		.msg_q = NULL,
		.supports_shutdown = true,
	};

	__wrap_watchdog_register_handler_AddCallback(&watchdog_callback);
	__wrap_module_start_ExpectAndReturn(&expected_module_data, 0);

	TEST_ASSERT_EQUAL(0, DEBUG_MODULE_EVT_HANDLER((struct event_header *)app_module_event));

	free(app_module_event);
}

void test_memfault_data_trigger_fix(void)
{
	setup_debug_module_in_init_state();

	__wrap__event_submit_ExpectAnyArgs();

	/* Update this function to expect the search time and number of satellites. */
	__wrap_memfault_metrics_heartbeat_set_unsigned_ExpectAndReturn(
						MEMFAULT_METRICS_KEY(GpsTimeToFix),
						60000,
						0);
	__wrap_memfault_metrics_heartbeat_set_unsigned_ExpectAndReturn(
						MEMFAULT_METRICS_KEY(GPSFixSatellitesTracked),
						4,
						0);
	__wrap_memfault_metrics_heartbeat_debug_trigger_Ignore();

	struct gps_module_event *gps_module_event = new_gps_module_event();

	gps_module_event->type = GPS_EVT_DATA_READY;
	gps_module_event->data.gps.satellites_tracked = 4;
	gps_module_event->data.gps.search_time = 60000;

	TEST_ASSERT_EQUAL(0, DEBUG_MODULE_EVT_HANDLER((struct event_header *)gps_module_event));

	free(gps_module_event);
}

void test_memfault_data_trigger_timeout(void)
{
	setup_debug_module_in_init_state();

	__wrap__event_submit_ExpectAnyArgs();

	/* Update this function to expect the search time and number of satellites. */
	__wrap_memfault_metrics_heartbeat_set_unsigned_ExpectAndReturn(
						MEMFAULT_METRICS_KEY(GpsTimeoutSearchTime),
						30000,
						0);
	__wrap_memfault_metrics_heartbeat_set_unsigned_ExpectAndReturn(
						MEMFAULT_METRICS_KEY(GpsTimeoutSatellitesTracked),
						2,
						0);
	__wrap_memfault_metrics_heartbeat_debug_trigger_Ignore();

	struct gps_module_event *gps_module_event = new_gps_module_event();

	gps_module_event->type = GPS_EVT_TIMEOUT;
	gps_module_event->data.gps.satellites_tracked = 2;
	gps_module_event->data.gps.search_time = 30000;

	TEST_ASSERT_EQUAL(0, DEBUG_MODULE_EVT_HANDLER((struct event_header *)gps_module_event));

	free(gps_module_event);
}

void test_memfault_data_trigger_unhandled_event(void)
{
	setup_debug_module_in_init_state();

	__wrap__event_submit_ExpectAnyArgs();

	struct gps_module_event *gps_module_event = new_gps_module_event();

	gps_module_event->type = GPS_EVT_ACTIVE;

	TEST_ASSERT_EQUAL(0, DEBUG_MODULE_EVT_HANDLER((struct event_header *)gps_module_event));

	free(gps_module_event);
}

void test_memfault_data_trigger_data_send_trigger(void)
{
	setup_debug_module_in_init_state();

	__wrap__event_submit_ExpectAnyArgs();

	__wrap_memfault_packetizer_data_available_ExpectAndReturn(1);
	__wrap_memfault_packetizer_get_chunk_ExpectAnyArgsAndReturn(1);
	__wrap_memfault_packetizer_get_chunk_ExpectAnyArgsAndReturn(0);

	/* Expect the gps module to generate an event with the fix data. */
	__wrap__event_submit_Stub(&validate_debug_data_ready_evt);

	struct data_module_event *data_module_event = new_data_module_event();

	data_module_event->type = DATA_EVT_DATA_SEND;

	TEST_ASSERT_EQUAL(0, DEBUG_MODULE_EVT_HANDLER((struct event_header *)data_module_event));

	free(data_module_event);
}

void main(void)
{
	(void)unity_main();
}
