cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_STANDARD 99)

project(test)

set(FID_LEVEL
  2
  CACHE INTEGER "definition of fid level,2:high 1:low or 0:none")

set(ELFNAME test)
include(${CMAKE_SOURCE_DIR}/../fid.cmake)
include_directories(${FID_INC_DIR})

set(SRC_FILES ${CMAKE_SOURCE_DIR}/test.c)

add_custom_command(
  OUTPUT test.dis
  COMMAND objcopy -S ${CMAKE_BINARY_DIR}/test.exe >
          ${CMAKE_BINARY_DIR}/test.dis
  DEPENDS ${ELFNAME})

# build dest bin file
add_executable(${ELFNAME} ${SRC_FILES} ${FID_SRC_FILES})

add_custom_target(test_dest ALL DEPENDS ${ELFNAME} test.dis)