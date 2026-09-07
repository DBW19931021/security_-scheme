set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "arm-none-eabi-gcc")
set(CMAKE_LINKER "arm-none-eabi-gcc")
set(CMAKE_OBJCOPY "arm-none-eabi-objcopy")
set(CMAKE_OBJDUMP "arm-none-eabi-objdump")
set(CMAKE_STRIP "arm-none-eabi-strip")
set(CMAKE_SIZE "arm-none-eabi-size")

set(CMAKE_C_FLAGS "-mcpu=cortex-m3 -mthumb")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} ${ASM_OPTIONS}")

set(CMAKE_EXE_LINKER_FLAGS
    "-nostartfiles -Xlinker --gc-sections --specs=nosys.specs"
)

set(LINK_SCRIPT_FLAG "-T")
set(LINK_MAP_FLAG "-Wl,-Map,")
set(LINK_FILE_SUFFIX ".ld")

option(TOOLCHAIN_IAR "The compiler is IAR" OFF)

message(STATUS "set toolchain: arm-none-eabi-gcc")
