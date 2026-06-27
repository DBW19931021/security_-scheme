set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR rv32imac)

set(CMAKE_C_COMPILER "riscv32-wing-elf-gcc")
set(CMAKE_CXX_COMPILER "riscv32-wing-elf-g++")
set(CMAKE_ASM_COMPILER "riscv32-wing-elf-gcc")
set(CMAKE_LINKER "riscv32-wing-elf-gcc")
set(CMAKE_OBJCOPY "riscv32-wing-elf-objcopy")
set(CMAKE_OBJDUMP "riscv32-wing-elf-objdump")
set(CMAKE_STRIP "riscv32-wing-elf-strip")
set(CMAKE_SIZE "riscv32-wing-elf-size")

set(CMAKE_C_FLAGS
    "-march=rv32im_zca_zcb_zcmp_zcmt -mtune=size -mabi=ilp32 -mcmodel=medany -mno-save-restore"
)
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} ${ASM_OPTIONS}")

set(CMAKE_EXE_LINKER_FLAGS
    "-nostartfiles -nostdlib -Xlinker --gc-sections --specs=nosys.specs")

set(LINK_MAP_FLAG "-Wl,-Map,")

set(LINK_SCRIPT_FLAG "-T")
set(LINK_FILE_SUFFIX ".ld")

set(OPT_LVL_NONE "-O0")
set(OPT_LVL_SPEED "-O2")
set(OPT_LVL_SIZE "-Os")

set(CPU_TYPE "m130")
