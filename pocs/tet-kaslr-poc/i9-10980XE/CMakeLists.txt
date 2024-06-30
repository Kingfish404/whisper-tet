cmake_minimum_required(VERSION 3.10)

project(Downfall LANGUAGES C CXX ASM)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(CMAKE_C_FLAGS "-O0")

add_executable(tet_cc_kaslr ${SRC_DIR}/tet_cc_kaslr.c)
add_executable(tet_cc_kaslr_test ${SRC_DIR}/tet_cc_kaslr_test.c)
add_executable(tet_c_kaslr_attack ${SRC_DIR}/tet_c_kaslr_attack.c)
