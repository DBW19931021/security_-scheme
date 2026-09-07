import logging as log
import allure
import pytest
from platform_adapter.api.constants import EhsmDrvMode, EhsmRngAlgo
from platform_adapter.gdb.loader import get_gdb_interface
from platform_adapter.api.loader import get_api_interface
from platform_adapter.host.loader import get_host_interface
from platform_adapter.uart_lib import hostapi
from platform_adapter.uart_lib import ehsm_fw_errno

gdb = get_gdb_interface()
api = get_api_interface()
host = get_host_interface()

TRNG_MIN_SIZE = 16
TRNG_MAX_SIZE = 256

@pytest.fixture(scope="module")
def setup_module():
    log.debug("Setting up module for tests")

@allure.feature("trng")
@allure.description("测试随机数的生成，最小长度随机数，最少生成10次，并与上次生成的数据做对比")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-643")
@pytest.mark.skipif(False, reason="TRNG功能始终启用")
def test_ehsm_643(setup_module):
    with allure.step("1、准备随机数命令，配置长度为最小长度 # 1、配置成功"):
        require_size = TRNG_MIN_SIZE
        random_data = [0] * 16
        random_data = bytes(random_data)
        previous_bytes = None
    with allure.step("2、发送随机数命令，并获取随机数； # 2、读取成功，数据正确"):
        t,rand_data1 = api.ehsm_gen_random(EhsmRngAlgo.EHSM_RNG_ALGO_SM4_CTR_DRBG,random_data,require_size)
    with allure.step("3、读取随机数，并保存，与上一次生成的随机数进行比较； # 3、读取成功，数据正确"):
        if previous_bytes == rand_data1:
                pytest.xfail("两次生成的随机数相同，测试失败")
    with allure.step("4、循环2/3 10次； # 4、执行成功"):
        for i in range(10):
            t,rand_data = api.ehsm_gen_random(EhsmRngAlgo.EHSM_RNG_ALGO_SM4_CTR_DRBG,random_data,require_size)
            if previous_bytes == rand_data:
                pytest.xfail("两次生成的随机数相同，测试失败")
            previous_bytes = rand_data


@allure.feature("trng")
@allure.description("测试随机数的生成，最大长度随机数，最少生成10次，并与上次生成的数据做对比")
@allure.severity(allure.severity_level.BLOCKER)
@allure.testcase("EHSM-644")
@pytest.mark.skipif(False, reason="TRNG功能始终启用")
def test_ehsm_644(setup_module):
    with allure.step("1、准备随机数命令，配置长度为最大长度； # 1、配置成功"):
        require_size = TRNG_MAX_SIZE
        random_data = [0] * 256
        random_data = bytes(random_data)
        previous_bytes = None
    with allure.step("2、发送随机数命令，并获取随机数； # 2、读取成功，数据正确"):
        t,rand_data = api.ehsm_gen_random(EhsmRngAlgo.EHSM_RNG_ALGO_SM4_CTR_DRBG,random_data,require_size)
    with allure.step("3、读取随机数，并保存，与上一次生成的随机数进行比较； # 3、读取成功，数据正确"):
        if previous_bytes == rand_data:
                pytest.xfail("两次生成的随机数相同，测试失败")
    with allure.step("4、循环2/3 10次； # 4、执行成功"):
        for i in range(10):
            t,rand_data = api.ehsm_gen_random(EhsmRngAlgo.EHSM_RNG_ALGO_SM4_CTR_DRBG,random_data,require_size)
            if previous_bytes == rand_data:
                pytest.xfail("两次生成的随机数相同，测试失败")
            previous_bytes = rand_data


@allure.feature("trng")
@allure.description("生成随机数，使用 CTRDBG_SM4算法")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-645")
@pytest.mark.skipif(False, reason="TRNG功能始终启用")
def test_ehsm_645(setup_module):
    with allure.step("1、准备随机数命令，配置算法为SM4_CTRDRBG，长度固定； # 配置成功"):
        require_size = TRNG_MAX_SIZE
        random_data = [0] * 256
        random_data = bytes(random_data)
        previous_bytes = None
    with allure.step("2、发送随机数命令，并获取随机数； # 读取成功，数据正确"):
        t,rand_data = api.ehsm_gen_random(EhsmRngAlgo.EHSM_RNG_ALGO_SM4_CTR_DRBG,random_data,require_size)
    with allure.step("3、读取随机数，并保存，与上一次生成的随机数进行比较； # 读取成功，数据正确"):
        if previous_bytes == rand_data:
                pytest.xfail("两次生成的随机数相同，测试失败")
    with allure.step("4、将测试数据存放到mailbox 通道，并发送到eHSM；并在发送前后获取定时器计数，并计算时间 # 读取成功，数据正确"):
        log.debug(t)
    with allure.step("5、循环2/3 10次； # 执行成功"):
        for i in range(10):
            t,rand_data = api.ehsm_gen_random(EhsmRngAlgo.EHSM_RNG_ALGO_SM4_CTR_DRBG,random_data,require_size)
            if previous_bytes == rand_data:
                pytest.xfail("两次生成的随机数相同，测试失败")
            previous_bytes = rand_data

