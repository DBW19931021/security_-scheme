cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)

# generate clangd compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# do not compile test program
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(NAME "ehsm_bl")

# ----- user input start ------- #
set(TOOLCHAIN
    "rv32-wing-gcc"
    CACHE STRING "toolchain cmake name in dir toolcain"
)
option(DEBUG
       "debug mode, will disable compiler optimize and enable LOG_DBG level"
       OFF
)

set(FID_LEVEL
    "2"
    CACHE STRING "definition of fid level,2:high 1:low or 0:none"
)
# ----- user input end ------- #

include(${CMAKE_SOURCE_DIR}/toolchain/${TOOLCHAIN}.cmake)
set(CRYPTOLIB osrcrypto)

if(NOT DEFINED CPU_TYPE)
  message(FATAL_ERROR "CPU_TYPE is not defined")
else()
  message(STATUS "CPU_TYPE: ${CPU_TYPE}")
endif()
message(STATUS "TOOLCHAIN: ${TOOLCHAIN}")

set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/src/driver/cpu/${CPU_TYPE})

find_package(Python3 COMPONENTS Interpreter)
if(Python3_Interpreter_FOUND)
  message(STATUS "Python 3 found: ${Python3_EXECUTABLE}")
else()
  message(FATAL_ERROR "Python 3 not found, please install it")
endif()

project(${NAME})

enable_language(ASM)

include(${CMAKE_SOURCE_DIR}/src/driver/cpu/${CPU_TYPE}/cpu-build.cmake)
include(${CMAKE_SOURCE_DIR}/fid_lib/fid.cmake)
include_directories(
  ${FID_INC_DIR}
)

if(DEBUG)
  add_compile_definitions(LOG_LEVEL=LOG_DBG)
  set(OPT_LVL ${OPT_LVL_NONE})
  message("enable debug mode")
else()

  set(OPT_LVL ${OPT_LVL_SIZE})

endif()

add_compile_options(
  -fdiagnostics-color=always
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
  config.h
)

add_compile_definitions(BUILD_EHSM_BL=1)

set(ELFNAME "${NAME}.elf")
set(DISNAME "${NAME}.dis")
set(MAPNAME "${NAME}.map")
set(BINNAME "${NAME}.bin")
set(HEXNAME "${NAME}.vhex")
set(SHANAME "${NAME}.sha256.txt")

  include(${CMAKE_SOURCE_DIR}/custom.cmake)
  set(CONFIG_DIR "${CMAKE_SOURCE_DIR}/inc")

# build dest bin file
add_executable(${ELFNAME} ${SRC_FILES} ${CPU_SRC_FILES} ${FID_SRC_FILES})

# extra compiler warning and error check
target_compile_options(
  ${ELFNAME}
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
          # stack usage of a function must <= 4K
          -Wstack-usage=4096
          -Wmaybe-uninitialized
          -Werror=strict-prototypes
          -Werror=return-type
          -Werror=implicit-function-declaration
          -Werror=implicit-int
          -Werror=uninitialized
)

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/link/link${LINK_FILE_SUFFIX})
# link flags
target_link_options(
  ${ELFNAME} PRIVATE ${LINK_MAP_FLAG}${MAPNAME} ${LINK_SCRIPT_FLAG}
  ${LINKER_SCRIPT}
)
add_custom_target(link_dependency ALL DEPENDS ${LINKER_SCRIPT})
add_dependencies(${ELFNAME} link_dependency)
set_target_properties(${ELFNAME} PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
set_property(
  TARGET ${ELFNAME}
  APPEND
  PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_BINARY_DIR}/${MAPNAME}
)

if(EXISTS ${CMAKE_SOURCE_DIR}/libs/lib${CRYPTOLIB}.a)
  target_link_directories(${ELFNAME} PRIVATE ${CMAKE_SOURCE_DIR}/libs)
endif()
target_link_libraries(${ELFNAME} PRIVATE ${CRYPTOLIB})

add_custom_command(
  OUTPUT ${DISNAME}
  COMMAND ${CMAKE_OBJDUMP} -S ${CMAKE_BINARY_DIR}/${ELFNAME} >
          ${CMAKE_BINARY_DIR}/${DISNAME}
  DEPENDS ${ELFNAME}
)

