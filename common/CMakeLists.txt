add_subdirectory(cjson)

add_library(smarthub_common INTERFACE)

target_include_directories(smarthub_common INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(smarthub_common INTERFACE cjson)