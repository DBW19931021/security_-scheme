import pytest
import logging
import subprocess
import sys
from pathlib import Path

def pytest_configure(config):
    logging.basicConfig(
        level=logging.DEBUG,  # 或 logging.INFO
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
    )


@pytest.fixture(scope="session", autouse=False)
def ensure_patch_images():
    """
    确保当前客户的 Patch 镜像存在且是最新的

    使用方式：
    1. 在需要 patch 镜像的测试文件中导入此 fixture
    2. 在测试函数中添加此 fixture 作为参数

    示例：
        def test_patch_function(ensure_patch_images):
            # 测试代码
            pass
    """
    project_root = Path(__file__).parent
    update_script = project_root / "tools" / "patch_update_current.py"

    if not update_script.exists():
        logging.warning(f"Patch 镜像更新脚本不存在: {update_script}")
        return

    try:
        # 调用更新脚本（不强制重建，只在需要时构建）
        result = subprocess.run(
            [sys.executable, str(update_script)],
            capture_output=True,
            text=True,
            encoding='utf-8',
            errors='ignore',  # 忽略无法解码的字节
            timeout=300  # 5分钟超时
        )

        if result.returncode == 0:
            logging.info("Patch 镜像检查/更新完成")
            if result.stdout:
                logging.debug(result.stdout)
        else:
            logging.warning(f"Patch 镜像更新失败: {result.stderr}")

    except subprocess.TimeoutExpired:
        logging.error("Patch 镜像更新超时")
    except Exception as e:
        logging.error(f"Patch 镜像更新出错: {e}")


@pytest.fixture(scope="session", autouse=True)
def session_fixture():
    from platform_adapter.api.constants import EhsmDrvMode
    from platform_adapter.api.loader import get_api_interface
    from platform_adapter.host.loader import get_host_interface
    host = get_host_interface()
    host.reset_ehsm()
    api = get_api_interface()
    api.ehsm_ctx_init(0, False)
    api.ehsm_driver_init_library(EhsmDrvMode.EHSM_DRV_MODE_WAIT_AND_POLL)
    try:
        yield api
    finally:
        api.ehsm_ctx_deinit()