/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <stdlib.h>
#include <mock_modules_common.h>
#include <mock_event_manager.h>

extern struct event_listener __event_listener_data_module;
extern struct _static_thread_data  _k_thread_data_data_module_thread;

#define DATA_MODULE_EVT_HANDLER(eh) __event_listener_data_module.notification(eh)
#define DATA_MODULE_INIT_ENTRY _k_thread_data_data_module_thread.init_entry

/* Dummy functions and objects. */

/* The following function needs to be stubbed this way because event manager
 * uses heap to allocate memory for events.
 * The other alternative is to mock the k_malloc using CMock framework, but
 * that will pollute the test code and will be an overkill.
 */
void *k_malloc(size_t size)
{
	return malloc(size);
}

int settings_subsys_init(void)
{
	/* Due to NCSDK-10918, this function is not mocked automatically by the
	 * CMock framework. Hence it is defined as a stub here.
	 */
	return 0;
}

/* Dummy structs to please linker. The EVENT_SUBSCRIBE macros in gps_module.c
 * depend on these to exist. But since we are unit testing, we dont need
 * these subscriptions and hence these structs can remain uninitialized.
 */
const struct event_type __event_type_gps_module_event;
const struct event_type __event_type_app_module_event;
const struct event_type __event_type_data_module_event;
const struct event_type __event_type_util_module_event;
const struct event_type __event_type_modem_module_event;
const struct event_type __event_type_cloud_module_event;
const struct event_type __event_type_sensor_module_event;
const struct event_type __event_type_ui_module_event;

/* Dummy functions and objects - End.  */

/* The following is required because unity is using a different main signature
 * (returns int) and zephyr expects main to not return value.
 */
extern int unity_main(void);

/* Suite teardown shall finalize with mandatory call to generic_suiteTearDown. */
extern int generic_suiteTearDown(int num_failures);

int test_suiteTearDown(int num_failures)
{
	return generic_suiteTearDown(num_failures);
}

void setUp(void)
{
	mock_modules_common_Init();
	mock_event_manager_Init();
}

void tearDown(void)
{
	mock_modules_common_Verify();
	mock_event_manager_Verify();
}

void test_data_module_start(void)
{
	DATA_MODULE_INIT_ENTRY();
}

void main(void)
{
	(void)unity_main();
}
