/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef _DEBUG_MODULE_EVENT_H_
#define _DEBUG_MODULE_EVENT_H_

/**
 * @brief Debug module event
 * @defgroup debug_module_event Debug module event
 * @{
 */

#include "event_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

enum debug_module_event_type {
	DEBUG_EVT_MEMFAULT_DATA_READY,
	DEBUG_EVT_ERROR
};

struct debug_module_memfault_data {
	uint8_t buf[CONFIG_DEBUG_MODULE_MEMFAULT_CHUNK_SIZE_MAX];
	size_t len;
};

/** @brief Debug event. */
struct debug_module_event {
	struct event_header header;
	enum debug_module_event_type type;

	union {
		struct debug_module_memfault_data memfault;
		int err;
	} data;
};

EVENT_TYPE_DECLARE(debug_module_event);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* _DEBUG_MODULE_EVENT_H_ */
