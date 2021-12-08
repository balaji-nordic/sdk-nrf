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
target_include_directories(app PRIVATE ${ZEPHYR_BASE}/modules/hal_nordic/nrfx)

# Define the device. Otherwise an "unknown device" warning will be triggered when compiling
# nrfx_config.h
target_compile_options(app PRIVATE -DNRF9160_XXAA)