add_custom_command(
  OUTPUT ${BINNAME}
  COMMAND ${CMAKE_OBJCOPY} -O binary -S ${CMAKE_BINARY_DIR}/${ELFNAME}
          ${CMAKE_BINARY_DIR}/${BINNAME}
  COMMAND ${CMAKE_SIZE} ${CMAKE_BINARY_DIR}/${ELFNAME}
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/file_size.py
          ${CMAKE_BINARY_DIR}/${BINNAME}
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_bin_version.py
          -V ${CMAKE_SOURCE_DIR}/inc/version.h -b ${CMAKE_BINARY_DIR}/${BINNAME}
  COMMAND
    ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_stack_size.py
    --build-dir ${CMAKE_BINARY_DIR} --linker-script ${LINKER_SCRIPT}
    --isr-overhead 256
  DEPENDS ${ELFNAME}
)

if(OSCCA_ENABLE)
  set(SIGELFNAME "${NAME}_with_sig.elf")
  set(SIGBINNAME "${NAME}_with_sig.bin")
  set(SIGHEXNAME "${NAME}_with_sig.vhex")
  set(APINAME "${NAME}_api.h")

  if(DEBUG)
    set(SIGN_BIN_ARG --rom-size 160)
  else()
    set(SIGN_BIN_ARG)
  endif()

  add_custom_command(
    OUTPUT ${SIGELFNAME} ${SIGBINNAME}
    COMMAND
      ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/sign_bin.py
      ${SIGN_BIN_ARG} --config-header ${CONFIG_DIR}/config.h --output
      ${CMAKE_BINARY_DIR}/sig_data.bin ${CMAKE_BINARY_DIR}/${BINNAME}
      --out-addr-file ${CMAKE_BINARY_DIR}/address.txt
    COMMAND
      ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/gen_sign_elf.py
      --address-file ${CMAKE_BINARY_DIR}/address.txt --sig-file
      ${CMAKE_BINARY_DIR}/sig_data.bin --output
      ${CMAKE_BINARY_DIR}/${SIGELFNAME} ${CMAKE_BINARY_DIR}/${ELFNAME}
      --objcopy-name ${CMAKE_OBJCOPY}
    COMMAND ${CMAKE_OBJCOPY} -O binary -S ${CMAKE_BINARY_DIR}/${SIGELFNAME}
            ${CMAKE_BINARY_DIR}/${SIGBINNAME}
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/sig_data.bin
            ${CMAKE_BINARY_DIR}/address.txt
    DEPENDS ${BINNAME} ${ELFNAME}
  )

  add_custom_command(
    OUTPUT ${SIGHEXNAME}
    COMMAND ${CMAKE_OBJCOPY} -O verilog ${CMAKE_BINARY_DIR}/${SIGELFNAME}
            ${CMAKE_BINARY_DIR}/${SIGHEXNAME}
    DEPENDS ${SIGELFNAME}
  )

  add_custom_command(
    OUTPUT ${APINAME}
    COMMAND
      ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/gen_bl_api.py --map
      ${CMAKE_BINARY_DIR}/${MAPNAME} --output ${CMAKE_BINARY_DIR}/${APINAME} -c
      ${CONFIG_DIR}/config.h
    DEPENDS ${ELFNAME}
  )

  add_custom_command(
    OUTPUT ${SHANAME}
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} sha256sum
            ${SIGBINNAME} ${SIGHEXNAME} >${SHANAME}
    DEPENDS ${SIGBINNAME} ${SIGHEXNAME}
  )

  add_custom_target(
    ${NAME} ALL
    DEPENDS ${ELFNAME}
            ${BINNAME}
            ${DISNAME}
            ${APINAME}
            ${SIGELFNAME}
            ${SIGBINNAME}
            ${SIGHEXNAME}
            ${SHANAME}
  )

else()
  add_custom_command(
    OUTPUT ${HEXNAME}
    COMMAND ${CMAKE_OBJCOPY} -O verilog ${CMAKE_BINARY_DIR}/${ELFNAME}
            ${CMAKE_BINARY_DIR}/${HEXNAME}
    DEPENDS ${ELFNAME}
  )

  add_custom_command(
    OUTPUT ${SHANAME}
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR} sha256sum ${BINNAME}
            ${HEXNAME} >${SHANAME}
    DEPENDS ${BINNAME} ${HEXNAME}
  )

  add_custom_target(
    ${NAME} ALL DEPENDS ${ELFNAME} ${BINNAME} ${DISNAME} ${HEXNAME} ${SHANAME}
  )

endif()
