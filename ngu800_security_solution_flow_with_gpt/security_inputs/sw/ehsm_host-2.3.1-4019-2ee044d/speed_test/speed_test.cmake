# 配置 speed test 使用的源文件
file(GLOB_RECURSE SPEED_TEST_SRC_FILES
    ${CMAKE_CURRENT_LIST_DIR}/*.c
)

# 配置 speed test 使用的头文件
set(SPEED_TEST_INC_DIRS
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../include/ehsmdrv/basic
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

# 编译 speed test 为可执行文件
add_executable(ehsm_speed_test ${SPEED_TEST_SRC_FILES} ${EHSM_RETARGET_SRC_FILE})

if(OTP_DEFAULT1)
    target_compile_definitions(ehsm_speed_test PRIVATE EHSM_PORT_OTP_DEFAULT_VALUE=1)
else()
    target_compile_definitions(ehsm_speed_test PRIVATE EHSM_PORT_OTP_DEFAULT_VALUE=0)
endif()

# 配置 OTP 默认值,可通过 cmake -DOTP_DEFAULT1=0/1 指定
# 0: OTP 填充全 0, 1: OTP 填充全 1 (默认值)
if(OTP_DEFAULT1)
    # 通过编译选项传入 EHSM_PORT_OTP_DEFAULT_VALUE
    target_compile_definitions(ehsm_speed_test PRIVATE EHSM_PORT_OTP_DEFAULT_VALUE=1)
else()
    # 通过编译选项传入 EHSM_PORT_OTP_DEFAULT_VALUE
    target_compile_definitions(ehsm_speed_test PRIVATE EHSM_PORT_OTP_DEFAULT_VALUE=0)
endif()

target_include_directories(ehsm_speed_test PRIVATE ${SPEED_TEST_INC_DIRS})


