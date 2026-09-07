# 配置 Host demo 使用的源文件
file(GLOB_RECURSE DEMO_SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/*.c
)

list(REMOVE_ITEM DEMO_SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/common/image_data/otp_keyid_map.c
    ${CMAKE_CURRENT_LIST_DIR}/common/tools/common_files/otp_keyid_map.c
)

if(OTP_DEFAULT1)
    list(REMOVE_ITEM DEMO_SRC_FILES
        ${CMAKE_CURRENT_LIST_DIR}/common/otp_data/default0/ehsm_demo_otp_data.c
    )
else()
    list(REMOVE_ITEM DEMO_SRC_FILES
        ${CMAKE_CURRENT_LIST_DIR}/common/otp_data/default1/ehsm_demo_otp_data.c
    )
endif()

# 配置 Host demo 使用的头文件
set(DEMO_INC_DIRS
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../include/ehsmdrv/basic
)

# 配置 Host demo 使用的 crypto lib 源文件
if("${CMAKE_C_COMPILER}" MATCHES "arm")
    set(DEMO_STATIC_CRYPTO_LIB ${CMAKE_CURRENT_LIST_DIR}/common/crypto/lib_arm_host_crypto.a)
elseif("${CMAKE_C_COMPILER}" MATCHES "wing")
    set(DEMO_STATIC_CRYPTO_LIB ${CMAKE_CURRENT_LIST_DIR}/common/crypto/lib_wing_host_crypto.a)
elseif("${CMAKE_C_COMPILER}" MATCHES "xuantie")
    set(DEMO_STATIC_CRYPTO_LIB ${CMAKE_CURRENT_LIST_DIR}/common/crypto/lib_axuantie_host_crypto.a)
else()
    message(
        FATAL ERROR
        "Host demo has no static crypto lib for ${CMAKE_C_COMPILER}"
    )
endif()

# 编译 Host demo 为可执行文件
add_executable(ehsm_host_demo ${DEMO_SRC_FILES} ${EHSM_RETARGET_SRC_FILE})

if(OTP_DEFAULT1)
    target_compile_definitions(ehsm_host_demo PRIVATE EHSM_PORT_OTP_DEFAULT_VALUE=1)
else()
    target_compile_definitions(ehsm_host_demo PRIVATE EHSM_PORT_OTP_DEFAULT_VALUE=0)
endif()

target_include_directories(ehsm_host_demo PRIVATE ${DEMO_INC_DIRS})

target_link_libraries(ehsm_host_demo ${DEMO_STATIC_CRYPTO_LIB})

# 定义宏，使得编译 Host demo 源代码，否则不会编译 Host demo 源代码
target_compile_definitions(ehsm_host_demo PRIVATE
    CONFIG_BUILD_HOST_DEMO
)

if(BUILD_HOST_BL_PATCH_TEST)
    target_compile_definitions(ehsm_host_demo PRIVATE
        CONFIG_HOST_BL_PATCH_TEST_ENABLE
    )
endif()

if(DEFINED DEMO_BL_PATCH_TEST_EXPECTED_MARKER)
    target_compile_definitions(ehsm_host_demo PRIVATE
        DEMO_BL_PATCH_TEST_EXPECTED_MARKER=${DEMO_BL_PATCH_TEST_EXPECTED_MARKER}
    )
endif()
