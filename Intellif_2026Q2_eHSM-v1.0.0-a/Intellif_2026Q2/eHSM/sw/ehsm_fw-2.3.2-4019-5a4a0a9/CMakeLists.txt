cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)

# generate clangd compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# do not compile test program
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(NAME "ehsm_fw")

# ----- user input start ------- #
set(CODE_AREA
    "iram"
    CACHE STRING "where to put code, irom, iram or nvm")
set(TOOLCHAIN
    "rv32-wing-gcc"
    CACHE STRING "toolchain cmake name in dir toolcain")
option(DEBUG
       "debug mode, will disable compiler optimize and enable LOG_DBG level"
       OFF)

set(FID_LEVEL
    "2"
    CACHE STRING "definition of fid level,2:high 1:low or 0:none")

# ----- user input end ------- #

include(${CMAKE_SOURCE_DIR}/toolchain/${TOOLCHAIN}.cmake)
if(TOOLCHAIN_IAR)
  set(CRYPTOLIB osrcrypto_iar)
else()
  set(CRYPTOLIB osrcrypto)
endif()

if(NOT DEFINED CPU_TYPE)
  message(FATAL_ERROR "CPU_TYPE is not defined")
else()
  message(STATUS "CPU_TYPE: ${CPU_TYPE}")
endif()
message(STATUS "TOOLCHAIN: ${TOOLCHAIN}")

set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/src/driver/cpu/${CPU_TYPE})

set(LINKCONF ${CODE_AREA})

find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
  message(STATUS "Python 3 found: ${Python3_EXECUTABLE}")
else()
  message(FATAL_ERROR "Python 3 not found, please install it")
endif()

project(ehsm_fw)

enable_language(ASM)

# debug settings
if(DEBUG)
  add_compile_definitions(LOG_LEVEL=LOG_DBG)
  set(OPT_LVL ${OPT_LVL_NONE})
  message("enable debug mode")
else()
  set(OPT_LVL ${OPT_LVL_SPEED})
endif()

# compiler flags
if(TOOLCHAIN_IAR)
  # IAR: compile options will also add to ASM flags where iasmriscv report
  # error, so we need to add C compiler flags which iasmriscv not supported to
  # CMAKE_C_FLAGS in rv32-iar.cmake
  set(CMAKE_C_FLAGS
      "${CMAKE_C_FLAGS} -e ${OPT_LVL} --diag-suppress=Pa082 --set_default_interrupt_alignment 64 --preinclude config.h"
  )
  set(EXENAME "${NAME}")

else()
  add_compile_options(
    -Wall
    ${OPT_LVL}
    -ffunction-sections
    -fdata-sections
    -fno-common
    -fno-inline-functions
    -ffreestanding
    -g3
    -fstack-usage
    -fdump-ipa-cgraph
    # force to include config.h
    -include
    config.h)

  set(EXENAME "${NAME}.elf")
endif()

set(ELFNAME "${NAME}.elf")
set(DISNAME "${NAME}.dis")
set(MAPNAME "${NAME}.map")
set(BINNAME "${NAME}.bin")
set(HEXNAME "${NAME}.vhex")
set(SHANAME "${NAME}.sha256.txt")

include(${VENDOR_DIR}/cpu-build.cmake)
include(${CMAKE_SOURCE_DIR}/fid_lib/fid.cmake)
include_directories(${FID_INC_DIR})

  include(${CMAKE_SOURCE_DIR}/custom.cmake)

# build dest bin file
add_executable(${EXENAME} ${SRC_FILES} ${CPU_SRC_FILES} ${FID_SRC_FILES})
if(TOOLCHAIN_IAR)
  target_compile_options(${EXENAME} PRIVATE --debug)
