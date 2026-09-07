cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 99)

# generate clangd compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# do not compile test program
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

set(PORT
    "port/m130"
    CACHE STRING
          "the port dir in port/, there should be a port.cmake file inside"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${PORT}/port.cmake)
  include(${CMAKE_CURRENT_SOURCE_DIR}/${PORT}/port.cmake)
else()
  message(
    FATAL_ERROR
      "port.cmake not found in ${CMAKE_CURRENT_SOURCE_DIR}/${PORT}"
  )
endif()

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo.cmake)
#   include(${CMAKE_CURRENT_SOURCE_DIR}/demo/demo.cmake)
# else()
#   message(
#     FATAL_ERROR
#       "demo.cmake not found in ${CMAKE_CURRENT_SOURCE_DIR}/demo"
#   )
# endif()

include_directories(include ${EHSM_PORT_INC_DIRS} ../stp_pro/c)

# 基础编译选项
add_compile_options(
  -Wall
  -Wextra
  -Wshadow
  -Wnull-dereference
  -Wundef
  -Wconversion
  -Wno-sign-compare
  -Wno-cast-qual
  -Wno-cast-align
  -Wimplicit-fallthrough=4
  -Wswitch-default
  -Wno-maybe-uninitialized
  -Werror=strict-prototypes
  -Werror=return-type
  -Werror=implicit-function-declaration
  -Werror=implicit-int
  -Werror=uninitialized
  -fno-builtin
  -ffunction-sections
  -fdata-sections
  -O2
  -g
  -Wformat=0
  -nostdlib
)

# 对系统头文件禁用转换警告
include_directories(SYSTEM ${EHSM_PORT_INC_DIRS})

set(DEMO_SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/cmd_table.c
    ${CMAKE_CURRENT_LIST_DIR}/basic_commands.c
    ${CMAKE_CURRENT_LIST_DIR}/hostapi_commands.c
    ${CMAKE_CURRENT_LIST_DIR}/server_main.c
    # STP协议实现
    ${CMAKE_CURRENT_LIST_DIR}/../stp_pro/c/uart_pro.c
)

project(ehsm_host)

enable_language(ASM)

add_library(ehsm_host STATIC src/mailbox.c src/api.c src/test_api.c ${EHSM_PORT_SRC_FILES})

add_executable(ehsm_host_demo ${DEMO_SRC_FILES} ${EHSM_RETARGET_SRC_FILE})

target_include_directories(ehsm_host_demo PRIVATE ${DEMO_INC_DIRS})

target_link_libraries(ehsm_host_demo ${DEMO_STATIC_CRYPTO_LIB})

target_link_libraries(ehsm_host_demo ehsm_host)

if(EHSM_PORT_LINK_FLAGS)
    target_link_options(ehsm_host_demo PRIVATE -Wl,--gc-sections,-Map=ehsm_host_demo.map ${EHSM_PORT_LINK_FLAGS})
endif()

# 添加自定义命令生成hex文件
add_custom_command(TARGET ehsm_host_demo POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:ehsm_host_demo> ehsm_host_demo.hex
    COMMENT "生成Intel HEX文件: ehsm_host_demo.hex"
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:ehsm_host_demo> ${CMAKE_CURRENT_BINARY_DIR}/ehsm_host_demo.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:ehsm_host_demo>
    COMMENT "Generate HEX and BIN files, show the useage of memory"
    VERBATIM
)

# 显示大小信息
add_custom_target(size
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:ehsm_host_demo>
    DEPENDS ehsm_host_demo
    COMMENT "show the useage of memory"
)

# 设置输出文件名包含.elf扩展名
set_target_properties(ehsm_host_demo PROPERTIES OUTPUT_NAME "ehsm_host_demo.elf")