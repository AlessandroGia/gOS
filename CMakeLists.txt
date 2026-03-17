cmake_minimum_required(VERSION 3.20)
project(gOS C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(bootloader)
add_subdirectory(kernel)
