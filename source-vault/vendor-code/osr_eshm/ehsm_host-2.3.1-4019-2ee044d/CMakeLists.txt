cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 99)

# generate clangd compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# do not compile test program
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

set(PORT
    "osr/m130"
    CACHE STRING
          "the port dir in port/, there should be a port.cmake file inside"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/port/${PORT}/port.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/port/${PORT}/port.cmake)
else()
    message(
    FATAL_ERROR
      "port.cmake not found in ${CMAKE_CURRENT_SOURCE_DIR}/port/${PORT}"
  )
endif()

include_directories(include ${EHSM_PORT_INC_DIRS})
include_directories(include ./src)

add_compile_options(
  -Wall
  -Wextra
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
  -Werror=uninitialized
  -fno-builtin
  -ffunction-sections
  -fdata-sections
  -O2
  -g
  -Wformat=0
)

project(ehsm_host)

enable_language(ASM)

add_library(ehsm_host STATIC src/mailbox.c src/api.c ${EHSM_PORT_SRC_FILES})

# 是否编译 Host demo 源代码
if(BUILD_HOST_DEMO)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/demo/demo.cmake)
        # 根据 demo.cmake 编译 Host demo
        message(STATUS "BUILD_HOST_DEMO = ${BUILD_HOST_DEMO}")
        include(${CMAKE_CURRENT_SOURCE_DIR}/demo/demo.cmake)
        target_link_libraries(ehsm_host_demo ehsm_host)

        if(EHSM_PORT_LINK_FLAGS)
            target_link_options(
        ehsm_host_demo PRIVATE -Wl,-Map=ehsm_host.map ${EHSM_PORT_LINK_FLAGS}
      )
        else()
            message(
        FATAL_ERROR "demo.cmake not found in ${CMAKE_CURRENT_SOURCE_DIR}/demo"
      )
        endif()
    else()
        message(
      STATUS
        "Need to add customer's own source codes to compiler, otherwise it will just use Host demo main function to implement the compilation witout compiling other Host demo source codes!"
    )
    endif()

endif()


# 是否编译性能测试 demo 源代码
if(BUILD_SPEED_TEST)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/speed_test/speed_test.cmake)
        # 根据 speed_test.cmake 编译性能测试 demo
        message(STATUS "BUILD_SPEED_TEST = ${BUILD_SPEED_TEST}")
        include(${CMAKE_CURRENT_SOURCE_DIR}/speed_test/speed_test.cmake)
        target_link_libraries(ehsm_speed_test ehsm_host)
        if(EHSM_PORT_LINK_FLAGS)
            target_link_options(
        ehsm_speed_test PRIVATE -Wl,-Map=ehsm_speed_host.map ${EHSM_PORT_LINK_FLAGS}
      )
        else()
            message(
        FATAL_ERROR "demo.cmake not found in ${CMAKE_CURRENT_SOURCE_DIR}/speed_test"
      )
        endif()
    else()
        message(
      STATUS
        "Need to add customer's own source codes to compiler, otherwise it will just use Host demo main function to implement the compilation witout compiling other Host demo source codes!"
    )
    endif()

endif()
