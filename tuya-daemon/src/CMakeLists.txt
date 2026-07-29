cmake_minimum_required(VERSION 3.10)
project(sys_monitor_daemon C)

set(DEMO_NAME "sys_monitor_daemon")

add_executable(
    ${DEMO_NAME}
    src/main.c
    src/sys_info.c
    src/json_formatter.c
    src/network.c
    src/tuya.c
)

target_include_directories(
    ${DEMO_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src
        $ENV{STAGING_DIR}/usr/include
        $ENV{STAGING_DIR}/usr/include/tuya
)

target_link_libraries(
    ${DEMO_NAME}
    PUBLIC
        ubus
        ubox
        blobmsg_json
        argp
        utils_modules
        platform_port
        middleware_implementation
        link_core
        pthread
        m
)