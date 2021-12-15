#
# Copyright (c) 2021 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(modem_trace_test)

# create mock
cmock_handle(${ZEPHYR_BASE}/../modules/hal/nordic/nrfx/drivers/include/nrfx_uarte.h )
cmock_handle(${ZEPHYR_BASE}/../modules/debug/segger/SEGGER/SEGGER_RTT.h )
cmock_handle(${ZEPHYR_BASE}/../nrfxlib/nrf_modem/include/nrf_modem_at.h )

# generate runner for the test
test_runner_generate(src/modem_trace_test.c)

target_include_directories(app PRIVATE src)

# add test file
target_sources(app PRIVATE src/modem_trace_test.c)

# add unit under test
target_sources(app PRIVATE ../../../lib/modem_trace/modem_trace.c)

# include paths
target_include_directories(app PRIVATE ${ZEPHYR_BASE}/../modules/hal/nordic/nrfx)
target_include_directories(app PRIVATE ${ZEPHYR_BASE}/../modules/hal/nordic/nrfx/mdk)
target_include_directories(app PRIVATE ${ZEPHYR_BASE}/../modules/hal/cmsis/CMSIS/Core/Include/)
target_include_directories(app PRIVATE ${ZEPHYR_BASE}/../modules/debug/segger/Config/)
target_include_directories(app PRIVATE ${ZEPHYR_BASE}/modules/hal_nordic/nrfx)
target_include_directories(app PRIVATE ${ZEPHYR_BASE}/../nrf/include/modem/)

# Define the device. Otherwise an "unknown device" warning will be triggered when compiling
# nrfx_config.h
target_compile_options(app PRIVATE -DNRF9160_XXAA)
# Define NRFX_DECLARE_ONLY to simplify generation of mocks. If this is not done, then the
# the compiler would be unable to compile the generated mocks for nrfx uart driver.
target_compile_options(app PRIVATE -DNRFX_DECLARE_ONLY)
# Since Device Tree is not available when running unit tests, we need to provide the hardware
# details that otherwise would be fetched by the UUT from the Device Tree.
target_compile_options(app PRIVATE -DDT_N_S_uart_1_P_rx_pin=17)
target_compile_options(app PRIVATE -DDT_N_S_uart_1_P_tx_pin=18)
target_compile_options(app PRIVATE -DNRFX_UARTE1_ENABLED)
# Define the Up and Down buffer count needed for compiling the Segger RTT headers.
target_compile_options(app PRIVATE
        -DCONFIG_SEGGER_RTT_MAX_NUM_UP_BUFFERS=3
        -DCONFIG_SEGGER_RTT_MAX_NUM_DOWN_BUFFERS=3)

# Define both trace transport mediums so that the all code is compiled in for
# unit testing
target_compile_options(app PRIVATE
        -DCONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_UART
        -DCONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT
        -DCONFIG_NRF_MODEM_LIB_TRACE_MEDIUM_RTT_BUF_SIZE=255)

# Preinclude file to the module under test to redefine IS_ENABLED() macro
# which is used in the module.
set_property(SOURCE ../../../lib/modem_trace/modem_trace.c PROPERTY COMPILE_FLAGS
    "-include modem_trace_test.h")
