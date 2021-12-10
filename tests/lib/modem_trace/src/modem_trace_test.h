/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef MODEM_TRACE_TEST_H_
#define MODEM_TRACE_TEST_H_

#include <stdbool.h>
#include <sys/util.h>
#include <devicetree.h>

/* Override IS_ENABLED macro so that the definition of the trace medium can be controlled by
 * the test case at run time.
 */
#undef IS_ENABLED
#define IS_ENABLED(x) runtime_##x

extern bool runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART;
extern bool runtime_CONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT;

/* Override the definition of DT_NODE_HAS_PROP defined in devicetree.h to always return true as
 * we are only doing unit testing and are not concerned with device tree properties.
 */
#undef DT_NODE_HAS_PROP
#define DT_NODE_HAS_PROP(x) true

#endif
