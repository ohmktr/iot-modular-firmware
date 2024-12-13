# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dev_space)

# เพิ่ม source files ที่ต้องการเข้าไปใน target
target_sources(app PRIVATE 
    src/main/main.c         # Main application source
    src/components/buzzer.c # Buzzer source file
    src/components/led.c    # LED source file
    src/components/led1.c
    src/components/uart_com.c
    src/application/protocol_check.c
)

# เพิ่ม include directory ของ components
zephyr_include_directories(src/components)
zephyr_include_directories(src/application)

# SPDX-License-Identifier: Apache-2.0