#
# Copyright (c) 2021 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(data_module_test)

# generate runner for the test
test_runner_generate(src/data_module_test.c)

# create mock
cmock_handle(../../src/modules/modules_common.h )
cmock_handle(../../src/cloud/cloud_codec/cloud_codec.h )
cmock_handle(${NRF_DIR}/include/event_manager.h )
cmock_handle(${NRF_DIR}/subsys/event_manager/event_manager_priv.h)
cmock_handle(${NRF_DIR}/include/date_time.h )
cmock_handle(${NRF_DIR}/modules/cjson/include/cJSON_os.h)
cmock_handle(${ZEPHYR_BASE}/include/settings/settings.h settings)
#cmock_handle(${ZEPHYR_BASE}/include/sys/reboot.h sys)

# add the unit under test
target_sources(app PRIVATE ../../src/modules/data_module.c)

# add test file
target_sources(app PRIVATE src/data_module_test.c)

target_include_directories(app PRIVATE .)
target_include_directories(app PRIVATE ../../src/)
target_include_directories(app PRIVATE ../../src/modules/)
target_include_directories(app PRIVATE ../../src/events/)
target_include_directories(app PRIVATE ${NRF_DIR}/subsys/event_manager)
target_include_directories(app PRIVATE ${NRF_DIR}/subsys/event_manager_profiler_tracer/ )
target_include_directories(app PRIVATE ${NRF_DIR}/modules/cjson/include)
target_include_directories(app PRIVATE ${ZEPHYR_NRFXLIB_MODULE_DIR}/nrf_modem/include/ )

# Options that cannot be passed through kconfig fragments.
target_compile_definitions(app PRIVATE
  	CONFIG_ASSET_TRACKER_V2_APP_VERSION_MAX_LEN=20
	CONFIG_EVENT_MANAGER_MAX_EVENT_CNT=32)