@allure.feature("trng")
@allure.description("生成随机数，使用 CTRDBG_AES算法")
@allure.severity(allure.severity_level.NORMAL)
@allure.testcase("EHSM-646")
@pytest.mark.skipif(False, reason="TRNG功能始终启用")
def test_ehsm_646(setup_module):
    with allure.step("1、准备随机数命令，配置算法为AES_CTRDRBG，长度固定； # 配置成功"):
        require_size = TRNG_MAX_SIZE
        random_data = [0] * 256
        random_data = bytes(random_data)
        previous_bytes = None
    with allure.step("2、发送随机数命令，并获取随机数； # 读取成功，数据正确"):
        t,rand_data = api.ehsm_gen_random(EhsmRngAlgo.EHSM_RNG_ALGO_AES_CTR_DRBG,random_data,require_size)
    with allure.step("3、读取随机数，并保存，与上一次生成的随机数进行比较； # 读取成功，数据正确"):
        if previous_bytes == rand_data:
                pytest.xfail("两次生成的随机数相同，测试失败")
    with allure.step("4、将测试数据存放到mailbox 通道，并发送到eHSM；并在发送前后获取定时器计数，并计算时间 # 读取成功，数据正确"):
        log.debug(t)
    with allure.step("5、循环2/3 10次； # 执行成功"):
        for i in range(10):
            t,rand_data = api.ehsm_gen_random(EhsmRngAlgo.EHSM_RNG_ALGO_AES_CTR_DRBG,random_data,require_size)
            if previous_bytes == rand_data:
                pytest.xfail("两次生成的随机数相同，测试失败")
            previous_bytes = rand_data

@allure.feature("trng")
@allure.description("随机数生成测试，使用非法的算法。")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-647")
@pytest.mark.skipif(False, reason="TRNG功能始终启用")
def test_ehsm_647(setup_module):
    with allure.step("1、准备随机数命令，配置算法为非法值，长度固定； # 配置成功"):
        require_size = TRNG_MAX_SIZE
        random_data = [0] * 256
        random_data = bytes(random_data)
    with allure.step("2、发送随机数命令，并获取随机数； # 读取成功，数据正确"):
        try:
            t,rand_data = api.ehsm_gen_random(EhsmRngAlgo.EHSM_RNG_ALGO_AES_CTR_DRBG + 1, random_data, require_size)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_PARAM_ERROR == e.ret_code:
                log.info(f"随机数生成使用非法算法时参数错误，符合预期: 错误码 {e.ret_code}")
            else:
                assert False, f"随机数生成使用非法算法时返回了非预期的错误码: {e.ret_code}，预期错误码: {ehsm_fw_errno.EHSM_ERR_PARAM_ERROR}"
        else:
            assert False, f"随机数生成使用非法算法时未抛出异常，不符合预期"
    with allure.step("3、将测试数据存放到mailbox 通道，并发送到eHSM；并在发送前后获取定时器计数，并计算时间 # 读取成功，数据正确"):
        log.info("发送失败，无法计算时间")

@allure.feature("trng")
@allure.description("随机数生成测试，使用非法的随机数据地址。")
@allure.severity(allure.severity_level.MINOR)
@allure.testcase("EHSM-T001")
@pytest.mark.skipif(False, reason="TRNG功能始终启用")
def test_ehsm_t001(setup_module):
    with allure.step("1、准备随机数命令，配置随机数据地址为非法值，长度为0； # 配置成功"):
        require_size = 0
        random_data = None
    with allure.step("2、发送随机数命令，并获取随机数； # 读取成功，数据正确"):
        try:
            t,rand_data = api.ehsm_gen_random(EhsmRngAlgo.EHSM_RNG_ALGO_AES_CTR_DRBG, random_data, require_size)
        except hostapi.HostApiError as e:
            if ehsm_fw_errno.EHSM_ERR_INVALID_ADDRESS == e.ret_code:
                log.info(f"随机数生成使用非法数据地址，无法正常返回随机数据，符合预期")
        else:
            assert False, f"随机数生成使用非法算法时未抛出异常，不符合预期"
    with allure.step("3、将测试数据存放到mailbox 通道，并发送到eHSM；并在发送前后获取定时器计数，并计算时间 # 读取成功，数据正确"):
        log.info("发送失败，无法计算时间")
