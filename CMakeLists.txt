# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(SRAD_RT C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(SRAD_RT main.cpp )

pico_set_program_name(SRAD_RT "SRAD_RT")
pico_set_program_version(SRAD_RT "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(SRAD_RT 0)
pico_enable_stdio_usb(SRAD_RT 1)

# add_subdirectory(Protothreads)

# Add the standard library to the build
target_link_libraries(SRAD_RT
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(SRAD_RT PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(SRAD_RT 
        pico_stdlib
        pico_btstack_ble
        pico_btstack_cyw43
        pico_cyw43_arch_none
        hardware_irq
        hardware_spi
        hardware_timer
        hardware_sync
        hardware_pio
        hardware_dma
        )

# Add the standard include files to the build
target_include_directories(SRAD_RT PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_btstack_make_gatt_header(SRAD_RT PRIVATE "${CMAKE_CURRENT_LIST_DIR}/GATT_Service/SRAD_RT_db.gatt")


pico_add_extra_outputs(SRAD_RT)