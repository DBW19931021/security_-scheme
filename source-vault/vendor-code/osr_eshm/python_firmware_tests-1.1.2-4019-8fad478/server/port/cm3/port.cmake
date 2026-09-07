include(${CMAKE_CURRENT_LIST_DIR}/arm-none-eabi-gcc.cmake)

# 端口源文件配置
set(EHSM_PORT_SRC_FILES
    # 系统初始化文件
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/uart_hal.c
    ${CMAKE_CURRENT_LIST_DIR}/System/cmsis/Device/ARM/CM3DS/Source/system_CM3DS.c
    ${CMAKE_CURRENT_LIST_DIR}/System/cmsis/Device/ARM/CM3DS/Source/ARM/startup_CM3DS_gcc.s
    ${CMAKE_CURRENT_LIST_DIR}/port/ehsm_host_port.c
    # 标准输出重定向
    ${CMAKE_CURRENT_LIST_DIR}/System/retarget/retarget_gcc.c
    # UART驱动
    ${CMAKE_CURRENT_LIST_DIR}/Uart/src/uart_stdout.c
)

# 配置重定向的 C 库函数文件，主要是实现 UART 对 _write 的重写，实现串口打印，对 _sbrk 的重写实现堆内存分配，以及对 C 库函数的重写，防止编译告警
set(EHSM_RETARGET_SRC_FILE
    ${CMAKE_CURRENT_LIST_DIR}/System/retarget/retarget_gcc.c
    CACHE STRING "Set retarget functions for libc"
)

# 端口包含目录
set(EHSM_PORT_INC_DIRS
    # 本地头文件
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_CURRENT_LIST_DIR}/port
    # CMSIS核心头文件
    ${CMAKE_CURRENT_LIST_DIR}/System/cmsis/CMSIS/Include
    # 设备特定头文件
    ${CMAKE_CURRENT_LIST_DIR}/System/cmsis/Device/ARM/CM3DS/Include
    # UART驱动头文件
    ${CMAKE_CURRENT_LIST_DIR}/Uart/include
    # STP协议头文件
    ${CMAKE_CURRENT_LIST_DIR}/../../stp_pro/c
)

# 链接器配置
set(LINKER_SCRIPT_PATH ${CMAKE_CURRENT_LIST_DIR}/System/linker/ehsm_host.ld)
if(NOT EXISTS ${LINKER_SCRIPT_PATH})
    message(FATAL_ERROR "链接脚本不存在: ${LINKER_SCRIPT_PATH}")
endif()
set(EHSM_PORT_LINK_FLAGS "-T${LINKER_SCRIPT_PATH}")

# 打印端口配置信息
message(STATUS "已配置CM3端口: 包含目录=${EHSM_PORT_INC_DIRS}")
