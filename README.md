<!--
SPDX-FileCopyrightText: 2025 Aaron White <w531t4@gmail.com>
SPDX-License-Identifier: MIT
-->
# ESPHome-core_load
Produce per-core cpu load metrics

# Getting Started
1. Must enable both CONFIG_FREERTOS_USE_TRACE_FACILITY and CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS (see example below)
    ```
    esp32:
        board: esp32dev
        framework:
            type: esp-idf
            sdkconfig_options:
                # <Used for producing CPU-core performance metrics>
                CONFIG_FREERTOS_USE_TRACE_FACILITY: y
                CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS: y
                # </Used for producing CPU-core performance metrics>
    ```
1. Add sensor to sensors:
    ````
    sensor:
    - platform: core_load
        id: cores
        update_interval: 10s
        core0:
        name: "ESP32 Core0 Load"
        core1:
        name: "ESP32 Core1 Load"
    ````
1. Change update_interval to your liking
