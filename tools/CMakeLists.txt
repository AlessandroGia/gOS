add_executable(mkgc
    ${CMAKE_CURRENT_SOURCE_DIR}/mkgc/mkgc.c
)

target_include_directories(mkgc PRIVATE
    ${CMAKE_SOURCE_DIR}
)

set_target_properties(mkgc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
)
