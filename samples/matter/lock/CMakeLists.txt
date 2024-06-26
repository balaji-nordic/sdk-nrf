#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

# Set Kconfig root files that will be processed as a first Kconfig for used child images.
set(mcuboot_KCONFIG_ROOT "\\\${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/config/nrfconnect/chip-module/Kconfig.mcuboot.root")
set(multiprotocol_rpmsg_KCONFIG_ROOT "\\\${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/config/nrfconnect/chip-module/Kconfig.multiprotocol_rpmsg.root")
set(hci_ipc_KCONFIG_ROOT "\\\${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/config/nrfconnect/chip-module/Kconfig.hci_ipc.root")

# For prj.conf the CONF_FILE is empty. In other cases extract the exact file name from the path string.
if(CONF_FILE)
    get_filename_component(CONF_FILE_NAME ${CONF_FILE} NAME CACHE)
endif()

if(CONF_FILE_NAME STREQUAL "prj_thread_wifi_switched.conf")
    set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/pm_static_dfu_wifi_ext_patch.yml)
    # TODO: Add Kconfig for adding custom Matter compilation flags
    set(CHIP_CFLAGS "-flto")
elseif(NOT CONF_FILE_NAME STREQUAL "prj_no_dfu.conf")
    set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/pm_static_dfu.yml)
endif()

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})

project(matter-lock)

# Enable GNU STD support and initialize the Matter Data Model.
include(${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/config/nrfconnect/app/enable-gnu-std.cmake)
include(${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/src/app/chip_data_model.cmake)

# NORDIC SDK APP START

# Include all source files that are located in the Matter common directory.
include(${ZEPHYR_NRF_MODULE_DIR}/samples/matter/common/cmake/source_common.cmake)

target_include_directories(app PRIVATE
    src
)

target_sources(app PRIVATE
    src/app_task.cpp
    src/bolt_lock_manager.cpp
    src/main.cpp
    src/zcl_callbacks.cpp
)

if(CONFIG_THREAD_WIFI_SWITCHING)
    target_sources(app PRIVATE src/thread_wifi_switch.cpp
        # TODO: Remove the following lines once there is LTO support for the Zephyr library
        src/zap-generated/callback-stub.cpp
        src/zap-generated/IMClusterCommandHandler.cpp
    )

    # Enable link-time optimization for the app and WPA supplicant libraries
    target_compile_options(app PRIVATE -flto)
    target_compile_options(..__nrf__modules__hostap PRIVATE -flto)
endif()

chip_configure_data_model(app
    INCLUDE_SERVER
    BYPASS_IDL
    GEN_DIR src/zap-generated
    ZAP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/lock.zap
)
# NORDIC SDK APP END
