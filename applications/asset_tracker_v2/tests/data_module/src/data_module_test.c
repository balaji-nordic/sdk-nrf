/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <unity.h>
#include <stdlib.h>
#include <mock_modules_common.h>
#include <mock_event_manager.h>
#include <mock_cJSON_os.h>
#include <mock_date_time.h>
#include <settings/mock_settings.h>

extern struct event_listener __event_listener_data_module;
extern struct _static_thread_data _k_thread_data_data_module_thread;

static k_tid_t data_module_thread_id;

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

/* Dummy structs to please linker. The EVENT_SUBSCRIBE macros in gps_module.c
 * depend on these to exist. But since we are unit testing, we dont need
 * these subscriptions and hence these structs can remain uninitialized.
 */
const struct event_type __event_type_gnss_module_event;
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

int module_start_callback(struct module_data *module, int number_of_calls)
{
	data_module_thread_id = module->thread_id;

	printf("Thread ID known!!");
	//k_thread_suspend(data_module_thread_id);
	return 0;
}

void test_data_module_start(void)
{
	__wrap_module_start_ExpectAnyArgsAndReturn(9);
	__wrap_module_start_AddCallback(&module_start_callback);
	__wrap_settings_load_subtree_ExpectAnyArgsAndReturn(0);
	__wrap_settings_subsys_init_ExpectAndReturn(0);
	__wrap_cJSON_Init_Expect();
	__wrap_module_get_next_msg_ExpectAnyArgsAndReturn(0);
	__wrap_date_time_register_handler_ExpectAnyArgs();

	/* Stimulus: Simulate a call to spawn the data module thread. */
	DATA_MODULE_INIT_ENTRY(NULL, NULL, NULL);

	/* Now that the thread ID of the data module should be available in the unit test, suspend the
	 * thread in order to avoid the kernel spawing the thread unnecessarily. */
	//k_thread_suspend(data_module_thread_id);
	//k_thread_abort(data_module_thread_id);
}

void main(void)
{
	(void)unity_main();
}
