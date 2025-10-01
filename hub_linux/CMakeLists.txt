cmake_minimum_required(VERSION 3.16)
project(hub_linux C)

set(CMAKE_C_STANDARD 23)

# --- Исходники ---
add_executable(hub_linux 
    src/main.c
    src/common.c
    src/device_io.c
)

# --- Директории с заголовками ---
target_include_directories(hub_linux PRIVATE include)
target_include_directories(hub_linux PRIVATE /usr/include) # обычно mosquitto.h лежит здесь

# --- Линки библиотек ---
target_link_libraries(hub_linux 
    mosquitto
    pthread
)
