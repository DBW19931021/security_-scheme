set(CPU_SEC_PATH ${CMAKE_SOURCE_DIR}/src/driver/cpu/m130)

include_directories(${CPU_SEC_PATH}/driver/inc)
include_directories(${CPU_SEC_PATH}/wmsis/core/inc)
include_directories(${CPU_SEC_PATH}/port)

set(CPU_CRT_FILE ${CPU_SEC_PATH}/wmsis/core/src/crt0.S)

set(CPU_SRC_FILES
    ${CPU_SEC_PATH}/driver/src/clic.c ${CPU_SEC_PATH}/port/port_m130.c
    ${CPU_SEC_PATH}/driver/src/pmp.c ${CPU_CRT_FILE})
