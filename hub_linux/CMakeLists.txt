cmake_minimum_required(VERSION 3.16)
project(hub_linux C)

set(CMAKE_C_STANDARD 23)

add_subdirectory(../common ${CMAKE_CURRENT_BINARY_DIR}/common)

add_executable(hub_linux 
    src/main.c
    src/common.c
    src/arduino_serial.c
    src/hub_commands.c
    src/esp32_mqtt.c
)

target_include_directories(hub_linux PRIVATE include)
target_include_directories(hub_linux PRIVATE /usr/include)

target_link_libraries(hub_linux
    PRIVATE
    smarthub_common
    mosquitto
    pthread
)
