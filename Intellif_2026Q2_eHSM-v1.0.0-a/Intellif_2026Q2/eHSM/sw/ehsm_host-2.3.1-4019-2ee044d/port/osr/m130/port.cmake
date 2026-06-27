# 引入交叉编译工具链配置文件
include(${CMAKE_CURRENT_LIST_DIR}/rv32-wing-gcc.cmake)

# 配置重定向的 C 库函数文件，主要是实现 UART 对 _write 的重写，实现串口打印，对 _sbrk 的重写实现堆内存分配，以及对 C 库函数的重写，防止编译告警
set(EHSM_RETARGET_SRC_FILE
    ${CMAKE_CURRENT_LIST_DIR}/port/sys_call.c
    CACHE STRING "Set retarget functions for libc"
)

# 设置平台相关的源文件
set(EHSM_PORT_SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/driver/src/clic.c
    ${CMAKE_CURRENT_LIST_DIR}/port/port_m130.c
    ${CMAKE_CURRENT_LIST_DIR}/driver/src/uart_stdout.c
    ${CMAKE_CURRENT_LIST_DIR}/port/ehsm_host_port.c
    ${EHSM_RETARGET_SRC_FILE}
    ${CMAKE_CURRENT_LIST_DIR}/wmsis/core/src/crt0.S
)

# 设置平台相关的头文件目录
set(EHSM_PORT_INC_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/driver/inc
    ${CMAKE_CURRENT_LIST_DIR}/wmsis/core/inc ${CMAKE_CURRENT_LIST_DIR}/port
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# 设置链接选项
set(EHSM_PORT_LINK_FLAGS "-T${CMAKE_CURRENT_LIST_DIR}/link.ld")
