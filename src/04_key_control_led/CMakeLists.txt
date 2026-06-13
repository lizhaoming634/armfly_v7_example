cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(04_key_control_led)

target_sources(app PRIVATE 
    src/main.c
    src/key_thread.c
    src/led_thread.c
)

target_include_directories(app PRIVATE 
    include
)