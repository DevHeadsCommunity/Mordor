# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mordor)

FILE(GLOB app_sources src/*.c)
FILE(GLOB foc_sources src/foc/*.c)

set(FOC_INC ${APPLICATION_SOURCE_DIR}/include/foc)

target_sources(app PRIVATE ${app_sources} ${foc_sources})

target_include_directories(app PRIVATE  ${FOC_INC})

# Optionally, include the tests directory if testing
if(CONFIG_TESTING)
    add_subdirectory(tests)
endif()