else()
  # extra compiler warning and error check
  target_compile_options(
    ${EXENAME}
    PRIVATE -Wextra
            -Wshadow
            -Wnull-dereference
            -Wundef
            -Wconversion
            -Wsign-compare
            -Wcast-qual
            -Wcast-align
            -Wimplicit-fallthrough=4
            -Wswitch-default
            -Wmaybe-uninitialized
            -Werror=strict-prototypes
            -Werror=return-type
            -Werror=implicit-function-declaration
            -Werror=implicit-int
            -Werror=uninitialized)
endif()

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/link/${LINKCONF}${LINK_FILE_SUFFIX})
# link flags
target_link_options(${EXENAME} PRIVATE ${LINK_MAP_FLAG}${MAPNAME}
                    ${LINK_SCRIPT_FLAG} ${LINKER_SCRIPT})
add_custom_target(link_dependency ALL DEPENDS ${LINKER_SCRIPT})
add_dependencies(${EXENAME} link_dependency)
set_target_properties(${EXENAME} PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
set_property(
  TARGET ${EXENAME}
  APPEND
  PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_BINARY_DIR}/${MAPNAME})

if(EXISTS ${CMAKE_SOURCE_DIR}/libs/lib${CRYPTOLIB}.a)
  target_link_directories(${EXENAME} PRIVATE ${CMAKE_SOURCE_DIR}/libs)
endif()

target_link_libraries(${EXENAME} PRIVATE ${CRYPTOLIB})
if(NOT TOOLCHAIN_IAR)
  target_link_libraries(${EXENAME} PRIVATE c)
endif()

add_custom_command(
  OUTPUT ${DISNAME}
  COMMAND ${CMAKE_OBJDUMP} ${CMAKE_DIS_FLAGS} ${CMAKE_BINARY_DIR}/${ELFNAME} >
          ${CMAKE_BINARY_DIR}/${DISNAME}
  DEPENDS ${ELFNAME})

if(NOT TOOLCHAIN_IAR)
  add_custom_command(
    OUTPUT ${BINNAME}
    COMMAND ${CMAKE_OBJCOPY} ${CMAKE_BIN_FLAGS} ${CMAKE_BINARY_DIR}/${ELFNAME}
            ${CMAKE_BINARY_DIR}/${BINNAME}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/file_size.py
            ${CMAKE_BINARY_DIR}/${BINNAME}
    COMMAND
      ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_bin_version.py -V
      ${CMAKE_SOURCE_DIR}/inc/version.h -b ${CMAKE_BINARY_DIR}/${BINNAME}
    COMMAND
      ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_stack_size.py
      --build-dir ${CMAKE_BINARY_DIR} --linker-script ${LINKER_SCRIPT}
      --isr-overhead 256
    DEPENDS ${ELFNAME})
else()
  add_custom_command(
    OUTPUT ${BINNAME}
    COMMAND ${CMAKE_OBJCOPY} ${CMAKE_BIN_FLAGS} ${CMAKE_BINARY_DIR}/${ELFNAME}
            ${CMAKE_BINARY_DIR}/${BINNAME}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/file_size.py
            ${CMAKE_BINARY_DIR}/${BINNAME}
    COMMAND
      ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_bin_version.py -V
      ${CMAKE_SOURCE_DIR}/inc/version.h -b ${CMAKE_BINARY_DIR}/${BINNAME}
    DEPENDS ${ELFNAME})
endif()

add_custom_command(
  OUTPUT ${HEXNAME}
  COMMAND ${CMAKE_HEX_CMD} ${CMAKE_HEX_FLAGS} ${CMAKE_BINARY_DIR}/${ELFNAME}
          ${CMAKE_BINARY_DIR}/${HEXNAME}
  DEPENDS ${ELFNAME})

add_custom_command(
  OUTPUT ${SHANAME}
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} sha256sum ${BINNAME}
          ${HEXNAME} >${SHANAME}
  DEPENDS ${BINNAME} ${HEXNAME})

add_custom_target(ehsm_dest ALL DEPENDS ${EXENAME} ${BINNAME} ${DISNAME}
                                        ${HEXNAME} ${SHANAME})
